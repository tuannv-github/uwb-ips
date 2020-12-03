#include <assert.h>
#include <dpl/dpl.h>
#include <hal/hal_gpio.h>
#include <bsp/bsp.h>

#if MYNEWT_VAL(BLE_MESH)

/* BLE */
#include "mesh/mesh.h"
#include "nimble/ble.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "mesh/glue.h"

#include <message/mesh_msg.h>
#include <rtls/ble_mesh/mesh_define.h>
#include <rtls/rtls/rtls.h>

#include <stats/stats.h>

#define LED_DELAY_MS   100

STATS_SECT_START(model_root_stat_t)
    STATS_SECT_ENTRY(send_loca_succed)
    STATS_SECT_ENTRY(send_loca_failed)
    STATS_SECT_ENTRY(send_dist_succed)
    STATS_SECT_ENTRY(send_dist_failed)
    STATS_SECT_ENTRY(recv_setmsg)
    STATS_SECT_ENTRY(recv_sttmsg)
STATS_SECT_END

STATS_SECT_DECL(model_root_stat_t) g_model_root_stat;

STATS_NAME_START(model_root_stat_t)
    STATS_NAME(model_root_stat_t, send_loca_succed)
    STATS_NAME(model_root_stat_t, send_loca_failed)
    STATS_NAME(model_root_stat_t, send_dist_succed)
    STATS_NAME(model_root_stat_t, send_dist_failed)
    STATS_NAME(model_root_stat_t, recv_setmsg)
    STATS_NAME(model_root_stat_t, recv_sttmsg)
STATS_NAME_END(model_root_stat_t)

static struct os_task g_rtls_location_task;
static os_stack_t g_task_rtls_location_stack[MYNEWT_VAL(APP_RTLS_LOCATION_TASK_STACK_SIZE)];

static struct os_task g_rtls_distance_task;
static os_stack_t g_task_rtls_distance_stack[MYNEWT_VAL(APP_RTLS_DISTANCE_TASK_STACK_SIZE)];

struct os_mutex g_location_distance_mutex;

static struct os_task g_led_task;
static os_stack_t g_task_led_stack[MYNEWT_VAL(TASK_LED_TASK_STACK_SIZE)];
static bool g_led_running = false;

static void
rtls_model_set(struct bt_mesh_model *model,
              struct bt_mesh_msg_ctx *ctx,
              struct os_mbuf *buf)
{  
    STATS_INC(g_model_root_stat, recv_setmsg);
    msg_rtls_t msg_rtls;
    uint16_t dstsrc;
    msg_parse_rtls(buf, &msg_rtls);
    rtls_get_address(&dstsrc);
    if(msg_rtls.dstsrc != dstsrc) return;

    switch (msg_rtls.type)
    {
    case MAVLINK_MSG_ID_LOCATION:
    {
        uint8_t ntype;
        rtls_get_ntype(&ntype);
        if(ntype != msg_rtls.node_type){
            rtls_set_ntype(msg_rtls.node_type);
        }
        rtls_set_location(msg_rtls.location_x, msg_rtls.location_y, msg_rtls.location_z);
        msg_print_rtls(&msg_rtls);
    }
        break;
    case MAVLINK_MSG_ID_ONOFF:
        if(msg_rtls.value & 0x01)
        {
            g_led_running = true;
            hal_gpio_write(LED_1, 0);
        }else{
            g_led_running = false;
            hal_gpio_write(LED_1, 1);
        }

        if(msg_rtls.value & 0x02){
            hal_gpio_write(9, 0);
        }
        else{
            hal_gpio_write(9, 1);
        }

        if(msg_rtls.value & 0x04){
            hal_gpio_write(10, 0);
        }
        else{
            hal_gpio_write(10, 1);
        }
        break;
    default:
        break;
    }
}

static const struct bt_mesh_model_op rtls_op[] = {
    { BT_MESH_MODEL_OP_SET, 0, rtls_model_set},
    BT_MESH_MODEL_OP_END,
};

static struct bt_mesh_cfg_srv cfg_srv = {
    .relay = BT_MESH_RELAY_DISABLED,
    .beacon = BT_MESH_BEACON_ENABLED,
    .frnd = BT_MESH_FRIEND_NOT_SUPPORTED,
    .gatt_proxy = BT_MESH_GATT_PROXY_ENABLED,
    .default_ttl = 7,
    .net_transmit = BT_MESH_TRANSMIT(1, 20),
    .relay_retransmit = BT_MESH_TRANSMIT(1, 20),
};
static struct bt_mesh_model_pub model_pub_rtls;

struct bt_mesh_model model_root[] = {
    BT_MESH_MODEL_CFG_SRV(&cfg_srv),
    BT_MESH_MODEL(BT_MESH_MODEL_ID_RTLS, rtls_op, &model_pub_rtls, NULL),
};

static struct bt_mesh_model *model;
static struct bt_mesh_model_pub *pub;
static msg_rtls_t msg_rtls;

static void
task_rtls_location_func(void *arg){

    while (1) {
        os_mutex_pend(&g_location_distance_mutex, OS_TIMEOUT_NEVER);

        dpl_time_delay(dpl_time_ms_to_ticks32(1000));
        if (pub->addr == BT_MESH_ADDR_UNASSIGNED) continue;

        msg_rtls.type = MAVLINK_MSG_ID_LOCATION;
        msg_rtls.opcode = BT_MESH_MODEL_OP_STATUS;
        rtls_get_ntype(&msg_rtls.node_type);
        rtls_get_address(&msg_rtls.dstsrc);
        rtls_get_location(&msg_rtls.location_x, &msg_rtls.location_y, &msg_rtls.location_z);

        msg_prepr_rtls(&pub->msg, &msg_rtls);

        int err = bt_mesh_model_publish(model);
        if (err) {
            STATS_INC(g_model_root_stat, send_loca_failed);
        }
        else{
            STATS_INC(g_model_root_stat, send_loca_succed);
        }
        os_mbuf_free(pub->msg);

        os_mutex_release(&g_location_distance_mutex);
    }
}

