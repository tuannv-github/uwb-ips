#include <os/mynewt.h>

#include <nimble/ble.h>
#include <host/ble_hs.h>
#include <mesh/mesh.h>

#define CID_VENDOR 0x05C3

static const uint8_t net_key[16] = {
	0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
	0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
};
static const uint8_t dev_key[16] = {
	0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
	0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
};
static const uint8_t app_key[16] = {
	0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
	0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
};

static const uint16_t net_idx;
static const uint16_t app_idx;
static const uint32_t iv_index;
static uint8_t flags;
static uint16_t addr = MYNEWT_VAL(ELEMENT_ADDRESS);
static uint8_t g_dev_uuid[16] = MYNEWT_VAL(BLE_MESH_DEV_UUID);

static struct bt_mesh_cfg_srv cfg_srv = {
    .relay = BT_MESH_RELAY_DISABLED,
    .beacon = BT_MESH_BEACON_ENABLED,
    .frnd = BT_MESH_FRIEND_DISABLED,
    .gatt_proxy = BT_MESH_GATT_PROXY_NOT_SUPPORTED,
    .default_ttl = 7,
    .net_transmit = BT_MESH_TRANSMIT(2, 20),
    .relay_retransmit = BT_MESH_TRANSMIT(2, 20),
};

static struct bt_mesh_model root_models[] = {
    BT_MESH_MODEL_CFG_SRV(&cfg_srv)
};

static struct bt_mesh_elem elements[] = {
    BT_MESH_ELEM(0, root_models, BT_MESH_MODEL_NONE),
};

static const struct bt_mesh_comp comp = {
    .cid = CID_VENDOR,
    .elem = elements,
    .elem_count = ARRAY_SIZE(elements),
};

static int output_number(bt_mesh_output_action_t action, uint32_t number)
{
    printf("OOB Number: %lu\n", number);
    return 0;
}

static void prov_complete(u16_t net_idx, u16_t addr)
{
    printf("Local node provisioned, primary address 0x%04x\n", addr);
}

static const struct bt_mesh_prov prov = {
    .uuid = g_dev_uuid,
    .output_size = 4,
    .output_actions = BT_MESH_DISPLAY_NUMBER,
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

    printf("Bluetooth initialized\n");

    printf("UUID: ");
    for(int i=0; i<16; i++){
        printf("0x%02x ", prov.uuid[i]);
    }
    printf(" \n");

    err = bt_mesh_init(BLE_ADDR_PUBLIC, &prov, &comp);
    if (err) {
        printf("Initializing mesh failed (err %d)\n", err);
        return;
    }
    printf("Mesh initialized\n");

    err = bt_mesh_provision(net_key, net_idx, flags, iv_index, addr, dev_key);
    if (err == -EALREADY) {
        printf("Using stored settings\n");
    } else if (err) {
        printf("Provisioning failed (err %d)\n", err);
        return;
    } else {
        printf("Provisioning completed\n");
    }
}

int main(int argc, char **argv){
    sysinit();

    /* Initialize the NimBLE host configuration. */
    ble_hs_cfg.reset_cb = blemesh_on_reset;
    ble_hs_cfg.sync_cb = blemesh_on_sync;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    while (1) {
        os_eventq_run(os_eventq_dflt_get());
    }
}