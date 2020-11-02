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

static
int loc_data_update(struct bt_mesh_model *mod){
    console_printf("#mesh loc_data_update\n");
    return 0;
}
static struct bt_mesh_model_pub gen_onoff_pub = {
    .period = BT_MESH_PUB_PERIOD_SEC(1),
    .update = loc_data_update,
    .ttl = BT_MESH_TTL_DEFAULT,
    .msg = (struct os_mbuf *)100,
};

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
    { BT_MESH_MODEL_OP_2(0x8A, 0x01), 0, gen_onoff_get },
    { BT_MESH_MODEL_OP_2(0x8A, 0x02), 2, gen_onoff_set },
    { BT_MESH_MODEL_OP_2(0x8A, 0x03), 2, gen_onoff_set_unack },
    BT_MESH_MODEL_OP_END,
};

struct bt_mesh_model model_light[1] = {
    BT_MESH_MODEL(BT_MESH_MODEL_ID_GEN_ONOFF_SRV, gen_onoff_op, &gen_onoff_pub, NULL),
};