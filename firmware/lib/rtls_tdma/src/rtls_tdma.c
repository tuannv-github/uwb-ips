#include <dpl/dpl.h>

#include <rtls_tdma/rtls_tdma.h>

// static void rst_joint_list_cb(tdma_slot_t *tdma_slot);
// static void rst_joint_reqt_cb(tdma_slot_t *tdma_slot);
// static void rst_joint_jted_cb(tdma_slot_t *tdma_slot);
static bool 
rtls_tdma_slot_listen(rtls_tdma_instance_t *rti, uwb_dev_modes_t mode, uint16_t idx);

static void bcn_slot_cb_mine(tdma_slot_t *tdma_slot);
static void bcn_slot_cb_othr(tdma_slot_t *tdma_slot);
static void svc_slot_cb(tdma_slot_t *tdma_slot);

static void
node_slot_map_printf(rtls_tdma_instance_t *rti){
    printf("- My slot: %d\n", rti->slot_idx);
    for(int i=0; i<MYNEWT_VAL(TDMA_NSLOTS); i++){
        if(rti->nodes[rti->slot_idx].slot_map & ((uint64_t)1 << i)){
            printf("--- Slot: %02d, addr: 0x%02x, map:", i, rti->nodes[i].addr);
            for(int j=0; j<MYNEWT_VAL(TDMA_NSLOTS); j++){
                printf("%d", (uint8_t)((rti->nodes[i].slot_map >> j) & 0x01));
            }
            printf(" \n");
        }
    }
}

static void
node_rmv_all(rtls_tdma_instance_t *rti){
    memset(rti->nodes, 0, MYNEWT_VAL(TDMA_NSLOTS) * sizeof(rtls_tdma_node_t));
    /* the first slot is reserved for synchronization */
    for(int i=0; i<MYNEWT_VAL(TDMA_NSLOTS); i++){
        rti->nodes[i].slot_map = 0x01;
    }
}

// static void 
// node_rmv(rtls_tdma_instance_t *rti, uint16_t idx){
//     rti->nodes[rti->my_slot].slot &= ~((uint64_t)1 << idx);
//     memset(&rti->nodes[idx], 0, sizeof(rtls_tdma_node_t));
//     /* the first slot is reserved for synchronization */
//     rti->nodes[idx].slot = 0x01;
// }

static void 
node_add(rtls_tdma_instance_t *rti, uint16_t idx, uint16_t addr){
    rti->nodes[rti->slot_idx].slot_map |= ((uint64_t)1 << idx);
    rti->nodes[idx].addr = addr;
}

static void
slot_cb(struct dpl_event * ev)
{
    tdma_slot_t *tdma_slot = (tdma_slot_t *)dpl_event_get_arg(ev);
    /* Slot 0 is used for synchronization */
    if(tdma_slot->idx == 0) return;

    rtls_tdma_instance_t *rti = (rtls_tdma_instance_t *)tdma_slot->arg;

    if(tdma_slot->idx <= MYNEWT_VAL(UWB_BCN_SLOT_MAX)){
        /* this is my node */
        if(rti->slot_idx == tdma_slot->idx){
            bcn_slot_cb_mine(tdma_slot);
        }
        /* Maintain anchor list */
        else
        {
            bcn_slot_cb_othr(tdma_slot);
        }
    }
    /* Request for slot if needed */
    else if(tdma_slot->idx == MYNEWT_VAL(UWB_SVC_SLOT)){
        svc_slot_cb(tdma_slot);
    }
    else{
        if(rti->slot_idx != 0){
            if(rti->rtls_tdma_cb != NULL){
                rti->rtls_tdma_cb(rti, tdma_slot);
            }
        }
    }
}

