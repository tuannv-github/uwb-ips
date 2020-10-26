#include <dpl/dpl.h>

#include <rtls_tdma/rtls_tdma.h>

typedef struct slot_cb_arg{
    uint16_t idx;
    rtls_tdma_instance_t *rtls_tdma_instance;
}slot_cb_arg_t;

static slot_cb_arg_t slot_cb_arg[MYNEWT_VAL(TDMA_NSLOTS)];

static bool
superframe_cb(struct uwb_dev * inst, struct uwb_mac_interface * cbs)
{
    rtls_tdma_instance_t *rtls_tdma_instance = (rtls_tdma_instance_t *)cbs->inst_ptr;
    printf("{\"utime\": %"PRIu32",\"msg\": \"ccp:superframe_cb\", \"period\": %"PRIu32"}\n",
            dpl_cputime_ticks_to_usecs(dpl_cputime_get32()), (uint32_t)uwb_dwt_usecs_to_usecs(rtls_tdma_instance->tdma->ccp->period));
    
    return false;
}

static void
slot_cb(struct dpl_event * ev)
{
    slot_cb_arg_t *slot_cb_arg = (slot_cb_arg_t*)dpl_event_get_arg(ev);
    printf("Slot idx: %d\n", slot_cb_arg->idx);
}

void rtls_tdma_start(rtls_tdma_instance_t *rtls_tdma_instance, struct uwb_dev* udev){
    tdma_instance_t *tdma = (tdma_instance_t*)uwb_mac_find_cb_inst_ptr(udev, UWBEXT_TDMA);
    assert(tdma);
    rtls_tdma_instance->tdma = tdma;

    rtls_ccp_start(rtls_tdma_instance->tdma->ccp);

    rtls_tdma_instance->cbs = (struct uwb_mac_interface){
        .id =  UWBEXT_APP0,
        .inst_ptr = rtls_tdma_instance,
        .superframe_cb = superframe_cb
    };
    uwb_mac_append_interface(udev, &rtls_tdma_instance->cbs);

    for (uint16_t i = 0; i < MYNEWT_VAL(TDMA_NSLOTS); i++){
        slot_cb_arg[i].rtls_tdma_instance = rtls_tdma_instance;
        slot_cb_arg[i].idx = i;
        tdma_assign_slot(rtls_tdma_instance->tdma, slot_cb,  i, &slot_cb_arg[i]);
    }
}