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

#define BT_MESH_MODEL_ID_LOC_DATA_SRV   0x5010
#define BT_MESH_MODEL_ID_LOC_CONF_SRV   0x5011
#define BT_MESH_MODEL_ID_LOC_CONF_CLI   0x5012
#define BT_MESH_MODEL_ID_LOC_DIST_SRV   0x5013

#define LOC_DATA_ADDR                   0xA000

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

static
int loc_data_update(struct bt_mesh_model *mod){
    console_printf("#mesh loc_data_update\n");
    return 0;
}

static void loc_data_get(struct bt_mesh_model *model,
              struct bt_mesh_msg_ctx *ctx,
              struct os_mbuf *buf)
{
    console_printf("#mesh loc_data_get\n");
    loc_data_update(model);
}

static struct bt_mesh_model_pub g_loc_data_pub = {
    .period = BT_MESH_PUB_PERIOD_SEC(1),
    .update = loc_data_update,
    .ttl = BT_MESH_TTL_DEFAULT,
};

void
loc_pub_init(void)
{
    g_loc_data_pub.msg  = NET_BUF_SIMPLE(1 + 3);
}

static const struct bt_mesh_model_op g_loc_data_op[] = {
    { BT_MESH_MODEL_OP_2(0x82, 0x01), 0, loc_data_get},
    BT_MESH_MODEL_OP_END,
};

struct bt_mesh_model model_location[4] = {
    BT_MESH_MODEL(BT_MESH_MODEL_ID_LOC_DATA_SRV, g_loc_data_op, &gen_onoff_pub, NULL),
    BT_MESH_MODEL(BT_MESH_MODEL_ID_LOC_CONF_SRV, gen_onoff_op, &gen_onoff_pub, NULL),
    BT_MESH_MODEL(BT_MESH_MODEL_ID_LOC_CONF_CLI, gen_onoff_op, &gen_onoff_pub, NULL),
    BT_MESH_MODEL(BT_MESH_MODEL_ID_LOC_DIST_SRV, gen_onoff_op, &gen_onoff_pub, NULL),
};