static void 
uwb_ccp_sync_cb(ccp_sync_t ccp_sync, void *arg){
    rtls_tdma_instance_t *rti = (rtls_tdma_instance_t *)arg;

    node_rmv_all(rti);

    switch (ccp_sync)
    {
    case CCP_SYNC_LOST:
        printf("CCP_SYNC_LOST\n");
        /* Slot idx = 0 when sync is lost */
        rti->slot_idx = 0;
        break;
    case CCP_SYNC_SYED:
        printf("CCP_SYNC_SYED\n");
        if(rti->tdma->ccp->config.role == CCP_ROLE_MASTER){
            rti->slot_idx = MYNEWT_VAL(RT_MASTER_SLOT);
            /* Add me to nodes list */
            node_add(rti, rti->slot_idx, rti->dev_inst->my_short_address);
            /* Master always accept its slot */
            rti->nodes[rti->slot_idx].accepted = true;
        }
        break;
    default:
        break;
    }

    node_slot_map_printf(rti);
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

static bool 
rtls_tdma_slot_listen(rtls_tdma_instance_t *rti, uwb_dev_modes_t mode, uint16_t idx)
{
    uwb_set_rx_timeout(rti->dev_inst, 3*rti->tdma->ccp->period/rti->tdma->nslots/4);
    uwb_set_delay_start(rti->dev_inst, tdma_rx_slot_start(rti->tdma, idx));

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

    return rti->dev_inst->status.rx_timeout_error == 0 ? true : false;
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
 * @fn superframe_cb(struct uwb_dev * inst, struct uwb_mac_interface * cbs)
 * @brief
 *
 * @param inst  Pointer to struct uwb_dev.
 * @param cbs   Pointer to struct uwb_mac_interface.
 *
 * @return bool based on the totality of the handling which is false this implementation.
 */
static bool
superframe_cb(struct uwb_dev * inst, struct uwb_mac_interface * cbs)
{
    rtls_tdma_instance_t *rti = (rtls_tdma_instance_t *)cbs->inst_ptr;
    for(int i=1; i<MYNEWT_VAL(TDMA_NSLOTS); i++){
        rti->nodes[i].available = false;
    }

    rti->nodes[rti->slot_idx].available = true;

    return false;
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
    rti->umi.superframe_cb = superframe_cb,

    uwb_mac_append_interface(rti->dev_inst, &rti->umi);

    rti->slot_idx = 0; /*rti->slot_idx = 0 mean no sync yet */
    rtls_ccp_set_sync_cb(rti->tdma->ccp, uwb_ccp_sync_cb, rti);
    rtls_ccp_start(rti->tdma->ccp);

    for (uint16_t i = 0; i < MYNEWT_VAL(TDMA_NSLOTS); i++){
        tdma_assign_slot(rti->tdma, slot_cb,  i, rti);
    }
}

static void
bcn_slot_cb_mine(tdma_slot_t *tdma_slot){

    uint8_t *rt_msg;
    uint16_t rt_msg_size;
    ieee_std_frame_hdr_t *ieee_std_frame_hdr;
    rtls_tdma_instance_t *rti = (rtls_tdma_instance_t *)tdma_slot->arg;

    bool accepted = true;
    for(int i=0; i<=MYNEWT_VAL(UWB_BCN_SLOT_MAX); i++){
        if(rti->nodes[i].addr != 0 && rti->nodes[i].accepted == false){
            accepted = false;
            break;
        }
    }
    /* Return if my slot is not accpedted by all node */
    if(!accepted) return;

    /* If I received a slot request, I will response for this request immediately */
    if(rti->slot_req > 0){

        rt_msg_size = sizeof(ieee_std_frame_hdr_t) + sizeof(rt_slot_t) + sizeof(rt_slot_t);
        rt_msg = calloc(1, rt_msg_size);
        if(!rt_msg) return;

        rt_slot_t *rt_slot = (rt_slot_t *)(rt_msg + sizeof(ieee_std_frame_hdr_t));
        rt_slot->msg_type = RT_ACPT_MSG;
        rt_slot->len = sizeof(struct _rt_slot_data_t);
        rt_slot->slot_map = (~rti->nodes[rti->slot_idx].slot_map) & rti->slot_req;

        /* Marks this slot as occupied */
        rti->nodes[rti->slot_idx].slot_map |=  rt_slot->slot_map;   
        for(int j=1; j<MYNEWT_VAL(TDMA_NSLOTS); j++){
            if(rt_slot->slot_map & ((slot_map_t)1 << j)){
                node_add(rti, j, rti->slot_req_addr);
                rti->nodes[j].accepted = true;
                break;
            }
        }

        ieee_std_frame_hdr = (ieee_std_frame_hdr_t *)rt_msg;
        ieee_std_frame_hdr->dst_address = rti->slot_req_addr;

        /* Remove request */
        rti->slot_req  = 0;

        rt_slot = (rt_slot_t *)(rt_msg + sizeof(ieee_std_frame_hdr_t) + sizeof(rt_slot_t));
        rt_slot->msg_type = RT_SLOT_MSG;
        rt_slot->len = sizeof(struct _rt_slot_data_t);
        rt_slot->slot_map = rti->nodes[rti->slot_idx].slot_map;
    }
    /* else: just do my beacon job */
    else{
        if(rti->seqno % 2){
            rt_msg_size = sizeof(ieee_std_frame_hdr_t) + sizeof(rt_loca_t);
            rt_msg = calloc(1, rt_msg_size);
            if(!rt_msg) return;

            rt_loca_t *rt_loca = (rt_loca_t *)(rt_msg + sizeof(ieee_std_frame_hdr_t));
            rt_loca->msg_type = RT_LOCA_MSG;
            rt_loca->len = sizeof(struct _rt_loca_data_t);
            rt_loca->location_x = rti->nodes[rti->slot_idx].location_x;
            rt_loca->location_y = rti->nodes[rti->slot_idx].location_y;
            rt_loca->location_z = rti->nodes[rti->slot_idx].location_z;
        }
        else{
            rt_msg_size = sizeof(ieee_std_frame_hdr_t) + sizeof(rt_slot_t);
            rt_msg = calloc(1, rt_msg_size);
            if(!rt_msg) return;

            rt_slot_t *rt_slot = (rt_slot_t *)(rt_msg + sizeof(ieee_std_frame_hdr_t));
            rt_slot->msg_type = RT_SLOT_MSG;
            rt_slot->len = sizeof(struct _rt_slot_data_t);
            rt_slot->slot_map = rti->nodes[rti->slot_idx].slot_map;
        }

        ieee_std_frame_hdr = (ieee_std_frame_hdr_t *)rt_msg;
        ieee_std_frame_hdr->dst_address = RT_BROADCAST_ADDR;
    }
    
    ieee_std_frame_hdr->fctrl = UWB_FCTRL_STD_DATA_FRAME;
    ieee_std_frame_hdr->seq_num = rti->seqno++;
    ieee_std_frame_hdr->PANID = rti->tdma->dev_inst->pan_id;
    ieee_std_frame_hdr->src_address = rti->tdma->dev_inst->my_short_address;

    uwb_write_tx(rti->dev_inst, rt_msg, 0, rt_msg_size);
    uwb_write_tx_fctrl(rti->dev_inst, rt_msg_size, 0);
    uwb_set_wait4resp(rti->dev_inst, false);

    uint64_t dx_time = tdma_tx_slot_start(rti->tdma, tdma_slot->idx) & 0xFFFFFFFFFE00UL;
    uwb_set_delay_start(rti->dev_inst, dx_time);
    if (uwb_start_tx(rti->dev_inst).start_tx_error) {
        printf("BCN TX error\n");
    }

    free(rt_msg);
}

static void
bcn_slot_cb_othr(tdma_slot_t *tdma_slot){
    rtls_tdma_instance_t *rti = (rtls_tdma_instance_t *)tdma_slot->arg;
    if(rtls_tdma_slot_listen(rti, UWB_BLOCKING, tdma_slot->idx)){

        ieee_std_frame_hdr_t *ieee_std_frame_hdr = (ieee_std_frame_hdr_t *)rti->dev_inst->rxbuf;
        if(ieee_std_frame_hdr->PANID != rti->dev_inst->pan_id) return;
        
        if(ieee_std_frame_hdr->src_address != rti->nodes[tdma_slot->idx].addr){
            if(rti->nodes[tdma_slot->idx].addr == 0){
                rti->nodes[tdma_slot->idx].addr = ieee_std_frame_hdr->src_address;
                rti->nodes[tdma_slot->idx].accepted = false;
            }else{
                printf("Slot colission\n");
                return;
            }
        }
        rti->nodes[tdma_slot->idx].available = true;

        if(rti->dev_inst->rxbuf_size - sizeof(ieee_std_frame_hdr_t) < sizeof(msg_hdr_t)) return;
        
        uint16_t frame_idx = sizeof(ieee_std_frame_hdr_t);
        while(frame_idx < rti->dev_inst->rxbuf_size){
            switch(rti->dev_inst->rxbuf[frame_idx]){
                case RT_ACPT_MSG:
                {
                    if(ieee_std_frame_hdr->dst_address != rti->dev_inst->my_short_address) return;
                    rt_slot_t *rt_slot = (rt_slot_t *)(&rti->dev_inst->rxbuf[frame_idx]);
                    if(rt_slot->slot_map > 0){
                        for(int i=1; i < MYNEWT_VAL(TDMA_NSLOTS); i++){
                            if(rti->nodes[i].addr != 0){
                                rti->nodes[i].accepted = true;
                                for(int j=1; j<MYNEWT_VAL(TDMA_NSLOTS); j++){
                                    if(rt_slot->slot_map & ((slot_map_t)1 << j)){
                                        rti->slot_idx = j;
                                        node_add(rti, rti->slot_idx, rti->dev_inst->my_short_address);
                                        rti->nodes[rti->slot_idx].accepted = true;
                                        break;
                                    }
                                }
                                break;
                            }
                        }
                    }
                }
                break;
                case RT_LOCA_MSG:
                {
                    rt_loca_t *rt_bcn_norm_payload = (rt_loca_t *)(&rti->dev_inst->rxbuf[frame_idx]);
                    rti->nodes[tdma_slot->idx].location_x = rt_bcn_norm_payload->location_x;
                    rti->nodes[tdma_slot->idx].location_y = rt_bcn_norm_payload->location_y;
                    rti->nodes[tdma_slot->idx].location_z = rt_bcn_norm_payload->location_z;

                    rti->nodes[rti->slot_idx].slot_map |= (slot_map_t)1 << tdma_slot->idx;
                }
                break;
                case RT_SLOT_MSG:
                {
                    rt_slot_t *rt_slot = (rt_slot_t *)(&rti->dev_inst->rxbuf[frame_idx]);
                    rti->nodes[tdma_slot->idx].slot_map = rt_slot->slot_map;
                    rti->nodes[rti->slot_idx].slot_map |= (slot_map_t)1 << tdma_slot->idx;
                }
                default:
                break;
            }
            uint8_t msg_len = rti->dev_inst->rxbuf[frame_idx+1];
            frame_idx += sizeof(msg_hdr_t) + msg_len;
        }
    }
}

static void
svc_slot_cb(tdma_slot_t *tdma_slot){
    rtls_tdma_instance_t *rti = (rtls_tdma_instance_t *)tdma_slot->arg;

    bool accepted = true;
    for(int i=1; i<=MYNEWT_VAL(UWB_BCN_SLOT_MAX); i++){
        if(rti->nodes[i].addr != 0 && rti->nodes[i].accepted == false){
            accepted = false;
            break;
        }
    }

    /* If I did have slot, I would listen for slot request */
    if(accepted){
        if(rtls_tdma_slot_listen(rti, UWB_BLOCKING, tdma_slot->idx)){

            ieee_std_frame_hdr_t *ieee_std_frame_hdr = (ieee_std_frame_hdr_t *)rti->dev_inst->rxbuf;
            if(ieee_std_frame_hdr->PANID != rti->dev_inst->pan_id) return;
            if(ieee_std_frame_hdr->dst_address != rti->dev_inst->my_short_address) return;

            if(rti->dev_inst->rxbuf_size - sizeof(ieee_std_frame_hdr_t) < sizeof(rt_slot_t)) return;

            rt_slot_t *rt_slot = (rt_slot_t *)(rti->dev_inst->rxbuf + sizeof(ieee_std_frame_hdr_t));

            rti->slot_req = rt_slot->slot_map;
            rti->slot_req_addr = ieee_std_frame_hdr->src_address;
        }
    }
    else{
        uint16_t rt_msg_size = sizeof(ieee_std_frame_hdr_t) + sizeof(rt_slot_t);
        uint8_t *rt_msg = calloc(1, rt_msg_size);

        ieee_std_frame_hdr_t *ieee_std_frame_hdr = (ieee_std_frame_hdr_t *)rt_msg;
        rt_slot_t *rt_slot = (rt_slot_t *)(rt_msg + sizeof(ieee_std_frame_hdr_t));

        ieee_std_frame_hdr->fctrl = UWB_FCTRL_STD_DATA_FRAME;
        ieee_std_frame_hdr->seq_num = rti->seqno++;
        ieee_std_frame_hdr->PANID = rti->tdma->dev_inst->pan_id;
        ieee_std_frame_hdr->src_address = rti->tdma->dev_inst->my_short_address;

        rt_slot->msg_type = RT_REQT_MSG;
        rt_slot->len = sizeof(struct _rt_slot_data_t);

        rti->slot_req = (slot_map_t)-1;
        if(rti->role == RTR_ANCHOR){
            for(int i=1; i<=MYNEWT_VAL(UWB_BCN_SLOT_MAX); i++){
                if(!rti->nodes[i].available) continue;
                rti->slot_req &= ~rti->nodes[i].slot_map;
                // printf("rti->nodes[i].slot_map %d, %lld\n", i, rti->nodes[i].slot_map);
            }
            for(int i=1; i<=MYNEWT_VAL(UWB_BCN_SLOT_MAX); i++){
                slot_map_t slot = ((slot_map_t)1 << i);
                if(rti->slot_req & slot){
                    rti->slot_req = slot;
                    rt_slot->slot_map = slot;
                    break;
                }
            }
        }
        else if(rti->role == RTR_TAG){
            for(int i=MYNEWT_VAL(UWB_BCN_SLOT_MAX) + 1; i<MYNEWT_VAL(TDMA_NSLOTS); i++){
                if(!rti->nodes[i].available) continue;
                rti->slot_req &= ~rti->nodes[i].slot_map;
            }
            for(int i=MYNEWT_VAL(UWB_BCN_SLOT_MAX) + 1; i<MYNEWT_VAL(TDMA_NSLOTS); i++){
                slot_map_t slot = ((slot_map_t)1 << i);
                if(rti->slot_req & slot){
                    rti->slot_req = slot;
                    rt_slot->slot_map = slot;
                    break;
                }
            }
        }

        for(int i=0; i<=MYNEWT_VAL(UWB_BCN_SLOT_MAX); i++){
            if(rti->nodes[i].addr != 0 && !rti->nodes[i].accepted){
                ieee_std_frame_hdr->dst_address = rti->nodes[i].addr;

                uwb_write_tx(rti->dev_inst, rt_msg, 0, rt_msg_size);
                uwb_write_tx_fctrl(rti->dev_inst, rt_msg_size, 0);
                uwb_set_wait4resp(rti->dev_inst, false);

                uint64_t dx_time = tdma_tx_slot_start(rti->tdma, tdma_slot->idx) & 0xFFFFFFFFFE00UL;
                uwb_set_delay_start(rti->dev_inst, dx_time);
                if (uwb_start_tx(rti->dev_inst).start_tx_error) {
                    printf("rst_joint_reqt_cb TX error\n");
                }
            }
        }
        free(rt_msg);
    }
}

// void rst_joint_list_cb(tdma_slot_t *tdma_slot){

//     if(tdma_slot->idx > MYNEWT_VAL(UWB_BCN_SLOT_MAX)) return;

//     rtls_tdma_instance_t *rti = (rtls_tdma_instance_t *)tdma_slot->arg;
//     assert(rti);

//     uwb_set_rx_timeout(rti->dev_inst, 3*rti->tdma->ccp->period/rti->tdma->nslots/4);
//     uwb_set_delay_start(rti->dev_inst, tdma_rx_slot_start(rti->tdma, tdma_slot->idx));
//     if(rtls_tdma_listen(rti, UWB_BLOCKING).start_rx_error){
//         printf("start_rx_error: %s:%d\n", __FILE__, __LINE__);
//     }
//     else{
//         if(rti->dev_inst->status.rx_timeout_error) return;
//         if(rti->dev_inst->fctrl != UWB_FCTRL_STD_DATA_FRAME) return;

//         /* Reset anchor timout */
//         rti->nodes[tdma_slot->idx].timeout = 0;

//         ieee_std_frame_hdr_t *ieee_std_frame_hdr = (ieee_std_frame_hdr_t *)rti->dev_inst->rxbuf;
//         if(ieee_std_frame_hdr->PANID != rti->dev_inst->pan_id) return;
//         if(ieee_std_frame_hdr->dst_address != RT_BROADCAST_ADDR) return;

//         /* Continue process only if there is at least sizeof(_msg_hdr_t) in frame payload */
//         if(rti->dev_inst->rxbuf_size - sizeof(ieee_std_frame_hdr_t) < sizeof(msg_hdr_t)) return;
//         uint16_t frame_idx = sizeof(ieee_std_frame_hdr_t);

//         while(frame_idx < rti->dev_inst->rxbuf_size){

//             if(ieee_std_frame_hdr->src_address != rti->nodes[tdma_slot->idx].addr){
//                 rti->nodes[tdma_slot->idx].addr = ieee_std_frame_hdr->src_address;
//                 rti->nodes[tdma_slot->idx].bcn_cnt = 0;
//             }

//             switch(rti->dev_inst->rxbuf[frame_idx]){
//                 case RT_LOCA_MSG:
//                 {
//                     rt_loca_t *rt_bcn_norm_payload = (rt_loca_t *)(&rti->dev_inst->rxbuf[frame_idx]);

//                     rti->nodes[tdma_slot->idx].location_x = rt_bcn_norm_payload->location_x;
//                     rti->nodes[tdma_slot->idx].location_y = rt_bcn_norm_payload->location_y;
//                     rti->nodes[tdma_slot->idx].location_z = rt_bcn_norm_payload->location_z;
//                 }
//                 break;
//                 case RT_SLOT_MSG:
//                 {
//                     rt_slot_t *rt_slot = (rt_slot_t *)(&rti->dev_inst->rxbuf[frame_idx]);

//                     rti->nodes[tdma_slot->idx].slot = rt_slot->slot;
//                     rti->nodes[tdma_slot->idx].bcn_cnt++;

//                     bool next_state = true;
//                     for(int i=0; i<=MYNEWT_VAL(UWB_BCN_SLOT_MAX); i++){
//                         if(rti->nodes[i].addr != 0 && rti->nodes[i].bcn_cnt < MYNEWT_VAL(RT_JOINT_LIST_THRESH)){
//                             next_state = false;
//                         }
//                     }

//                     if(next_state){
//                         node_slot_map_printf(rti);
//                         rti->joint_reqt_cnt = 0;
//                         rti->joint_reqt = true;
//                         rti->cstate = RTS_JOINT_REQT;
//                         printf("state: RTS_JOINT_REQT\n");
//                     }
//                 }
//                 break;
//                 default:
//                 break;
//             }
//             uint8_t msg_len = rti->dev_inst->rxbuf[frame_idx+1];
//             frame_idx += 2 + msg_len;
//         }
//     }
// }

// void rst_joint_reqt_cb(tdma_slot_t *tdma_slot){
//     rtls_tdma_instance_t *rti = (rtls_tdma_instance_t *)tdma_slot->arg;
//     assert(rti);

//     if(tdma_slot->idx == MYNEWT_VAL(UWB_SVC_SLOT)){
//         if(rti->joint_reqt_cnt > MYNEWT_VAL(RT_BCN_REQT_THRESH)){
//             rti->joint_reqt = false;
//             rti->cstate = RTS_JOINT_LIST;
//             printf("state: RTS_JOINT_LIST\n");
//             return;
//         }

//         uint16_t rt_msg_size = sizeof(ieee_std_frame_hdr_t) + sizeof(rt_slot_t);;
//         uint8_t *rt_msg = calloc(1, rt_msg_size);

//         ieee_std_frame_hdr_t *ieee_std_frame_hdr = (ieee_std_frame_hdr_t *)rt_msg;
//         rt_slot_t *rt_slot = (rt_slot_t *)(rt_msg + sizeof(ieee_std_frame_hdr_t));

//         ieee_std_frame_hdr->fctrl = UWB_FCTRL_STD_DATA_FRAME;
//         ieee_std_frame_hdr->seq_num = rti->seqno++;
//         ieee_std_frame_hdr->PANID = rti->tdma->dev_inst->pan_id;
//         ieee_std_frame_hdr->dst_address = RT_BROADCAST_ADDR;
//         ieee_std_frame_hdr->src_address = rti->tdma->dev_inst->my_short_address;

//         rt_slot->msg_type = RT_REQT_MSG;
//         rt_slot->len = sizeof(struct _rt_slot_data_t);
//         rti->joint_reqt_slot = 0;

//         if(rti->role == RTR_ANCHOR){
//             for(int i=0; i<=MYNEWT_VAL(UWB_BCN_SLOT_MAX); i++){
//                 if(rti->nodes[i].addr == 0) continue;
//                 rti->joint_reqt_slot |= ~rti->nodes[i].slot;
//             }
//             for(int i=0; i<=MYNEWT_VAL(UWB_BCN_SLOT_MAX); i++){
//                 uint16_t slot = (0x0001 << i);
//                 if(rti->joint_reqt_slot & slot){
//                     rti->joint_reqt_slot    = slot;
//                     rt_slot->slot           = slot;
//                     break;
//                 }
//             }
//         }
//         else if(rti->role == RTR_TAG){
//             for(int i=MYNEWT_VAL(UWB_BCN_SLOT_MAX) + 1; i<MYNEWT_VAL(TDMA_NSLOTS); i++){
//                 if(rti->nodes[i].addr == 0) continue;
//                 rti->joint_reqt_slot |= ~rti->nodes[i].slot;
//             }
//             for(int i=MYNEWT_VAL(UWB_BCN_SLOT_MAX) + 1; i<MYNEWT_VAL(TDMA_NSLOTS); i++){
//                 uint16_t slot = (0x0001 << i);
//                 if(rti->joint_reqt_slot & slot){
//                     rti->joint_reqt_slot    = slot;
//                     rt_slot->slot           = slot;
//                     break;
//                 }
//             }
//         }
//         else{
//             printf("No RTLS TDMA role configrued\n");
//             rti->cstate = RTS_JOINT_LIST;
//             return;
//         }

//         if(rti->joint_reqt_slot == 0){
//             printf("No slot available\n");
//             rti->cstate = RTS_JOINT_LIST;
//             return;
//         }

//         for(int i=0; i<=MYNEWT_VAL(UWB_BCN_SLOT_MAX); i++){
//             if(rti->nodes[i].addr == 0) continue;
//         }

//         uwb_write_tx(rti->dev_inst, rt_msg, 0, rt_msg_size);
//         uwb_write_tx_fctrl(rti->dev_inst, rt_msg_size, 0);
//         uwb_set_wait4resp(rti->dev_inst, false);

//         uint64_t dx_time = tdma_tx_slot_start(rti->tdma, tdma_slot->idx) & 0xFFFFFFFFFE00UL;
//         uwb_set_delay_start(rti->dev_inst, dx_time);
//         if (uwb_start_tx(rti->dev_inst).start_tx_error) {
//             printf("rst_joint_reqt_cb TX error\n");
//         }
//     }
//     else if (tdma_slot->idx <= MYNEWT_VAL(UWB_BCN_SLOT_MAX)){

//         uwb_set_rx_timeout(rti->dev_inst, 3*rti->tdma->ccp->period/rti->tdma->nslots/4);
//         uwb_set_delay_start(rti->dev_inst, tdma_rx_slot_start(rti->tdma, tdma_slot->idx));
//         if(rtls_tdma_listen(rti, UWB_BLOCKING).start_rx_error){
//             printf("start_rx_error: %s:%d\n", __FILE__, __LINE__);
//         }
//         else{
//             if(rti->dev_inst->status.rx_timeout_error) return;

//             rti->nodes[tdma_slot->idx].timeout = 0;

//             ieee_std_frame_hdr_t *ieee_std_frame_hdr = (ieee_std_frame_hdr_t *)rti->dev_inst->rxbuf;
//             if(ieee_std_frame_hdr->PANID != rti->dev_inst->pan_id) return;
//             if(ieee_std_frame_hdr->dst_address != rti->dev_inst->my_short_address) return;
//             if(rti->nodes[tdma_slot->idx].addr != ieee_std_frame_hdr->src_address){
//                 rti->cstate = RTS_JOINT_LIST;
//                 return;
//             }

//             if(rti->dev_inst->rxbuf_size - sizeof(ieee_std_frame_hdr_t) < sizeof(msg_hdr_t)) return;
//             uint16_t frame_idx = sizeof(ieee_std_frame_hdr_t);
//             while(frame_idx < rti->dev_inst->rxbuf_size){
//                 switch(rti->dev_inst->rxbuf[frame_idx]){
//                     case RT_ACPT_MSG:
//                     {
//                         rt_slot_t *rt_slot = (rt_slot_t *)(&rti->dev_inst->rxbuf[frame_idx]);
//                         rti->joint_reqt_slot &= rt_slot->slot;

//                         bool all_accepted = true;
//                         for(int i=0; i<=MYNEWT_VAL(UWB_BCN_SLOT_MAX); i++){
//                             if(rti->nodes[i].addr != 0 && !rti->nodes[i].accepted){
//                                 all_accepted = false;
//                                 break;
//                             }
//                         }

//                         if(all_accepted && rti->joint_reqt_slot != 0){
//                             rti->cstate = RTS_JOINT_JTED;
//                             for(rti->my_slot = 0; rti->my_slot<=MYNEWT_VAL(UWB_BCN_SLOT_MAX); rti->my_slot++){
//                                 if(rti->joint_reqt_slot & (0x0001 << rti->my_slot)) break;
//                             }
//                             printf("jointed: slot:%d\n", rti->my_slot);
//                             rti->nodes[rti->my_slot].addr = rti->dev_inst->my_short_address;
//                             rti->nodes[rti->my_slot].slot |= 0x0001 << rti->my_slot;
//                         }
//                     }
//                     break;
//                 }
//                 uint8_t msg_len = rti->dev_inst->rxbuf[frame_idx+1];
//                 frame_idx += msg_len;
//             }
//         }
//     }
// }

// void rst_joint_jted_cb(tdma_slot_t *tdma_slot){

//     rtls_tdma_instance_t *rti = (rtls_tdma_instance_t *)tdma_slot->arg;

//     if(tdma_slot->idx <= MYNEWT_VAL(UWB_BCN_SLOT_MAX)){
//         if(tdma_slot->idx == rti->my_slot){
            
//             /* Reset anchor timout */
//             rti->anchors[tdma_slot->idx].timeout = 0;

//             if(rti->joint_reqt_cnt > MYNEWT_VAL(RT_BCN_REQT_THRESH)){
//                 rti->joint_reqt_cnt = 0;
//                 rti->joint_reqt = false;
//             }

//             uint8_t *rt_msg;
//             uint16_t rt_msg_size;

//             ieee_std_frame_hdr_t *ieee_std_frame_hdr;

//             if(rti->joint_reqt){

//                 rt_msg_size = sizeof(ieee_std_frame_hdr_t) + sizeof(rt_slot_t);
//                 rt_msg = calloc(1, rt_msg_size);
//                 if(!rt_msg) return;

//                 rt_slot_t *rt_slot = (rt_slot_t *)(rt_msg + sizeof(ieee_std_frame_hdr_t));
//                 rt_slot->msg_type = RT_ACPT_MSG;
//                 rt_slot->len = sizeof(struct _rt_slot_data_t);
//                 rt_slot->slot = (~rti->anchors[rti->my_slot].slot) & rti->joint_reqt_slot;

//                 if(rt_slot->slot){
//                     rti->anchors[rti->my_slot].slot |= rt_slot->slot;
//                 }

//                 ieee_std_frame_hdr = (ieee_std_frame_hdr_t *)rt_msg;
//                 ieee_std_frame_hdr->dst_address = rti->joint_reqt_src;   
//             }
//             else{
//                 if(rti->seqno % 2){
//                     rt_msg_size = sizeof(ieee_std_frame_hdr_t) + sizeof(rt_loca_t);
//                     rt_msg = calloc(1, rt_msg_size);
//                     if(!rt_msg) return;

//                     rt_loca_t *rt_bcn_norm_payload = (rt_loca_t *)(rt_msg + sizeof(ieee_std_frame_hdr_t));
//                     rt_bcn_norm_payload->msg_type = RT_LOCA_MSG;
//                     rt_bcn_norm_payload->len = sizeof(struct _rt_loca_data_t);
//                     rt_bcn_norm_payload->location_x = rti->anchors[rti->my_slot].location_x;
//                     rt_bcn_norm_payload->location_y = rti->anchors[rti->my_slot].location_y;
//                     rt_bcn_norm_payload->location_z = rti->anchors[rti->my_slot].location_z;
//                 }
//                 else{
//                     rt_msg_size = sizeof(ieee_std_frame_hdr_t) + sizeof(rt_slot_t);
//                     rt_msg = calloc(1, rt_msg_size);
//                     if(!rt_msg) return;

//                     rt_slot_t *rt_slot = (rt_slot_t *)(rt_msg + sizeof(ieee_std_frame_hdr_t));
//                     rt_slot->msg_type = RT_SLOT_MSG;
//                     rt_slot->len = sizeof(struct _rt_slot_data_t);
//                     rt_slot->slot = rti->anchors[rti->my_slot].slot;
//                 }

//                 ieee_std_frame_hdr = (ieee_std_frame_hdr_t *)rt_msg;
//                 ieee_std_frame_hdr->dst_address = RT_BROADCAST_ADDR;
//             }
    
//             ieee_std_frame_hdr->fctrl = UWB_FCTRL_STD_DATA_FRAME;
//             ieee_std_frame_hdr->seq_num = rti->seqno++;
//             ieee_std_frame_hdr->PANID = rti->tdma->dev_inst->pan_id;
//             ieee_std_frame_hdr->src_address = rti->tdma->dev_inst->my_short_address;

//             uwb_write_tx(rti->dev_inst, rt_msg, 0, rt_msg_size);
//             uwb_write_tx_fctrl(rti->dev_inst, rt_msg_size, 0);
//             uwb_set_wait4resp(rti->dev_inst, false);

//             uint64_t dx_time = tdma_tx_slot_start(rti->tdma, tdma_slot->idx) & 0xFFFFFFFFFE00UL;
//             uwb_set_delay_start(rti->dev_inst, dx_time);
//             if (uwb_start_tx(rti->dev_inst).start_tx_error) {
//                 printf("BCN TX error\n");
//             }

//             free(rt_msg);
//         }
//         else{

//             uwb_set_rx_timeout(rti->dev_inst, 3*rti->tdma->ccp->period/rti->tdma->nslots/4);
//             uwb_set_delay_start(rti->dev_inst, tdma_rx_slot_start(rti->tdma, tdma_slot->idx));
//             if(rtls_tdma_listen(rti, UWB_BLOCKING).start_rx_error){
//                 printf("start_rx_error: %s:%d\n", __FILE__, __LINE__);
//             }
//             else{
//                 if(rti->dev_inst->status.rx_timeout_error) return;

//                 /* Check if it is a beacon message from correct anchor */
//                 ieee_std_frame_hdr_t *ieee_std_frame_hdr = (ieee_std_frame_hdr_t *)rti->dev_inst->rxbuf;
//                 if(ieee_std_frame_hdr->src_address != rti->nodes[tdma_slot->idx].addr) return;
//                 if(ieee_std_frame_hdr->dst_address != RT_BROADCAST_ADDR) return;
//                 if(ieee_std_frame_hdr->PANID != rti->dev_inst->pan_id) return;

//                 /* Reset anchor timout */
//                 rti->nodes[tdma_slot->idx].timeout = 0;

//                 if(rti->dev_inst->rxbuf_size - sizeof(ieee_std_frame_hdr_t) < sizeof(msg_hdr_t)) return;
//                 uint16_t frame_idx = sizeof(ieee_std_frame_hdr_t);

//                 if(rti->joint_reqt && rti->joint_reqt_src == ieee_std_frame_hdr->src_address){
//                     rti->joint_reqt_cnt = 0;
//                     rti->joint_reqt = false;
//                 }

//                 while(frame_idx < rti->dev_inst->rxbuf_size){

//                     switch(rti->dev_inst->rxbuf[frame_idx]){
//                         case RT_LOCA_MSG:
//                         {
//                             rt_loca_t *rt_bcn_norm_payload = (rt_loca_t *)(&rti->dev_inst->rxbuf[frame_idx]);

//                             rti->nodes[tdma_slot->idx].location_x = rt_bcn_norm_payload->location_x;
//                             rti->nodes[tdma_slot->idx].location_y = rt_bcn_norm_payload->location_y;
//                             rti->nodes[tdma_slot->idx].location_z = rt_bcn_norm_payload->location_z;
//                         }
//                         break;
//                         case RT_SLOT_MSG:
//                         {
//                             rt_slot_t *rt_slot = (rt_slot_t *)(&rti->dev_inst->rxbuf[frame_idx]);

//                             rti->nodes[tdma_slot->idx].slot = rt_slot->slot;
//                             rti->nodes[tdma_slot->idx].listen_cnt++;
//                         }
//                         break;
//                     }
//                     uint8_t msg_len = rti->dev_inst->rxbuf[frame_idx+1];
//                     frame_idx += msg_len;
//                 }
//             }
//         }
//     }
//     else if(tdma_slot->idx == MYNEWT_VAL(UWB_SVC_SLOT)){
//         uwb_set_rx_timeout(rti->dev_inst, 3*rti->tdma->ccp->period/rti->tdma->nslots/4);
//         uwb_set_delay_start(rti->dev_inst, tdma_rx_slot_start(rti->tdma, tdma_slot->idx));
//         if(rtls_tdma_listen(rti, UWB_BLOCKING).start_rx_error){
//             printf("start_rx_error: %s:%d\n", __FILE__, __LINE__);
//         }
//         else{
//             if(rti->dev_inst->status.rx_timeout_error) return;
//             if(rti->joint_reqt) return;

//             ieee_std_frame_hdr_t *ieee_std_frame_hdr = (ieee_std_frame_hdr_t *)rti->dev_inst->rxbuf;
//             if(ieee_std_frame_hdr->PANID != rti->dev_inst->pan_id) return;
//             if(ieee_std_frame_hdr->dst_address != RT_BROADCAST_ADDR) return;

//             if(rti->dev_inst->rxbuf_size - sizeof(ieee_std_frame_hdr_t) < sizeof(msg_hdr_t)) return;
//             uint16_t frame_idx = sizeof(ieee_std_frame_hdr_t);
//             while(frame_idx < rti->dev_inst->rxbuf_size){
//                 switch(rti->dev_inst->rxbuf[frame_idx]){
//                     case RT_REQT_MSG:
//                     {
//                         rt_slot_t *rt_slot = (rt_slot_t *)(&rti->dev_inst->rxbuf[frame_idx]);
//                         rti->joint_reqt = true;
//                         rti->joint_reqt_src = ieee_std_frame_hdr->src_address;
//                         rti->joint_reqt_slot = rt_slot->slot;
//                         rti->joint_reqt_cnt = 0;
//                     }
//                     break;
//                 }
//                 uint8_t msg_len = rti->dev_inst->rxbuf[frame_idx+1];
//                 frame_idx += msg_len;
//             }
//         }
//     }
//     else{
//         if(rti->rtls_tdma_cb != NULL){
//             rti->rtls_tdma_cb(rti, tdma_slot);
//         }
//     }
// }