#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <bsp/bsp.h>
#include <dpl/dpl_os.h>
#include <hal/hal_gpio.h>
#include <dpl/dpl_cputime.h>

#include <uwb/uwb.h>
#include <uwb/uwb_ftypes.h>
#include <rtls_ccp/rtls_ccp.h>

#if MYNEWT_VAL(UWB_CCP_STATS)
#include <stats/stats.h>
STATS_NAME_START(uwb_ccp_stat_section)
    STATS_NAME(uwb_ccp_stat_section, master_cnt)
    STATS_NAME(uwb_ccp_stat_section, slave_cnt)
    STATS_NAME(uwb_ccp_stat_section, send)
    STATS_NAME(uwb_ccp_stat_section, listen)
    STATS_NAME(uwb_ccp_stat_section, tx_complete)
    STATS_NAME(uwb_ccp_stat_section, rx_complete)
    STATS_NAME(uwb_ccp_stat_section, rx_relayed)
    STATS_NAME(uwb_ccp_stat_section, rx_start_error)
    STATS_NAME(uwb_ccp_stat_section, rx_unsolicited)
    STATS_NAME(uwb_ccp_stat_section, rx_other_frame)
    STATS_NAME(uwb_ccp_stat_section, txrx_error)
    STATS_NAME(uwb_ccp_stat_section, tx_start_error)
    STATS_NAME(uwb_ccp_stat_section, tx_relay_error)
    STATS_NAME(uwb_ccp_stat_section, tx_relay_ok)
    STATS_NAME(uwb_ccp_stat_section, irq_latency)
    STATS_NAME(uwb_ccp_stat_section, os_lat_behind)
    STATS_NAME(uwb_ccp_stat_section, os_lat_margin)
    STATS_NAME(uwb_ccp_stat_section, rx_timeout)
    STATS_NAME(uwb_ccp_stat_section, sem_timeout)
    STATS_NAME(uwb_ccp_stat_section, reset)
    STATS_NAME(uwb_ccp_stat_section, see_another_master)
STATS_NAME_END(uwb_ccp_stat_section)

#define CCP_STATS_INC(__X) STATS_INC(ccp->stat, __X)
#define CCP_STATS_SET(__X, __N) {STATS_CLEAR(ccp->stat, __X);STATS_INCN(ccp->stat, __X, __N);}
#else
#define CCP_STATS_INC(__X) {}
#define CCP_STATS_SET(__X, __N) {}
#endif

static bool rx_complete_cb(struct uwb_dev * inst, struct uwb_mac_interface * cbs);
static bool tx_complete_cb(struct uwb_dev * inst, struct uwb_mac_interface * cbs);
static bool rx_timeout_cb(struct uwb_dev * inst, struct uwb_mac_interface * cbs);
static bool error_cb(struct uwb_dev * inst, struct uwb_mac_interface * cbs);
static bool reset_cb(struct uwb_dev * inst, struct uwb_mac_interface * cbs);

static struct uwb_ccp_status ccp_send(struct uwb_ccp_instance * ccp);
static struct uwb_ccp_status ccp_listen(struct uwb_ccp_instance * ccp, uint64_t dx_time);

static void ccp_master_timer_ev_cb(struct dpl_event *ev);
static void ccp_slave_timer_ev_cb(struct dpl_event *ev);
static void uwb_ccp_change_role(struct uwb_ccp_instance *ccp);

static void
ccp_master_timer_ev_cb(struct dpl_event *ev)
{
    int rc;
    assert(ev != NULL);
    assert(dpl_event_get_arg(ev));
    // printf("ccp_master_timer_ev_cb\n");

    struct uwb_ccp_instance * ccp = (struct uwb_ccp_instance *) dpl_event_get_arg(ev);

    if (!ccp->status.enabled) return;

    CCP_STATS_INC(master_cnt);
    ccp->status.timer_restarted = 0;

    if (ccp_send(ccp).start_tx_error) {
        // CCP failed to send, probably because os_latency wasn't enough
        // margin to get in to prep the frame for sending.
        // NOTE that os_epoch etc will be updated in ccp_send if it fails
        // to send so timer update below will still point to next beacon time */
        printf("start_tx_error\n");
        if (!ccp->status.enabled) return;
    }

    if (!ccp->status.timer_restarted && ccp->status.enabled) {
        rc = dpl_cputime_timer_start(&ccp->master_slave_timer, 
              ccp->os_epoch
            + dpl_cputime_usecs_to_ticks(uwb_dwt_usecs_to_usecs(ccp->period))
            - dpl_cputime_usecs_to_ticks(MYNEWT_VAL(OS_LATENCY))
        );
        if (rc == 0) ccp->status.timer_restarted = 1;
    }
    return;
}

