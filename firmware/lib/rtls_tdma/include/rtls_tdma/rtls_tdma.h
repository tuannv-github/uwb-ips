
#ifndef _RTLS_TDMA_H_
#define _RTLS_TDMA_H_

#include <uwb/uwb.h>
#include <tdma/tdma.h>

typedef enum{
    RTS_JOINT_LIST,
    RTS_JOINT_PREP,
    RTS_JOINT_POLL,
    RTS_JOINT_RESP,
    RTS_JOINT_REQT,
    RTS_JOINT_JTED  
}rts_t;

typedef struct _rtls_tdma_instance_t {
    tdma_instance_t *tdma;                   //!< Pointer to tdma instant
    rts_t cstate;                            //!< Current rtls tdma state
    uint16_t my_slot;                        //!< Current rtls tdma slot
}rtls_tdma_instance_t;

void rtls_tdma_start(rtls_tdma_instance_t *rtls_tdma_instance, struct uwb_dev* udev);

#endif