#ifndef _BLE_MESH_H_
#define _BLE_MESH_H_

void ble_mesh_init();
void get_net_to_ble_mqueue_eventq(struct os_mqueue **mqueue, struct os_eventq **eventq);

#endif 