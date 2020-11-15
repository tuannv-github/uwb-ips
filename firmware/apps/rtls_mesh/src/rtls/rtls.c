#include <rtls_mesh/rtls/rtls.h>

#if __GNUC__ >= 9
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"
#endif
#include <rtls_mesh/gateway/mavlink/protocol/mavlink.h>

float g_location[3] = {0.2, 123.456, 567};
uint16_t g_address = 0x1234;
uint16_t g_ntype = ANCHOR;

void rtls_get_location(float *x, float *y, float *z){
    *x = g_location[0];
    *y = g_location[1];
    *z = g_location[2];
}

void rtls_set_location(float x, float y, float z){
    g_location[0] = x;
    g_location[1] = y;
    g_location[2] = z;
}

void rtls_get_address(uint16_t *address){
    *address = g_address;
}

void rtls_get_ntype(uint8_t *ntype){
    *ntype = g_ntype;
}