static void
ccp_slave_timer_ev_cb(struct dpl_event *ev)
{
    int rc;
    uint16_t timeout;
    uint64_t dx_time = 0;
    uint32_t timer_expiry;
    assert(ev != NULL);
    assert(dpl_event_get_arg(ev));
    static uint16_t rx_timeout_acc;
    // printf("ccp_slave_timer_ev_cb\n");

    struct uwb_ccp_instance * ccp = (struct uwb_ccp_instance *) dpl_event_get_arg(ev);
    struct uwb_dev * inst = ccp->dev_inst;

    if (!ccp->status.enabled) {
        return;
    }

    /* Sync lost since earlier, just set a long rx timeout and
     * keep listening */
    if (ccp->status.rx_timeout_error) {
        printf("rx_timeout_error\n");
        uwb_set_rx_timeout(inst, MYNEWT_VAL(UWB_CCP_LONG_RX_TO));
        ccp_listen(ccp, 0);

        /* Timout accumulate */
        rx_timeout_acc++;
        printf("ccp->rx_timeout_acc: %d\n", rx_timeout_acc);

        /* May change to master role if rx_timeout_acc reach thresh */
        if(rx_timeout_acc > MYNEWT_VAL(UWB_RX_TIMEOUT_THRESH) + ccp->dev_inst->euid%MYNEWT_VAL(UWB_RX_TIMEOUT_THRESH)){
            rx_timeout_acc = 0;
            /* I only try to change to master mode if master mode is allowed in config */
            if(ccp->uwb_ccp_role & CCP_ROLE_MASTER){
                printf("Thresh reached: change CCP_ROLE_MASTER\n");
                ccp->config.role = CCP_ROLE_MASTER;
            }
            else{
                printf("Thresh reached: change CCP_ROLE_SLAVE\n");
                ccp->config.role = CCP_ROLE_SLAVE;
            }
            if(dpl_sem_get_count(&ccp->sem) == 0){
                dpl_sem_release(&ccp->sem);
            }
            uwb_ccp_change_role(ccp);
            return;
        }
        goto reset_timer;
    }

    CCP_STATS_INC(slave_cnt);

    /* Calculate when to expect the ccp packet */
    dx_time = ccp->local_epoch;
    dx_time += ((uint64_t)ccp->period << 16);
    dx_time -= ((uint64_t)ceilf(uwb_usecs_to_dwt_usecs(uwb_phy_SHR_duration(inst) +
                                                       inst->config.rx.timeToRxStable)) << 16);

    timeout = ccp->blink_frame_duration + MYNEWT_VAL(XTALT_GUARD);
    /* Adjust timeout if we're using cascading ccp in anchors */
    timeout += (ccp->config.tx_holdoff_dly + ccp->blink_frame_duration) * MYNEWT_VAL(UWB_CCP_MAX_CASCADE_RPTS);

    uwb_set_rx_timeout(inst, timeout);
    ccp_listen(ccp, dx_time);
    if(ccp->status.start_rx_error){
        /* Sync lost, set a long rx timeout */
        uwb_set_rx_timeout(inst, MYNEWT_VAL(UWB_CCP_LONG_RX_TO));
        ccp_listen(ccp, 0);
        printf("start_rx_error: ccp_listen\n");
    }

reset_timer:
    /* Check if we're still enabled */
    if (!ccp->status.enabled) {
        return;
    }

    /* Ensure timer isn't active */
    dpl_cputime_timer_stop(&ccp->master_slave_timer);

    if (ccp->status.rx_timeout_error) {
        /* No ccp received, reschedule immediately */
        printf(" No ccp received, reschedule immediately\n");
        dpl_sem_pend(&ccp->sem, DPL_TIMEOUT_NEVER);
        rc = dpl_cputime_timer_relative(&ccp->master_slave_timer, 0);
        dpl_sem_release(&ccp->sem);
    } else {
        /* Reset rx_timeout_acc each time received blink message */
        rx_timeout_acc = 0;
        /* ccp received ok, or close enough - reschedule for next ccp event */
        ccp->status.rx_timeout_error = 0;
        timer_expiry = ccp->os_epoch + dpl_cputime_usecs_to_ticks(
            - MYNEWT_VAL(OS_LATENCY)
            + (uint32_t)uwb_dwt_usecs_to_usecs(ccp->period)
            - ccp->blink_frame_duration - inst->config.rx.timeToRxStable);
        rc = dpl_cputime_timer_start(&ccp->master_slave_timer, timer_expiry);
    }
    if (rc == 0) ccp->status.timer_restarted = 1;
}

static void
adjust_for_epoch_to_rm(struct uwb_ccp_instance * ccp, uint16_t epoch_to_rm_us)
{
    ccp->master_epoch.timestamp -= ((uint64_t)epoch_to_rm_us << 16);
    ccp->local_epoch -= ((uint64_t)epoch_to_rm_us << 16);
    ccp->os_epoch -= dpl_cputime_usecs_to_ticks(uwb_dwt_usecs_to_usecs(epoch_to_rm_us));
}

