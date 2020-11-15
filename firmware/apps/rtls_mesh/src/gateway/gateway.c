#if __GNUC__ >= 9
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"
#endif

#include <dpl/dpl.h>
#include <os/mynewt.h>
#include <rtls_mesh/gateway/gateway.h>

#include <serial/serial.h>
#include <rtls_mesh/gateway/mavlink/protocol/mavlink.h>
#include <rtls_mesh/ble_mesh/mesh_msg.h>

static struct os_task g_task_downlink;
os_stack_t task_downlink_stack[MYNEWT_VAL(APP_GATEWAY_DOWNLINK_TASK_STACK_SZ)];

static struct os_task g_task_uplink;
os_stack_t task_uplink_stack[MYNEWT_VAL(APP_GATEWAY_UPLINK_TASK_STACK_SZ)];

static mavlink_message_t msg;
static mavlink_status_t  status;

static struct os_mqueue mqueue_ble_to_net;
static struct os_eventq eventq_ble_to_net;

static void
task_downlink_func(void *arg)
{
    char chr;
    while(1){
        chr = serial_read();
        uint8_t msg_received = mavlink_parse_char(MAVLINK_COMM_0, chr, &msg, &status);
		if(msg_received){
			switch (msg.msgid)
            {
            case MAVLINK_MSG_ID_LOCATION:
                printf("MAVLINK_MSG_ID_LOCATION\n");
                break;
            case MAVLINK_MSG_ID_ONOFF:
                printf("MAVLINK_MSG_ID_ONOFF\n");
                break;
            default:
                break;
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
        mavlink_msg_location_pack(0,0, &mavlink_msg, msg_rtls.dstsrc, msg_rtls.msg_type, msg_rtls.node_type, msg_rtls.location_x, msg_rtls.location_y, msg_rtls.location_z);
        len = mavlink_msg_to_send_buffer((uint8_t*)mav_send_buf, &mavlink_msg);
        serial_write(mav_send_buf, len);
        os_mbuf_free_chain(om);
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
}