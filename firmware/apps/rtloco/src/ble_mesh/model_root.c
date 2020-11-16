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
#include <rtloco/mesh_define.h>
#include <rtloco/rtls.h>

static void
rtls_model_set(struct bt_mesh_model *model,
              struct bt_mesh_msg_ctx *ctx,
              struct os_mbuf *buf)
{  
    printf("rtls_model_set()\n");
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

        pub->msg = NET_BUF_SIMPLE(1+sizeof(msg_rtls_t));
        bt_mesh_model_msg_init(pub->msg, BT_MESH_MODEL_OP_STATUS);

        msg_rtls.msg_type = MAVLINK_MSG_ID_LOCATION,
        rtls_get_ntype(&msg_rtls.node_type);
        rtls_get_address(&msg_rtls.dstsrc);
        rtls_get_location(&msg_rtls.location_x, &msg_rtls.location_y, &msg_rtls.location_z);

        msg_prepr_rtls(pub->msg, &msg_rtls);

        int err = bt_mesh_model_publish(model);
        if (err) {
            printf("bt_mesh_model_publish err %d\n", err);
        }
        os_mbuf_free(pub->msg);
    }
}

static struct os_task g_rtls_task;
static os_stack_t g_task_rtls_stack[MYNEWT_VAL(APP_RTLS_TASK_STACK_SIZE)];
void model_rtls_init(){
    os_task_init(&g_rtls_task, "m_rtls",
                task_rtls_func,
                NULL,
                MYNEWT_VAL(APP_RTLS_TASK_PRIORITY), 
                OS_WAIT_FOREVER,
                g_task_rtls_stack,
                MYNEWT_VAL(APP_RTLS_TASK_STACK_SIZE));
}