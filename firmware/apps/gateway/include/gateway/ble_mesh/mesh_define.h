#ifndef _MESH_DEFINE_H_
#define _MESH_DEFINE_H_

#include "mesh/mesh.h"
#include "nimble/ble.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "mesh/glue.h"

#define CID_VENDOR                              0x05C3

#define BT_MESH_MODEL_OP_GEN_ONOFF_GET			BT_MESH_MODEL_OP_2(0x82, 0x01)
#define BT_MESH_MODEL_OP_GEN_ONOFF_SET			BT_MESH_MODEL_OP_2(0x82, 0x02)
#define BT_MESH_MODEL_OP_GEN_ONOFF_SET_UNACK	BT_MESH_MODEL_OP_2(0x82, 0x03)
#define BT_MESH_MODEL_OP_GEN_ONOFF_STATUS		BT_MESH_MODEL_OP_2(0x82, 0x04)

#define BT_MESH_MODEL_ID_RTLS                   0x1000
#define BT_MESH_MODEL_OP_GET			        BT_MESH_MODEL_OP_1(0x01)
#define BT_MESH_MODEL_OP_SET			        BT_MESH_MODEL_OP_1(0x02)
#define BT_MESH_MODEL_OP_SET_UNACK	            BT_MESH_MODEL_OP_1(0x03)
#define BT_MESH_MODEL_OP_STATUS		            BT_MESH_MODEL_OP_1(0x04)

#endif