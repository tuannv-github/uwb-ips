
#ifndef _RTLS_TDMA_H_
#define _RTLS_TDMA_H_

#include <uwb/uwb.h>
#include <rtls_tdma/tdma.h>
#include <uwb_nrng/nrng.h>

typedef enum{
    RTS_JOINT_NONE,
    RTS_JOINT_LIST,
    RTS_JOINT_REQT,
    RTS_JOINT_JTED  
}rts_t;

typedef enum{
    RTR_ANCHOR = 1,
    RTR_TAG = 2
}rtr_t;

typedef uint64_t slot_map_t;

typedef struct _rtls_tdma_node_t{   

    uint16_t addr;              // Address of the node
    slot_map_t slot_map;        // Slot map of the node

    float location_x;       // Node location
    float location_y;       // Node location
    float location_z;       // Node location

    uint8_t bcn_cnt;        // Number of bcn message received from this node
    uint8_t timeout;        // Keep track if jointed node leaved the network

    bool available;         // This anchor available in this supper frame
    bool accepted;          // This anchor accept for my slot 
}rtls_tdma_node_t;

typedef struct _rtls_tdma_instance_t rtls_tdma_instance_t;

typedef void (*rtls_tdma_cb_t)(rtls_tdma_instance_t *rtls_tdma_instance, tdma_slot_t *tdma_slot);

struct _rtls_tdma_instance_t {
    tdma_instance_t *tdma;                      //!< Pointer to tdma instant
    struct uwb_dev *dev_inst;                   //!< UWB device instance
    struct uwb_mac_interface umi;               //!< Mac interface

    rts_t cstate;                               //!< Current rtls tdma state
    struct dpl_sem sem;                         //!< Structure containing os semaphores
    rtr_t role;                                 //!< RTLS TDMA role
    uint8_t seqno;
    uint16_t slot_idx;                          //!< slot_idx != 0 means I have slot already

    uint16_t slot_reqt;
    uint16_t slot_reqt_cntr;
    uint16_t slot_reqt_addr;
    
    rtls_tdma_cb_t rtls_tdma_cb;

    rtls_tdma_node_t nodes[MYNEWT_VAL(TDMA_NSLOTS)]; // nodes[0] save my node info
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
        uint8_t slot;
        float location_x;
        float location_y;
        float location_z;
 
    }__attribute__((__packed__,aligned(1)));
}__attribute__((__packed__,aligned(1))) rt_loca_t;

 struct _rt_slot_reqt_t{
    struct _msg_hdr_t;
    struct _rt_slot_reqt_data_t
    {
        uint8_t slot;
        uint8_t slot_reqt;
    }__attribute__((__packed__,aligned(1)));
}__attribute__((__packed__,aligned(1)));

typedef struct _rt_slot_reqt_t rt_slot_reqt_t;
typedef struct _rt_slot_reqt_data_t rt_slot_reqt_data_t;

typedef struct _rt_slot_reqt_t rt_slot_acpt_t;
typedef struct _rt_slot_reqt_data_t rt_slot_acpt_data_t;

struct _rt_slot_map_t{
    struct _msg_hdr_t;
    struct _rt_slot_map_data_t
    {
        uint8_t slot;
        slot_map_t slot_map;
    }__attribute__((__packed__,aligned(1)));
}__attribute__((__packed__,aligned(1)));

typedef struct _rt_slot_map_t rt_slot_map_t;
typedef struct _rt_slot_map_data_t rt_slot_map_data_t;

void rtls_tdma_start(rtls_tdma_instance_t *rtls_tdma_instance, struct uwb_dev* udev);

#define UWB_TX(rti, msg, msg_size, slot)                            \
    uint64_t dx_time = tdma_tx_slot_start(rti->tdma, slot);         \
    dx_time &= 0xFFFFFFFFFE00UL;                                    \
    uwb_write_tx(rti->dev_inst, msg, 0, msg_size);                  \
    uwb_write_tx_fctrl(rti->dev_inst, msg_size, 0);                 \
    uwb_set_wait4resp(rti->dev_inst, false);                        \
    uwb_set_delay_start(rti->dev_inst, dx_time);                    \
    if (uwb_start_tx(rti->dev_inst).start_tx_error)                 \
    {                                                               \
        printf("TX error %s:%d\n", __FILE__, __LINE__);             \
    }

#define UWB_RX_ERROR(rti)       (rti->dev_inst->status.start_rx_error)
#define UWB_RX_TIMEOUT(rti)     (rti->dev_inst->status.rx_timeout_error)

#endif