static bool
rx_complete_cb(struct uwb_dev * inst, struct uwb_mac_interface * cbs)
{
    struct uwb_ccp_instance *ccp = (struct uwb_ccp_instance *)cbs->inst_ptr;

    /* CCP filter function control */
    if (inst->fctrl != FCNTL_IEEE_BLINK_CCP_64){
        if(dpl_sem_get_count(&ccp->sem) == 0){
            // We're hunting for a ccp but received something else,
            // set a long timeout and keep listening
            uwb_adj_rx_timeout(inst, MYNEWT_VAL(UWB_CCP_LONG_RX_TO));
            CCP_STATS_INC(rx_other_frame);
            return true;
        }
        return true;
    }

    /* CCP filter if able to process packet */
    uwb_ccp_frame_t *frame = ccp->frames[(ccp->idx+1)%ccp->nframes];  // speculative frame advance
    if(inst->frame_len != sizeof(uwb_ccp_blink_frame_t)) return true;
    memcpy(frame->array, inst->rxbuf, sizeof(uwb_ccp_blink_frame_t));

    /* CCP filter for master role requesting */
    if (ccp->config.role & CCP_ROLE_MASTER) {
        /* Master only receive ccp packet when master requesting */
        if(ccp->master_role_request) {
            if(frame->euid < ccp->dev_inst->euid){
                ccp->master_role_request = false;
            }
        }
        else{
            // if(frame->euid < ccp->dev_inst->euid){
                ccp->config.role = CCP_ROLE_SLAVE;
                dpl_eventq_put(&ccp->eventq, &ccp->change_role_event);
                CCP_STATS_INC(see_another_master); 
            // }
        }
        goto exit;
    }

    /* CCP filter for slave with strange master */
    else if(ccp->config.role & CCP_ROLE_SLAVE){
        if(ccp->my_master == 0){
            ccp->my_master = frame->euid;
        }
        else if(ccp->my_master != frame->euid){
            ccp->my_master = frame->euid;
            ccp->config.role = CCP_ROLE_SLAVE;
            dpl_eventq_put(&ccp->eventq, &ccp->change_role_event);
            CCP_STATS_INC(see_another_master);
            goto exit;
        }
    }

    if(dpl_sem_get_count(&ccp->sem) != 0){
        //unsolicited inbound
        CCP_STATS_INC(rx_unsolicited);
        return false;
    }

    if (inst->status.lde_error){
        return true;
    }

    #if MYNEWT_VAL(UWB_CCP_TEST_CASCADE_RPTS)
    if (frame->rpt_count < MYNEWT_VAL(UWB_CCP_TEST_CASCADE_RPTS)) {
        return true;
    }
    #endif
    /* A good ccp packet has been received, stop the receiver */
    uwb_stop_rx(inst); //Prevent timeout event

    ccp->idx++; // confirmed frame advance
    ccp->seq_num = frame->seq_num;

    /* Read os_time and correct for interrupt latency */
    uint32_t delta_0 = 0xffffffffU&(uwb_read_systime_lo32(inst) - (uint32_t)(inst->rxtimestamp&0xFFFFFFFFUL));
    ccp->os_epoch = dpl_cputime_get32();
    uint32_t delta_1 = 0xffffffffU&(uwb_read_systime_lo32(inst) - (uint32_t)(inst->rxtimestamp&0xFFFFFFFFUL));
    uint32_t delta = (delta_0>>1) + (delta_1>>1);
    ccp->os_epoch -= dpl_cputime_usecs_to_ticks(uwb_dwt_usecs_to_usecs(delta>>16));
    CCP_STATS_SET(irq_latency, uwb_dwt_usecs_to_usecs(delta>>16));

    CCP_STATS_INC(rx_complete);
    ccp->status.rx_timeout_error = 0;

    if (frame->transmission_timestamp.timestamp < ccp->master_epoch.timestamp ||
        frame->euid != ccp->master_euid) {
        ccp->master_euid = frame->euid;
        ccp->status.valid = (MYNEWT_VAL(UWB_CCP_VALID_THRESHOLD)==0);
        ccp->status.valid_count = 0;
    }else{
        if(ccp->status.valid_count < MYNEWT_VAL(UWB_CCP_VALID_THRESHOLD)-1){
            ccp->status.valid_count++;
        }
        ccp->status.valid |= (ccp->status.valid_count == MYNEWT_VAL(UWB_CCP_VALID_THRESHOLD)-1);
    }

    ccp->master_epoch.timestamp = frame->transmission_timestamp.timestamp;
    ccp->local_epoch = frame->reception_timestamp = inst->rxtimestamp;
    ccp->period = (frame->transmission_interval >> 16);

    /* Adjust for delay between epoch and rmarker */
    adjust_for_epoch_to_rm(ccp, frame->epoch_to_rm_us);

    frame->carrier_integrator = inst->carrier_integrator;
    if (inst->config.rxttcko_enable) {
        frame->rxttcko = inst->rxttcko;
    } else {
        frame->rxttcko = 0;
    }

    /* Compensate if not receiving the master ccp packet directly */
    if (frame->rpt_count != 0) {
        CCP_STATS_INC(rx_relayed);
        /* Assume ccp intervals are a multiple of 0x10000 dwt usec -> 0x100000000 dwunits */
        uint64_t master_interval = ((frame->transmission_interval/0x100000000UL+1)*0x100000000UL);
        ccp->period = master_interval>>16;
        uint64_t repeat_dly = master_interval - frame->transmission_interval;
        ccp->master_epoch.timestamp = (ccp->master_epoch.timestamp - repeat_dly);
        ccp->local_epoch = (ccp->local_epoch - repeat_dly) & 0x0FFFFFFFFFFUL;
        frame->reception_timestamp = ccp->local_epoch;
        /* master_interval and transmission_interval are expressed as dwt_usecs */
        ccp->os_epoch -= dpl_cputime_usecs_to_ticks(uwb_dwt_usecs_to_usecs((repeat_dly >> 16)));
        /* Carrier integrator is only valid if direct from the master */
        frame->carrier_integrator = 0;
        frame->rxttcko = 0;
    }

    /* Cascade relay of ccp packet if relay role is configured */
    if ((ccp->uwb_ccp_role & CCP_ROLE_RELAY) && ccp->status.valid && frame->rpt_count < frame->rpt_max) {

        uwb_ccp_frame_t tx_frame;
        memcpy(tx_frame.array, frame->array, sizeof(uwb_ccp_frame_t));

        /* Only replace the short id, retain the euid to know which master this originates from */
        tx_frame.short_address = inst->my_short_address;
        tx_frame.rpt_count++;
        uint64_t tx_timestamp = frame->reception_timestamp;
        tx_timestamp += tx_frame.rpt_count*((uint64_t)ccp->config.tx_holdoff_dly<<16);

        /* Shift frames so as to reduce risk of frames corrupting each other */
        tx_timestamp += (inst->slot_id%4)*(((uint64_t)ccp->blink_frame_duration)<<16);
        tx_timestamp &= 0x0FFFFFFFE00UL;
        uwb_set_delay_start(inst, tx_timestamp);

        /* Need to add antenna delay */
        tx_timestamp += inst->tx_antenna_delay;

        /* Calculate the transmission time of our packet in the masters reference */
        uint64_t tx_delay = (tx_timestamp - frame->reception_timestamp);
        tx_frame.transmission_timestamp.timestamp += tx_delay;

        /* Adjust the transmission interval so listening units can calculate the
         * original master's timestamp */
        tx_frame.transmission_interval = frame->transmission_interval - tx_delay;

        uwb_write_tx(inst, tx_frame.array, 0, sizeof(uwb_ccp_blink_frame_t));
        uwb_write_tx_fctrl(inst, sizeof(uwb_ccp_blink_frame_t), 0);
        ccp->status.start_tx_error = uwb_start_tx(inst).start_tx_error;
        if (ccp->status.start_tx_error){
            CCP_STATS_INC(tx_relay_error);
        } else {
            CCP_STATS_INC(tx_relay_ok);
        }
    }

    /* Call all available superframe callbacks */
    struct uwb_mac_interface * lcbs = NULL;
    if(!(SLIST_EMPTY(&inst->interface_cbs))) {
        SLIST_FOREACH(lcbs, &inst->interface_cbs, next) {
            if (lcbs != NULL && lcbs->superframe_cb) {
                if(lcbs->superframe_cb((struct uwb_dev*)inst, lcbs)) continue;
            }
        }
    }

    if (ccp->config.postprocess && ccp->status.valid) {
        dpl_eventq_put(&ccp->eventq, &ccp->postprocess_event);
    }

exit:
    dpl_sem_release(&ccp->sem);
    return false;
}

