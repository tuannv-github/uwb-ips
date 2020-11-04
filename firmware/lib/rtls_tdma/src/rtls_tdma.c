#include <dpl/dpl.h>

#include <rtls_tdma/rtls_tdma.h>

static void rst_joint_list_cb(tdma_slot_t *tdma_slot);
static void rst_joint_reqt_cb(tdma_slot_t *tdma_slot);
static void rst_joint_jted_cb(tdma_slot_t *tdma_slot);

static void
node_slot_map_printf(rtls_tdma_instance_t *rti){
    printf("- My slot: %d\n", rti->my_slot);
    for(int i=0; i<MYNEWT_VAL(TDMA_NSLOTS); i++){
        if(rti->nodes[rti->my_slot].slot & ((uint64_t)1 << i)){
            printf("--- Slot: %02d, addr: 0x%02x, map:", i, rti->nodes[i].addr);
            for(int j=0; j<MYNEWT_VAL(TDMA_NSLOTS); j++){
                printf("%d", (uint8_t)((rti->nodes[i].slot >> j) & 0x01));
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
        rti->nodes[i].slot = 0x01;
    }
}

static void 
node_rmv(rtls_tdma_instance_t *rti, uint16_t idx){
    rti->nodes[rti->my_slot].slot &= ~((uint64_t)1 << idx);
    memset(&rti->nodes[idx], 0, sizeof(rtls_tdma_node_t));

    /* the first slot is reserved for synchronization */
    rti->nodes[idx].slot = 0x01;
}

static void 
node_add(rtls_tdma_instance_t *rti, uint16_t idx, uint16_t addr){
    rti->nodes[rti->my_slot].slot |= ((uint64_t)1 << idx);
    rti->nodes[idx].addr = addr;
}

static void
slot_cb(struct dpl_event * ev)
{
    tdma_slot_t *tdma_slot = (tdma_slot_t *)dpl_event_get_arg(ev);
    if(tdma_slot->idx == 0) return;

    switch (((rtls_tdma_instance_t *)tdma_slot->arg)->cstate)
    {
    case RTS_JOINT_LIST:
        rst_joint_list_cb(tdma_slot);
        break;
    case RTS_JOINT_REQT:
        rst_joint_reqt_cb(tdma_slot);
        break;
    case RTS_JOINT_JTED:
        rst_joint_jted_cb(tdma_slot);
        break;
    case RTS_JOINT_NONE:
        /* It must be a bug if slot_cb() is call when out of sync */
        break;
    default:
        break;
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
        break;
    case CCP_SYNC_SYED:
        printf("CCP_SYNC_SYED\n");
        if(rti->tdma->ccp->config.role == CCP_ROLE_MASTER){
            rti->my_slot = MYNEWT_VAL(RT_MASTER_SLOT);
            
            /* Add me to nodes list */
            node_add(rti, rti->my_slot, rti->dev_inst->my_short_address);

            rti->cstate = RTS_JOINT_JTED;
            printf("state: RTS_JOINT_JTED\n");
        }
        else{
            rti->cstate = RTS_JOINT_NONE;
            printf("state: RTS_JOINT_LIST\n");
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
    /* Joint request timeout */
    rtls_tdma_instance_t *rti = (rtls_tdma_instance_t *)cbs->inst_ptr;
    if(rti->joint_reqt){
        rti->joint_reqt_cnt++;
        if(rti->cstate == RTS_JOINT_REQT && rti->joint_reqt_cnt > MYNEWT_VAL(RT_BCN_REQT_THRESH)){
            rti->cstate = RTS_JOINT_LIST;
            printf("joint_reqt_timeout\nrti->cstate:RTS_JOINT_LIST\n");
        }
    }

    /* Node timeout*/
    for(int i=0; i<MYNEWT_VAL(TDMA_NSLOTS); i++){
        if(rti->nodes[rti->my_slot].slot & ((slot_type_t)1 << i)) {
            rti->nodes[i].timeout++;
            if(rti->nodes[i].timeout > MYNEWT_VAL(RT_ANCHOR_TIMEOUT)){
                printf("Anchor timeout: %d\n", i);
                if(rti->cstate == RTS_JOINT_JTED){
                    rti->nodes[rti->my_slot].slot &= ~((slot_type_t)1 << i);
                }
            }
        }
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
    rti->umi.superframe_cb = superframe_cb,

    uwb_mac_append_interface(rti->dev_inst, &rti->umi);

    rtls_ccp_set_sync_cb(rti->tdma->ccp, uwb_ccp_sync_cb, rti);
    rtls_ccp_start(rti->tdma->ccp);

    for (uint16_t i = 0; i < MYNEWT_VAL(TDMA_NSLOTS); i++){
        tdma_assign_slot(rti->tdma, slot_cb,  i, rti);
    }
}

void rst_joint_list_cb(tdma_slot_t *tdma_slot){

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
        if(rti->dev_inst->fctrl != UWB_FCTRL_STD_DATA_FRAME) return;

        /* Reset anchor timout */
        rti->nodes[tdma_slot->idx].timeout = 0;

        ieee_std_frame_hdr_t *ieee_std_frame_hdr = (ieee_std_frame_hdr_t *)rti->dev_inst->rxbuf;
        if(ieee_std_frame_hdr->PANID != rti->dev_inst->pan_id) return;
        if(ieee_std_frame_hdr->dst_address != RT_BROADCAST_ADDR) return;

        /* Continue process only if there is at least sizeof(_msg_hdr_t) in frame payload */
        if(rti->dev_inst->rxbuf_size - sizeof(ieee_std_frame_hdr_t) < sizeof(msg_hdr_t)) return;
        uint16_t frame_idx = sizeof(ieee_std_frame_hdr_t);

        while(frame_idx < rti->dev_inst->rxbuf_size){

            if(ieee_std_frame_hdr->src_address != rti->nodes[tdma_slot->idx].addr){
                rti->nodes[tdma_slot->idx].addr = ieee_std_frame_hdr->src_address;
                rti->nodes[tdma_slot->idx].bcn_cnt = 0;
            }

            switch(rti->dev_inst->rxbuf[frame_idx]){
                case RT_LOCA_MSG:
                {
                    rt_loca_t *rt_bcn_norm_payload = (rt_loca_t *)(&rti->dev_inst->rxbuf[frame_idx]);

                    rti->nodes[tdma_slot->idx].location_x = rt_bcn_norm_payload->location_x;
                    rti->nodes[tdma_slot->idx].location_y = rt_bcn_norm_payload->location_y;
                    rti->nodes[tdma_slot->idx].location_z = rt_bcn_norm_payload->location_z;
                }
                break;
                case RT_SLOT_MSG:
                {
                    rt_slot_t *rt_slot = (rt_slot_t *)(&rti->dev_inst->rxbuf[frame_idx]);

                    rti->nodes[tdma_slot->idx].slot = rt_slot->slot;
                    rti->nodes[tdma_slot->idx].bcn_cnt++;

                    bool next_state = true;
                    for(int i=0; i<=MYNEWT_VAL(UWB_BCN_SLOT_MAX); i++){
                        if(rti->nodes[i].addr != 0 && rti->nodes[i].bcn_cnt < MYNEWT_VAL(RT_JOINT_LIST_THRESH)){
                            next_state = false;
                        }
                    }

                    if(next_state){
                        node_slot_map_printf(rti);
                        rti->joint_reqt_cnt = 0;
                        rti->joint_reqt = true;
                        rti->cstate = RTS_JOINT_REQT;
                        printf("state: RTS_JOINT_REQT\n");
                    }
                }
                break;
                default:
                break;
            }
            uint8_t msg_len = rti->dev_inst->rxbuf[frame_idx+1];
            frame_idx += 2 + msg_len;
        }
    }
}

void rst_joint_reqt_cb(tdma_slot_t *tdma_slot){
    rtls_tdma_instance_t *rti = (rtls_tdma_instance_t *)tdma_slot->arg;
    assert(rti);

    if(tdma_slot->idx == MYNEWT_VAL(UWB_SVC_SLOT)){
        if(rti->joint_reqt_cnt > MYNEWT_VAL(RT_BCN_REQT_THRESH)){
            rti->joint_reqt = false;
            rti->cstate = RTS_JOINT_LIST;
            printf("state: RTS_JOINT_LIST\n");
            return;
        }

        uint16_t rt_msg_size = sizeof(ieee_std_frame_hdr_t) + sizeof(rt_slot_t);;
        uint8_t *rt_msg = calloc(1, rt_msg_size);

        ieee_std_frame_hdr_t *ieee_std_frame_hdr = (ieee_std_frame_hdr_t *)rt_msg;
        rt_slot_t *rt_slot = (rt_slot_t *)(rt_msg + sizeof(ieee_std_frame_hdr_t));

        ieee_std_frame_hdr->fctrl = UWB_FCTRL_STD_DATA_FRAME;
        ieee_std_frame_hdr->seq_num = rti->seqno++;
        ieee_std_frame_hdr->PANID = rti->tdma->dev_inst->pan_id;
        ieee_std_frame_hdr->dst_address = RT_BROADCAST_ADDR;
        ieee_std_frame_hdr->src_address = rti->tdma->dev_inst->my_short_address;

        rt_slot->msg_type = RT_REQT_MSG;
        rt_slot->len = sizeof(struct _rt_slot_data_t);
        rti->joint_reqt_slot = 0;

        if(rti->role == RTR_ANCHOR){
            for(int i=0; i<=MYNEWT_VAL(UWB_BCN_SLOT_MAX); i++){
                if(rti->nodes[i].addr == 0) continue;
                rti->joint_reqt_slot |= ~rti->nodes[i].slot;
            }
            for(int i=0; i<=MYNEWT_VAL(UWB_BCN_SLOT_MAX); i++){
                uint16_t slot = (0x0001 << i);
                if(rti->joint_reqt_slot & slot){
                    rti->joint_reqt_slot    = slot;
                    rt_slot->slot           = slot;
                    break;
                }
            }
        }
        else if(rti->role == RTR_TAG){
            for(int i=MYNEWT_VAL(UWB_BCN_SLOT_MAX) + 1; i<MYNEWT_VAL(TDMA_NSLOTS); i++){
                if(rti->nodes[i].addr == 0) continue;
                rti->joint_reqt_slot |= ~rti->nodes[i].slot;
            }
            for(int i=MYNEWT_VAL(UWB_BCN_SLOT_MAX) + 1; i<MYNEWT_VAL(TDMA_NSLOTS); i++){
                uint16_t slot = (0x0001 << i);
                if(rti->joint_reqt_slot & slot){
                    rti->joint_reqt_slot    = slot;
                    rt_slot->slot           = slot;
                    break;
                }
            }
        }
        else{
            printf("No RTLS TDMA role configrued\n");
            rti->cstate = RTS_JOINT_LIST;
            return;
        }

        if(rti->joint_reqt_slot == 0){
            printf("No slot available\n");
            rti->cstate = RTS_JOINT_LIST;
            return;
        }

        for(int i=0; i<=MYNEWT_VAL(UWB_BCN_SLOT_MAX); i++){
            if(rti->nodes[i].addr == 0) continue;
        }

        uwb_write_tx(rti->dev_inst, rt_msg, 0, rt_msg_size);
        uwb_write_tx_fctrl(rti->dev_inst, rt_msg_size, 0);
        uwb_set_wait4resp(rti->dev_inst, false);

        uint64_t dx_time = tdma_tx_slot_start(rti->tdma, tdma_slot->idx) & 0xFFFFFFFFFE00UL;
        uwb_set_delay_start(rti->dev_inst, dx_time);
        if (uwb_start_tx(rti->dev_inst).start_tx_error) {
            printf("rst_joint_reqt_cb TX error\n");
        }
    }
    else if (tdma_slot->idx <= MYNEWT_VAL(UWB_BCN_SLOT_MAX)){

        uwb_set_rx_timeout(rti->dev_inst, 3*rti->tdma->ccp->period/rti->tdma->nslots/4);
        uwb_set_delay_start(rti->dev_inst, tdma_rx_slot_start(rti->tdma, tdma_slot->idx));
        if(rtls_tdma_listen(rti, UWB_BLOCKING).start_rx_error){
            printf("start_rx_error: %s:%d\n", __FILE__, __LINE__);
        }
        else{
            if(rti->dev_inst->status.rx_timeout_error) return;

            rti->nodes[tdma_slot->idx].timeout = 0;

            ieee_std_frame_hdr_t *ieee_std_frame_hdr = (ieee_std_frame_hdr_t *)rti->dev_inst->rxbuf;
            if(ieee_std_frame_hdr->PANID != rti->dev_inst->pan_id) return;
            if(ieee_std_frame_hdr->dst_address != rti->dev_inst->my_short_address) return;
            if(rti->nodes[tdma_slot->idx].addr != ieee_std_frame_hdr->src_address){
                rti->cstate = RTS_JOINT_LIST;
                return;
            }

            if(rti->dev_inst->rxbuf_size - sizeof(ieee_std_frame_hdr_t) < sizeof(msg_hdr_t)) return;
            uint16_t frame_idx = sizeof(ieee_std_frame_hdr_t);
            while(frame_idx < rti->dev_inst->rxbuf_size){
                switch(rti->dev_inst->rxbuf[frame_idx]){
                    case RT_ACPT_MSG:
                    {
                        rt_slot_t *rt_slot = (rt_slot_t *)(&rti->dev_inst->rxbuf[frame_idx]);
                        rti->joint_reqt_slot &= rt_slot->slot;

                        bool all_accepted = true;
                        for(int i=0; i<=MYNEWT_VAL(UWB_BCN_SLOT_MAX); i++){
                            if(rti->nodes[i].addr != 0 && !rti->nodes[i].accepted){
                                all_accepted = false;
                                break;
                            }
                        }

                        if(all_accepted && rti->joint_reqt_slot != 0){
                            rti->cstate = RTS_JOINT_JTED;
                            for(rti->my_slot = 0; rti->my_slot<=MYNEWT_VAL(UWB_BCN_SLOT_MAX); rti->my_slot++){
                                if(rti->joint_reqt_slot & (0x0001 << rti->my_slot)) break;
                            }
                            printf("jointed: slot:%d\n", rti->my_slot);
                            rti->nodes[rti->my_slot].addr = rti->dev_inst->my_short_address;
                            rti->nodes[rti->my_slot].slot |= 0x0001 << rti->my_slot;
                        }
                    }
                    break;
                }
                uint8_t msg_len = rti->dev_inst->rxbuf[frame_idx+1];
                frame_idx += msg_len;
            }
        }
    }
}

void rst_joint_jted_cb(tdma_slot_t *tdma_slot){

    rtls_tdma_instance_t *rti = (rtls_tdma_instance_t *)tdma_slot->arg;

    if(tdma_slot->idx <= MYNEWT_VAL(UWB_BCN_SLOT_MAX)){
        if(tdma_slot->idx == rti->my_slot){
            
            /* Reset anchor timout */
            rti->anchors[tdma_slot->idx].timeout = 0;

            if(rti->joint_reqt_cnt > MYNEWT_VAL(RT_BCN_REQT_THRESH)){
                rti->joint_reqt_cnt = 0;
                rti->joint_reqt = false;
            }

            uint8_t *rt_msg;
            uint16_t rt_msg_size;

            ieee_std_frame_hdr_t *ieee_std_frame_hdr;

            if(rti->joint_reqt){

                rt_msg_size = sizeof(ieee_std_frame_hdr_t) + sizeof(rt_slot_t);
                rt_msg = calloc(1, rt_msg_size);
                if(!rt_msg) return;

                rt_slot_t *rt_slot = (rt_slot_t *)(rt_msg + sizeof(ieee_std_frame_hdr_t));
                rt_slot->msg_type = RT_ACPT_MSG;
                rt_slot->len = sizeof(struct _rt_slot_data_t);
                rt_slot->slot = (~rti->anchors[rti->my_slot].slot) & rti->joint_reqt_slot;

                if(rt_slot->slot){
                    rti->anchors[rti->my_slot].slot |= rt_slot->slot;
                }

                ieee_std_frame_hdr = (ieee_std_frame_hdr_t *)rt_msg;
                ieee_std_frame_hdr->dst_address = rti->joint_reqt_src;   
            }
            else{
                if(rti->seqno % 2){
                    rt_msg_size = sizeof(ieee_std_frame_hdr_t) + sizeof(rt_loca_t);
                    rt_msg = calloc(1, rt_msg_size);
                    if(!rt_msg) return;

                    rt_loca_t *rt_bcn_norm_payload = (rt_loca_t *)(rt_msg + sizeof(ieee_std_frame_hdr_t));
                    rt_bcn_norm_payload->msg_type = RT_LOCA_MSG;
                    rt_bcn_norm_payload->len = sizeof(struct _rt_loca_data_t);
                    rt_bcn_norm_payload->location_x = rti->anchors[rti->my_slot].location_x;
                    rt_bcn_norm_payload->location_y = rti->anchors[rti->my_slot].location_y;
                    rt_bcn_norm_payload->location_z = rti->anchors[rti->my_slot].location_z;
                }
                else{
                    rt_msg_size = sizeof(ieee_std_frame_hdr_t) + sizeof(rt_slot_t);
                    rt_msg = calloc(1, rt_msg_size);
                    if(!rt_msg) return;

                    rt_slot_t *rt_slot = (rt_slot_t *)(rt_msg + sizeof(ieee_std_frame_hdr_t));
                    rt_slot->msg_type = RT_SLOT_MSG;
                    rt_slot->len = sizeof(struct _rt_slot_data_t);
                    rt_slot->slot = rti->anchors[rti->my_slot].slot;
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
        else{

            uwb_set_rx_timeout(rti->dev_inst, 3*rti->tdma->ccp->period/rti->tdma->nslots/4);
            uwb_set_delay_start(rti->dev_inst, tdma_rx_slot_start(rti->tdma, tdma_slot->idx));
            if(rtls_tdma_listen(rti, UWB_BLOCKING).start_rx_error){
                printf("start_rx_error: %s:%d\n", __FILE__, __LINE__);
            }
            else{
                if(rti->dev_inst->status.rx_timeout_error) return;

                /* Check if it is a beacon message from correct anchor */
                ieee_std_frame_hdr_t *ieee_std_frame_hdr = (ieee_std_frame_hdr_t *)rti->dev_inst->rxbuf;
                if(ieee_std_frame_hdr->src_address != rti->nodes[tdma_slot->idx].addr) return;
                if(ieee_std_frame_hdr->dst_address != RT_BROADCAST_ADDR) return;
                if(ieee_std_frame_hdr->PANID != rti->dev_inst->pan_id) return;

                /* Reset anchor timout */
                rti->nodes[tdma_slot->idx].timeout = 0;

                if(rti->dev_inst->rxbuf_size - sizeof(ieee_std_frame_hdr_t) < sizeof(msg_hdr_t)) return;
                uint16_t frame_idx = sizeof(ieee_std_frame_hdr_t);

                if(rti->joint_reqt && rti->joint_reqt_src == ieee_std_frame_hdr->src_address){
                    rti->joint_reqt_cnt = 0;
                    rti->joint_reqt = false;
                }

                while(frame_idx < rti->dev_inst->rxbuf_size){

                    switch(rti->dev_inst->rxbuf[frame_idx]){
                        case RT_LOCA_MSG:
                        {
                            rt_loca_t *rt_bcn_norm_payload = (rt_loca_t *)(&rti->dev_inst->rxbuf[frame_idx]);

                            rti->nodes[tdma_slot->idx].location_x = rt_bcn_norm_payload->location_x;
                            rti->nodes[tdma_slot->idx].location_y = rt_bcn_norm_payload->location_y;
                            rti->nodes[tdma_slot->idx].location_z = rt_bcn_norm_payload->location_z;
                        }
                        break;
                        case RT_SLOT_MSG:
                        {
                            rt_slot_t *rt_slot = (rt_slot_t *)(&rti->dev_inst->rxbuf[frame_idx]);

                            rti->nodes[tdma_slot->idx].slot = rt_slot->slot;
                            rti->nodes[tdma_slot->idx].listen_cnt++;
                        }
                        break;
                    }
                    uint8_t msg_len = rti->dev_inst->rxbuf[frame_idx+1];
                    frame_idx += msg_len;
                }
            }
        }
    }
    else if(tdma_slot->idx == MYNEWT_VAL(UWB_SVC_SLOT)){
        uwb_set_rx_timeout(rti->dev_inst, 3*rti->tdma->ccp->period/rti->tdma->nslots/4);
        uwb_set_delay_start(rti->dev_inst, tdma_rx_slot_start(rti->tdma, tdma_slot->idx));
        if(rtls_tdma_listen(rti, UWB_BLOCKING).start_rx_error){
            printf("start_rx_error: %s:%d\n", __FILE__, __LINE__);
        }
        else{
            if(rti->dev_inst->status.rx_timeout_error) return;
            if(rti->joint_reqt) return;

            ieee_std_frame_hdr_t *ieee_std_frame_hdr = (ieee_std_frame_hdr_t *)rti->dev_inst->rxbuf;
            if(ieee_std_frame_hdr->PANID != rti->dev_inst->pan_id) return;
            if(ieee_std_frame_hdr->dst_address != RT_BROADCAST_ADDR) return;

            if(rti->dev_inst->rxbuf_size - sizeof(ieee_std_frame_hdr_t) < sizeof(msg_hdr_t)) return;
            uint16_t frame_idx = sizeof(ieee_std_frame_hdr_t);
            while(frame_idx < rti->dev_inst->rxbuf_size){
                switch(rti->dev_inst->rxbuf[frame_idx]){
                    case RT_REQT_MSG:
                    {
                        rt_slot_t *rt_slot = (rt_slot_t *)(&rti->dev_inst->rxbuf[frame_idx]);
                        rti->joint_reqt = true;
                        rti->joint_reqt_src = ieee_std_frame_hdr->src_address;
                        rti->joint_reqt_slot = rt_slot->slot;
                        rti->joint_reqt_cnt = 0;
                    }
                    break;
                }
                uint8_t msg_len = rti->dev_inst->rxbuf[frame_idx+1];
                frame_idx += msg_len;
            }
        }
    }
    else{
        if(rti->rtls_tdma_cb != NULL){
            rti->rtls_tdma_cb(rti, tdma_slot);
        }
    }
}