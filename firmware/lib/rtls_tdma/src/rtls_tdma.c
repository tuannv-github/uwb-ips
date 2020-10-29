#include <dpl/dpl.h>

#include <rtls_tdma/rtls_tdma.h>

static void rst_joint_list_cb(tdma_slot_t *tdma_slot);
static void rst_joint_prep_cb(tdma_slot_t *tdma_slot);
static void rst_joint_poll_cb(tdma_slot_t *tdma_slot);
static void rst_joint_resp_cb(tdma_slot_t *tdma_slot);
static void rst_joint_reqt_cb(tdma_slot_t *tdma_slot);
static void rst_joint_jted_cb(tdma_slot_t *tdma_slot);

static void
slot_cb(struct dpl_event * ev)
{
    tdma_slot_t *tdma_slot = (tdma_slot_t *)dpl_event_get_arg(ev);

    switch (((rtls_tdma_instance_t *)tdma_slot->arg)->cstate)
    {
    case RTS_JOINT_LIST:
        rst_joint_list_cb(tdma_slot);
        break;
    case RTS_JOINT_PREP:
        rst_joint_prep_cb(tdma_slot);
        break;
    case RTS_JOINT_POLL:
        rst_joint_poll_cb(tdma_slot);
        break;
    case RTS_JOINT_RESP:
        rst_joint_resp_cb(tdma_slot);
        break;
    case RTS_JOINT_REQT:
        rst_joint_reqt_cb(tdma_slot);
        break;
    case RTS_JOINT_JTED:
        rst_joint_jted_cb(tdma_slot);
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
            rtls_tdma_instance->my_slot = 1;
            rtls_tdma_instance->cstate = RTS_JOINT_JTED;
        }
        break;
    default:
        break;
    }
}

/**
 * @fn rx_complete_cb(struct uwb_dev * inst, struct uwb_mac_interface * cbs)
 * @brief This is an internal static function that executes on both the pan_master Node and the TAG/ANCHOR
 * that initiated the blink. On the pan_master the postprocess function should allocate a PANID and a SLOTID,
 * while on the TAG/ANCHOR the returned allocations are assigned and the PAN discover event is stopped. The pan
 * discovery resources can be released.
 *
 * @param inst    Pointer to struct uwb_dev.
 * @param cbs     Pointer to struct uwb_mac_interface.
 *
 * @return bool
 */
static bool
rx_complete_cb(struct uwb_dev * inst, struct uwb_mac_interface * cbs){
    rtls_tdma_instance_t *rti = (rtls_tdma_instance_t *)cbs->inst_ptr;
    if (dpl_sem_get_count(&rti->sem) == 0){
        dpl_error_t err = dpl_sem_release(&rti->sem);
        assert(err == DPL_OK);
        return true;
    }
    return false;
}

/**
 * @fn rx_timeout_cb(struct uwb_dev * inst, struct uwb_mac_interface * cbs)
 * @brief API for receive timeout callback.
 *
 * @param inst    Pointer to struct uwb_dev.
 * @param cbs     Pointer to struct uwb_mac_interface.
 *
 * @return bool
 */
static bool
rx_timeout_cb(struct uwb_dev *inst, struct uwb_mac_interface *cbs)
{
    rtls_tdma_instance_t *rti = (rtls_tdma_instance_t *)cbs->inst_ptr;
    if (dpl_sem_get_count(&rti->sem) == 0){
        dpl_error_t err = dpl_sem_release(&rti->sem);
        assert(err == DPL_OK);
        return true;
    }
    return false;
}

/**
 * @fn rtls_tdma_listen(struct uwb_dev * inst, uwb_dev_modes_t mode)
 * @brief Listen for polls / requests
 *
 * @param inst          Pointer to struct uwb_dev.
 * @param mode          uwb_dev_modes_t of UWB_BLOCKING and UWB_NONBLOCKING.
 *
 * @return struct uwb_dev_status
 */
struct uwb_dev_status
rtls_tdma_listen(rtls_tdma_instance_t *rti, uwb_dev_modes_t mode)
{
    dpl_error_t err = dpl_sem_pend(&rti->sem,  DPL_TIMEOUT_NEVER);
    assert(err == DPL_OK);

    if(uwb_start_rx(rti->dev_inst).start_rx_error){
        err = dpl_sem_release(&rti->sem);
        assert(err == DPL_OK);
    }

    if (mode == UWB_BLOCKING){
        err = dpl_sem_pend(&rti->sem, DPL_TIMEOUT_NEVER);
        assert(err == DPL_OK);
        err = dpl_sem_release(&rti->sem);
        assert(err == DPL_OK);
    }

    return rti->dev_inst->status;
}