static bool
tx_complete_cb(struct uwb_dev * inst, struct uwb_mac_interface * cbs)
{
    struct uwb_ccp_instance * ccp = (struct uwb_ccp_instance *)cbs->inst_ptr;
    if(dpl_sem_get_count(&ccp->sem) == 1)
        return false;

    CCP_STATS_INC(tx_complete);
    if (ccp->config.role != CCP_ROLE_MASTER)
        return false;

    uwb_ccp_frame_t * frame = ccp->frames[(++ccp->idx)%ccp->nframes];

    /* Read os_time and correct for interrupt latency */
    uint32_t delta_0 = 0xffffffffU&(uwb_read_systime_lo32(inst) - frame->transmission_timestamp.lo);
    ccp->os_epoch = dpl_cputime_get32();
    uint32_t delta_1 = 0xffffffffU&(uwb_read_systime_lo32(inst) - frame->transmission_timestamp.lo);
    uint32_t delta = (delta_0>>1) + (delta_1>>1);
    ccp->os_epoch -= dpl_cputime_usecs_to_ticks(uwb_dwt_usecs_to_usecs(delta>>16));
    CCP_STATS_SET(irq_latency, uwb_dwt_usecs_to_usecs(delta>>16));

    ccp->local_epoch = frame->transmission_timestamp.lo;
    ccp->master_epoch = frame->transmission_timestamp;
    ccp->period = (frame->transmission_interval >> 16);

    /* Adjust for delay between epoch and rmarker */
    adjust_for_epoch_to_rm(ccp, frame->epoch_to_rm_us);

    ccp->status.valid |= ccp->idx > 1;

    /* Call all available superframe callbacks */
    struct uwb_mac_interface * lcbs = NULL;
    if(!(SLIST_EMPTY(&inst->interface_cbs))) {
        SLIST_FOREACH(lcbs, &inst->interface_cbs, next) {
            if (lcbs != NULL && lcbs->superframe_cb) {
                if(lcbs->superframe_cb((struct uwb_dev*)inst, lcbs)) continue;
            }
        }
    }

    if (ccp->config.postprocess && ccp->status.valid)
        dpl_eventq_put(&ccp->eventq, &ccp->postprocess_event);

    if(dpl_sem_get_count(&ccp->sem) == 0){
        dpl_error_t err = dpl_sem_release(&ccp->sem);
        assert(err == DPL_OK);
    }

    return false;
}

