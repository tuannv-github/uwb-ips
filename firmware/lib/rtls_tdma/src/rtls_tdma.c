#include <dpl/dpl.h>
#include <stats/stats.h>

#include <rtls_tdma/rtls_tdma.h>

STATS_SECT_START(rtls_tdma_stat_t)
    STATS_SECT_ENTRY(start_rx_error)
STATS_SECT_END

STATS_SECT_DECL(rtls_tdma_stat_t) g_rtls_tdma_stat;

STATS_NAME_START(rtls_tdma_stat_t)
    STATS_NAME(rtls_tdma_stat_t, start_rx_error)
STATS_NAME_END(rtls_tdma_stat_t)

static void bcn_slot_cb_mine(tdma_slot_t *tdma_slot);
static void bcn_slot_cb_othr(tdma_slot_t *tdma_slot);
static void svc_slot_cb(tdma_slot_t *tdma_slot);

static uint16_t current_ranging_slot = 0;

static bool
rx_complete_cb(struct uwb_dev * inst, struct uwb_mac_interface * cbs){
    rtls_tdma_instance_t *rti = (rtls_tdma_instance_t *)cbs->inst_ptr;

    if(inst->fctrl == FCNTL_IEEE_RANGE_16){
        if(current_ranging_slot != 0){
            nrng_request_frame_t * _frame = (nrng_request_frame_t * )inst->rxbuf;
            if(_frame->src_address == rti->nodes[current_ranging_slot].addr){
                rti->nodes[current_ranging_slot].timeout = 0;
            }
        }
    }

    if (dpl_sem_get_count(&rti->sem) == 0){
        dpl_error_t err = dpl_sem_release(&rti->sem);
        assert(err == DPL_OK);
        return true;
    }
    return false;
}

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

static bool 
rtls_tdma_slot_listen(rtls_tdma_instance_t *rti, uint16_t idx)
{
    uwb_set_rx_timeout(rti->dev_inst, 3*rti->tdma->ccp->period/rti->tdma->nslots/4);
    uwb_set_delay_start(rti->dev_inst, tdma_rx_slot_start(rti->tdma, idx));

    dpl_error_t err = dpl_sem_pend(&rti->sem,  DPL_TIMEOUT_NEVER);
    assert(err == DPL_OK);

    if(uwb_start_rx(rti->dev_inst).start_rx_error){
        err = dpl_sem_release(&rti->sem);
        assert(err == DPL_OK);
    }

    err = dpl_sem_pend(&rti->sem, DPL_TIMEOUT_NEVER);
    assert(err == DPL_OK);
    err = dpl_sem_release(&rti->sem);
    assert(err == DPL_OK);

    return rti->dev_inst->status.rx_timeout_error == 0 ? true : false;
}

static void
node_slot_map_printf(rtls_tdma_instance_t *rti){
    printf("- My slot: %d\n", rti->slot_idx);
    for(int i=1; i<MYNEWT_VAL(TDMA_NSLOTS); i++){
        if(rti->nodes[i].addr!=0){
            printf("--- Slot: %02d, addr: 0x%04x, accepted: %d, map:", i, rti->nodes[i].addr, rti->nodes[i].accepted);
            for(int j=0; j<MYNEWT_VAL(TDMA_NSLOTS); j++){
                printf("%d", (uint8_t)((rti->nodes[i].slot_map >> j) & 0x01));
            }
            printf(" \n");
        }
    }
}

static void 
node_rmv(rtls_tdma_instance_t *rti, uint16_t idx){
    /* Remove node from slot map */
    rti->nodes[rti->slot_idx].slot_map &= ~((uint64_t)1 << idx);

    /* Clear node's data from node table */
    memset(&rti->nodes[idx], 0, sizeof(rtls_tdma_node_t));
    /* The first slot is reserved for synchronization */
    rti->nodes[idx].slot_map = 0x01;
}

