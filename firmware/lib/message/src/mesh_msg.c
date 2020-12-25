#include <message/mesh_msg.h>

#if MYNEWT_VAL(BLE_MESH)
#include <mesh/access.h>

void msg_prepr_onoff(struct os_mbuf *os_mbuf, msg_onoff_t *msg){
    net_buf_simple_add_u8(os_mbuf, msg->value);
}

void msg_parse_onoff(struct os_mbuf *os_mbuf, msg_onoff_t *msg){
    msg->value = net_buf_simple_pull_u8(os_mbuf);
}

void msg_prepr_rtls(struct os_mbuf **mbuf, msg_rtls_t *msg){
    uint8_t header;
    if (msg->opcode < 0x100) header = 1;
	else if (msg->opcode < 0x10000) header = 2;
    else header = 3; 

    switch (msg->type)
    {
    case MAVLINK_MSG_ID_LOCATION:
        *mbuf = NET_BUF_SIMPLE(header + 15);
        bt_mesh_model_msg_init(*mbuf, msg->opcode);
        net_buf_simple_add_u8(*mbuf, (msg->type & 0x0F) | (msg->node_type << 4));
        net_buf_simple_add_be16(*mbuf, msg->uwb_address);
        net_buf_simple_add_be32(*mbuf, *((uint32_t *)(&msg->location_x)));
        net_buf_simple_add_be32(*mbuf, *((uint32_t *)(&msg->location_y)));
        net_buf_simple_add_be32(*mbuf, *((uint32_t *)(&msg->location_z)));
        break;
    case MAVLINK_MSG_ID_ONOFF:
        *mbuf = NET_BUF_SIMPLE(header + 4);
        bt_mesh_model_msg_init(*mbuf, msg->opcode);
        net_buf_simple_add_u8(*mbuf, msg->type);
        net_buf_simple_add_be16(*mbuf, msg->uwb_address);
        net_buf_simple_add_u8(*mbuf, msg->value);
        break;
    case MAVLINK_MSG_ID_DISTANCE:
        *mbuf = NET_BUF_SIMPLE(header + 4);
        bt_mesh_model_msg_init(*mbuf, msg->opcode);
        net_buf_simple_add_u8(*mbuf, msg->type);
        net_buf_simple_add_be16(*mbuf, msg->uwb_address);
        net_buf_simple_add_be16(*mbuf, msg->anchor);
        net_buf_simple_add_be32(*mbuf, *((uint32_t *)(&msg->distance)));
        break;
    case MAVLINK_MSG_ID_TOF:
        *mbuf = NET_BUF_SIMPLE(header + 4);
        bt_mesh_model_msg_init(*mbuf, msg->opcode);
        net_buf_simple_add_u8(*mbuf, msg->type);
        net_buf_simple_add_be16(*mbuf, msg->uwb_address);
        net_buf_simple_add_be16(*mbuf, msg->anchor);
        net_buf_simple_add_be32(*mbuf, msg->tof);
        break;
    case MAVLINK_MSG_ID_SLOT:
        *mbuf = NET_BUF_SIMPLE(header + 4);
        bt_mesh_model_msg_init(*mbuf, msg->opcode);
        net_buf_simple_add_u8(*mbuf, msg->type);
        net_buf_simple_add_be16(*mbuf, msg->uwb_address);
        net_buf_simple_add_u8(*mbuf, msg->slot);
        break;
    default:
        break;
    }
}

void msg_parse_rtls(struct os_mbuf *mbuf, msg_rtls_t *msg){
    uint8_t type = net_buf_simple_pull_u8(mbuf);
    msg->type = type & 0x0F;
    msg->uwb_address = net_buf_simple_pull_be16(mbuf);
    switch (msg->type)
    {
    case MAVLINK_MSG_ID_LOCATION:
        msg->node_type = type >> 4;
        *((uint32_t *)(&msg->location_x)) = net_buf_simple_pull_be32(mbuf);
        *((uint32_t *)(&msg->location_y)) = net_buf_simple_pull_be32(mbuf);
        *((uint32_t *)(&msg->location_z)) = net_buf_simple_pull_be32(mbuf);
        break;
    case MAVLINK_MSG_ID_ONOFF:
        msg->value = net_buf_simple_pull_u8(mbuf);
        break;
    case MAVLINK_MSG_ID_DISTANCE:
        msg->anchor = net_buf_simple_pull_be16(mbuf);
        *((uint32_t *)(&msg->distance)) = net_buf_simple_pull_be32(mbuf);
        break;
    case MAVLINK_MSG_ID_TOF:
        msg->anchor = net_buf_simple_pull_be16(mbuf);
        msg->tof = net_buf_simple_pull_be32(mbuf);
        break;
    case MAVLINK_MSG_ID_SLOT:
        msg->slot = net_buf_simple_pull_u8(mbuf);
        break;
    default:
        break;
    }
}