static bool
rx_timeout_cb(struct uwb_dev * inst, struct uwb_mac_interface * cbs)
{
    struct uwb_ccp_instance *ccp = (struct uwb_ccp_instance *)cbs->inst_ptr;
    if (dpl_sem_get_count(&ccp->sem) == 0){
        ccp->status.rx_timeout_error = 1;
        dpl_error_t err = dpl_sem_release(&ccp->sem);
        assert(err == DPL_OK);
        CCP_STATS_INC(rx_timeout);
    }
    return false;
}

static bool
error_cb(struct uwb_dev * inst, struct uwb_mac_interface * cbs)
{
    struct uwb_ccp_instance *ccp = (struct uwb_ccp_instance *)cbs->inst_ptr;
    if(dpl_sem_get_count(&ccp->sem) == 0) {
        if (ccp->config.role != CCP_ROLE_MASTER) {
            ccp->status.rx_error = 1;
        }
        dpl_error_t err = dpl_sem_release(&ccp->sem);
        assert(err == DPL_OK);
        CCP_STATS_INC(txrx_error);
        return true;
    }
    return false;
}

static bool
reset_cb(struct uwb_dev *inst, struct uwb_mac_interface * cbs)
{
    struct uwb_ccp_instance *ccp = (struct uwb_ccp_instance *)cbs->inst_ptr;
    if(dpl_sem_get_count(&ccp->sem) == 0){
        dpl_error_t err = dpl_sem_release(&ccp->sem);
        assert(err == DPL_OK);
        CCP_STATS_INC(reset);
        return true;
    }
    return false;
}

static struct uwb_ccp_status
ccp_send(struct uwb_ccp_instance *ccp)
{
    assert(ccp);
    struct uwb_dev * inst = ccp->dev_inst;
    CCP_STATS_INC(send);
    uwb_phy_forcetrxoff(inst);
    dpl_error_t err = dpl_sem_pend(&ccp->sem, DPL_TIMEOUT_NEVER);
    assert(err == DPL_OK);

    uwb_ccp_frame_t * previous_frame = ccp->frames[(uint16_t)(ccp->idx)%ccp->nframes];
    uwb_ccp_frame_t * frame = ccp->frames[(ccp->idx+1)%ccp->nframes];
    frame->rpt_count = 0;
    frame->rpt_max = MYNEWT_VAL(UWB_CCP_MAX_CASCADE_RPTS);
    frame->epoch_to_rm_us = uwb_phy_SHR_duration(inst);

    uint64_t timestamp = previous_frame->transmission_timestamp.timestamp
                        + ((uint64_t)ccp->period << 16);

    timestamp = timestamp & 0xFFFFFFFFFFFFFE00ULL; /* Mask off the last 9 bits */
    uwb_set_delay_start(inst, timestamp);
    timestamp += inst->tx_antenna_delay;
    frame->transmission_timestamp.timestamp = timestamp;

    frame->seq_num = ++ccp->seq_num;
    frame->euid = inst->euid;
    frame->short_address = inst->my_short_address;
    frame->transmission_interval = ((uint64_t)ccp->period << 16);

    uwb_write_tx(inst, frame->array, 0, sizeof(uwb_ccp_blink_frame_t));
    uwb_write_tx_fctrl(inst, sizeof(uwb_ccp_blink_frame_t), 0);
    uwb_set_wait4resp(inst, false);
    ccp->status.start_tx_error = uwb_start_tx(inst).start_tx_error;
    if (ccp->status.start_tx_error) {
        uint64_t systime = uwb_read_systime(inst);
        uint64_t late_us = ((systime - timestamp)&UWB_DTU_40BMASK) >> 16;
        CCP_STATS_INC(tx_start_error);
        previous_frame->transmission_timestamp.timestamp = (frame->transmission_timestamp.timestamp
                        + ((uint64_t)ccp->period << 16));
        ccp->idx++;

        /* Handle missed transmission and update epochs as tx_complete would have done otherwise */
        ccp->os_epoch += dpl_cputime_usecs_to_ticks(uwb_dwt_usecs_to_usecs(ccp->period - late_us));
        ccp->os_epoch -= dpl_cputime_usecs_to_ticks(MYNEWT_VAL(OS_LATENCY));
        ccp->master_epoch.timestamp += ((uint64_t)ccp->period)<<16;
        ccp->local_epoch += ((uint64_t)ccp->period)<<16;
        err = dpl_sem_release(&ccp->sem);
        assert(err == DPL_OK);

    }

    #if MYNEWT_VAL(UWB_CCP_STATS)
    uint32_t margin = 0xffffffffU&(frame->transmission_timestamp.lo - uwb_read_systime_lo32(inst));
    CCP_STATS_SET(os_lat_margin, uwb_dwt_usecs_to_usecs(margin>>16));
    #endif
    err = dpl_sem_pend(&ccp->sem, DPL_TIMEOUT_NEVER); // Wait for completion of transactions
    assert(err == DPL_OK);
    err = dpl_sem_release(&ccp->sem);
    assert(err == DPL_OK);

    return ccp->status;
}

