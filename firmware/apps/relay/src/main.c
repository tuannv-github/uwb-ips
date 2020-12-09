/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <assert.h>
#include "os/mynewt.h"
#include "mesh/mesh.h"
#include "console/console.h"
#include "hal/hal_system.h"
#include "hal/hal_gpio.h"
#include "bsp/bsp.h"
#include "shell/shell.h"
#include <controller/ble_hw.h>
#include "mesh/mesh.h"
#include "nimble/ble.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "mesh/glue.h"

#define CID_VENDOR                              0x05C3

static uint8_t g_dev_uuid[16] = MYNEWT_VAL(BLE_MESH_DEV_UUID);

static struct bt_mesh_cfg_srv cfg_srv = {
    .relay = BT_MESH_RELAY_ENABLED,
    .beacon = BT_MESH_BEACON_ENABLED,
    .frnd = BT_MESH_FRIEND_DISABLED,
    .gatt_proxy = BT_MESH_GATT_PROXY_ENABLED,

    /* 7 transmissions with 20ms interval */
    .default_ttl = 7,
    .net_transmit = BT_MESH_TRANSMIT(7, 20),
    .relay_retransmit = BT_MESH_TRANSMIT(7, 20),
};

struct bt_mesh_model model_root[] = {
    BT_MESH_MODEL_CFG_SRV(&cfg_srv)
};

/* Element definition */
static struct bt_mesh_elem g_elements[] = {
    BT_MESH_ELEM(0, model_root, BT_MESH_MODEL_NONE),
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
    printf("Resetting state; reason=%d\n", reason);
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

    ble_hw_get_public_addr(&addr);

    printf("HW address: ");
    for(int i=0; i<6; i++){
        printf("0x%02x ", addr.val[i]);
    }
    printf(" \n");

    uint8_t *uuid = (uint8_t *)prov.uuid;
    memcpy(uuid, addr.val, 6);

    printf("UUID: ");
    for(int i=0; i<16; i++){
        printf("0x%02x ", prov.uuid[i]);
    }
    printf(" \n");
    
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

int
main(int argc, char **argv)
{
    sysinit();
    
    /* Initialize the NimBLE host configuration. */
    ble_hs_cfg.reset_cb = blemesh_on_reset;
    ble_hs_cfg.sync_cb = blemesh_on_sync;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    while (1) {
        os_eventq_run(os_eventq_dflt_get());
    }
    return 0;
}
