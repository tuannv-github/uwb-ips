#ifndef _RTLS_H_
#define _RTLS_H_

#include <stdint.h>

void rtls_get_location(float *x, float *y, float *z);
void rtls_set_location(float x, float y, float z);

void rtls_get_address(uint16_t *address);

void rtls_get_ntype(uint8_t *ntype);
void rtls_set_ntype(uint8_t ntype);

void rtls_init();

#endif