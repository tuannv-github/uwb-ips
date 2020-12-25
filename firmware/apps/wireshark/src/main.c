#include <os/os.h>
#include <sysinit/sysinit.h>

#include <uwb/uwb.h>
#include <uwb/uwb_ftypes.h>
#include <uwbcfg/uwbcfg.h>
#include <serial/serial.h>

struct uwb_msg_hdr {
    uint64_t utime;
    uint32_t ts;
    int32_t  carrier_integrator;
    uint16_t dlen;
    uint16_t diag_offset;
    uint16_t cir_offset;
};

#define MBUF_PKTHDR_OVERHEAD    sizeof(struct os_mbuf_pkthdr) + sizeof(struct uwb_msg_hdr)
#define MBUF_MEMBLOCK_OVERHEAD  sizeof(struct os_mbuf) + MBUF_PKTHDR_OVERHEAD
#define MBUF_PAYLOAD_SIZE       MYNEWT_VAL(UWB_MBUF_SIZE)
#define MBUF_BUF_SIZE           OS_ALIGN(MBUF_PAYLOAD_SIZE, 4)
#define MBUF_MEMBLOCK_SIZE      (MBUF_MEMBLOCK_OVERHEAD + MBUF_BUF_SIZE)
#define MBUF_NUM_MBUFS          MYNEWT_VAL(UWB_NUM_MBUFS)
#define MBUF_MEMPOOL_SIZE       OS_MEMPOOL_SIZE(MBUF_NUM_MBUFS, MBUF_MEMBLOCK_SIZE)

static struct dpl_callout g_rx_enable_callout;
static struct os_mqueue g_rx_pkt_q;
static os_membuf_t g_mbuf_buffer[MBUF_MEMPOOL_SIZE];
static struct os_mempool g_mempool;
static struct os_mbuf_pool g_mbuf_pool;
static struct os_task g_wireshark_task;
static os_stack_t g_wireshark_task_stack[MYNEWT_VAL(TASK_WIRESHARK_STACK_SIZE)];
static bool g_running = false;

bool rx_complete_cb(struct uwb_dev * inst, struct uwb_mac_interface * cbs);
static struct uwb_mac_interface 
uwb_mac_if = {
    .id = UWBEXT_APP0,
    .rx_complete_cb = rx_complete_cb,
};

static void
create_mbuf_pool(void)
{
    int rc;

    rc = os_mempool_init(&g_mempool, MBUF_NUM_MBUFS,
                         MBUF_MEMBLOCK_SIZE, g_mbuf_buffer, "uwb_mbuf_pool");
    assert(rc == 0);

    rc = os_mbuf_pool_init(&g_mbuf_pool, &g_mempool, MBUF_MEMBLOCK_SIZE,
                           MBUF_NUM_MBUFS);
    assert(rc == 0);
}

bool
rx_complete_cb(struct uwb_dev * inst, struct uwb_mac_interface * cbs)
{
    int rc;
    struct os_mbuf *om;

    struct uwb_dev *udev = uwb_dev_idx_lookup(0);

    om = os_mbuf_get_pkthdr(&g_mbuf_pool, sizeof(struct uwb_msg_hdr));
    /* Not enough memory to handle incoming packet, drop it */
    if (!om) return true;

    struct uwb_msg_hdr *hdr = (struct uwb_msg_hdr*)OS_MBUF_USRHDR(om);
    memset(hdr, 0, sizeof(struct uwb_msg_hdr));

    hdr->utime              = os_cputime_ticks_to_usecs(os_cputime_get32());
    if (!udev->status.lde_error) hdr->ts = udev->rxtimestamp;
    hdr->carrier_integrator = inst->carrier_integrator;
    hdr->dlen               = inst->frame_len;

    rc = os_mbuf_copyinto(om, 0, inst->rxbuf, hdr->dlen);
    if (rc != 0) {
        os_mbuf_free_chain(om);
        return true;
    }
    hdr->diag_offset = hdr->dlen;

    rc = os_mbuf_copyinto(om, hdr->diag_offset, udev->rxdiag, udev->rxdiag->rxd_len);
    if (rc != 0) {
        os_mbuf_free_chain(om);
        return true;
    }

    rc = os_mqueue_put(&g_rx_pkt_q, os_eventq_dflt_get(), om);
    if (rc != 0) {
        return true;
    }

    return true;
}