static struct uwb_ccp_status
ccp_listen(struct uwb_ccp_instance *ccp, uint64_t dx_time)
{

    struct uwb_dev *inst = ccp->dev_inst;
    uwb_phy_forcetrxoff(inst);

    dpl_error_t err = dpl_sem_pend(&ccp->sem,  DPL_TIMEOUT_NEVER);
    assert(err == DPL_OK);

    CCP_STATS_INC(listen);

    if (dx_time) {
        uwb_set_delay_start(inst, dx_time);
    }

    ccp->status.rx_timeout_error = 0;
    ccp->status.start_rx_error = uwb_start_rx(inst).start_rx_error;
    if (ccp->status.start_rx_error) {
        printf("start_rx_error\n");
        #if MYNEWT_VAL(UWB_CCP_STATS)
        uint32_t behind = 0xffffffffU&(uwb_read_systime_lo32(inst) - dx_time);
        CCP_STATS_SET(os_lat_behind, uwb_dwt_usecs_to_usecs(behind>>16));
        #endif
        CCP_STATS_INC(rx_start_error);
        err = dpl_sem_release(&ccp->sem);
        assert(err == DPL_OK);
        return ccp->status;
    }

    #if MYNEWT_VAL(UWB_CCP_STATS)
    if (dx_time) {
        uint32_t margin = 0xffffffffU&(dx_time - uwb_read_systime_lo32(inst));
        CCP_STATS_SET(os_lat_margin, uwb_dwt_usecs_to_usecs(margin>>16));
    }
    #endif
    /* Wait for completion of transactions */
    dpl_sem_pend(&ccp->sem, DPL_TIMEOUT_NEVER);
    if(dpl_sem_get_count(&ccp->sem) == 0){
        err = dpl_sem_release(&ccp->sem);
        assert(err == DPL_OK);
    }
    return ccp->status;
}

static void
uwb_ccp_start(struct uwb_ccp_instance *ccp, uwb_ccp_role_t role)
{
    assert(ccp);
    struct uwb_dev * inst = ccp->dev_inst;
    uint16_t epoch_to_rm = uwb_phy_SHR_duration(inst);

    #if MYNEWT_VAL(RTLS_CPP_VERBOSE)
    // Initialise frame timestamp to current time
    printf("{\"utime\": %lu,\"msg\": \"uwb_ccp_start\"}\n",
            dpl_cputime_ticks_to_usecs(dpl_cputime_get32()));
    #endif

    ccp->idx = 0x0;
    ccp->status.valid = false;
    ccp->master_euid = 0x0;
    ccp->status.enabled = 1;
    ccp->config.role = role;
    if(role == CCP_ROLE_SLAVE){
        ccp->my_master = 0;
        printf("CCP_ROLE_SLAVE\n");
    }
    else if(role == CCP_ROLE_MASTER){
        printf("CCP_ROLE_MASTER\n");
    }

    uwb_ccp_frame_t *frame = ccp->frames[(ccp->idx)%ccp->nframes];

    /* Setup CCP to send/listen for the first packet ASAP */
    ccp->os_epoch = dpl_cputime_get32() - epoch_to_rm;
    uint64_t ts = (uwb_read_systime(inst) - (((uint64_t)ccp->period)<<16))&UWB_DTU_40BMASK;
    ts += ((uint64_t)ccp->config.tx_holdoff_dly + 2 * MYNEWT_VAL(OS_LATENCY))<<16;

    if (role == CCP_ROLE_MASTER){
        ccp->local_epoch = frame->transmission_timestamp.lo = ts;
        frame->transmission_timestamp.hi = 0;
    } else {
        ccp->local_epoch = frame->reception_timestamp = ts;
    }
    ccp->local_epoch -= epoch_to_rm;
    ccp->local_epoch &= UWB_DTU_40BMASK;

    ccp->status.timer_enabled = true;
    ccp->blink_frame_duration = uwb_phy_frame_duration(inst, sizeof(uwb_ccp_blink_frame_t));

    if (role == CCP_ROLE_MASTER){
        dpl_event_init(&ccp->master_slave_event, ccp_master_timer_ev_cb, (void *) ccp);
    } else {
        dpl_event_init(&ccp->master_slave_event, ccp_slave_timer_ev_cb, (void *) ccp);
    }
    dpl_cputime_timer_relative(&ccp->master_slave_timer, 0);
}

static void
uwb_ccp_stop(struct uwb_ccp_instance *ccp){
    ccp->status.enabled = 0;
    dpl_cputime_timer_stop(&ccp->master_slave_timer);
    if(dpl_sem_get_count(&ccp->sem) == 0){
        uwb_phy_forcetrxoff(ccp->dev_inst);
        dpl_error_t err = dpl_sem_release(&ccp->sem);
        assert(err == DPL_OK);
    }
}

