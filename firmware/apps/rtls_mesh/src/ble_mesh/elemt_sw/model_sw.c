#include <assert.h>
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

#include <rtls_sw/ble_mesh/model.h>
#include <rtls_sw/ble_mesh/ble_mesh.h>
#include <button/button.h>

/* Model Operation Codes */
#define BT_MESH_MODEL_OP_GEN_ONOFF_GET			BT_MESH_MODEL_OP_2(0x82, 0x01)
#define BT_MESH_MODEL_OP_GEN_ONOFF_SET			BT_MESH_MODEL_OP_2(0x82, 0x02)
#define BT_MESH_MODEL_OP_GEN_ONOFF_SET_UNACK	BT_MESH_MODEL_OP_2(0x82, 0x03)
#define BT_MESH_MODEL_OP_GEN_ONOFF_STATUS		BT_MESH_MODEL_OP_2(0x82, 0x04)

typedef struct _app_sw_t{
    struct dpl_eventq eventq;
    struct dpl_event event;                       
    struct dpl_task task;            
    dpl_stack_t task_stack[MYNEWT_VAL(APP_SW_TASK_STACK_SZ)];
    int button;
    int state;
    struct bt_mesh_model *model;
}app_sw_t;

static app_sw_t app_sw;

static void gen_onoff_set(struct bt_mesh_model *model,
              struct bt_mesh_msg_ctx *ctx,
              struct os_mbuf *buf)
{
    console_printf("#mesh-level STATUS\n");
}


static const struct bt_mesh_model_op gen_onoff_op[] = {
    { BT_MESH_MODEL_OP_GEN_ONOFF_SET, 3, gen_onoff_set},
    BT_MESH_MODEL_OP_END,
};

static struct bt_mesh_model_pub gen_onoff_pub;

struct bt_mesh_model model_sw[] = {
    BT_MESH_MODEL(BT_MESH_MODEL_ID_GEN_ONOFF_CLI, gen_onoff_op, &gen_onoff_pub, NULL),
};

void button_handler(int pin, void *arg){
    console_printf("BTN click\n");

    app_sw_t *app_sw = (app_sw_t *)arg;
    
    static bool onoff = true;
    onoff = !onoff;

    app_sw->state = onoff;

    dpl_eventq_put(&app_sw->eventq, &app_sw->event);
}

static void *
task(void *arg)
{
    app_sw_t *inst = arg;
    while (1) {
        dpl_eventq_run(&inst->eventq);
    }
    return NULL;
}

static void
app_event_fn(struct dpl_event *ev){
    app_sw_t *app_sw = (app_sw_t *)dpl_event_get_arg(ev);

    struct bt_mesh_model *model_sw = app_sw->model;
	struct bt_mesh_model_pub *pub_cli =  model_sw->pub;

	if (pub_cli->addr == BT_MESH_ADDR_UNASSIGNED) {
		return;
	}

	printf("publish to 0x%04x onoff %d\n", pub_cli->addr, app_sw->state);

	bt_mesh_model_msg_init(pub_cli->msg, BT_MESH_MODEL_OP_GEN_ONOFF_SET);
	net_buf_simple_add_u8(pub_cli->msg, app_sw->state);

	int err = bt_mesh_model_publish(model_sw);
	if (err) {
		printk("bt_mesh_model_publish err %d\n", err);
	}
}

void model_sw_init(){
    button_init(BUTTON_2, button_handler, &app_sw, HAL_GPIO_TRIG_FALLING, HAL_GPIO_PULL_UP);
    model_sw->pub->msg = NET_BUF_SIMPLE(3);

    app_sw.model = &model_sw[0];

    if (!dpl_eventq_inited(&app_sw.eventq))
    {
        dpl_event_init(&app_sw.event, app_event_fn, &app_sw);
        dpl_eventq_init(&app_sw.eventq);
        dpl_task_init(&app_sw.task, "app_sw",
                      task,
                      (void *)&app_sw,
                      MYNEWT_VAL(APP_SW_TASK_PRIORITY), 
                      DPL_WAIT_FOREVER,
                      app_sw.task_stack,
                      MYNEWT_VAL(APP_SW_TASK_STACK_SZ));
    }
}