static void
process_rx_data_queue(struct os_event *ev)
{
    struct os_mbuf *om = 0;
    struct uwb_dev *udev = uwb_dev_idx_lookup(0);
    static char buff[512];
    static char sbuff[32];

    while((om = os_mqueue_get(&g_rx_pkt_q)) != NULL) {
        if(g_running){
            int payload_len = OS_MBUF_PKTLEN(om);
            payload_len = (payload_len > sizeof(buff)) ? sizeof(buff) : payload_len;
            struct uwb_msg_hdr *hdr = (struct uwb_msg_hdr*)(OS_MBUF_USRHDR(om));
            int rc = os_mbuf_copydata(om, 0, payload_len, buff);
            if (!rc) {
                serial_write_str("received: ");
                for (int i=0; i<sizeof(buff) && i<hdr->dlen; i++)
                {
                    sprintf(sbuff, "%02X", buff[i]);
                    serial_write_str(sbuff);
                }

                struct uwb_dev_rxdiag *diag = (struct uwb_dev_rxdiag *)(buff + hdr->diag_offset);
                float rssi = uwb_calc_rssi(udev, diag);

                sprintf(sbuff, " power: %d lqi: 0 time: %lld\r\n", (int)rssi, hdr->utime);
                serial_write_str(sbuff);
            }
        }
        os_mbuf_free_chain(om);
    }
}

static void
task_wireshark_func(void *arg){
    static char line[256];
    static int channel;
    struct uwb_dev *inst = uwb_dev_idx_lookup(0);
    while (1)
    {
        serial_read_line(line, 256);
        if(sscanf(line, "channel %d", &channel)){
            inst->config.channel = channel;
            uwb_mac_config(inst, &inst->config);
            dpl_callout_reset(&g_rx_enable_callout, 0);
            serial_write_line("OK");
        }
        else if(!strcmp(line, "start")){
            g_running = true;
            serial_write_line("OK");
        }
        else if(!strcmp(line, "stop")){
            g_running = false;
            serial_write_line("");
            serial_write_line("OK");
        }
    }
}

static void 
rx_enable_ev_cb(struct dpl_event *ev) {
    printf("RX enable\n");
    struct uwb_dev *udev = uwb_dev_idx_lookup(0);
    uwb_set_rx_timeout(udev, 0);
    uwb_start_rx(udev);
}

int 
main(int argc, char **argv){
    /* Declare some variables */
    struct uwb_dev *udev;
    uint32_t utime;

    /* System init */
    sysinit();

    /* MBuf pool init */
    create_mbuf_pool();
    os_mqueue_init(&g_rx_pkt_q, process_rx_data_queue, NULL);

    /* UWB device init */
    udev  = uwb_dev_idx_lookup(0);
    assert(udev);
    uwb_mac_append_interface(udev, &uwb_mac_if);
    udev->config = (struct uwb_dev_config){
        .rxdiag_enable = 1,
        .bias_correction_enable = 0,
        .LDE_enable = 1,
        .LDO_enable = 0,
        .sleep_enable = 0,
        .wakeup_rx_enable = 1,
        .trxoff_enable = 1,
        .dblbuffon_enabled = 0,
        .rxauto_enable = 1,
        .channel = 5
    };
    uwb_mac_config(udev, &udev->config);
    dpl_callout_init(&g_rx_enable_callout, dpl_eventq_dflt_get(), rx_enable_ev_cb, NULL);
    dpl_callout_reset(&g_rx_enable_callout, 0);

    /* Print UWB information */
    utime = dpl_cputime_ticks_to_usecs(dpl_cputime_get32());
    printf("{\"utime\": %lu,\"exe\": \"%s\"}\n",                    utime,  __FILE__);
    printf("{\"utime\": %lu,\"msg\": \"device_id    = 0x%lX\"}\n",  utime,  udev->device_id);
    printf("{\"utime\": %lu,\"msg\": \"PANID        = 0x%X\"}\n",   utime,  udev->pan_id);
    printf("{\"utime\": %lu,\"msg\": \"DeviceID     = 0x%X\"}\n",   utime,  udev->uid);
    printf("{\"utime\": %lu,\"msg\": \"partID       = 0x%lX\"}\n",  utime,  (uint32_t)(udev->euid&0xffffffff));
    printf("{\"utime\": %lu,\"msg\": \"lotID        = 0x%lX\"}\n",  utime,  (uint32_t)(udev->euid>>32));
    printf("{\"utime\": %lu,\"msg\": \"SHR_duration = %d usec\"}\n",utime,  uwb_phy_SHR_duration(udev));

    /* Serial task */
    os_task_init(&g_wireshark_task, "wireshark",
        task_wireshark_func,
        NULL,
        MYNEWT_VAL(TASK_WIRESHARK_PRIORITY), 
        OS_WAIT_FOREVER,
        g_wireshark_task_stack,
        MYNEWT_VAL(TASK_WIRESHARK_STACK_SIZE));

    /* Loop */
    while (1) {
        os_eventq_run(os_eventq_dflt_get());
    }
}
