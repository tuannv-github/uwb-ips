
#ifndef _RTLS_TDMA_H_
#define _RTLS_TDMA_H_

#include <uwb/uwb.h>
#include <tdma/tdma.h>

typedef struct _rtls_tdma_instance_t {
    struct uwb_dev *dev_inst;                //!< Pointer to associated uwb_dev
    tdma_instance_t *tdma;
    struct uwb_mac_interface cbs;
}rtls_tdma_instance_t;

void rtls_tdma_start(rtls_tdma_instance_t *rtls_tdma_instance, struct uwb_dev* udev);

#endif