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

#include "rtls_sw/ble_mesh.h"
#include "rtls_sw/model.h"

static const uint8_t g_dev_uuid[16] = MYNEWT_VAL(BLE_MESH_DEV_UUID);

/* Element definition */
static struct bt_mesh_elem g_elements[] = {
    BT_MESH_ELEM(0, model_root, model_vnd),
    BT_MESH_ELEM(0, model_sw, BT_MESH_MODEL_NONE),
};

/* Node definition */
static const struct bt_mesh_comp g_comp = {
    .cid = CID_VENDOR,
    .elem = g_elements,
    .elem_count = ARRAY_SIZE(g_elements)
};

static int output_number(bt_mesh_output_action_t action, uint32_t number)
{
    console_printf("OOB Number: %lu\n", number);

    return 0;
}

static void prov_complete(u16_t net_idx, u16_t addr)
{
    console_printf("Local node provisioned, primary address 0x%04x\n", addr);
}

static const struct bt_mesh_prov prov = {
    .uuid = g_dev_uuid,
    .output_size = 4,
    .output_actions = BT_MESH_DISPLAY_NUMBER | BT_MESH_BEEP | BT_MESH_VIBRATE | BT_MESH_BLINK,
    .output_number = output_number,
    .complete = prov_complete,
};

static void
blemesh_on_reset(int reason)
{
    BLE_HS_LOG(ERROR, "Resetting state; reason=%d\n", reason);
}

static void
blemesh_on_sync(void)
{
    int err;
    ble_addr_t addr;

    console_printf("Bluetooth initialized\n");

    /* Use NRPA */
    err = ble_hs_id_gen_rnd(1, &addr);
    assert(err == 0);
    err = ble_hs_id_set_rnd(addr.val);
    assert(err == 0);

    err = bt_mesh_init(addr.type, &prov, &g_comp);
    if (err) {
        console_printf("Initializing mesh failed (err %d)\n", err);
        return;
    }

    console_printf("Mesh initialized\n");

    if (IS_ENABLED(CONFIG_SETTINGS)) {
        settings_load();
    }

    if (bt_mesh_is_provisioned()) {
        printk("Mesh network restored from flash\n");
    }
}

void ble_mesh_init(){

    /* Initialize the NimBLE host configuration. */
    ble_hs_cfg.reset_cb = blemesh_on_reset;
    ble_hs_cfg.sync_cb = blemesh_on_sync;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;
}