static void 
node_add_me(rtls_tdma_instance_t *rti, uint16_t idx){
    /* Change my index */
    rti->slot_idx = idx;
    rti->dev_inst->slot_id = idx;
    /* Copy slot map from temp slot 0 to new slot */ 
    rti->nodes[idx].slot_map = rti->nodes[0].slot_map | ((slot_map_t)0x01 << idx);
    /* Update address */
    rti->nodes[idx].addr = rti->dev_inst->my_short_address;
    /* I always accept mysefl*/
    rti->nodes[idx].accepted = true;
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
    rtls_tdma_instance_t *rti = (rtls_tdma_instance_t *)tdma_slot->arg;
    /* Slot 0 is used for synchronization */
    if(tdma_slot->idx == 0) return;

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
                if(tdma_slot->idx == rti->slot_idx) rti->nodes[rti->slot_idx].timeout = 0;
                current_ranging_slot = tdma_slot->idx;
                rti->rtls_tdma_cb(rti, tdma_slot);
                current_ranging_slot = 0;
            }
        }
    }
}

static void 
uwb_ccp_sync_cb(ccp_sync_t ccp_sync, void *arg){
    rtls_tdma_instance_t *rti = (rtls_tdma_instance_t *)arg;

    /* My slot idx return to 0 when sync is lost */
    rti->slot_idx = 0;

    /* Remove nodes from slot map */
    rti->nodes[rti->slot_idx].slot_map = 0x01;

    /* Clear node's data from node table */
    memset(rti->nodes, 0, MYNEWT_VAL(TDMA_NSLOTS) * sizeof(rtls_tdma_node_t));
    /* The first slot is reserved for synchronization */
    for(int i=0; i<MYNEWT_VAL(TDMA_NSLOTS); i++){
        rti->nodes[i].slot_map = 0x01;
    }

    switch (ccp_sync)
    {
    case CCP_SYNC_LOST:
        printf("CCP_SYNC_LOST\n");
        break;
    case CCP_SYNC_SYED:
        printf("CCP_SYNC_SYED\n");
        if(rti->tdma->ccp->config.role == CCP_ROLE_MASTER){
            /* Add me to nodes list */
            node_add_me(rti, MYNEWT_VAL(RT_MASTER_SLOT));
            node_slot_map_printf(rti);
        }
        break;
    default:
        break;
    }
}

static bool
superframe_cb(struct uwb_dev * inst, struct uwb_mac_interface * cbs)
{
    rtls_tdma_instance_t *rti = (rtls_tdma_instance_t *)cbs->inst_ptr;
    /* Check timeout and availability of ANCHOR only */
    for(int i=1; i<MYNEWT_VAL(TDMA_NSLOTS); i++){
        rti->nodes[i].available = false;
        if(rti->nodes[i].addr != 0){
            rti->nodes[i].timeout++;
            if(rti->nodes[i].timeout > MYNEWT_VAL(RT_ANCHOR_TIMEOUT)){
                node_rmv(rti, i);
                printf("Remove node: %d\n", i);
                node_slot_map_printf(rti);

                /* Anchor may down when trying to get a slot */
                if(rti->slot_reqt != 0 && node_all_accepted(rti) && rti->role == RTR_TAG){
                    node_add_me(rti, rti->slot_reqt);
                    printf("All node has accepted in slot: %d\n", rti->slot_reqt);
                    node_slot_map_printf(rti);
                    /* Reset slot_reqt to change it purpose */
                    // rti->slot_reqt = 0;
                };
            }
        }
    }

    if(rti->slot_reqt){
        rti->slot_reqt_cntr++;
    }
    return false;
}