static void
uwb_ccp_change_role(struct uwb_ccp_instance *ccp){
    struct uwb_dev *inst = ccp->dev_inst;
    uwb_ccp_stop(ccp);

    ccp->master_role_request = false;
    /* sync lost event */
    if(ccp->uwb_ccp_sync_cb != NULL) ccp->uwb_ccp_sync_cb(CCP_SYNC_LOST, ccp->uwb_ccp_sync_arg);

    if(ccp->config.role == CCP_ROLE_MASTER){

        uwb_ccp_frame_t *frame = ccp->frames[0];
        frame->seq_num++;
        ccp->master_role_request = true;

        uint16_t cnt = 0;
        while(cnt < MYNEWT_VAL(UWB_CCP_PERIOD)/10000 && ccp->master_role_request){
            cnt++;
            // printf("\33[2K\rTry to be master time: %d", cnt);
            printf("Try to be master time: %d\n", cnt);
            uwb_set_rx_timeout(inst, MYNEWT_VAL(UWB_CCP_LONG_RX_TO) + inst->euid%MYNEWT_VAL(UWB_CCP_LONG_RX_TO));
            ccp_listen(ccp, 0);

            uwb_write_tx(inst, frame->array, 0, sizeof(uwb_ccp_blink_frame_t));
            uwb_write_tx_fctrl(inst, sizeof(uwb_ccp_blink_frame_t), 0);
            uwb_start_tx(inst);
        }

        if(ccp->master_role_request){
            printf("CCP_ROLE_MASTER\n");
            uwb_ccp_start(ccp, CCP_ROLE_MASTER);
            hal_gpio_write(MYNEWT_VAL(RTLS_CPP_MASTER_LED), MYNEWT_VAL(RTLS_CPP_MASTER_LED_ON));
            hal_gpio_write(MYNEWT_VAL(RTLS_CPP_SLAVE_LED), MYNEWT_VAL(RTLS_CPP_MASTER_LED_OFF));
        }
        else
        {
            ccp->my_master = 0;
            printf("CCP_ROLE_SLAVE\n");
            uwb_ccp_start(ccp, CCP_ROLE_SLAVE);
            hal_gpio_write(MYNEWT_VAL(RTLS_CPP_MASTER_LED), MYNEWT_VAL(RTLS_CPP_MASTER_LED_OFF));
            hal_gpio_write(MYNEWT_VAL(RTLS_CPP_SLAVE_LED), MYNEWT_VAL(RTLS_CPP_MASTER_LED_ON));
        }
    }
    else{
        printf("CCP_ROLE_SLAVE\n");
        ccp->my_master = 0;
        uwb_ccp_start(ccp, ccp->config.role);
        hal_gpio_write(MYNEWT_VAL(RTLS_CPP_MASTER_LED), MYNEWT_VAL(RTLS_CPP_MASTER_LED_OFF));
        hal_gpio_write(MYNEWT_VAL(RTLS_CPP_SLAVE_LED), MYNEWT_VAL(RTLS_CPP_MASTER_LED_ON));
    }

    /* sync synced event */
    if(ccp->uwb_ccp_sync_cb != NULL) ccp->uwb_ccp_sync_cb(CCP_SYNC_SYED, ccp->uwb_ccp_sync_arg);
}

static void
uwb_ccp_change_role_evt(struct dpl_event * ev)
{
    assert(ev != NULL);
    assert(dpl_event_get_arg(ev));
    printf("uwb_ccp_change_role\n");
    struct uwb_ccp_instance * ccp = (struct uwb_ccp_instance *) dpl_event_get_arg(ev);
    uwb_ccp_change_role(ccp);
}

void
rtls_ccp_start_role(struct uwb_ccp_instance *ccp, uwb_ccp_role_t uwb_ccp_role){
    dpl_event_init(&ccp->change_role_event, uwb_ccp_change_role_evt, (void *) ccp);
    
    ccp->config.role = CCP_ROLE_SLAVE;
    ccp->uwb_ccp_role = uwb_ccp_role;
    dpl_eventq_put(&ccp->eventq, &ccp->change_role_event);
}

void
rtls_ccp_set_sync_cb(struct uwb_ccp_instance *ccp, uwb_ccp_sync_cb_t uwb_ccp_sync_cb, void *arg){
    ccp->uwb_ccp_sync_cb = uwb_ccp_sync_cb;
    ccp->uwb_ccp_sync_arg = arg;
}

static void
ccp_timer_irq(void * arg){
    assert(arg);
    struct uwb_ccp_instance *ccp = (struct uwb_ccp_instance*)arg;
    dpl_eventq_put(&ccp->eventq, &ccp->master_slave_event);
}

static void *
ccp_task(void *arg)
{
    struct uwb_ccp_instance *ccp = arg;
    while (1) {
        dpl_eventq_run(&ccp->eventq);
    }
    return NULL;
}

