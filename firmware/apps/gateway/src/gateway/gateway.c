#if __GNUC__ >= 9
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"
#endif

#include <dpl/dpl.h>
#include <os/mynewt.h>
#include <gateway/gateway/gateway.h>

#include <serial/serial.h>
#include <gateway/gateway/mavlink/protocol/mavlink.h>
#include <gateway/ble_mesh/mesh_msg.h>
#include <gateway/ble_mesh/mesh_if.h>

#define MBUF_PKTHDR_OVERHEAD    (sizeof(struct os_mbuf_pkthdr) + 0)
#define MBUF_MEMBLOCK_OVERHEAD  (sizeof(struct os_mbuf) + MBUF_PKTHDR_OVERHEAD)

#define MBUF_NUM_MBUFS      (10)
#define MBUF_PAYLOAD_SIZE   (sizeof(msg_rtls_t))
#define MBUF_BUF_SIZE       OS_ALIGN(MBUF_PAYLOAD_SIZE, 4)
#define MBUF_MEMBLOCK_SIZE  (MBUF_BUF_SIZE + MBUF_MEMBLOCK_OVERHEAD)
#define MBUF_MEMPOOL_SIZE   OS_MEMPOOL_SIZE(MBUF_NUM_MBUFS, MBUF_MEMBLOCK_SIZE)

static struct os_mbuf_pool g_mbuf_pool;
static struct os_mempool g_mbuf_mempool;
static os_membuf_t g_mbuf_buffer[MBUF_MEMPOOL_SIZE];

static struct os_task g_task_downlink;
os_stack_t task_downlink_stack[MYNEWT_VAL(APP_GATEWAY_DOWNLINK_TASK_STACK_SZ)];

static struct os_task g_task_uplink;
os_stack_t task_uplink_stack[MYNEWT_VAL(APP_GATEWAY_UPLINK_TASK_STACK_SZ)];

static mavlink_message_t g_mavlink_msg;
static mavlink_status_t  g_mavlink_status;

static struct os_mqueue mqueue_ble_to_net;
static struct os_eventq eventq_ble_to_net;

static void
task_downlink_func(void *arg)
{
    struct os_mbuf *om;
    struct os_mqueue *mqueue;
    struct os_eventq *eventq;
    msg_rtls_t msg_rtls;
    char chr;
    int rc;

    get_net_to_ble_mqueue_eventq(&mqueue, &eventq);

    while(1){
        chr = serial_read();
        uint8_t msg_received = mavlink_parse_char(MAVLINK_COMM_0, chr, &g_mavlink_msg, &g_mavlink_status);
		if(msg_received){

            om = os_mbuf_get_pkthdr(&g_mbuf_pool, 0);
            if(!om) {
                printf("Full mqueue. Message drop!\n");
                continue;
            }

			switch (g_mavlink_msg.msgid)
            {
            case MAVLINK_MSG_ID_LOCATION:
            {
                printf("MAVLINK_MSG_ID_LOCATION\n");
                mavlink_location_t mavlink_location;
                mavlink_msg_location_decode(&g_mavlink_msg, &mavlink_location);
                msg_rtls = (msg_rtls_t){
                    .msg_type = MAVLINK_MSG_ID_LOCATION,
                    .node_type = mavlink_location.node,
                    .dstsrc = mavlink_location.dstsrc,
                    .location_x = mavlink_location.location_x,
                    .location_y = mavlink_location.location_y,
                    .location_z = mavlink_location.location_z,
                };
                rc = os_mbuf_copyinto(om, 0, &msg_rtls, sizeof(struct _msg_rtls_header_t) + sizeof(struct _msg_rtls_location_t));
                if (rc) {
                    printf("error: os_mbuf_copyinto()");
                    continue;
                }
            }
                break;
            case MAVLINK_MSG_ID_ONOFF:
            {
                printf("MAVLINK_MSG_ID_ONOFF\n");
                mavlink_onoff_t mavlink_onoff;
                mavlink_msg_onoff_decode(&g_mavlink_msg, &mavlink_onoff);
                msg_rtls = (msg_rtls_t){
                    .msg_type = MAVLINK_MSG_ID_ONOFF,
                    .dstsrc = mavlink_onoff.dstsrc,
                    .value = mavlink_onoff.value
                };
                rc = os_mbuf_copyinto(om, 0, &msg_rtls, sizeof(struct _msg_rtls_header_t) + sizeof(struct _msg_rtls_onoff_t));
                if (rc) {
                    printf("error: os_mbuf_copyinto()");
                    continue;
                }
            }
                break;
            default:
                break;
            }

            rc = os_mqueue_put(mqueue, eventq, om);
            if (rc) {
                printf("error: os_mqueue_put()");
                continue;
            }
		}
    }
    return;
}