void rtls_tdma_start(rtls_tdma_instance_t *rti, struct uwb_dev* udev){
    tdma_instance_t *tdma = (tdma_instance_t*)uwb_mac_find_cb_inst_ptr(udev, UWBEXT_TDMA);
    assert(tdma);
    rti->tdma = tdma;
    rti->dev_inst = tdma->dev_inst;

    dpl_error_t err = dpl_sem_init(&rti->sem, 0x1);
    assert(err == DPL_OK);

    rti->umi.id = UWBEXT_APP0,
    rti->umi.inst_ptr = rti,
    rti->umi.rx_complete_cb = rx_complete_cb,
    rti->umi.rx_timeout_cb = rx_timeout_cb,
    uwb_mac_append_interface(rti->dev_inst, &rti->umi);

    rtls_ccp_set_sync_cb(rti->tdma->ccp, uwb_ccp_sync_cb, rti);
    rtls_ccp_start(rti->tdma->ccp);

    for (uint16_t i = 0; i < MYNEWT_VAL(TDMA_NSLOTS); i++){
        tdma_assign_slot(rti->tdma, slot_cb,  i, rti);
    }
}

void rst_joint_list_cb(tdma_slot_t *tdma_slot){

    if(tdma_slot->idx == 0) return;
    if(tdma_slot->idx > MYNEWT_VAL(UWB_BCN_SLOT_MAX)) return;

    rtls_tdma_instance_t *rti = (rtls_tdma_instance_t *)tdma_slot->arg;
    assert(rti);

    uwb_set_rx_timeout(rti->dev_inst, 3*rti->tdma->ccp->period/rti->tdma->nslots/4);
    uwb_set_delay_start(rti->dev_inst, tdma_rx_slot_start(rti->tdma, tdma_slot->idx));
    if(rtls_tdma_listen(rti, UWB_BLOCKING).start_rx_error){
        printf("start_rx_error: %s:%d\n", __FILE__, __LINE__);
    }
    else{
        if(rti->dev_inst->status.rx_timeout_error) return;

        ieee_std_frame_hdr_t *ieee_std_frame_hdr = (ieee_std_frame_hdr_t *)rti->dev_inst->rxbuf;
        if(ieee_std_frame_hdr->PANID != rti->dev_inst->pan_id) return;
        if(ieee_std_frame_hdr->dst_address != RT_BROADCAST_ADDR) return;

        if(rti->dev_inst->rxbuf_size - sizeof(ieee_std_frame_hdr_t) < sizeof(rt_bcn_norm_payload_t)) return;
        rt_bcn_norm_payload_t *rt_bcn_norm_payload = (rt_bcn_norm_payload_t *)(rti->dev_inst->rxbuf + sizeof(ieee_std_frame_hdr_t));

        if(rt_bcn_norm_payload->msg_type != RT_BCN_NORM_MSG) return;

        if(ieee_std_frame_hdr->src_address != rti->anchors[tdma_slot->idx].addr){
            rti->anchors[tdma_slot->idx].addr = ieee_std_frame_hdr->src_address;
            rti->anchors[tdma_slot->idx].location_x = rt_bcn_norm_payload->location_x;
            rti->anchors[tdma_slot->idx].location_y = rt_bcn_norm_payload->location_y;
            rti->anchors[tdma_slot->idx].location_z = rt_bcn_norm_payload->location_z;
            rti->anchors[tdma_slot->idx].listen_cnt = 0;
        }
        rti->anchors[tdma_slot->idx].listen_cnt++;

        bool next_state = true;
        for(int i=0; i<MYNEWT_VAL(UWB_BCN_SLOT_MAX); i++){
            if(rti->anchors[i].addr != 0 && rti->anchors[i].listen_cnt < MYNEWT_VAL(RT_JOINT_LIST_THRESH)){
                next_state = false;
            }
        }

        if(next_state){
            printf("Anchor list: \n");
            for(int i=0; i<MYNEWT_VAL(UWB_BCN_SLOT_MAX); i++){
                if(rti->anchors[i].addr != 0){
                    printf("-- 0x%X:%d\n", rti->anchors[i].addr, rti->anchors[i].listen_cnt);
                }
            }
            printf("Change to state: RTS_JOINT_PREP\n");
            rti->cstate = RTS_JOINT_PREP;
        }
    }
}

void rst_joint_prep_cb(tdma_slot_t *tdma_slot){
    // printf("rst_joint_prep_cb\n");
}

void rst_joint_poll_cb(tdma_slot_t *tdma_slot){

}

void rst_joint_resp_cb(tdma_slot_t *tdma_slot){

}

void rst_joint_reqt_cb(tdma_slot_t *tdma_slot){

}