struct uwb_ccp_instance *
uwb_ccp_init(struct uwb_dev* dev, uint16_t nframes)
{
    assert(dev);
    assert(nframes > 1);

    struct uwb_ccp_instance *ccp = (struct uwb_ccp_instance*)uwb_mac_find_cb_inst_ptr(dev, UWBEXT_CCP);
    if (ccp == NULL) {
        ccp = (struct uwb_ccp_instance *) calloc(1, sizeof(struct uwb_ccp_instance) + nframes * sizeof(uwb_ccp_frame_t *));
        assert(ccp);
        ccp->status.selfmalloc = 1;
        ccp->nframes = nframes;
        uwb_ccp_frame_t ccp_default = {
            .fctrl = FCNTL_IEEE_BLINK_CCP_64,    // frame control (FCNTL_IEEE_BLINK_64 to indicate a data frame using 64-bit addressing).
            .seq_num = 0xFF,
            .rpt_count = 0,
            .rpt_max = MYNEWT_VAL(UWB_CCP_MAX_CASCADE_RPTS)
        };
        
        ccp->dev_inst = dev;

        for(int i = 0; i < ccp->nframes; i++){
            ccp->frames[i] = (uwb_ccp_frame_t *) calloc(1, sizeof(uwb_ccp_frame_t));
            assert(ccp->frames[i]);
            memcpy(ccp->frames[i], &ccp_default, sizeof(uwb_ccp_frame_t));
            ccp->frames[i]->seq_num = 0;
        }

        /* TODO: fixme below */
        ccp->task_prio = dev->task_prio - 0x4;
    }else{
        assert(ccp->nframes == nframes);
    }
    ccp->period = MYNEWT_VAL(UWB_CCP_PERIOD);
    ccp->config = (struct uwb_ccp_config){
        .postprocess = false,
        .tx_holdoff_dly = MYNEWT_VAL(UWB_CCP_RPT_HOLDOFF_DLY),
    };

    dpl_error_t err = dpl_sem_init(&ccp->sem, 0x1);
    assert(err == DPL_OK);

    ccp->cbs = (struct uwb_mac_interface){
        .id = UWBEXT_CCP,
        .inst_ptr = (void*)ccp,
        .tx_complete_cb = tx_complete_cb,
        .rx_complete_cb = rx_complete_cb,
        .rx_timeout_cb = rx_timeout_cb,
        .rx_error_cb = error_cb,
        .tx_error_cb = error_cb,
        .reset_cb = reset_cb
    };
    uwb_mac_append_interface(dev, &ccp->cbs);

    /* Check if the tasks are already initiated */
    if (!dpl_eventq_inited(&ccp->eventq))
    {
        /* Use a dedicate event queue for ccp events */
        dpl_eventq_init(&ccp->eventq);
        dpl_task_init(&ccp->task_str, "ccp",
                      ccp_task,
                      (void *) ccp,
                      ccp->task_prio, DPL_WAIT_FOREVER,
                      ccp->task_stack,
                      MYNEWT_VAL(UWB_CCP_TASK_STACK_SZ));
    }
    ccp->status.initialized = 1;

    #if MYNEWT_VAL(UWB_CCP_STATS)
    int rc = stats_init(
                STATS_HDR(ccp->stat),
                STATS_SIZE_INIT_PARMS(ccp->stat, STATS_SIZE_32),
                STATS_NAME_INIT_PARMS(uwb_ccp_stat_section)
            );
    assert(rc == 0);
    rc = stats_register("ccp", STATS_HDR(ccp->stat));
    assert(rc == 0);
    #endif

    dpl_cputime_timer_init(&ccp->master_slave_timer, ccp_timer_irq, (void *) ccp);

    return ccp;
}

static void
uwb_ccp_deinit(struct uwb_ccp_instance *ccp)
{
    assert(ccp);

    #if MYNEWT_VAL(RTLS_CPP_VERBOSE)
    printf("{\"utime\": %lu,\"msg\": \"uwb_ccp_deinit\"}\n",
            dpl_cputime_ticks_to_usecs(dpl_cputime_get32()));
    #endif

    dpl_cputime_timer_stop(&ccp->master_slave_timer);
    dpl_eventq_deinit(&ccp->eventq);
    dpl_task_remove(&ccp->task_str);
    if(dpl_sem_get_count(&ccp->sem) == 0){
        uwb_phy_forcetrxoff(ccp->dev_inst);
        dpl_error_t err = dpl_sem_release(&ccp->sem);
        assert(err == DPL_OK);
    }

    ccp->status.enabled = 0;
    dpl_sem_release(&ccp->sem);
    uwb_mac_remove_interface(ccp->dev_inst, ccp->cbs.id);

    if (ccp->status.selfmalloc) {
        for(int i = 0; i < ccp->nframes; i++) {
            free(ccp->frames[i]);
        }
        free(ccp);
    }
}

void
uwb_ccp_pkg_init(void)
{
    int i;
    struct uwb_dev *udev;
    struct uwb_ccp_instance * ccp __attribute__((unused));
    #if MYNEWT_VAL(RTLS_CPP_VERBOSE)
        printf("{\"utime\": %lu,\"msg\": \"uwb_ccp_pkg_init\"}\n",
            dpl_cputime_ticks_to_usecs(dpl_cputime_get32()));
    #endif

    for (i = 0; i < MYNEWT_VAL(UWB_DEVICE_MAX); i++) {
        udev = uwb_dev_idx_lookup(i);
        if (!udev) {
            continue;
        }
        ccp = uwb_ccp_init(udev, 2);
    }

    #if MYNEWT_VAL(RTLS_CPP_USE_LED)
    hal_gpio_init_out(MYNEWT_VAL(RTLS_CPP_MASTER_LED), MYNEWT_VAL(RTLS_CPP_MASTER_LED_OFF));
    hal_gpio_init_out(MYNEWT_VAL(RTLS_CPP_SLAVE_LED), MYNEWT_VAL(RTLS_CPP_SLAVE_LED_OFF));
    #endif
}

int
uwb_ccp_pkg_down(int reason)
{
    int i;
    struct uwb_dev *udev;
    struct uwb_ccp_instance * ccp;

    for (i = 0; i < MYNEWT_VAL(UWB_DEVICE_MAX); i++) {
        udev = uwb_dev_idx_lookup(i);
        if (!udev) {
            continue;
        }
        ccp = (struct uwb_ccp_instance*)uwb_mac_find_cb_inst_ptr(udev, UWBEXT_CCP);
        if (!ccp) {
            continue;
        }
        if (ccp->status.enabled) {
            uwb_ccp_deinit(ccp);
        }
    }

    return 0;
}