static void
task_rtls_distance_func(void *arg){
    while(1){
        os_mutex_pend(&g_location_distance_mutex, OS_TIMEOUT_NEVER);
        dpl_time_delay(dpl_time_ms_to_ticks32(100));
        if (pub->addr == BT_MESH_ADDR_UNASSIGNED) continue;

        distance_t *distances = get_distances();
        for(int i=0; i<ANCHOR_NUM; i++){
            if(distances->updated[i]){
                distances->updated[i] = false;

                msg_rtls.type = MAVLINK_MSG_ID_TOF;
                msg_rtls.opcode = BT_MESH_MODEL_OP_STATUS;
                rtls_get_address(&msg_rtls.dstsrc);
                msg_rtls.anchor = distances->anchors[i];
                msg_rtls.tof = distances->tofs[i];

                msg_prepr_rtls(&pub->msg, &msg_rtls);

                int err = bt_mesh_model_publish(model);
                if (err) {
                    STATS_INC(g_model_root_stat, send_dist_failed);
                }
                else{
                    STATS_INC(g_model_root_stat, send_dist_succed);
                }
                os_mbuf_free(pub->msg);
                dpl_time_delay(dpl_time_ms_to_ticks32(100));
            }
        }

        os_mutex_release(&g_location_distance_mutex);
    }
}

static void
task_led_func(void *arg){

    while (1) {
        if(!g_led_running) {
            dpl_time_delay(dpl_time_ms_to_ticks32(500));
            continue;
        }

        hal_gpio_write(12, 1);
        hal_gpio_write(9, 0);
        hal_gpio_write(10, 0);

        dpl_time_delay(dpl_time_ms_to_ticks32(LED_DELAY_MS));
        hal_gpio_write(13, 0);
        dpl_time_delay(dpl_time_ms_to_ticks32(LED_DELAY_MS));
        hal_gpio_write(23, 0);
        dpl_time_delay(dpl_time_ms_to_ticks32(LED_DELAY_MS));
        hal_gpio_write(22, 0);
        dpl_time_delay(dpl_time_ms_to_ticks32(LED_DELAY_MS));
        hal_gpio_write(14, 0);

        dpl_time_delay(dpl_time_ms_to_ticks32(LED_DELAY_MS));
        hal_gpio_write(13, 1);
        dpl_time_delay(dpl_time_ms_to_ticks32(LED_DELAY_MS));
        hal_gpio_write(23, 1);
        dpl_time_delay(dpl_time_ms_to_ticks32(LED_DELAY_MS));
        hal_gpio_write(22, 1);
        dpl_time_delay(dpl_time_ms_to_ticks32(LED_DELAY_MS));
        hal_gpio_write(14, 1);

        hal_gpio_write(12, 0);
        hal_gpio_write(9, 1);
        hal_gpio_write(10, 1);
        dpl_time_delay(dpl_time_ms_to_ticks32(1000));

    }
}

void model_gateway_init(){
    int rc;

    model = &model_root[1];
    pub =  model->pub;

    /* RTLS LED */
    hal_gpio_init_out(14, 1);
    hal_gpio_init_out(22, 1);
    hal_gpio_init_out(23, 1);
    hal_gpio_init_out(13, 1);

    /* RTLS Buzzer */
    hal_gpio_init_out(12, 0);

    /* DWM1001-DEV LED */
    hal_gpio_init_out(LED_1, 1);

    /* RTLS Light Bulb */
    hal_gpio_init_out(9, 1);
    hal_gpio_init_out(10, 1);

    os_mutex_init(&g_location_distance_mutex);

    os_task_init(&g_led_task, "led",
                task_led_func,
                NULL,
                MYNEWT_VAL(TASK_LED_TASK_PRIORITY), 
                OS_WAIT_FOREVER,
                g_task_led_stack,
                MYNEWT_VAL(TASK_LED_TASK_STACK_SIZE));
    
    os_task_init(&g_rtls_location_task, "location",
                task_rtls_location_func,
                NULL,
                MYNEWT_VAL(APP_RTLS_LOCATION_TASK_PRIORITY), 
                OS_WAIT_FOREVER,
                g_task_rtls_location_stack,
                MYNEWT_VAL(APP_RTLS_LOCATION_TASK_STACK_SIZE));

    os_task_init(&g_rtls_distance_task, "distance",
                task_rtls_distance_func,
                NULL,
                MYNEWT_VAL(APP_RTLS_DISTANCE_TASK_PRIORITY), 
                OS_WAIT_FOREVER,
                g_task_rtls_distance_stack,
                MYNEWT_VAL(APP_RTLS_DISTANCE_TASK_STACK_SIZE));

    rc = stats_init(
        STATS_HDR(g_model_root_stat),
        STATS_SIZE_INIT_PARMS(g_model_root_stat, STATS_SIZE_32),
        STATS_NAME_INIT_PARMS(model_root_stat_t));
    assert(rc == 0);

    rc = stats_register("mrtls", STATS_HDR(g_model_root_stat));
    assert(rc == 0);
}

#endif