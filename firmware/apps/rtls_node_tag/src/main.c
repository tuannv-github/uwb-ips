#include <os/mynewt.h>
#include <config/config.h>

#include <uwb/uwb.h>
#include <rtls_tdma/rtls_tdma.h>
#include <uwb_rng/uwb_rng.h>

void rtls_tdma_cb(rtls_tdma_instance_t *rtls_tdma_instance, tdma_slot_t *slot){
  
    tdma_instance_t * tdma = slot->parent;
    uint16_t idx = slot->idx;
    struct uwb_rng_instance *rng = rtls_tdma_instance->uri;

    uint64_t dx_time = tdma_tx_slot_start(tdma, idx) & 0xFFFFFFFFFE00UL;

    /* Range with the clock master by default */
    struct uwb_ccp_instance *ccp = tdma->ccp;

    uint16_t node_address = ccp->frames[ccp->idx%ccp->nframes]->short_address;

    uwb_rng_request_delay_start(rng, node_address, dx_time, UWB_DATA_CODE_SS_TWR);
}

rtls_tdma_instance_t rtls_tdma_instance = {
    .rtls_tdma_cb = rtls_tdma_cb
};

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