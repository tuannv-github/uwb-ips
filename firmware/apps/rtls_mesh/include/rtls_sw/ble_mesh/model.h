#ifndef _MODEL_H_
#define _MODEL_H_

/* BLE */
#include "nimble/ble.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "mesh/glue.h"

#define CID_VENDOR          0x05C3
#define STANDARD_TEST_ID    0x00
#define TEST_ID             0x01
#define FAULT_ARR_SIZE      2

extern struct bt_mesh_model model_root[3];
extern struct bt_mesh_model model_vnd[1];

struct bt_mesh_model model_sw[1];

void model_sw_init();

#endif