#ifndef _RTLS_H_
#define _RTLS_H_

#include <stdint.h>
#include <dpl/dpl.h>

#define ANCHOR_NUM  (MYNEWT_VAL(UWB_BCN_SLOT_MAX) + 1)
typedef struct{
    uint8_t updated[ANCHOR_NUM];
    uint16_t anchors[ANCHOR_NUM];
    union{
        float ranges[ANCHOR_NUM];
        uint32_t tofs[ANCHOR_NUM];
    };
}distance_t;

bool rtls_get_location(float *x, float *y, float *z);
void rtls_set_location(float x, float y, float z);

void rtls_get_address(uint16_t *address);

void rtls_get_ntype(uint8_t *ntype);
void rtls_set_ntype(uint8_t ntype);

void rtls_get_slot(uint8_t *slot);

distance_t *get_distances();

void rtls_init();

#endif