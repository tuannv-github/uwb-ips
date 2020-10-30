
#ifndef _RTLS_TDMA_H_
#define _RTLS_TDMA_H_

#include <uwb/uwb.h>
#include <tdma/tdma.h>

typedef enum{
    RTS_JOINT_LIST,
    RTS_JOINT_REQT,
    RTS_JOINT_JTED  
}rts_t;

typedef struct _rtls_tdma_anchor_t{
    uint16_t addr;
    uint16_t slot;
    float location_x;
    float location_y;
    float location_z;

    uint8_t listen_cnt;
    bool accepted;
}rtls_tdma_anchor_t;

typedef struct _rtls_tdma_instance_t {
    tdma_instance_t *tdma;                   //!< Pointer to tdma instant
    struct uwb_dev * dev_inst;               //!< UWB device instance
    struct uwb_mac_interface umi;            //!< Mac interface

    rts_t cstate;                            //!< Current rtls tdma state
    uint16_t my_slot;                        //!< Current rtls tdma slot
    struct dpl_sem sem;                      //!< Structure containing os semaphores
    uint8_t seqno;

    bool        joint_reqt_recved;
    uint16_t    joint_reqt_src;
    uint16_t    joint_reqt_slot;
    uint8_t     joint_reqt_cnt;
    
    rtls_tdma_anchor_t anchors[MYNEWT_VAL(UWB_BCN_SLOT_MAX)+1];
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
    RT_LOCA_MSG = 0x11,
    RT_SLOT_MSG,
    RT_REQT_MSG,
    RT_ACPT_MSG
}rt_msg_type_t;

struct _msg_hdr_t{
    uint8_t msg_type;
    uint8_t len;
}__attribute__((__packed__,aligned(1))) msg_hdr_t;

typedef struct _rt_bcn_loca_t{
    struct _msg_hdr_t;
    struct _rt_loca_data_t{
        float location_x;
        float location_y;
        float location_z;
    }__attribute__((__packed__,aligned(1)));
}__attribute__((__packed__,aligned(1))) rt_loca_t;

typedef struct _rt_slot_t{
    struct _msg_hdr_t;
    struct _rt_slot_data_t
    {
        uint16_t slot;
    }__attribute__((__packed__,aligned(1)));
}__attribute__((__packed__,aligned(1))) rt_slot_t;

void rtls_tdma_start(rtls_tdma_instance_t *rtls_tdma_instance, struct uwb_dev* udev);

#endif