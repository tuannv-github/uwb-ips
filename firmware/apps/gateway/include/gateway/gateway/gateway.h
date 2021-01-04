#ifndef _GATEWAY_H_
#define _GATEWAY_H_

#include <os/mynewt.h>

#define LED_UPLINK_0        LED_1
#define LED_UPLINK_1        LED_4

#define LED_DOWNLINK_0      LED_3
#define LED_DOWNLINK_1      LED_2

#define LED_ON              0
#define LED_OFF             1

void gateway_init();
void get_ble_to_net_mqueue_eventq(struct os_mqueue **mqueue, struct os_eventq **eventq);

#endif // _GATEWAY_H_