static void
process_ble_to_net_queue(struct os_event *ev)
{
    msg_rtls_t msg_rtls;
    mavlink_message_t mavlink_msg;
    char mav_send_buf[MYNEWT_VAL(APP_GATEWAY_UPLINK_MAV_BUFFER_SIZE)];
    uint16_t len;

    struct os_mbuf *om;
    struct os_mqueue *mqueue = (struct os_mqueue *)ev->ev_arg;
    while ((om = os_mqueue_get(mqueue)) != NULL) {
        msg_parse_rtls(om, &msg_rtls);
        switch(msg_rtls.msg_type){
            case MAVLINK_MSG_ID_LOCATION:
                mavlink_msg_location_pack(0, 0, &mavlink_msg, msg_rtls.dstsrc, STATUS, msg_rtls.node_type, msg_rtls.location_x, msg_rtls.location_y, msg_rtls.location_z);
                break;
            case MAVLINK_MSG_ID_ONOFF:
                mavlink_msg_onoff_pack(0, 0, &mavlink_msg, msg_rtls.dstsrc, STATUS, msg_rtls.value);
                break;
            default:
                continue;
        }
        
        len = mavlink_msg_to_send_buffer((uint8_t*)mav_send_buf, &mavlink_msg);
        serial_write(mav_send_buf, len);
        os_mbuf_free_chain(om);
        printf("BLE->NET\n");
    }
}

static void
task_uplink_func(void *arg){
    while (1) {
        os_eventq_run(&eventq_ble_to_net);
    }
    return;
}

void get_ble_to_net_mqueue_eventq(struct os_mqueue **mqueue, struct os_eventq **eventq){
    *mqueue = &mqueue_ble_to_net;
    *eventq = &eventq_ble_to_net;
}

void gateway_init(){
    int rc;

    os_mqueue_init(&mqueue_ble_to_net, process_ble_to_net_queue, &mqueue_ble_to_net);
    os_eventq_init(&eventq_ble_to_net);

    os_task_init(&g_task_downlink, "gw_dl",
                task_downlink_func,
                NULL,
                MYNEWT_VAL(APP_GATEWAY_DOWNLINK_TASK_PRIORITY), 
                OS_WAIT_FOREVER,
                task_downlink_stack,
                MYNEWT_VAL(APP_GATEWAY_DOWNLINK_TASK_STACK_SZ));

    os_task_init(&g_task_uplink, "gw_ul",
                task_uplink_func,
                NULL,
                MYNEWT_VAL(APP_GATEWAY_UPLINK_TASK_PRIORITY), 
                OS_WAIT_FOREVER,
                task_uplink_stack,
                MYNEWT_VAL(APP_GATEWAY_UPLINK_TASK_STACK_SZ));
    
    rc = os_mempool_init(&g_mbuf_mempool, MBUF_NUM_MBUFS,
                          MBUF_MEMBLOCK_SIZE, &g_mbuf_buffer[0], "mbuf_pool");
    assert(rc == 0);

    rc = os_mbuf_pool_init(&g_mbuf_pool, &g_mbuf_mempool, MBUF_MEMBLOCK_SIZE,
                           MBUF_NUM_MBUFS);
    assert(rc == 0);
}