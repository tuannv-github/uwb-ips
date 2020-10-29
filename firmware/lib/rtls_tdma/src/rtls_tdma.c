#include <dpl/dpl.h>

#include <rtls_tdma/rtls_tdma.h>

typedef struct slot_cb_arg{
    uint16_t idx;
    rtls_tdma_instance_t *rtls_tdma_instance;
}slot_cb_arg_t;

static slot_cb_arg_t slot_cb_arg[MYNEWT_VAL(TDMA_NSLOTS)];

static void
slot_cb(struct dpl_event * ev)
{
    slot_cb_arg_t *slot_cb_arg = (slot_cb_arg_t*)dpl_event_get_arg(ev);
    switch (slot_cb_arg->rtls_tdma_instance->cstate)
    {
    case RTS_JOINT_LIST:
        break;
    case RTS_JOINT_PREP:
        break;
    case RTS_JOINT_POLL:
        break;
    case RTS_JOINT_RESP:
        break;
    case RTS_JOINT_REQT:
        break;
    case RTS_JOINT_JTED:
        break;
    default:
        break;
    }
}

static void 
uwb_ccp_sync_cb(ccp_sync_t ccp_sync, void *arg){
    rtls_tdma_instance_t *rtls_tdma_instance = (rtls_tdma_instance_t *)arg;
    switch (ccp_sync)
    {
    case CCP_SYNC_LOST:
        printf("CCP_SYNC_LOST\n");
        rtls_tdma_instance->cstate = RTS_JOINT_LIST;
        break;
    case CCP_SYNC_SYED:
        printf("CCP_SYNC_SYED\n");
        if(rtls_tdma_instance->tdma->ccp->config.role == CCP_ROLE_MASTER){
            rtls_tdma_instance->my_slot = 0;
            rtls_tdma_instance->cstate = RTS_JOINT_JTED;
        }
        break;
    default:
        break;
    }
}

void rtls_tdma_start(rtls_tdma_instance_t *rtls_tdma_instance, struct uwb_dev* udev){
    tdma_instance_t *tdma = (tdma_instance_t*)uwb_mac_find_cb_inst_ptr(udev, UWBEXT_TDMA);
    assert(tdma);
    rtls_tdma_instance->tdma = tdma;

    rtls_ccp_set_sync_cb(rtls_tdma_instance->tdma->ccp, uwb_ccp_sync_cb, rtls_tdma_instance);
    rtls_ccp_start(rtls_tdma_instance->tdma->ccp);

    for (uint16_t i = 0; i < MYNEWT_VAL(TDMA_NSLOTS); i++){
        slot_cb_arg[i].rtls_tdma_instance = rtls_tdma_instance;
        slot_cb_arg[i].idx = i;
        tdma_assign_slot(rtls_tdma_instance->tdma, slot_cb,  i, &slot_cb_arg[i]);
    }
}