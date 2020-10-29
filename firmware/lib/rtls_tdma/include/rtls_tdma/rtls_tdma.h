
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

typedef struct _rtls_tdma_anchor_t{
    uint16_t addr;
    float location_x;
    float location_y;
    float location_z;
    uint8_t listen_cnt;
}rtls_tdma_anchor_t;

typedef struct _rtls_tdma_instance_t {
    tdma_instance_t *tdma;                   //!< Pointer to tdma instant
    struct uwb_dev * dev_inst;               //!< UWB device instance
    struct uwb_mac_interface umi;            //!< Mac interface

    rts_t cstate;                            //!< Current rtls tdma state
    uint16_t my_slot;                        //!< Current rtls tdma slot
    struct dpl_sem sem;                      //!< Structure containing os semaphores

    bool joint_poll_recved;
    bool joint_reqt_recved;
    uint16_t channel;
    uint16_t reqt_poll_src;
    uint8_t reqt_channel;
    uint8_t seqno;
    
    rtls_tdma_anchor_t anchors[MYNEWT_VAL(UWB_BCN_SLOT_MAX)];
}rtls_tdma_instance_t;

#define RT_BROADCAST_ADDR   0xFFFF

typedef struct _ieee_std_frame_hdr_t{
    uint16_t fctrl;             //!< Frame control (0x8841 to indicate a data frame using 16-bit addressing)
    uint8_t seq_num;            //!< Sequence number, incremented for each new frame
    uint16_t PANID;             //!< PANID
    uint16_t dst_address;       //!< Destination address
    uint16_t src_address;       //!< Source address
}__attribute__((__packed__,aligned(1))) ieee_std_frame_hdr_t;

typedef enum{
    RT_BCN_NORM_MSG = 0x11,
    RT_SVC_POLL_MSG,
    RT_BCN_RESP_MSG,
    RT_SVC_REQT_MSG,
    RT_BCN_ACPT_MSG
}rt_msg_type_t;

typedef struct _rt_bcn_norm_payload_t{
    uint8_t msg_type;
    uint8_t len;
    float location_x;
    float location_y;
    float location_z;
}__attribute__((__packed__,aligned(1))) rt_bcn_norm_payload_t;

typedef struct _rt_svc_poll_payload_t{
    uint8_t msg_type;
    uint8_t len;
}__attribute__((__packed__,aligned(1))) rt_svc_poll_payload_t;

typedef struct _rt_bcn_resp_payload_t{
    uint8_t msg_type;
    uint8_t len;
    uint16_t channel;
}__attribute__((__packed__,aligned(1))) rt_bcn_resp_payload_t;

typedef struct _rt_svc_reqt_payload_t{
    uint8_t msg_type;
    uint8_t len;
    uint16_t channel;
}__attribute__((__packed__,aligned(1))) rt_svc_reqt_payload_t;

typedef struct _rt_bcn_acpt_payload_t{
    uint8_t msg_type;
    uint8_t len;
    uint16_t channel;
}__attribute__((__packed__,aligned(1))) rt_bcn_acpt_payload_t;

void rtls_tdma_start(rtls_tdma_instance_t *rtls_tdma_instance, struct uwb_dev* udev);

#endif