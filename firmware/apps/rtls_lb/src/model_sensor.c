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

#include <rtls_lb/model.h>

static struct bt_mesh_model_pub gen_onoff_pub;

static void gen_onoff_status(struct bt_mesh_model *model,
                             struct bt_mesh_msg_ctx *ctx)
{
    console_printf("#mesh-level STATUS\n");
}

static void gen_onoff_get(struct bt_mesh_model *model,
              struct bt_mesh_msg_ctx *ctx,
              struct os_mbuf *buf)
{
    gen_onoff_status(model, ctx);
}

static void gen_onoff_set(struct bt_mesh_model *model,
              struct bt_mesh_msg_ctx *ctx,
              struct os_mbuf *buf)
{
    console_printf("#mesh-onoff SET\n");
    gen_onoff_status(model, ctx);
}

static void gen_onoff_set_unack(struct bt_mesh_model *model,
                struct bt_mesh_msg_ctx *ctx,
                struct os_mbuf *buf)
{
    console_printf("#mesh-onoff SET-UNACK\n");
}

static const struct bt_mesh_model_op gen_onoff_op[] = {
    { BT_MESH_MODEL_OP_2(0x82, 0x01), 0, gen_onoff_get },
    { BT_MESH_MODEL_OP_2(0x82, 0x02), 2, gen_onoff_set },
    { BT_MESH_MODEL_OP_2(0x82, 0x03), 2, gen_onoff_set_unack },
    BT_MESH_MODEL_OP_END,
};

struct bt_mesh_model model_sensor[1] = {
    BT_MESH_MODEL(BT_MESH_MODEL_ID_SENSOR_SRV, gen_onoff_op, &gen_onoff_pub, NULL),
};