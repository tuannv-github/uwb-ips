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

#include <rtls_mesh/ble_mesh/elemt_00_node.h>
#include <rtls_mesh/ble_mesh/mesh_msg.h>
#include <rtls_mesh/ble_mesh/mesh_define.h>

typedef struct _app_root_t{ 
    struct dpl_sem sem;  
    bool running;
    struct dpl_task task;            
    dpl_stack_t task_stack[MYNEWT_VAL(APP_ROOT_TASK_STACK_SZ)];
    struct bt_mesh_model *model;
}app_root_t;

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
static app_root_t g_app_root;

static void gen_onoff_get(struct bt_mesh_model *model,
              struct bt_mesh_msg_ctx *ctx,
              struct os_mbuf *buf)
{
    printf("#mesh-onoff GET\n");
    struct os_mbuf *msg = NET_BUF_SIMPLE(2+1);
    msg_onoff_t msg_onoff;
    msg_onoff.value = g_app_root.running;

    bt_mesh_model_msg_init(msg, BT_MESH_MODEL_OP_GEN_ONOFF_STATUS);
    msg_prepr_onoff(msg, &msg_onoff);

    if (bt_mesh_model_send(model, ctx, msg, NULL, NULL)) {
        printf("#mesh-onoff STATUS: send status failed\n");
    }

    os_mbuf_free_chain(msg);
}

static void gen_onoff_set_unack(struct bt_mesh_model *model,
                struct bt_mesh_msg_ctx *ctx,
                struct os_mbuf *buf)
{
    msg_onoff_t msg_onoff;
    msg_parse_onoff(buf, &msg_onoff);

    if(msg_onoff.value){
        dpl_sem_release(&g_app_root.sem);
        g_app_root.running = true;
    }
    else{
        g_app_root.running = false;
    }
}

static void gen_onoff_set(struct bt_mesh_model *model,
              struct bt_mesh_msg_ctx *ctx,
              struct os_mbuf *buf)
{
    printf("#mesh-onoff SET\n");
    gen_onoff_set_unack(model, ctx, buf);
	gen_onoff_get(model, ctx, buf);
}

static const struct bt_mesh_model_op gen_onoff_op[] = {
    { BT_MESH_MODEL_OP_2(0x82, 0x01), 0, gen_onoff_get },
    { BT_MESH_MODEL_OP_2(0x82, 0x02), 0, gen_onoff_set },
    { BT_MESH_MODEL_OP_2(0x82, 0x03), 0, gen_onoff_set_unack },
    BT_MESH_MODEL_OP_END,
};

static const struct bt_mesh_model_op gen_batty_op[] = {
    BT_MESH_MODEL_OP_END,
};

static struct bt_mesh_model_pub gen_batty_pub;
static struct bt_mesh_model_pub gen_onoff_pub;

struct bt_mesh_model model_root[3] = {
    BT_MESH_MODEL_CFG_SRV(&cfg_srv),
    BT_MESH_MODEL(BT_MESH_MODEL_ID_GEN_BATTERY_SRV, gen_batty_op, &gen_batty_pub, NULL),
    BT_MESH_MODEL(BT_MESH_MODEL_ID_GEN_ONOFF_SRV, gen_onoff_op, &gen_onoff_pub, NULL),
};

static void *
task(void *arg)
{
    app_root_t *app_root = (app_root_t *)arg;
    // bool value = 0;

    while (1) {
        dpl_sem_pend(&app_root->sem, DPL_TIMEOUT_NEVER);

        // value = !value;
        dpl_time_delay(dpl_time_ms_to_ticks32(200));
        hal_gpio_write(LED_1, 0);
        dpl_time_delay(dpl_time_ms_to_ticks32(200));
        hal_gpio_write(LED_4, 0);
        dpl_time_delay(dpl_time_ms_to_ticks32(200));
        hal_gpio_write(LED_3, 0);
        dpl_time_delay(dpl_time_ms_to_ticks32(200));
        hal_gpio_write(LED_2, 0);
        dpl_time_delay(dpl_time_ms_to_ticks32(200));

        hal_gpio_write(LED_1, 1);
        hal_gpio_write(LED_2, 1);
        hal_gpio_write(LED_3, 1);
        hal_gpio_write(LED_4, 1);

        dpl_sem_release(&app_root->sem);
        if(!app_root->running) {
            dpl_sem_pend(&app_root->sem, DPL_TIMEOUT_NEVER);
            hal_gpio_init_out(LED_1, 1);
            hal_gpio_init_out(LED_2, 1);
            hal_gpio_init_out(LED_3, 1);
            hal_gpio_init_out(LED_4, 1);
        }
    }
    return NULL;
}

void model_root_init(){
    hal_gpio_init_out(LED_1, 1);
    hal_gpio_init_out(LED_2, 1);
    hal_gpio_init_out(LED_3, 1);
    hal_gpio_init_out(LED_4, 1);

    dpl_sem_init(&g_app_root.sem, 0x00);

    dpl_task_init(&g_app_root.task, "ma_root",
                    task,
                    (void *)&g_app_root,
                    MYNEWT_VAL(APP_ROOT_TASK_PRIORITY), 
                    DPL_WAIT_FOREVER,
                    g_app_root.task_stack,
                    MYNEWT_VAL(APP_ROOT_TASK_STACK_SZ));
}