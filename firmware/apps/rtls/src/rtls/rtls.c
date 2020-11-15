#include <rtls/rtls/rtls.h>

#include <rtls_tdma/rtls_tdma.h>

#if __GNUC__ >= 9
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"
#endif
#include <message/mavlink/protocol/mavlink.h>

#include <uwb/uwb.h>
#include <uwb_rng/uwb_rng.h>

float g_location[3] = {0.2, 123.456, 567};
uint16_t g_address = 0x1234;
uint16_t g_ntype = ANCHOR;

void rtls_get_location(float *x, float *y, float *z){
    *x = g_location[0];
    *y = g_location[1];
    *z = g_location[2];
}

void rtls_set_location(float x, float y, float z){
    g_location[0] = x;
    g_location[1] = y;
    g_location[2] = z;
}

void rtls_get_address(uint16_t *address){
    *address = g_address;
}

void rtls_get_ntype(uint8_t *ntype){
    *ntype = g_ntype;
}

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

void rtls_init(){
    struct uwb_dev *udev = uwb_dev_idx_lookup(0);
    uwb_set_dblrxbuff(udev, false);

    rtls_tdma_instance.uri  = (struct uwb_rng_instance *)uwb_mac_find_cb_inst_ptr(udev, UWBEXT_RNG);
    assert(rtls_tdma_instance.uri);
    struct uwb_mac_interface cbs = (struct uwb_mac_interface){
        .id =  UWBEXT_APP0,
        .inst_ptr = rtls_tdma_instance.uri,
        .complete_cb = complete_cb,
    };
    uwb_mac_append_interface(udev, &cbs);

    // rtls_tdma_start(&rtls_tdma_instance, udev);
}