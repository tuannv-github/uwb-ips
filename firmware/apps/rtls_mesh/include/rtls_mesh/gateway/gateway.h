#ifndef _GATEWAY_H_
#define _GATEWAY_H_

#include <os/mynewt.h>

void gateway_init();
void get_ble_to_net_mqueue_eventq(struct os_mqueue **mqueue, struct os_eventq **eventq);

#endif // _GATEWAY_H_