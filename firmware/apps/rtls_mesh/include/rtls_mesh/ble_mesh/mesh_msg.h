#ifndef _MESH_MSG_H_
#define _MESH_MSG_H_

#include <stdint.h>
#include <dpl/dpl.h>

/* BLE */
#include "nimble/ble.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "mesh/glue.h"

typedef struct _msg_onoff_t
{
    uint8_t value;
}__attribute__((__packed__, aligned(1))) msg_onoff_t;
void msg_prepr_onoff(struct os_mbuf *os_mbuf, msg_onoff_t *msg_onoff);
void msg_parse_onoff(struct os_mbuf *os_mbuf, msg_onoff_t *msg_onoff);

#endif