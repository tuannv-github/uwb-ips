#include <os/mynewt.h>
#include <config/config.h>

#include <uwb/uwb.h>
#include <rtls_tdma/rtls_tdma.h>
#include <uwb_rng/uwb_rng.h>

void rtls_tdma_cb(rtls_tdma_instance_t *rtls_tdma_instance, tdma_slot_t *slot){
    
    static uint16_t timeout = 0;
    struct uwb_rng_instance *rng = rtls_tdma_instance->uri;
    struct uwb_dev *inst = rtls_tdma_instance->dev_inst;
    uint16_t idx = slot->idx;

    if (!timeout) {
        timeout = uwb_usecs_to_dwt_usecs(uwb_phy_frame_duration(inst, sizeof(ieee_rng_request_frame_t)))
            + rng->config.rx_timeout_delay;
        printf("# timeout set to: %d %d = %d\n",
               uwb_phy_frame_duration(inst, sizeof(ieee_rng_request_frame_t)),
               rng->config.rx_timeout_delay, timeout);
    }
    tdma_instance_t *tdma = slot->parent;

    uwb_rng_listen_delay_start(rng, tdma_rx_slot_start(tdma, idx), timeout, UWB_BLOCKING);
}

rtls_tdma_instance_t rtls_tdma_instance = {
    .rtls_tdma_cb = rtls_tdma_cb
};

/*!
 * @fn complete_cb
 *
 * @brief This callback is part of the  struct uwb_mac_interface extension interface and invoked of the completion of a range request.
 * The struct uwb_mac_interface is in the interrupt context and is used to schedule events an event queue. Processing should be kept
 * to a minimum giving the interrupt context. All algorithms activities should be deferred to a thread on an event queue.
 * The callback should return true if and only if it can determine if it is the sole recipient of this event.
 *
 * NOTE: The MAC extension interface is a link-list of callbacks, subsequent callbacks on the list will be not be called in the
 * event of returning true.
 *
 * @param inst  - struct uwb_dev *
 * @param cbs   - struct uwb_mac_interface *
 *
 * output parameters
 *
 * returns bool
 */
/* The timer callout */
static bool
complete_cb(struct uwb_dev * inst, struct uwb_mac_interface * cbs)
{
    if (inst->fctrl != FCNTL_IEEE_RANGE_16 &&
        inst->fctrl != (FCNTL_IEEE_RANGE_16|UWB_FCTRL_ACK_REQUESTED)) {
        return false;
    }
    return true;
}

int main(int argc, char **argv){
    int rc;
    
    sysinit();
    conf_load();

    struct uwb_dev *udev = uwb_dev_idx_lookup(0);
    uwb_set_dblrxbuff(udev, false);

    uint32_t utime = os_cputime_ticks_to_usecs(os_cputime_get32());
    printf("{\"utime\": %lu,\"exec\": \"%s\"}\n",utime,__FILE__);
    printf("{\"device_id\"=\"%lX\"",udev->device_id);
    printf(",\"panid=\"%X\"",udev->pan_id);
    printf(",\"addr\"=\"%X\"",udev->uid);
    printf(",\"part_id\"=\"%lX\"",(uint32_t)(udev->euid&0xffffffff));
    printf(",\"lot_id\"=\"%lX\"}\n",(uint32_t)(udev->euid>>32));
    printf("{\"utime\": %lu,\"msg\": \"SHR_duration = %d usec\"}\n",utime, uwb_phy_SHR_duration(udev));

    rtls_tdma_instance.uri  = (struct uwb_rng_instance *)uwb_mac_find_cb_inst_ptr(udev, UWBEXT_RNG);
    assert(rtls_tdma_instance.uri);
    struct uwb_mac_interface cbs = (struct uwb_mac_interface){
        .id =  UWBEXT_APP0,
        .inst_ptr = rtls_tdma_instance.uri,
        .complete_cb = complete_cb,
    };
    uwb_mac_append_interface(udev, &cbs);

    rtls_tdma_start(&rtls_tdma_instance, udev);

    while (1) {
        os_eventq_run(os_eventq_dflt_get());
    }
    assert(0);
    return rc;
}