#include <assert.h>
#include <dpl/dpl.h>
#include <hal/hal_gpio.h>
#include <bsp/bsp.h>

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

STATS_SECT_START(model_root_stat_t)
    STATS_SECT_ENTRY(send_failed)
    STATS_SECT_ENTRY(recv_recved)
    STATS_SECT_ENTRY(recv_succed)
STATS_SECT_END

STATS_SECT_DECL(model_root_stat_t) g_model_root_stat;

STATS_NAME_START(model_root_stat_t)
    STATS_NAME(model_root_stat_t, send_failed)
    STATS_NAME(model_root_stat_t, recv_recved)
    STATS_NAME(model_root_stat_t, recv_succed)
STATS_NAME_END(model_root_stat_t)

static void
rtls_model_set(struct bt_mesh_model *model,
              struct bt_mesh_msg_ctx *ctx,
              struct os_mbuf *buf)
{  
    STATS_INC(g_model_root_stat, recv_recved);
    msg_rtls_t msg_rtls;
    uint16_t dstsrc;
    msg_parse_rtls(buf, &msg_rtls);
    rtls_get_address(&dstsrc);
    if(msg_rtls.dstsrc != dstsrc) return;

    switch (msg_rtls.type)
    {
    case MAVLINK_MSG_ID_LOCATION:
        STATS_INC(g_model_root_stat, recv_succed);
        uint8_t ntype;
        rtls_get_ntype(&ntype);
        if(ntype != msg_rtls.node_type){
            rtls_set_ntype(msg_rtls.node_type);
        }
        rtls_set_location(msg_rtls.location_x, msg_rtls.location_y, msg_rtls.location_z);
        msg_print_rtls(&msg_rtls);
        break;
    case MAVLINK_MSG_ID_ONOFF:
        hal_gpio_init_out(LED_1, msg_rtls.value);
        STATS_INC(g_model_root_stat, recv_succed);
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
    #if MYNEWT_VAL(BLE_MESH_FRIEND)
        .frnd = BT_MESH_FRIEND_ENABLED,
    #else
        .gatt_proxy = BT_MESH_GATT_PROXY_NOT_SUPPORTED,
    #endif
    #if MYNEWT_VAL(BLE_MESH_GATT_PROXY)
        .gatt_proxy = BT_MESH_GATT_PROXY_ENABLED,
    #else
        .gatt_proxy = BT_MESH_GATT_PROXY_NOT_SUPPORTED,
    #endif
    .default_ttl = 7,

    /* 3 transmissions with 20ms interval */
    .net_transmit = BT_MESH_TRANSMIT(2, 20),
    .relay_retransmit = BT_MESH_TRANSMIT(2, 20),
};
static struct bt_mesh_model_pub model_pub_rtls;

struct bt_mesh_model model_root[] = {
    BT_MESH_MODEL_CFG_SRV(&cfg_srv),
    BT_MESH_MODEL(BT_MESH_MODEL_ID_RTLS, rtls_op, &model_pub_rtls, NULL),
};

static void
task_rtls_func(void *arg){
    struct bt_mesh_model *model = &model_root[1];
    struct bt_mesh_model_pub *pub =  model->pub;
    msg_rtls_t msg_rtls;

    while (1) {
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
            STATS_INC(g_model_root_stat, send_failed);
        }
        os_mbuf_free(pub->msg);
    }
}

static struct os_task g_rtls_task;
static os_stack_t g_task_rtls_stack[MYNEWT_VAL(APP_RTLS_TASK_STACK_SIZE)];
void model_gateway_init(){
    int rc;

    hal_gpio_init_out(LED_1, 1);
    os_task_init(&g_rtls_task, "m_rtls",
                task_rtls_func,
                NULL,
                MYNEWT_VAL(APP_RTLS_TASK_PRIORITY), 
                OS_WAIT_FOREVER,
                g_task_rtls_stack,
                MYNEWT_VAL(APP_RTLS_TASK_STACK_SIZE));

    rc = stats_init(
        STATS_HDR(g_model_root_stat),
        STATS_SIZE_INIT_PARMS(g_model_root_stat, STATS_SIZE_32),
        STATS_NAME_INIT_PARMS(model_root_stat_t));
    assert(rc == 0);

    rc = stats_register("m_rtls", STATS_HDR(g_model_root_stat));
    assert(rc == 0);
}