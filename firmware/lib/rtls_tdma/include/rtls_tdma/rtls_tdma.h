
#ifndef _RTLS_TDMA_H_
#define _RTLS_TDMA_H_

#include <uwb/uwb.h>
#include <tdma/tdma.h>
#include <uwb_rng/uwb_rng.h>

typedef enum{
    RTS_JOINT_NONE,
    RTS_JOINT_LIST,
    RTS_JOINT_REQT,
    RTS_JOINT_JTED  
}rts_t;

typedef enum{
    RTR_ANCHOR,
    RTR_TAG
}rtr_t;

typedef uint64_t slot_type_t;

typedef struct _rtls_tdma_node_t{   

    uint16_t addr;          // Address of the node
    slot_type_t slot;       // Slot map of the node

    float location_x;       // Node location
    float location_y;       // Node location
    float location_z;       // Node location

    uint8_t bcn_cnt;        // Number of bcn message received from this node
    uint8_t timeout;        // Keep track if jointed node leaved the network  
}rtls_tdma_node_t;

typedef struct _rtls_tdma_instance_t rtls_tdma_instance_t;

typedef void (*rtls_tdma_cb_t)(rtls_tdma_instance_t *rtls_tdma_instance, tdma_slot_t *tdma_slot);

struct _rtls_tdma_instance_t {
    tdma_instance_t *tdma;                      //!< Pointer to tdma instant
    struct uwb_dev *dev_inst;                   //!< UWB device instance
    struct uwb_mac_interface umi;               //!< Mac interface
    struct uwb_rng_instance *uri;               //!= UWB ranging instance
    
    rts_t cstate;                               //!< Current rtls tdma state
    uint16_t my_slot;                           //!< Current rtls tdma slot
    struct dpl_sem sem;                         //!< Structure containing os semaphores
    rtr_t role;                                 //!< RTLS TDMA role
    uint8_t seqno;

    bool        joint_reqt;
    uint16_t    joint_reqt_src;
    uint64_t    joint_reqt_slot;
    uint8_t     joint_reqt_cnt;
    
    rtls_tdma_cb_t rtls_tdma_cb;

    rtls_tdma_node_t nodes[MYNEWT_VAL(TDMA_NSLOTS)];
};

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
        uint64_t slot;
    }__attribute__((__packed__,aligned(1)));
}__attribute__((__packed__,aligned(1))) rt_slot_t;

void rtls_tdma_start(rtls_tdma_instance_t *rtls_tdma_instance, struct uwb_dev* udev);

#endif