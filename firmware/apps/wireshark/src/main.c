#include <os/mynewt.h>

#include <bsp/bsp.h>
#include <hal/hal_gpio.h>

#include <uwbcfg/uwbcfg.h>
#include <uwb/uwb.h>

#include <wireshark/config.h>

static int uwb_config_updated();

/* Incoming messages mempool and queue */
struct uwb_msg_hdr {
    uint32_t utime;
    uint16_t dlen;
    uint16_t diag_offset;
    uint16_t cir_offset;
    int32_t  carrier_integrator;
    uint64_t ts;
};
#define MBUF_PKTHDR_OVERHEAD    sizeof(struct os_mbuf_pkthdr) + sizeof(struct uwb_msg_hdr)
#define MBUF_MEMBLOCK_OVERHEAD  sizeof(struct os_mbuf) + MBUF_PKTHDR_OVERHEAD
#define MBUF_PAYLOAD_SIZE       MYNEWT_VAL(UWB_MBUF_SIZE)
#define MBUF_BUF_SIZE           OS_ALIGN(MBUF_PAYLOAD_SIZE, 4)
#define MBUF_MEMBLOCK_SIZE      (MBUF_MEMBLOCK_OVERHEAD + MBUF_BUF_SIZE)
#define MBUF_NUM_MBUFS          MYNEWT_VAL(UWB_NUM_MBUFS)
#define MBUF_MEMPOOL_SIZE       OS_MEMPOOL_SIZE(MBUF_NUM_MBUFS, MBUF_MEMBLOCK_SIZE)
static os_membuf_t g_mbuf_buffer[MBUF_MEMPOOL_SIZE];
static struct os_mempool g_mempool;
static struct os_mbuf_pool g_mbuf_pool;

static struct os_mqueue rxpkt_q;
static struct dpl_callout rx_reenable_callout;

static void
create_mbuf_pool(void)
{
    int rc;

    /* Prepare memory buffer */
    for (int i=0; i<sizeof(g_mbuf_buffer)/sizeof(g_mbuf_buffer[0]); i++) {
        g_mbuf_buffer[i] = 0xdeadbeef;
    }

    rc = os_mempool_init(&g_mempool, MBUF_NUM_MBUFS,
                         MBUF_MEMBLOCK_SIZE, &g_mbuf_buffer[0], "uwb_mbuf_pool");
    assert(rc == 0);

    rc = os_mbuf_pool_init(&g_mbuf_pool, &g_mempool, MBUF_MEMBLOCK_SIZE,
                           MBUF_NUM_MBUFS);
    assert(rc == 0);
}

static uint8_t print_buffer[1024];
static void
process_rx_data_queue(struct os_event *ev)
{
    struct os_mbuf *om = 0;

    hal_gpio_init_out(LED_BLINK_PIN, 0);
    while ((om = os_mqueue_get(&rxpkt_q)) != NULL) {

        int payload_len = OS_MBUF_PKTLEN(om);
        payload_len = (payload_len > sizeof(print_buffer)) ? sizeof(print_buffer) : payload_len;
        struct uwb_msg_hdr *hdr = (struct uwb_msg_hdr*)(OS_MBUF_USRHDR(om));
        int rc = os_mbuf_copydata(om, 0, payload_len, print_buffer);
        if (!rc) {
            printf("{\"utime\":[%lu],\"ts\":[%llu],\"dlen\":[%d],\"d\":[", hdr->utime, hdr->ts, hdr->dlen);
            for (int i=0; i<sizeof(print_buffer) && i<hdr->dlen; i++)
            {
                printf("%X", print_buffer[i]);
            }
            printf("]}\n");
        }
        os_mbuf_free_chain(om);
        memset(print_buffer, 0, sizeof(print_buffer));
    }
    hal_gpio_init_out(LED_BLINK_PIN, 1);
}

static bool
rx_complete_cb(struct uwb_dev * inst, struct uwb_mac_interface * cbs)
{
    int rc;
    struct os_mbuf *om;

    om = os_mbuf_get_pkthdr(&g_mbuf_pool, sizeof(struct uwb_msg_hdr));
    if (!om) {
        /* Not enough memory to handle incoming packet, drop it */
        return true;
    }

    struct uwb_msg_hdr *hdr = (struct uwb_msg_hdr*)OS_MBUF_USRHDR(om);
    memset(hdr, 0, sizeof(struct uwb_msg_hdr));
    hdr->utime = os_cputime_ticks_to_usecs(os_cputime_get32());
    hdr->dlen = inst->frame_len;
    hdr->carrier_integrator = inst->carrier_integrator;
    uint16_t offset = 0;
    rc = os_mbuf_copyinto(om, offset, inst->rxbuf, hdr->dlen);
    if (rc != 0) {
        os_mbuf_free_chain(om);
        return true;
    }
    offset += hdr->dlen;
    hdr->diag_offset = offset;

    struct uwb_dev *udev = uwb_dev_idx_lookup(0);
    rc = os_mbuf_copyinto(om, offset, udev->rxdiag, udev->rxdiag->rxd_len);
    if (rc != 0) {
        os_mbuf_free_chain(om);
        return true;
    }

    if (!udev->status.lde_error) {
        hdr->ts = udev->rxtimestamp;
    }

    rc = os_mqueue_put(&rxpkt_q, os_eventq_dflt_get(), om);
    if (rc != 0) {
        return true;
    }

    return true;
}

