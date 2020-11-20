#include <rtls/rtls/rtls.h>
#include <config/config.h>
#include <hal/hal_system.h>

#include <rtls_tdma/rtls_tdma.h>

#if __GNUC__ >= 9
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"
#endif
#include <message/mavlink/protocol/mavlink.h>

#include <uwb/uwb.h>
#include <uwb_rng/uwb_rng.h>

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

void rtls_tdma_cb(rtls_tdma_instance_t *rti, tdma_slot_t *slot){
    
#if MYNEWT_VAL(RTR_ROLE) == RTR_ANCHOR
    static uint16_t timeout = 0;
    struct uwb_rng_instance *rng = rti->uri;
    struct uwb_dev *inst = rti->dev_inst;
    uint16_t idx = slot->idx;

    if (!timeout) {
        timeout = uwb_usecs_to_dwt_usecs(uwb_phy_frame_duration(inst, sizeof(ieee_rng_request_frame_t)))
            + rng->config.rx_timeout_delay;
        printf("Range request set timeout to: %d %d = %d\n",
               uwb_phy_frame_duration(inst, sizeof(ieee_rng_request_frame_t)),
               rng->config.rx_timeout_delay, timeout);
    }
    tdma_instance_t *tdma = slot->parent;
    uwb_rng_listen_delay_start(rng, tdma_rx_slot_start(tdma, idx), timeout, UWB_BLOCKING);
    // struct uwb_dev_status status = uwb_rng_listen_delay_start(rng, tdma_rx_slot_start(tdma, idx), timeout, UWB_BLOCKING);
    // if(!status.rx_timeout_error){
    //     printf("Rx timeout Slot %d\n", idx);
    // }

#elif MYNEWT_VAL(RTR_ROLE) == RTR_TAG

    /* Only start range requests if I this is my slot*/
    if(rti->slot_idx == slot->idx){
        tdma_instance_t * tdma = slot->parent;
        uint16_t idx = slot->idx;
        struct uwb_rng_instance *rng = rti->uri;

        uint64_t dx_time = tdma_tx_slot_start(tdma, idx) & 0xFFFFFFFFFE00UL;

        /* Range with the clock master by default */
        uint16_t node_address = rti->nodes[1].addr;

        uwb_rng_request_delay_start(rng, node_address, dx_time, UWB_DATA_CODE_SS_TWR);
    }

#endif
}

rtls_tdma_instance_t g_rtls_tdma_instance = {
    .rtls_tdma_cb = rtls_tdma_cb
};
struct uwb_mac_interface g_cbs;

static bool
complete_cb(struct uwb_dev * inst, struct uwb_mac_interface * cbs)
{
    if (inst->fctrl != FCNTL_IEEE_RANGE_16 &&
        inst->fctrl != (FCNTL_IEEE_RANGE_16|UWB_FCTRL_ACK_REQUESTED)) {
        return false;
    }
    struct uwb_rng_instance *rng = (struct uwb_rng_instance *)cbs->inst_ptr;

    dpl_float64_t time_of_flight = uwb_rng_twr_to_tof(rng, rng->idx_current);
    double r = uwb_rng_tof_to_meters(time_of_flight);
    printf("d: %d.%d\n", (int)r, (int)(1000*(r - (int)r)));
    return true;
}

void rtls_init(){
    int rc;

    udev = uwb_dev_idx_lookup(0);
    uwb_set_dblrxbuff(udev, false);

    g_rtls_tdma_instance.uri  = (struct uwb_rng_instance *)uwb_mac_find_cb_inst_ptr(udev, UWBEXT_RNG);

    g_cbs = (struct uwb_mac_interface){
        .id =  UWBEXT_APP0,
        .inst_ptr = g_rtls_tdma_instance.uri,
        .complete_cb = complete_cb,
    };
    uwb_mac_append_interface(udev, &g_cbs);
    
    g_rtls_tdma_instance.role = MYNEWT_VAL(RTR_ROLE);
    rtls_tdma_start(&g_rtls_tdma_instance, udev);

    rc = conf_register(&rtls_handler);
    if(rc){
        printf("Config register failed: %d\n", rc);
    }
    else{
        conf_load();
        printf("RTLS config loaded\n");
    }
}