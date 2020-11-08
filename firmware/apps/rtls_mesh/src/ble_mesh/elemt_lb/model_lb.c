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

/* Model Operation Codes */
#define BT_MESH_MODEL_OP_GEN_ONOFF_GET			BT_MESH_MODEL_OP_2(0x82, 0x01)
#define BT_MESH_MODEL_OP_GEN_ONOFF_SET			BT_MESH_MODEL_OP_2(0x82, 0x02)
#define BT_MESH_MODEL_OP_GEN_ONOFF_SET_UNACK	BT_MESH_MODEL_OP_2(0x82, 0x03)
#define BT_MESH_MODEL_OP_GEN_ONOFF_STATUS		BT_MESH_MODEL_OP_2(0x82, 0x04)

static void gen_onoff_set(struct bt_mesh_model *model,
              struct bt_mesh_msg_ctx *ctx,
              struct os_mbuf *buf)
{
    console_printf("#mesh-level STATUS\n");
    int val = net_buf_simple_pull_u8(buf);
	printk("addr 0x%02x state 0x%02x\n", bt_mesh_model_elem(model)->addr, val);
    hal_gpio_init_out(LED_1, val);
}

static const struct bt_mesh_model_op gen_onoff_op[] = {
    { BT_MESH_MODEL_OP_GEN_ONOFF_SET, 0, gen_onoff_set},
    BT_MESH_MODEL_OP_END,
};

static struct bt_mesh_model_pub gen_onoff_pub;

struct bt_mesh_model model_lb[] = {
    BT_MESH_MODEL(BT_MESH_MODEL_ID_GEN_ONOFF_SRV, gen_onoff_op, &gen_onoff_pub, NULL),
};

void model_lb_init(){
    hal_gpio_init_out(LED_1, 1);
}