bool
error_cb(struct uwb_dev * inst, struct uwb_mac_interface * cbs)
{
    return true;
}

bool
timeout_cb(struct uwb_dev * inst, struct uwb_mac_interface * cbs)
{
    /* Restart receivers */
    printf("Timeout\n");
    struct uwb_dev *udev = uwb_dev_idx_lookup(0);
    uwb_set_rx_timeout(udev, 0);
    uwb_start_rx(udev);
    return true;
}

bool
reset_cb(struct uwb_dev * inst, struct uwb_mac_interface * cbs)
{
    return true;
}

void
uwb_config_update(struct os_event * ev)
{
    printf("UWB config update\n");
    struct uwb_dev *inst = uwb_dev_idx_lookup(0);
    uwb_mac_config(inst, NULL);
    uwb_txrf_config(inst, &inst->config.txrf);
    uwb_set_rx_timeout(inst, 0);
    inst->config.rxdiag_enable = (app_conf_get_verbose()&VERBOSE_RX_DIAG) != 0;
    uwb_start_rx(inst);
}

static int
uwb_config_updated()
{
    static struct os_event ev = {
        .ev_queued = 0,
        .ev_cb = uwb_config_update,
        .ev_arg = 0};
    os_eventq_put(os_eventq_dflt_get(), &ev);

    return 0;
}

static
void rx_reenable_ev_cb(struct dpl_event *ev) {
    /* Restart receivers */
    printf("rx_reenable_ev_cb\n");
    struct uwb_dev *udev = uwb_dev_idx_lookup(0);
    uwb_set_rx_timeout(udev, 0);
    uwb_start_rx(udev);
}

static struct uwbcfg_cbs uwb_cb = {
    .uc_update = uwb_config_updated
};

static struct uwb_mac_interface g_cbs = {
    .id = UWBEXT_APP0,
    .rx_complete_cb = rx_complete_cb,
    .rx_timeout_cb = timeout_cb,
    .rx_error_cb = error_cb,
    .tx_error_cb = 0,
    .tx_complete_cb = 0,
    .reset_cb = reset_cb
};

int main(int argc, char **argv){
    int rc;
    struct uwb_dev *udev;

    sysinit();
    hal_gpio_init_out(LED_BLINK_PIN, 1);

    app_conf_init(uwb_config_updated);

    /* Load any saved uwb settings */
    uwbcfg_register(&uwb_cb);
    udev = uwb_dev_idx_lookup(0);
    udev->config.rxdiag_enable = (app_conf_get_verbose()&VERBOSE_RX_DIAG) != 0;
    udev->config.bias_correction_enable = 0;
    udev->config.LDE_enable = 1;
    udev->config.LDO_enable = 0;
    udev->config.sleep_enable = 0;
    udev->config.wakeup_rx_enable = 1;
    udev->config.trxoff_enable = 1;
    udev->config.dblbuffon_enabled = 0;
    udev->config.rxauto_enable = 1;
    uwb_set_dblrxbuff(udev, false);
    uwb_mac_append_interface(udev, &g_cbs);

    uint32_t utime = os_cputime_ticks_to_usecs(os_cputime_get32());
    printf("{\"utime\": %lu,\"exec\": \"%s\"}\n",utime,__FILE__);
    printf("{\"utime\": %lu,\"msg\": \"device_id = 0x%lX\"}\n",utime,udev->device_id);
    printf("{\"utime\": %lu,\"msg\": \"PANID = 0x%X\"}\n",utime,udev->pan_id);
    printf("{\"utime\": %lu,\"msg\": \"DeviceID = 0x%X\"}\n",utime,udev->uid);
    printf("{\"utime\": %lu,\"msg\": \"partID = 0x%lX\"}\n", utime,(uint32_t)(udev->euid&0xffffffff));
    printf("{\"utime\": %lu,\"msg\": \"lotID = 0x%lX\"}\n", utime,(uint32_t)(udev->euid>>32));
    printf("{\"utime\": %lu,\"msg\": \"SHR_duration = %d usec\"}\n",utime, uwb_phy_SHR_duration(udev));

    /* Sync clocks if available */
    if (uwb_sync_to_ext_clock(udev).ext_sync || 1) {
        printf("{\"ext_sync\"=\"");
        printf("%d", udev->status.ext_sync);
        printf("\"}\n");
    }

    create_mbuf_pool();

    os_mqueue_init(&rxpkt_q, process_rx_data_queue, NULL);
    dpl_callout_init(&rx_reenable_callout, dpl_eventq_dflt_get(), rx_reenable_ev_cb, NULL);

    /* Start timeout-free rx on all devices */
    uwb_set_rx_timeout(udev, 0);
    uwb_start_rx(udev);

    while (1) {
        os_eventq_run(os_eventq_dflt_get());
    }
    assert(0);
    return rc;
}