void msg_print_rtls(msg_rtls_t *msg){
    switch (msg->type)
    {
    case MAVLINK_MSG_ID_LOCATION:
        printf("{opcode: %ld, type: %d, uwb_address: 0x%04x, node_type: %d, location: [%d.%d, %d.%d, %d.%d]}\n", 
                msg->opcode, msg->type, msg->uwb_address, msg->node_type, 
                (int)msg->location_x, (int)(1000*(msg->location_x - (int)msg->location_x)),
                (int)msg->location_y, (int)(1000*(msg->location_y - (int)msg->location_y)),
                (int)msg->location_z, (int)(1000*(msg->location_z - (int)msg->location_z)));
        break;
    case MAVLINK_MSG_ID_ONOFF:
        printf("{opcode: %ld, type: %d, uwb_address: 0x%02x, value: %d}\n", 
                msg->opcode, msg->type, msg->uwb_address, msg->value);
        break;
    case MAVLINK_MSG_ID_DISTANCE:
        printf("{opcode: %ld, type: %d, uwb_address: 0x%02x, anchor 0x%04X, value: %d.%d}\n", 
                msg->opcode, msg->type, msg->uwb_address, msg->anchor, (int)msg->location_z, 
                (int)(1000*(msg->distance - (int)msg->distance)));
        break;
    case MAVLINK_MSG_ID_TOF:
        printf("{opcode: %ld, type: %d, uwb_address: 0x%02x, anchor 0x%04X, tof: %ld}\n", 
                msg->opcode, msg->type, msg->uwb_address, msg->anchor, msg->tof);
        break;
    case MAVLINK_MSG_ID_SLOT:
        printf("{opcode: %ld, type: %d, uwb_address: 0x%02x, slot: %d}\n", 
                msg->opcode, msg->type, msg->uwb_address, msg->slot);
        break;
    default:
        break;
    }
}

void msg_parse_rtls_pipe(struct os_mbuf *mbuf, msg_rtls_t *msg){
    msg->type = net_buf_simple_pull_u8(mbuf);
    msg->uwb_address = net_buf_simple_pull_be16(mbuf);
    msg->opcode = net_buf_simple_pull_be32(mbuf);
    switch (msg->type)
    {
    case MAVLINK_MSG_ID_LOCATION:
        msg->mesh_address = net_buf_simple_pull_be16(mbuf);
        msg->node_type = net_buf_simple_pull_u8(mbuf);
        *((uint32_t *)(&msg->location_x)) = net_buf_simple_pull_be32(mbuf);
        *((uint32_t *)(&msg->location_y)) = net_buf_simple_pull_be32(mbuf);
        *((uint32_t *)(&msg->location_z)) = net_buf_simple_pull_be32(mbuf);
        break;
    case MAVLINK_MSG_ID_ONOFF:
        msg->mesh_address = net_buf_simple_pull_be16(mbuf);
        msg->value = net_buf_simple_pull_u8(mbuf);
        break;
    case MAVLINK_MSG_ID_DISTANCE:
        msg->mesh_address = net_buf_simple_pull_be16(mbuf);
        msg->anchor = net_buf_simple_pull_be16(mbuf);
        *((uint32_t *)(&msg->distance)) = net_buf_simple_pull_be32(mbuf);
        break;
    case MAVLINK_MSG_ID_TOF:
        msg->mesh_address = net_buf_simple_pull_be16(mbuf);
        msg->anchor = net_buf_simple_pull_be16(mbuf);
        msg->tof = net_buf_simple_pull_be32(mbuf);
        break;
    case MAVLINK_MSG_ID_SLOT:
        msg->mesh_address = net_buf_simple_pull_be16(mbuf);
        msg->slot = net_buf_simple_pull_u8(mbuf);
        break;
    default:
        break;
    }
}

void msg_prepr_rtls_pipe(struct os_mbuf *mbuf, msg_rtls_t *msg){
    net_buf_simple_add_u8(mbuf, msg->type);
    net_buf_simple_add_be16(mbuf, msg->uwb_address);
    net_buf_simple_add_be32(mbuf, msg->opcode);
    switch (msg->type)
    {
    case MAVLINK_MSG_ID_LOCATION:
        net_buf_simple_add_be16(mbuf, msg->mesh_address);
        net_buf_simple_add_u8(mbuf, msg->node_type);
        net_buf_simple_add_be32(mbuf, *((uint32_t *)(&msg->location_x)));
        net_buf_simple_add_be32(mbuf, *((uint32_t *)(&msg->location_y)));
        net_buf_simple_add_be32(mbuf, *((uint32_t *)(&msg->location_z)));
        break;
    case MAVLINK_MSG_ID_ONOFF:
        net_buf_simple_add_be16(mbuf, msg->mesh_address);
        net_buf_simple_add_u8(mbuf, msg->value);
        break;
    case MAVLINK_MSG_ID_DISTANCE:
        net_buf_simple_add_be16(mbuf, msg->mesh_address);
        net_buf_simple_add_be16(mbuf, msg->anchor);
        net_buf_simple_add_be32(mbuf, *((uint32_t *)(&msg->distance)));
        break;
    case MAVLINK_MSG_ID_TOF:
        net_buf_simple_add_be16(mbuf, msg->mesh_address);
        net_buf_simple_add_be16(mbuf, msg->anchor);
        net_buf_simple_add_be32(mbuf, msg->tof);
        break;
    case MAVLINK_MSG_ID_SLOT:
        net_buf_simple_add_be16(mbuf, msg->mesh_address);
        net_buf_simple_add_u8(mbuf, msg->slot);
        break;
    default:
        break;
    }
}

#endif