#include <rtls_mesh/ble_mesh/mesh_msg.h>

void msg_prepr_onoff(struct os_mbuf *os_mbuf, msg_onoff_t *msg_onoff){
    net_buf_simple_add_u8(os_mbuf, msg_onoff->value);
}

void msg_parse_onoff(struct os_mbuf *os_mbuf, msg_onoff_t *msg_onoff){
    msg_onoff->value = net_buf_simple_pull_u8(os_mbuf);
}