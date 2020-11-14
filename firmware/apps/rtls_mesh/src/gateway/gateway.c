#if __GNUC__ >= 9
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"
#endif

#include <dpl/dpl.h>
#include "os/mynewt.h"
#include <rtls_mesh/gateway/gateway.h>

#include <serial/serial.h>
#include <rtls_mesh/gateway/mavlink/protocol/mavlink.h>
#include <rtls_mesh/ble_mesh/mesh_msg.h>

static struct dpl_task g_task_downlink;
dpl_stack_t task_downlink_stack[MYNEWT_VAL(APP_GATEWAY_DOWNLINK_TASK_STACK_SZ)];

static struct dpl_task g_task_uplink;
dpl_stack_t task_uplink_stack[MYNEWT_VAL(APP_GATEWAY_UPLINK_TASK_STACK_SZ)];

static mavlink_message_t msg;
static mavlink_status_t  status;

static struct dpl_mqueue mqueue_ble_to_net;
static struct dpl_eventq eventq_ble_to_net;

mavlink_message_t mavlink_msg;
msg_rtls_t msg_rtls;
char gmav_send_buf[100];
uint16_t len;

static void *
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
    return NULL;
}


static void
process_ble_to_net_queue(struct dpl_event *ev)
{
    struct dpl_mbuf *om;
    struct dpl_mqueue *mqueue = (struct dpl_mqueue *)dpl_event_get_arg(ev);
    while ((om = dpl_mqueue_get(mqueue)) != NULL) {
        printf("process message_queue\n");
        msg_parse_rtls((struct os_mbuf *)om, &msg_rtls);

        if(msg_rtls.type == MAVLINK_MSG_ID_LOCATION){
            printf("Src: %d, ", msg_rtls.dstsrc);
            printf("location: %d, %d, %d\n", (int)(msg_rtls.location_x*1000), (int)(msg_rtls.location_y*1000), (int)(msg_rtls.location_z*1000));
        }

        mavlink_msg_location_pack(0,0, &mavlink_msg, msg_rtls.dstsrc, msg_rtls.type, 1, msg_rtls.location_x, msg_rtls.location_y, msg_rtls.location_z);
        len = mavlink_msg_to_send_buffer((uint8_t*)gmav_send_buf, &mavlink_msg);
        printf("msg len: %d\n", len);
        serial_write(gmav_send_buf, len);


        dpl_mbuf_free_chain(om);
    }
}

static void *
task_uplink_func(void *arg){
    while (1) {
        dpl_eventq_run(&eventq_ble_to_net);
    }
    return NULL;
}

void get_ble_to_net_mqueue_eventq(struct dpl_mqueue **mqueue, struct dpl_eventq **eventq){
    *mqueue = &mqueue_ble_to_net;
    *eventq = &eventq_ble_to_net;
}

void gateway_init(){
    dpl_mqueue_init(&mqueue_ble_to_net, process_ble_to_net_queue, &mqueue_ble_to_net);
    dpl_eventq_init(&eventq_ble_to_net);

    dpl_task_init(&g_task_downlink, "gw_dl",
                task_downlink_func,
                NULL,
                MYNEWT_VAL(APP_GATEWAY_DOWNLINK_TASK_PRIORITY), 
                OS_WAIT_FOREVER,
                task_downlink_stack,
                MYNEWT_VAL(APP_GATEWAY_DOWNLINK_TASK_STACK_SZ));

    dpl_task_init(&g_task_uplink, "gw_ul",
                task_uplink_func,
                NULL,
                MYNEWT_VAL(APP_GATEWAY_UPLINK_TASK_PRIORITY), 
                OS_WAIT_FOREVER,
                task_uplink_stack,
                MYNEWT_VAL(APP_GATEWAY_UPLINK_TASK_STACK_SZ));
}