void 
rtls_tdma_start(rtls_tdma_instance_t *rti, struct uwb_dev* udev){
    int rc;

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

    if(rti->role == RTR_TAG){
        rtls_ccp_start_role(rti->tdma->ccp, CCP_ROLE_SLAVE);
    }
    else if(rti->role == RTR_ANCHOR){
        rtls_ccp_start_role(rti->tdma->ccp, CCP_ROLE_MASTER | CCP_ROLE_SLAVE | CCP_ROLE_RELAY);
    }
    else{
        printf("Undefined RTR role\n");
    }

    for (uint16_t i = 0; i < MYNEWT_VAL(TDMA_NSLOTS); i++){
        tdma_assign_slot(rti->tdma, slot_cb,  i, rti);
    }

    rc = stats_init(
        STATS_HDR(g_rtls_tdma_stat),
        STATS_SIZE_INIT_PARMS(g_rtls_tdma_stat, STATS_SIZE_32),
        STATS_NAME_INIT_PARMS(rtls_tdma_stat_t));
    assert(rc == 0);

    rc = stats_register("rtls_tdma", STATS_HDR(g_rtls_tdma_stat));
    assert(rc == 0);
}

static void
bcn_slot_cb_mine(tdma_slot_t *tdma_slot){

    uint8_t *rt_msg;
    uint16_t rt_msg_size;
    ieee_std_frame_hdr_t *ieee_std_frame_hdr;
    rtls_tdma_instance_t *rti = (rtls_tdma_instance_t *)tdma_slot->arg;

    /* I do have slot, so I always available in this supper frame, also reset node timer */
    rti->nodes[rti->slot_idx].available = true;
    rti->nodes[tdma_slot->idx].timeout = 0;

    /* If I received a slot request, I will response for this request immediately */
    if(rti->slot_reqt){

        rt_msg_size = sizeof(ieee_std_frame_hdr_t) + sizeof(rt_slot_acpt_t) + sizeof(rt_slot_map_t);
        rt_msg = calloc(1, rt_msg_size);
        if(!rt_msg) return;

        rt_slot_acpt_t *rt_slot_acpt = (rt_slot_acpt_t *)(rt_msg + sizeof(ieee_std_frame_hdr_t));
        rt_slot_acpt->msg_type = RT_ACPT_MSG;
        rt_slot_acpt->len = sizeof(rt_slot_acpt_data_t);
        rt_slot_acpt->slot = tdma_slot->idx;
        rt_slot_acpt->slot_reqt = ((~rti->nodes[rti->slot_idx].slot_map) & ((slot_map_t)0x01 << rti->slot_reqt)) ? rti->slot_reqt : 0;

        /* I can now receive new request if I do not accept this request */
        if(rt_slot_acpt->slot_reqt == 0) {
            printf("I do not accept 0x%04X for slot %d\n", rti->slot_reqt_addr, rti->slot_reqt);
            rti->slot_reqt = 0;
        }

        ieee_std_frame_hdr = (ieee_std_frame_hdr_t *)rt_msg;
        ieee_std_frame_hdr->dst_address = rti->slot_reqt_addr;

        rt_slot_map_t *rt_slot_map = (rt_slot_map_t *)(rt_msg + sizeof(rt_slot_reqt_t) + sizeof(ieee_std_frame_hdr_t));
        rt_slot_map->msg_type = RT_SLOT_MSG;
        rt_slot_map->len = sizeof(rt_slot_map_data_t);
        rt_slot_map->slot = tdma_slot->idx;
        rt_slot_map->slot_map = rti->nodes[rti->slot_idx].slot_map;
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
            rt_loca->location_x = rti->x;
            rt_loca->location_y = rti->y;
            rt_loca->location_z = rti->z;
        }
        else{
            rt_msg_size = sizeof(ieee_std_frame_hdr_t) + sizeof(rt_slot_map_t);
            rt_msg = calloc(1, rt_msg_size);
            if(!rt_msg) return;

            rt_slot_map_t *rt_slot_map = (rt_slot_map_t *)(rt_msg + sizeof(ieee_std_frame_hdr_t));
            rt_slot_map->msg_type = RT_SLOT_MSG;
            rt_slot_map->len = sizeof(rt_slot_map_data_t);
            rt_slot_map->slot = tdma_slot->idx;
            rt_slot_map->slot_map = rti->nodes[rti->slot_idx].slot_map;
        }

        ieee_std_frame_hdr = (ieee_std_frame_hdr_t *)rt_msg;
        ieee_std_frame_hdr->dst_address = RT_BROADCAST_ADDR;
    }
    
    ieee_std_frame_hdr->fctrl = UWB_FCTRL_STD_DATA_FRAME;
    ieee_std_frame_hdr->seq_num = rti->seqno++;
    ieee_std_frame_hdr->PANID = rti->tdma->dev_inst->pan_id;
    ieee_std_frame_hdr->src_address = rti->tdma->dev_inst->my_short_address;
    
    UWB_TX(rti, rt_msg, rt_msg_size, tdma_slot->idx);

    if(rti->slot_reqt) {
        printf("I accept 0x%04X for slot %d\n", rti->slot_reqt_addr, rti->slot_reqt);
        /* If I accept for a RANGE slot, I do not wait for see its BCN message */
        if(rti->slot_reqt > MYNEWT_VAL(UWB_SVC_SLOT)){
            rti->nodes[rti->slot_idx].slot_map |= (slot_map_t)0x01 << rti->slot_reqt;
            rti->nodes[rti->slot_reqt].addr = rti->slot_reqt_addr;
            /* Release request context if it is not BCN slot */
            rti->slot_reqt = 0;
            node_slot_map_printf(rti);
        }
    }

    if(rt_msg) free(rt_msg);
}

