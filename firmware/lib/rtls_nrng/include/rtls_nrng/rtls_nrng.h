#ifndef _RTLS_NRNG_H_
#define _RTLS_NRNG_H_

#include <stdint.h>

typedef struct ieee_std_frame_hdr{
    uint16_t    fctrl;            
    uint8_t     seq_num;
    uint16_t    PANID; 
    uint16_t    dst_address;
    uint16_t    src_address;
}__attribute__((__packed__,aligned(1))) ieee_std_frame_hdr_t;

typedef enum{
    RTLS_NRNG_P0,
    RTLS_NRNG_P1,
    RTLS_NRNG_P2,
    RTLS_NRNG_P3
}rtls_nrng_msg_id_t;

typedef struct rtls_nrng_msg{
    ieee_std_frame_hdr_t header;
    rtls_nrng_msg_id_t rtls_nrng_id;
    uint64_t recv_timestamp;
    uint64_t send_timestamp;
}__attribute__((__packed__,aligned(1))) rtls_nrng_msg_t;

typedef struct rtls_nrng{
    rtls_nrng_msg_t rtls_nrng_msg[4][4];
}rtls_nrng_t;

void rtls_nrng_listen(rtls_nrng_t *rtls_nrng);
void rtls_nrng_requst(rtls_nrng_t *rtls_nrng);

#endif