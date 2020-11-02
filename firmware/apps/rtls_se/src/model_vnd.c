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

#include <rtls_sw/model.h>

static struct bt_mesh_model_pub vnd_model_pub;

static void vnd_model_recv(struct bt_mesh_model *model,
                           struct bt_mesh_msg_ctx *ctx,
                           struct os_mbuf *buf)
{
    struct os_mbuf *msg = NET_BUF_SIMPLE(3);

    console_printf("#vendor-model-recv\n");

    console_printf("data:%s len:%d\n", bt_hex(buf->om_data, buf->om_len),
                   buf->om_len);

    bt_mesh_model_msg_init(msg, BT_MESH_MODEL_OP_3(0x01, CID_VENDOR));
    os_mbuf_append(msg, buf->om_data, buf->om_len);

    if (bt_mesh_model_send(model, ctx, msg, NULL, NULL)) {
        console_printf("#vendor-model-recv: send rsp failed\n");
    }

    os_mbuf_free_chain(msg);
}

static const struct bt_mesh_model_op vnd_model_op[] = {
    { BT_MESH_MODEL_OP_3(0x01, CID_VENDOR), 0, vnd_model_recv },
    BT_MESH_MODEL_OP_END,
};

struct bt_mesh_model model_vnd[1] = {
    BT_MESH_MODEL_VND(CID_VENDOR, BT_MESH_MODEL_ID_GEN_ONOFF_SRV, vnd_model_op, &vnd_model_pub, NULL),
};
