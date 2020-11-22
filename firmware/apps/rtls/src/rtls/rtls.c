#include <rtls/rtls/rtls.h>
#include <config/config.h>
#include <hal/hal_system.h>

#include <rtls_tdma/rtls_tdma.h>

#if __GNUC__ >= 9
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"
#endif
#include <message/mavlink/protocol/mavlink.h>

#include <uwb/uwb.h>
#include <uwb_nrng/nrng.h>
#include <uwb/uwb_mac.h>

static struct {
    uint8_t node_type;
    float location_x;
    float location_y;
    float location_z;
} rtls_conf = {
    ANCHOR,
    0,0,0
};

static struct{
    char node_type[8];
    char location_x[8];
    char location_y[8];
    char location_z[8];
} rtls_conf_str = {
    "ANCHOR",
    "0.0", "0.0", "0.0"
};

static char *
rtls_get(int argc, char **argv, char *val, int val_len_max)
{
    if (argc == 1) {
        if (!strcmp(argv[0], "node_type"))  return rtls_conf_str.node_type;
        else if (!strcmp(argv[0], "location_x"))  return rtls_conf_str.location_x;
        else if (!strcmp(argv[0], "location_y"))  return rtls_conf_str.location_y;
        else if (!strcmp(argv[0], "location_z"))  return rtls_conf_str.location_z;
    }
    return NULL;
}

static int
rtls_set(int argc, char **argv, char *val)
{
    int rc = -1;
    if (argc == 1) {
        if (!strcmp(argv[0], "node_type")) {
            if(!strcmp(val, "TAG") || !strcmp(val, "ANCHOR")){
                rc = CONF_VALUE_SET(val, CONF_STRING, rtls_conf_str.node_type);
                if(rc) goto done;
                if(!strcmp(rtls_conf_str.node_type, "TAG")){
                    rtls_conf.node_type = TAG;
                    printf("Node type: TAG\n");
                }
                else if(!strcmp(rtls_conf_str.node_type, "ANCHOR")){
                    rtls_conf.node_type = ANCHOR;
                    printf("Node type: ANCHOR\n");
                }else{
                    printf("Invalid node type: %s\n", rtls_conf_str.node_type);
                    return -1;
                }
            }
            else{
                printf("Invalid node type!\n");
            }
        }
        else if (!strcmp(argv[0], "location_x")) {
            rc = CONF_VALUE_SET(val, CONF_STRING, rtls_conf_str.location_x);
            if(rc) goto done;
            rtls_conf.location_x = strtod(rtls_conf_str.location_x, NULL);
        }
        else if (!strcmp(argv[0], "location_y")) {
            rc = CONF_VALUE_SET(val, CONF_STRING, rtls_conf_str.location_y);
            if(rc) goto done;
            rtls_conf.location_y = strtod(rtls_conf_str.location_y, NULL);
        }
        else if (!strcmp(argv[0], "location_z")) {
            rc = CONF_VALUE_SET(val, CONF_STRING, rtls_conf_str.location_z);
            if(rc) goto done;
            rtls_conf.location_z = strtod(rtls_conf_str.location_z, NULL);
        }
    }

done:
    return rc;
}

static int
rtls_export(void (*export_func)(char *name, char *val),
             enum conf_export_tgt tgt)
{
    export_func("rtls/node_type", rtls_conf_str.node_type);
    export_func("rtls/location_x", rtls_conf_str.location_x);
    export_func("rtls/location_y", rtls_conf_str.location_y);
    export_func("rtls/location_z", rtls_conf_str.location_z);
    return 0;
}

static struct conf_handler rtls_handler = {
    .ch_name = "rtls",
    .ch_get = rtls_get,
    .ch_set = rtls_set,
    .ch_commit = NULL,
    .ch_export = rtls_export,
};

struct uwb_dev *udev;
void rtls_get_location(float *x, float *y, float *z){
    *x = rtls_conf.location_x;
    *y = rtls_conf.location_y;
    *z = rtls_conf.location_z;
}

void rtls_set_location(float x, float y, float z){
    rtls_conf.location_x = x;
    sprintf(rtls_conf_str.location_x, "%d.%d", (int)rtls_conf.location_x, (int)(1000*(rtls_conf.location_x - (int)rtls_conf.location_x)));
    conf_save_one("rtls/location_x", rtls_conf_str.location_x);
    rtls_conf.location_y = y;
    sprintf(rtls_conf_str.location_y, "%d.%d", (int)rtls_conf.location_y, (int)(1000*(rtls_conf.location_y - (int)rtls_conf.location_y)));
    conf_save_one("rtls/location_y", rtls_conf_str.location_y);
    rtls_conf.location_z = z;
    sprintf(rtls_conf_str.location_z, "%d.%d", (int)rtls_conf.location_z, (int)(1000*(rtls_conf.location_z - (int)rtls_conf.location_z)));
    conf_save_one("rtls/location_z", rtls_conf_str.location_z);
}

