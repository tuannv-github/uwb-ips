#include <dpl/dpl.h>

#include <rtls_tdma/rtls_tdma.h>

static bool 
rtls_tdma_slot_listen(rtls_tdma_instance_t *rti, uwb_dev_modes_t mode, uint16_t idx);

static void bcn_slot_cb_mine(tdma_slot_t *tdma_slot);
static void bcn_slot_cb_othr(tdma_slot_t *tdma_slot);
static void svc_slot_cb(tdma_slot_t *tdma_slot);

static void
node_slot_map_printf(rtls_tdma_instance_t *rti){
    printf("- My slot: %d\n", rti->slot_idx);
    for(int i=1; i<MYNEWT_VAL(TDMA_NSLOTS); i++){
        if(rti->nodes[i].addr!=0){
            printf("--- Slot: %02d, addr: 0x%02x, accepted: %d, map:", i, rti->nodes[i].addr, rti->nodes[i].accepted);
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

static void 
node_rmv(rtls_tdma_instance_t *rti, uint16_t idx){
    rti->nodes[rti->slot_idx].slot_map &= ~((uint64_t)1 << idx);
    memset(&rti->nodes[idx], 0, sizeof(rtls_tdma_node_t));
    /* the first slot is reserved for synchronization */
    rti->nodes[idx].slot_map = 0x01;
}

static void 
node_add(rtls_tdma_instance_t *rti, uint16_t idx, uint16_t addr){
    rti->nodes[rti->slot_idx].slot_map |= ((slot_map_t)1 << idx);
    rti->nodes[idx].addr = addr;
}

static bool
node_all_accepted(rtls_tdma_instance_t *rti){
    bool accepted = true;
    uint8_t cnt = 0;
    if(rti->tdma->ccp->config.role == CCP_ROLE_MASTER) return true;
    for(int i=1; i<=MYNEWT_VAL(UWB_BCN_SLOT_MAX); i++){
        if(rti->nodes[i].addr == 0) continue;
        cnt++;
        if(rti->nodes[i].accepted == false){
            accepted = false;
            break;
        }
    }
    return cnt == 0 ? false : accepted;
}

static void
slot_cb(struct dpl_event * ev)
{
    tdma_slot_t *tdma_slot = (tdma_slot_t *)dpl_event_get_arg(ev);
    /* Slot 0 is used for synchronization */
    if(tdma_slot->idx == 0) return;

    rtls_tdma_instance_t *rti = (rtls_tdma_instance_t *)tdma_slot->arg;

    if(tdma_slot->idx <= MYNEWT_VAL(UWB_BCN_SLOT_MAX)){
        /* This is my node */
        if(rti->slot_idx == tdma_slot->idx && node_all_accepted(rti)){
            /* my slot */
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
        if(rti->nodes[i].addr != 0 && i != rti->slot_idx){
            rti->nodes[i].timeout++;
            if(rti->nodes[i].timeout > MYNEWT_VAL(RT_ANCHOR_TIMEOUT)){
                node_rmv(rti, i);
                printf("Remove node: %d\n", i);
                node_slot_map_printf(rti);
            }
        }
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

    rti->nodes[rti->slot_idx].slot_map |= (slot_map_t)1 << tdma_slot->idx;

    /* If I received a slot request, I will response for this request immediately */
    if(rti->slot_req){

        rt_msg_size = sizeof(ieee_std_frame_hdr_t) + sizeof(rt_slot_t) + sizeof(rt_slot_t);
        rt_msg = calloc(1, rt_msg_size);
        if(!rt_msg) return;

        rt_slot_t *rt_slot = (rt_slot_t *)(rt_msg + sizeof(ieee_std_frame_hdr_t));
        rt_slot->msg_type = RT_ACPT_MSG;
        rt_slot->len = sizeof(struct _rt_slot_data_t);
        rt_slot->slot = tdma_slot->idx;
        rt_slot->slot_map = (~rti->nodes[rti->slot_idx].slot_map) & rti->slot_req;

        if(rt_slot->slot_map == 0) {
            rti->slot_req = 0;
            printf("Do not accept slot: 0x%02llx\n", rti->slot_req);
        }
        else{
            printf("Accept slot: 0x%02llx/0x%02llx/0x%02llx\n", rti->nodes[rti->slot_idx].slot_map,  rti->slot_req, (~rti->nodes[rti->slot_idx].slot_map) & rti->slot_req);
        }

        ieee_std_frame_hdr = (ieee_std_frame_hdr_t *)rt_msg;
        ieee_std_frame_hdr->dst_address = rti->slot_req_addr;

        rt_slot = (rt_slot_t *)(rt_msg + sizeof(rt_slot_t) + sizeof(ieee_std_frame_hdr_t));
        rt_slot->msg_type = RT_SLOT_MSG;
        rt_slot->len = sizeof(struct _rt_slot_data_t);
        rt_slot->slot = tdma_slot->idx;
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
            rt_loca->slot = tdma_slot->idx;
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
            rt_slot->slot = tdma_slot->idx;
            rt_slot->slot_map = rti->nodes[rti->slot_idx].slot_map;
        }

        ieee_std_frame_hdr = (ieee_std_frame_hdr_t *)rt_msg;
        ieee_std_frame_hdr->dst_address = RT_BROADCAST_ADDR;
    }
    
    ieee_std_frame_hdr->fctrl = UWB_FCTRL_STD_DATA_FRAME;
    ieee_std_frame_hdr->seq_num = rti->seqno++;
    ieee_std_frame_hdr->PANID = rti->tdma->dev_inst->pan_id;
    ieee_std_frame_hdr->src_address = rti->tdma->dev_inst->my_short_address;
    
    UWB_TX(rti, rt_msg, rt_msg_size, tdma_slot->idx);

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
                if(rti->slot_req != 0 && rti->slot_req_addr == ieee_std_frame_hdr->src_address){
                    printf("Node up: 0x%x\n", ieee_std_frame_hdr->src_address);
                    rti->nodes[tdma_slot->idx].accepted = true;
                    rti->slot_req = 0;
                }

                else{
                    printf("Anchor: 0x%x\n", ieee_std_frame_hdr->src_address);
                    rti->nodes[tdma_slot->idx].accepted = false; 
                }
            }else{
                printf("Slot colission: %d\n", tdma_slot->idx);
                return;
            }
        }
        rti->nodes[tdma_slot->idx].available = true;
        rti->nodes[tdma_slot->idx].timeout = 0;
        
        if(rti->dev_inst->rxbuf_size - sizeof(ieee_std_frame_hdr_t) < sizeof(msg_hdr_t)) return;
        uint16_t frame_idx = sizeof(ieee_std_frame_hdr_t);
        while(frame_idx < rti->dev_inst->rxbuf_size){
            switch(rti->dev_inst->rxbuf[frame_idx]){
                case RT_ACPT_MSG:
                {
                    if(ieee_std_frame_hdr->dst_address != rti->dev_inst->my_short_address) return;
                    rt_slot_t *rt_slot = (rt_slot_t *)(&rti->dev_inst->rxbuf[frame_idx]);
                    if(rt_slot->slot_map & ((slot_map_t)1 << rti->slot_idx)){
                        rti->nodes[tdma_slot->idx].accepted = true;
                        printf("0x%x has been accepted in slot: %d\n", ieee_std_frame_hdr->src_address, rti->slot_idx);
                        /* Add me to node list */
                        if(node_all_accepted(rti)){
                            node_add(rti, rti->slot_idx, rti->dev_inst->my_short_address);
                            rti->nodes[rti->slot_idx].accepted = true;
                            printf("I has been accepted in slot: %d/%llx\n", rti->slot_idx, rt_slot->slot_map);
                        };
                    }
                }
                break;
                case RT_LOCA_MSG:
                {
                    rt_loca_t *rt_loca = (rt_loca_t *)(&rti->dev_inst->rxbuf[frame_idx]);
                    rti->nodes[tdma_slot->idx].location_x = rt_loca->location_x;
                    rti->nodes[tdma_slot->idx].location_y = rt_loca->location_y;
                    rti->nodes[tdma_slot->idx].location_z = rt_loca->location_z;
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

    /* If I did have slot, I would listen for slot request */
    if(node_all_accepted(rti)){
        if(rtls_tdma_slot_listen(rti, UWB_BLOCKING, tdma_slot->idx)){
            ieee_std_frame_hdr_t *ieee_std_frame_hdr = (ieee_std_frame_hdr_t *)rti->dev_inst->rxbuf;
            if(ieee_std_frame_hdr->PANID != rti->dev_inst->pan_id) return;
            if(ieee_std_frame_hdr->dst_address != rti->dev_inst->my_short_address) return;

            if(rti->dev_inst->rxbuf_size - sizeof(ieee_std_frame_hdr_t) < sizeof(rt_slot_t)) return;

            rt_slot_t *rt_slot = (rt_slot_t *)(rti->dev_inst->rxbuf + sizeof(ieee_std_frame_hdr_t));

            /* Only receive first RT_REQT_MSG each svc slot */
            if(rti->slot_req == 0 && rt_slot->msg_type == RT_REQT_MSG){
                printf("Recv slot reqt: 0x%llx\n", rti->slot_req);
                rti->slot_req = rt_slot->slot_map;
                rti->slot_req_addr = ieee_std_frame_hdr->src_address;
            }
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
        rt_slot->slot = tdma_slot->idx;

        rt_slot->slot_map = (slot_map_t)-1;
        if(rti->role == RTR_ANCHOR){
            for(int i=1; i<=MYNEWT_VAL(UWB_BCN_SLOT_MAX); i++){
                /* Ignore if node is not available in this frame and my slot node*/
                if(rti->nodes[i].addr == 0 || !rti->nodes[i].available) continue; 
                rt_slot->slot_map &= ~rti->nodes[i].slot_map;
            }
            for(int i=1; i<=MYNEWT_VAL(UWB_BCN_SLOT_MAX); i++){
                slot_map_t slot = ((slot_map_t)1 << i);
                if(rt_slot->slot_map & slot){
                    rt_slot->slot_map = slot;
                    rti->slot_idx = i;
                    break;
                }
            }
        }
        else if(rti->role == RTR_TAG){
            for(int i=MYNEWT_VAL(UWB_BCN_SLOT_MAX) + 1; i<MYNEWT_VAL(TDMA_NSLOTS); i++){
                /* Ignore if node is not available in this frame and my slot node*/
                if(!rti->nodes[i].available) continue; 
                rt_slot->slot_map &= ~rti->nodes[i].slot_map;
            }
            for(int i=MYNEWT_VAL(UWB_BCN_SLOT_MAX) + 1; i<MYNEWT_VAL(TDMA_NSLOTS); i++){
                slot_map_t slot = ((slot_map_t)1 << i);
                if(rt_slot->slot_map & slot){
                    rt_slot->slot_map = slot;
                    rti->slot_idx = i;
                    break;
                }
            }
        }
        printf("RQT slot: 0x%llx/%d\n", rt_slot->slot_map, rti->slot_idx);

        for(int i=0; i<=MYNEWT_VAL(UWB_BCN_SLOT_MAX); i++){
            if(rti->nodes[i].addr != 0 && !rti->nodes[i].accepted && rti->nodes[i].available){
                ieee_std_frame_hdr->dst_address = rti->nodes[i].addr;
                UWB_TX(rti, rt_msg, rt_msg_size, tdma_slot->idx);
            }
        }
        free(rt_msg);
    }
}