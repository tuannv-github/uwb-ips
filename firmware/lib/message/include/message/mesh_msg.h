#ifndef _MESH_MSG_H_
#define _MESH_MSG_H_

#include <stdint.h>
#include <dpl/dpl.h>

#if MYNEWT_VAL(BLE_MESH)

/* BLE */
#include "nimble/ble.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "mesh/glue.h"

#if __GNUC__ >= 9
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"
#endif
#include <message/mavlink/protocol/mavlink.h>

typedef struct _msg_onoff_t
{
    uint8_t value;
}__attribute__((__packed__, aligned(1))) msg_onoff_t;
void msg_prepr_onoff(struct os_mbuf *os_mbuf, msg_onoff_t *msg);
void msg_parse_onoff(struct os_mbuf *os_mbuf, msg_onoff_t *msg);

typedef struct _msg_rtls_t
{
    struct _msg_rtls_header_t
    {
        uint8_t type;
        uint16_t dstsrc;
        uint32_t opcode;
    };
    union{
        struct _msg_rtls_location_t
        {
            uint8_t node_type;
            float location_x;
            float location_y;
            float location_z;
        };
        struct _msg_rtls_onoff_t
        {
            uint8_t value;
        };
        struct _msg_rtls_distance_t{
            uint16_t anchor;
            float distance;
        };
    };
}__attribute__((__packed__, aligned(1))) msg_rtls_t;

void msg_prepr_rtls(struct os_mbuf **mbuf, msg_rtls_t *msg);
void msg_parse_rtls(struct os_mbuf *mbuf, msg_rtls_t *msg);

void msg_prepr_rtls_pipe(struct os_mbuf *mbuf, msg_rtls_t *msg);
void msg_parse_rtls_pipe(struct os_mbuf *mbuf, msg_rtls_t *msg);

void msg_print_rtls(msg_rtls_t *msg);

#endif

#endif