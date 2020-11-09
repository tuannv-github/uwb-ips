#ifndef _MODEL_H_
#define _MODEL_H_

/* BLE */
#include "nimble/ble.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "mesh/glue.h"

#define CID_VENDOR          0x05C3

/* Model Operation Codes */
#define BT_MESH_MODEL_OP_GEN_ONOFF_GET			BT_MESH_MODEL_OP_2(0x82, 0x01)
#define BT_MESH_MODEL_OP_GEN_ONOFF_SET			BT_MESH_MODEL_OP_2(0x82, 0x02)
#define BT_MESH_MODEL_OP_GEN_ONOFF_SET_UNACK	BT_MESH_MODEL_OP_2(0x82, 0x03)
#define BT_MESH_MODEL_OP_GEN_ONOFF_STATUS		BT_MESH_MODEL_OP_2(0x82, 0x04)

extern struct bt_mesh_model model_root[3];
extern struct bt_mesh_model model_vnd[1];

extern struct bt_mesh_model model_sw[1];
extern struct bt_mesh_model model_lb[1];

void model_root_init();
void model_sw_init();
void model_lb_init();

#endif