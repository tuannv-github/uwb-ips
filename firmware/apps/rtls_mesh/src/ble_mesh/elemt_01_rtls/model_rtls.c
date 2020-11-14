#include <assert.h>
#include <dpl/dpl.h>

#include "os/mynewt.h"
#include "mesh/mesh.h"
#include "console/console.h"
#include "hal/hal_system.h"
#include "hal/hal_gpio.h"
#include "bsp/bsp.h"
#include "shell/shell.h"

/* BLE */
#include "nimble/ble.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "mesh/glue.h"

#include <rtls_mesh/ble_mesh/elemt_01_rtls.h>
#include <rtls_mesh/ble_mesh/mesh_define.h>
#include <rtls_mesh/ble_mesh/mesh_msg.h>
#include <rtls_mesh/gateway/gateway.h>

#define MBUF_PKTHDR_OVERHEAD    sizeof(struct os_mbuf_pkthdr) + sizeof(msg_rtls_header_t)
#define MBUF_MEMBLOCK_OVERHEAD  sizeof(struct os_mbuf) + MBUF_PKTHDR_OVERHEAD

#define MBUF_NUM_MBUFS      (2)
#define MBUF_PAYLOAD_SIZE   (sizeof(msg_rtls_t))
#define MBUF_BUF_SIZE       OS_ALIGN(MBUF_PAYLOAD_SIZE, 4)
#define MBUF_MEMBLOCK_SIZE  (MBUF_BUF_SIZE + MBUF_MEMBLOCK_OVERHEAD)
#define MBUF_MEMPOOL_SIZE   OS_MEMPOOL_SIZE(MBUF_NUM_MBUFS, MBUF_MEMBLOCK_SIZE)

struct dpl_mbuf_pool g_mbuf_pool;
struct dpl_mempool g_mbuf_mempool;
dpl_membuf_t g_mbuf_buffer[MBUF_MEMPOOL_SIZE];

static void 
rtls_model_srv_set(struct bt_mesh_model *model,
              struct bt_mesh_msg_ctx *ctx,
              struct os_mbuf *buf)
{
    console_printf("rtls_model_srv_set\n");
}

static void 
rtls_model_srv_status(struct bt_mesh_model *model,
              struct bt_mesh_msg_ctx *ctx,
              struct os_mbuf *buf)
{   
    int rc;
    msg_rtls_t msg_rtls;
    msg_parse_rtls(buf, &msg_rtls);

    struct dpl_mbuf *om;
    om = dpl_mbuf_get_pkthdr(&g_mbuf_pool, sizeof(msg_rtls_header_t));
    if (om) {
        rc = dpl_mbuf_copyinto(om, 0, &msg_rtls, sizeof(msg_rtls_t));
        if (rc) {
            return;
        }
        struct dpl_mqueue *mqueue;
        struct dpl_eventq *event;
        get_ble_to_net_mqueue_eventq(&mqueue, &event);
        rc = dpl_mqueue_put(mqueue, event, om);
        if (rc != 0) {
            printf("Unable to put mqueue: %d\n", rc);
        }
    }
}

static const struct bt_mesh_model_op rtls_cli_op[] = {
    BT_MESH_MODEL_OP_END,
};
static const struct bt_mesh_model_op rtls_srv_op[] = {
    { BT_MESH_MODEL_OP_SET, 0, rtls_model_srv_set},
    { BT_MESH_MODEL_OP_STATUS, 0, rtls_model_srv_status},
    BT_MESH_MODEL_OP_END,
};

static struct bt_mesh_model_pub rtls_cli_pub;
static struct bt_mesh_model_pub rtls_srv_pub;

struct bt_mesh_model model_rtls[] = {
    BT_MESH_MODEL(BT_MESH_MODEL_ID_CLI, rtls_cli_op, &rtls_cli_pub, NULL),
    BT_MESH_MODEL(BT_MESH_MODEL_ID_SRV, rtls_srv_op, &rtls_srv_pub, NULL),
};

static struct dpl_task g_rtls_task;
static dpl_stack_t g_task_rtls_stack[MYNEWT_VAL(APP_RTLS_TASK_STACK_SIZE)];
static struct dpl_task g_loca_task;
static dpl_stack_t g_task_loca_stack[MYNEWT_VAL(APP_LOCA_TASK_STACK_SIZE)];
static struct dpl_mqueue mqueue_net_to_ble;
static struct dpl_eventq eventq_net_to_ble;

static void
process_net_to_ble_queue(struct dpl_event *ev)
{
    struct dpl_mbuf *om;
    struct dpl_mqueue *mqueue = (struct dpl_mqueue *)dpl_event_get_arg(ev);
    while ((om = dpl_mqueue_get(mqueue)) != NULL) {
        dpl_mbuf_free_chain(om);
    }
}

static void *
task_rtls_func(void *arg){
    while (1) {
        dpl_eventq_run(&eventq_net_to_ble);
    }
    return NULL;
}

static void *
task_loca_func(void *arg){

    struct bt_mesh_model *model = &model_rtls[0];
    struct bt_mesh_model_pub *pub =  model->pub;

    while (1) {
        dpl_time_delay(dpl_time_ms_to_ticks32(1000));
        if (pub->addr == BT_MESH_ADDR_UNASSIGNED) continue;

        bt_mesh_model_msg_init(pub->msg, BT_MESH_MODEL_OP_STATUS);

        msg_rtls_t msg_rtls = {
            .type = MAVLINK_MSG_ID_LOCATION,
            .dstsrc = 0xABCD,
            .location_x = 0.123,
            .location_y = 123.456,
            .location_z = 987.654
        };

        msg_prepr_rtls(pub->msg, &msg_rtls);

        int err = bt_mesh_model_publish(model);
        if (err) {
            printk("bt_mesh_model_publish err %d\n", err);
        }

    }
    return NULL;
}

void 
get_net_to_ble_mqueue_eventq(struct dpl_mqueue **mqueue, struct dpl_eventq **eventq){
    *mqueue = &mqueue_net_to_ble;
    *eventq = &eventq_net_to_ble;
}

void 
model_rtls_init(){
    int rc;

    dpl_mqueue_init(&mqueue_net_to_ble, process_net_to_ble_queue, &mqueue_net_to_ble);
    dpl_eventq_init(&eventq_net_to_ble);

    dpl_task_init(&g_rtls_task, "ma_rtls",
                task_rtls_func,
                NULL,
                MYNEWT_VAL(APP_RTLS_TASK_PRIORITY), 
                OS_WAIT_FOREVER,
                g_task_rtls_stack,
                MYNEWT_VAL(APP_RTLS_TASK_STACK_SIZE));

    dpl_task_init(&g_loca_task, "ma_loca",
                task_loca_func,
                NULL,
                MYNEWT_VAL(APP_LOCA_TASK_PRIORITY), 
                OS_WAIT_FOREVER,
                g_task_loca_stack,
                MYNEWT_VAL(APP_LOCA_TASK_STACK_SIZE));
    
    struct bt_mesh_model *model = &model_rtls[0];
    model->pub->msg = NET_BUF_SIMPLE(1+sizeof(msg_rtls_t));

    rc = dpl_mempool_init(&g_mbuf_mempool, MBUF_NUM_MBUFS,
                          MBUF_MEMBLOCK_SIZE, &g_mbuf_buffer[0], "mbuf_pool");
    assert(rc == 0);

    rc = dpl_mbuf_pool_init(&g_mbuf_pool, &g_mbuf_mempool, MBUF_MEMBLOCK_SIZE,
                           MBUF_NUM_MBUFS);
    assert(rc == 0);
}