void rst_joint_jted_cb(tdma_slot_t *tdma_slot){

    if(tdma_slot->idx == 0) return;
    rtls_tdma_instance_t *rti = (rtls_tdma_instance_t *)tdma_slot->arg;

    if(tdma_slot->idx <= MYNEWT_VAL(UWB_BCN_SLOT_MAX)){
        if(tdma_slot->idx == rti->my_slot){
            uint8_t *rt_msg;
            uint16_t rt_msg_size;

            ieee_std_frame_hdr_t *ieee_std_frame_hdr;

            if(rti->joint_poll_recved){
                rti->joint_poll_recved = false;

                rt_msg_size = sizeof(ieee_std_frame_hdr_t) + sizeof(rt_bcn_resp_payload_t);
                rt_msg = calloc(1, rt_msg_size);
                if(!rt_msg) return;
                rt_bcn_resp_payload_t *rt_bcn_resp_payload = (rt_bcn_resp_payload_t *)(rt_msg + sizeof(ieee_std_frame_hdr_t));
                rt_bcn_resp_payload->msg_type = RT_BCN_RESP_MSG;
                rt_bcn_resp_payload->len = 2;
                rt_bcn_resp_payload->channel = rti->channel;

                ieee_std_frame_hdr = (ieee_std_frame_hdr_t *)rt_msg;
                ieee_std_frame_hdr->dst_address = rti->reqt_poll_src;
            }
            else if(rti->joint_reqt_recved){
                rti->joint_reqt_recved = false;

                rt_msg_size = sizeof(ieee_std_frame_hdr_t) + sizeof(rt_bcn_acpt_payload_t);
                rt_msg = calloc(1, rt_msg_size);
                if(!rt_msg) return;
                rt_bcn_acpt_payload_t *rt_bcn_acpt_payload = (rt_bcn_acpt_payload_t *)(rt_msg + sizeof(ieee_std_frame_hdr_t));
                rt_bcn_acpt_payload->msg_type = RT_BCN_ACPT_MSG;
                rt_bcn_acpt_payload->len = 2;

                uint16_t reqt_channel = 0x0001 << rti->reqt_channel;
                if(rti->channel & reqt_channel){
                    rt_bcn_acpt_payload->channel = 0;
                }else{
                    rt_bcn_acpt_payload->channel = reqt_channel;
                    rti->channel |= reqt_channel;
                }

                ieee_std_frame_hdr = (ieee_std_frame_hdr_t *)rt_msg;
                ieee_std_frame_hdr->dst_address = rti->reqt_poll_src;
            }
            else{
                rt_msg_size = sizeof(ieee_std_frame_hdr_t) + sizeof(rt_bcn_norm_payload_t);
                rt_msg = calloc(1, rt_msg_size);
                if(!rt_msg) return;
                rt_bcn_norm_payload_t *rt_bcn_norm_payload = (rt_bcn_norm_payload_t *)(rt_msg + sizeof(ieee_std_frame_hdr_t));
                rt_bcn_norm_payload->msg_type = RT_BCN_NORM_MSG;
                rt_bcn_norm_payload->len = 12;
                rt_bcn_norm_payload->location_x = rti->anchors[rti->my_slot].location_x;
                rt_bcn_norm_payload->location_y = rti->anchors[rti->my_slot].location_y;
                rt_bcn_norm_payload->location_z = rti->anchors[rti->my_slot].location_z;

                ieee_std_frame_hdr = (ieee_std_frame_hdr_t *)rt_msg;
                ieee_std_frame_hdr->dst_address = RT_BROADCAST_ADDR;
            }

            ieee_std_frame_hdr->fctrl = UWB_FCTRL_STD_DATA_FRAME;
            ieee_std_frame_hdr->seq_num = rti->seqno++;
            ieee_std_frame_hdr->PANID = rti->tdma->dev_inst->pan_id;
            ieee_std_frame_hdr->src_address = rti->tdma->dev_inst->my_short_address;

            uwb_write_tx(rti->dev_inst, rt_msg, 0, rt_msg_size);
            uwb_write_tx_fctrl(rti->tdma->dev_inst, rt_msg_size, 0);
            uwb_set_wait4resp(rti->tdma->dev_inst, false);

            uint64_t dx_time = tdma_tx_slot_start(rti->tdma, tdma_slot->idx) & 0xFFFFFFFFFE00UL;
            uwb_set_delay_start(rti->dev_inst, dx_time);
            if (uwb_start_tx(rti->dev_inst).start_tx_error) {
                printf("BCN TX error\n");
            }

            free(rt_msg);
        }
    }
    else if(tdma_slot->idx == MYNEWT_VAL(UWB_SVC_SLOT)){

    }else{

    }
}