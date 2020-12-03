#include <assert.h>
#include <hal/hal_gpio.h>
#include <bsp/bsp.h>

#if MYNEWT_VAL(BLE_MESH)

/* BLE */
#include "mesh/mesh.h"
#include "nimble/ble.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "mesh/glue.h"

#include <rtls/ble_mesh/mesh_define.h>

#include <stats/stats.h>

#define LED_DELAY_MS   100

STATS_SECT_START(model_root_stat_t)
    STATS_SECT_ENTRY(send_succed)
    STATS_SECT_ENTRY(send_failed)
    STATS_SECT_ENTRY(recv_setmsg)
    STATS_SECT_ENTRY(recv_sttmsg)
STATS_SECT_END

STATS_SECT_DECL(model_root_stat_t) g_model_root_stat;

STATS_NAME_START(model_root_stat_t)
    STATS_NAME(model_root_stat_t, send_succed)
    STATS_NAME(model_root_stat_t, send_failed)
    STATS_NAME(model_root_stat_t, recv_setmsg)
    STATS_NAME(model_root_stat_t, recv_sttmsg)
STATS_NAME_END(model_root_stat_t)

static struct os_task g_rtls_task;
static os_stack_t g_task_rtls_stack[MYNEWT_VAL(APP_RTLS_TASK_STACK_SIZE)];

static void
rtls_model_set(struct bt_mesh_model *model,
              struct bt_mesh_msg_ctx *ctx,
              struct os_mbuf *buf)
{  
    STATS_INC(g_model_root_stat, recv_setmsg);
}

static void
rtls_model_status(struct bt_mesh_model *model,
              struct bt_mesh_msg_ctx *ctx,
              struct os_mbuf *buf)
{  
    STATS_INC(g_model_root_stat, recv_sttmsg);
}

static const struct bt_mesh_model_op rtls_op[] = {
    { BT_MESH_MODEL_OP_SET, 0, rtls_model_set},
    { BT_MESH_MODEL_OP_STATUS, 0, rtls_model_status},
    BT_MESH_MODEL_OP_END,
};

static struct bt_mesh_cfg_srv cfg_srv = {
    .relay = BT_MESH_RELAY_DISABLED,
    .beacon = BT_MESH_BEACON_ENABLED,
    .frnd = BT_MESH_FRIEND_DISABLED,
    .gatt_proxy = BT_MESH_GATT_PROXY_ENABLED,
    .default_ttl = 7,
    .net_transmit = BT_MESH_TRANSMIT(5, 20),
    .relay_retransmit = BT_MESH_TRANSMIT(5, 20),
};
static struct bt_mesh_model_pub model_pub_rtls;

struct bt_mesh_model model_root[] = {
    BT_MESH_MODEL_CFG_SRV(&cfg_srv),
    BT_MESH_MODEL(BT_MESH_MODEL_ID_RTLS, rtls_op, &model_pub_rtls, NULL),
};

static int 
update(struct bt_mesh_model *mod){
    return 0;
}

static void
task_rtls_func(void *arg){
    struct bt_mesh_model *model = &model_root[1];
    struct bt_mesh_model_pub *pub =  model->pub;
    pub->update = update;

    while (1) {
        os_time_delay(os_time_ms_to_ticks32(100));
        if (pub->addr == BT_MESH_ADDR_UNASSIGNED) continue;
        
        pub->msg = NET_BUF_SIMPLE(1 + 15);
        bt_mesh_model_msg_init(pub->msg, BT_MESH_MODEL_OP_STATUS);
        
        for(int i=0; i<10; i++){
            net_buf_simple_add_u8(pub->msg, 0xAB);
        }

        int err = bt_mesh_model_publish(model);
        if (err) {
            STATS_INC(g_model_root_stat, send_failed);
        }
        else{
            STATS_INC(g_model_root_stat, send_succed);
        }
        os_mbuf_free(pub->msg);
    }
}

void model_root_init(){
    int rc;
    
    rc = os_task_init(&g_rtls_task, "rtls",
                    task_rtls_func,
                    NULL,
                    MYNEWT_VAL(APP_RTLS_TASK_PRIORITY), 
                    OS_WAIT_FOREVER,
                    g_task_rtls_stack,
                    MYNEWT_VAL(APP_RTLS_TASK_STACK_SIZE));
    assert(rc == 0);

    rc = stats_init(
        STATS_HDR(g_model_root_stat),
        STATS_SIZE_INIT_PARMS(g_model_root_stat, STATS_SIZE_32),
        STATS_NAME_INIT_PARMS(model_root_stat_t));
    assert(rc == 0);

    rc = stats_register("mrtls", STATS_HDR(g_model_root_stat));
    assert(rc == 0);
}

#endif