void rtls_get_address(uint16_t *address){
    *address = udev->my_short_address;
}

void rtls_get_ntype(uint8_t *ntype){
    *ntype = rtls_conf.node_type;
}

void rtls_set_ntype(uint8_t ntype){
    switch (ntype)
    {
    case ANCHOR:
        sprintf(rtls_conf_str.node_type, "%s", "ANCHOR");
        break;
    case TAG:
        sprintf(rtls_conf_str.node_type, "%s", "TAG");
        break;
    default:
        printf("Invalid node type!");
        return;
    }
    conf_save_one("rtls/node_type", rtls_conf_str.node_type);
    printf("Role change: %s\nSystem reset\n", rtls_conf_str.node_type);
    hal_system_reset();
}

void rtls_tdma_cb(rtls_tdma_instance_t *rti, tdma_slot_t *slot);

static rtls_tdma_instance_t g_rtls_tdma_instance = {
    .rtls_tdma_cb = rtls_tdma_cb
};
static struct uwb_mac_interface g_cbs;
static struct nrng_instance* g_nrng;

void rtls_tdma_cb(rtls_tdma_instance_t *rti, tdma_slot_t *slot){
    rti->dev_inst->slot_id = g_rtls_tdma_instance.slot_idx;

    if(rtls_conf.node_type == RTR_ANCHOR){
        /* Listen for a ranging tag */
        uwb_set_delay_start(udev, tdma_rx_slot_start(slot->parent, slot->idx));
        uint16_t timeout = uwb_phy_frame_duration(udev, sizeof(nrng_request_frame_t))
            + g_nrng->config.rx_timeout_delay;

        /* Padded timeout to allow us to receive any nmgr packets too */
        uwb_set_rx_timeout(udev, timeout + 0x1000);
        nrng_listen(g_nrng, UWB_BLOCKING);
    }
    else if(rtls_conf.node_type == RTR_TAG){
        if(slot->idx == g_rtls_tdma_instance.slot_idx){
        /* Range with the anchors */
            uint64_t dx_time = tdma_tx_slot_start(slot->parent, slot->idx) & 0xFFFFFFFFFE00UL;
            uint32_t slot_mask = 0;
            for (uint16_t i = MYNEWT_VAL(NODE_START_SLOT_ID);
                i <= MYNEWT_VAL(NODE_END_SLOT_ID); i++) {
                slot_mask |= 1UL << i;
            }

            if(nrng_request_delay_start(
                g_nrng, UWB_BROADCAST_ADDRESS, dx_time,
                UWB_DATA_CODE_SS_TWR_NRNG, slot_mask, 0).start_tx_error) {
                uint32_t utime = os_cputime_ticks_to_usecs(os_cputime_get32());
                printf("{\"utime\": %lu,\"msg\": \"slot_timer_cb_%d:start_tx_error\"}\n",
                    utime, slot->idx);
            }
        }
    }
}

static bool
complete_cb(struct uwb_dev * inst, struct uwb_mac_interface * cbs)
{
    if (inst->fctrl != FCNTL_IEEE_RANGE_16 &&
        inst->fctrl != (FCNTL_IEEE_RANGE_16|UWB_FCTRL_ACK_REQUESTED)) {
        return false;
    }
    struct nrng_instance *nrng = (struct nrng_instance *)cbs->inst_ptr;

    uint16_t anchors = MYNEWT_VAL(NODE_END_SLOT_ID) - MYNEWT_VAL(NODE_START_SLOT_ID) + 1;
    float ranges[anchors];
    uint16_t addresses[anchors];
    memset(addresses, 0, sizeof(addresses));
    nrng_get_ranges_addresses( nrng, ranges, addresses, 10, nrng->idx);
    for(int i=0; i<anchors; i++){
        if(addresses[i] != 0){
            float r = ranges[i];
            printf("0x%04X: %d.%d\n", addresses[i], (int)r, (int)(1000*(r - (int)r)));  
        }
    }

    return true;
}

void rtls_init(){
    int rc;

    rc = conf_register(&rtls_handler);
    if(rc){
        printf("Config register failed: %d\n", rc);
    }
    else{
        conf_load();
        printf("RTLS config loaded\n");
    }

    udev = uwb_dev_idx_lookup(0);
    uwb_set_dblrxbuff(udev, false);

    g_nrng = (struct nrng_instance*)uwb_mac_find_cb_inst_ptr(udev, UWBEXT_NRNG);
    assert(g_nrng);

    g_cbs = (struct uwb_mac_interface){
        .id =  UWBEXT_APP0,
        .inst_ptr = g_nrng,
        .complete_cb = complete_cb,
    };
    uwb_mac_append_interface(udev, &g_cbs);
    
    g_rtls_tdma_instance.role = rtls_conf.node_type;
    printf("Role: %d\n", rtls_conf.node_type);
    rtls_tdma_start(&g_rtls_tdma_instance, udev);
}