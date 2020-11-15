#ifndef _MESH_MSG_H_
#define _MESH_MSG_H_

#include <stdint.h>
#include <dpl/dpl.h>

/* BLE */
#include "nimble/ble.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "mesh/glue.h"

#if __GNUC__ >= 9
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"
#endif
#include <rtls_mesh/gateway/mavlink/protocol/mavlink.h>

typedef struct _msg_onoff_t
{
    uint8_t value;
}__attribute__((__packed__, aligned(1))) msg_onoff_t;
void msg_prepr_onoff(struct os_mbuf *os_mbuf, msg_onoff_t *msg);
void msg_parse_onoff(struct os_mbuf *os_mbuf, msg_onoff_t *msg);

typedef struct _msg_rtls_t
{
    uint8_t msg_type;
    uint8_t node_type;
    uint16_t dstsrc;
    union{
        struct
        {
            float location_x;
            float location_y;
            float location_z;
        };
        struct
        {
            uint8_t value;
        };
    };
}__attribute__((__packed__, aligned(1))) msg_rtls_t;
void msg_prepr_rtls(struct os_mbuf *mbuf, msg_rtls_t *msg);
void msg_parse_rtls(struct os_mbuf *mbuf, msg_rtls_t *msg);

typedef struct _msg_rtls_header_t
{
    uint8_t cmd;
}__attribute__((__packed__, aligned(1))) msg_rtls_header_t;
#endif