#ifndef _GATEWAY_H_
#define _GATEWAY_H_

#include <dpl/dpl.h>

void gateway_init();
void get_ble_to_net_mqueue_eventq(struct dpl_mqueue **mqueue, struct dpl_eventq **eventq);

#endif // _GATEWAY_H_