static void
bcn_slot_cb_othr(tdma_slot_t *tdma_slot){
    rtls_tdma_instance_t *rti = (rtls_tdma_instance_t *)tdma_slot->arg;
    if(rtls_tdma_slot_listen(rti, tdma_slot->idx)){

        ieee_std_frame_hdr_t *ieee_std_frame_hdr = (ieee_std_frame_hdr_t *)rti->dev_inst->rxbuf;
        if(ieee_std_frame_hdr->PANID != rti->dev_inst->pan_id) return;
        
        /* Strange node detected */
        if(ieee_std_frame_hdr->src_address != rti->nodes[tdma_slot->idx].addr){
            /* I do not have see any node in this bcn slot before, it is a new node for this slot */
            if(rti->nodes[tdma_slot->idx].addr == 0){
                /* I am a network member */
                if(rti->slot_idx != 0){
                    /* I see a slot request from this node before. It is up now */
                    if(rti->slot_reqt != 0 && rti->slot_reqt_addr == ieee_std_frame_hdr->src_address){
                        rti->nodes[tdma_slot->idx].addr = ieee_std_frame_hdr->src_address;
                        printf("Node up: 0x%04X\n", ieee_std_frame_hdr->src_address);
                        /* This have request thi slot before, it must acctept my slot before joining */
                        rti->nodes[tdma_slot->idx].accepted = true;
                        /* Release request context */
                        rti->slot_reqt = 0;
                    }
                    /* I am a tag, and I see a new anchor */
                    else if(rti->role == RTR_TAG){
                        printf("I am a tag and I see new anchor!\n");
                        rti->nodes[tdma_slot->idx].addr = ieee_std_frame_hdr->src_address;
                        rti->nodes[tdma_slot->idx].accepted = false;
                    }
                    /* I am a anchor */
                    else{
                        /* I has joint the network and I do not see any request from this node before */
                        /* This node take this slot without permission */
                        printf("Unknown node: 0x%04X at slot %d\n", ieee_std_frame_hdr->src_address, tdma_slot->idx);
                        rti->nodes[tdma_slot->idx].addr = ieee_std_frame_hdr->src_address;
                        rti->nodes[tdma_slot->idx].accepted = true;
                        return;
                    }
                }
                /* I am not a network member, just update this slot in table */
                else{
                    rti->nodes[tdma_slot->idx].addr = ieee_std_frame_hdr->src_address;
                    node_slot_map_printf(rti);
                }
            }else{
                printf("Slot colission: 0x%04X at slot %d\n", ieee_std_frame_hdr->src_address, tdma_slot->idx);
                return;
            }
        }

        /* If I received message from tabled node. Mark it available in this supperframe and reset timer */
        rti->nodes[tdma_slot->idx].available = true;
        rti->nodes[tdma_slot->idx].timeout = 0;
        rti->nodes[rti->slot_idx].slot_map |= (slot_map_t)1 << tdma_slot->idx;

        if(rti->dev_inst->rxbuf_size - sizeof(ieee_std_frame_hdr_t) < sizeof(msg_hdr_t)) return;
        uint16_t frame_idx = sizeof(ieee_std_frame_hdr_t);
        while(frame_idx < rti->dev_inst->rxbuf_size){
            switch(rti->dev_inst->rxbuf[frame_idx]){
                case RT_ACPT_MSG:
                {
                    if(ieee_std_frame_hdr->dst_address != rti->dev_inst->my_short_address) return;
                    rt_slot_acpt_t *rt_slot_acpt = (rt_slot_acpt_t *)(&rti->dev_inst->rxbuf[frame_idx]);
                    if(rt_slot_acpt->slot_reqt  == rti->slot_reqt){
                        rti->nodes[tdma_slot->idx].accepted = true;
                        printf("0x%x accepted me in slot: %d\n", ieee_std_frame_hdr->src_address, rti->slot_reqt);
                        /* Add me to node list */
                        if(node_all_accepted(rti)){
                            node_add_me(rti, rti->slot_reqt);
                            printf("All node has accepted in slot: %d\n", rti->slot_reqt);
                            node_slot_map_printf(rti);
                            /* Reset slot_reqt to change it purpose */
                            rti->slot_reqt = 0;
                        };
                    }
                    else{
                        printf("0x%04X do not accept for slot %d\n", ieee_std_frame_hdr->src_address, rti->slot_reqt);
                    }
                }
                break;
                case RT_LOCA_MSG:
                {
                    rt_loca_t *rt_loca = (rt_loca_t *)(&rti->dev_inst->rxbuf[frame_idx]);
                    rti->nodes[tdma_slot->idx].location_x = rt_loca->location_x;
                    rti->nodes[tdma_slot->idx].location_y = rt_loca->location_y;
                    rti->nodes[tdma_slot->idx].location_z = rt_loca->location_z;
                }
                break;
                case RT_SLOT_MSG:
                {
                    rt_slot_map_t *rt_slot = (rt_slot_map_t *)(&rti->dev_inst->rxbuf[frame_idx]);
                    rti->nodes[tdma_slot->idx].slot_map = rt_slot->slot_map;
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

    /* If I did have slot */
    if(node_all_accepted(rti)){
        /* I have been waiting for requested node to up too long. It must request again */
        if(rti->slot_reqt && rti->slot_reqt_cntr > MYNEWT_VAL(RT_SLOT_REQT_TIMEOUT)){
            printf("Node 0x%04X request slot %d accept timeout\n", rti->slot_reqt_addr, rti->slot_reqt);
            /* Release request context */
            rti->slot_reqt = 0;
        }

        /* I would listen for slot request */
        if(rtls_tdma_slot_listen(rti, tdma_slot->idx) && rti->role == RTR_ANCHOR){
            ieee_std_frame_hdr_t *ieee_std_frame_hdr = (ieee_std_frame_hdr_t *)rti->dev_inst->rxbuf;
            if(ieee_std_frame_hdr->PANID != rti->dev_inst->pan_id) return;
            if(ieee_std_frame_hdr->dst_address != RT_BROADCAST_ADDR) return;

            if(rti->dev_inst->rxbuf_size - sizeof(ieee_std_frame_hdr_t) < sizeof(rt_slot_reqt_t)) return;

            rt_slot_reqt_t *rt_slot_reqt = (rt_slot_reqt_t *)(rti->dev_inst->rxbuf + sizeof(ieee_std_frame_hdr_t));

            /* Only receive one RT_REQT_MSG for a session */
            if(rti->slot_reqt == 0 && rt_slot_reqt->msg_type == RT_REQT_MSG){
                /* Lock request context */
                rti->slot_reqt = rt_slot_reqt->slot_reqt;
                rti->slot_reqt_cntr = 0;
                rti->slot_reqt_addr = ieee_std_frame_hdr->src_address;
                printf("Request slot %d from 0x%04X\n", rti->slot_reqt, ieee_std_frame_hdr->src_address);
            }
        }
    }
    /* My slot is not accepted by all node mean I have to send more request */
    else{
        uint16_t rt_msg_size = sizeof(ieee_std_frame_hdr_t) + sizeof(rt_slot_reqt_t);
        uint8_t *rt_msg = calloc(1, rt_msg_size);
        if(!rt_msg) return;

        ieee_std_frame_hdr_t *ieee_std_frame_hdr = (ieee_std_frame_hdr_t *)rt_msg;
        rt_slot_reqt_t *rt_slot_reqt = (rt_slot_reqt_t *)(rt_msg + sizeof(ieee_std_frame_hdr_t));

        ieee_std_frame_hdr->fctrl = UWB_FCTRL_STD_DATA_FRAME;
        ieee_std_frame_hdr->seq_num = rti->seqno++;
        ieee_std_frame_hdr->PANID = rti->tdma->dev_inst->pan_id;
        ieee_std_frame_hdr->src_address = rti->tdma->dev_inst->my_short_address;

        rt_slot_reqt->msg_type = RT_REQT_MSG;
        rt_slot_reqt->len = sizeof(rt_slot_reqt_data_t);
        rt_slot_reqt->slot = tdma_slot->idx;

        slot_map_t slot_map = (slot_map_t)-1;

        /* Find ANCHOR slot */
        if(rti->role == RTR_ANCHOR){
            for(int i=1; i<=MYNEWT_VAL(UWB_BCN_SLOT_MAX); i++){
                /* Ignore if node is not available in this frame and my slot node*/
                if(rti->nodes[i].addr == 0 || !rti->nodes[i].available) continue; 
                slot_map &= ~rti->nodes[i].slot_map;
            }
            for(int i=1; i<=MYNEWT_VAL(UWB_BCN_SLOT_MAX); i++){
                if(slot_map & ((slot_map_t)1 << i)){
                    rt_slot_reqt->slot_reqt = rti->slot_reqt = i;
                    break;
                }
            }
        }
        /* Find TAG slot */
        else if(rti->role == RTR_TAG){
            for(int i=1; i<=MYNEWT_VAL(UWB_BCN_SLOT_MAX); i++){
                /* Ignore if node is not available in this frame and my slot node*/
                if(rti->nodes[i].addr == 0 || !rti->nodes[i].available) continue; 
                slot_map &= ~rti->nodes[i].slot_map;
            }
            for(int i=MYNEWT_VAL(UWB_SVC_SLOT) + 1; i<MYNEWT_VAL(TDMA_NSLOTS); i++){
                if(slot_map & ((slot_map_t)1 << i)){
                    rt_slot_reqt->slot_reqt = rti->slot_reqt = i;
                    break;
                }
            }
        }

        /* Slot 1 is automatically assigned for MASTER */
        /* rt_slot_reqt->slot_reqt = 1 mean I no anchor available */
        if(rt_slot_reqt->slot_reqt > 1){
            ieee_std_frame_hdr->dst_address = RT_BROADCAST_ADDR;
            UWB_TX(rti, rt_msg, rt_msg_size, tdma_slot->idx);
            printf("I (0x%04X) request slot: %d\n", ieee_std_frame_hdr->src_address, rt_slot_reqt->slot_reqt);
        }

        free(rt_msg);
    }
}

void rtls_tdma_find_node(rtls_tdma_instance_t *rti, uint16_t addr, rtls_tdma_node_t **node){
    for(int i =0; i<MYNEWT_VAL(TDMA_NSLOTS); i++){
        if(rti->nodes[i].addr == addr){
            *node= &rti->nodes[i];
            break;
        }
    }
}