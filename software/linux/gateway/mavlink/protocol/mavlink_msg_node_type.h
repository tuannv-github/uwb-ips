#pragma once
// MESSAGE NODE_TYPE PACKING

#define MAVLINK_MSG_ID_NODE_TYPE 4


typedef struct __mavlink_node_type_t {
 uint16_t uwb_address; /*<  */
 uint8_t node_type; /*<  */
} mavlink_node_type_t;

#define MAVLINK_MSG_ID_NODE_TYPE_LEN 3
#define MAVLINK_MSG_ID_NODE_TYPE_MIN_LEN 3
#define MAVLINK_MSG_ID_4_LEN 3
#define MAVLINK_MSG_ID_4_MIN_LEN 3

#define MAVLINK_MSG_ID_NODE_TYPE_CRC 130
#define MAVLINK_MSG_ID_4_CRC 130



#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_NODE_TYPE { \
    4, \
    "NODE_TYPE", \
    2, \
    {  { "uwb_address", NULL, MAVLINK_TYPE_UINT16_T, 0, 0, offsetof(mavlink_node_type_t, uwb_address) }, \
         { "node_type", NULL, MAVLINK_TYPE_UINT8_T, 0, 2, offsetof(mavlink_node_type_t, node_type) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_NODE_TYPE { \
    "NODE_TYPE", \
    2, \
    {  { "uwb_address", NULL, MAVLINK_TYPE_UINT16_T, 0, 0, offsetof(mavlink_node_type_t, uwb_address) }, \
         { "node_type", NULL, MAVLINK_TYPE_UINT8_T, 0, 2, offsetof(mavlink_node_type_t, node_type) }, \
         } \
}
#endif

/**
 * @brief Pack a node_type message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param uwb_address  
 * @param node_type  
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_node_type_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               uint16_t uwb_address, uint8_t node_type)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_NODE_TYPE_LEN];
    _mav_put_uint16_t(buf, 0, uwb_address);
    _mav_put_uint8_t(buf, 2, node_type);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_NODE_TYPE_LEN);
#else
    mavlink_node_type_t packet;
    packet.uwb_address = uwb_address;
    packet.node_type = node_type;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_NODE_TYPE_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_NODE_TYPE;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_NODE_TYPE_MIN_LEN, MAVLINK_MSG_ID_NODE_TYPE_LEN, MAVLINK_MSG_ID_NODE_TYPE_CRC);
}

/**
 * @brief Pack a node_type message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param uwb_address  
 * @param node_type  
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_node_type_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   uint16_t uwb_address,uint8_t node_type)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_NODE_TYPE_LEN];
    _mav_put_uint16_t(buf, 0, uwb_address);
    _mav_put_uint8_t(buf, 2, node_type);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_NODE_TYPE_LEN);
#else
    mavlink_node_type_t packet;
    packet.uwb_address = uwb_address;
    packet.node_type = node_type;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_NODE_TYPE_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_NODE_TYPE;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_NODE_TYPE_MIN_LEN, MAVLINK_MSG_ID_NODE_TYPE_LEN, MAVLINK_MSG_ID_NODE_TYPE_CRC);
}

/**
 * @brief Encode a node_type struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param node_type C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_node_type_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_node_type_t* node_type)
{
    return mavlink_msg_node_type_pack(system_id, component_id, msg, node_type->uwb_address, node_type->node_type);
}

/**
 * @brief Encode a node_type struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param node_type C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_node_type_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_node_type_t* node_type)
{
    return mavlink_msg_node_type_pack_chan(system_id, component_id, chan, msg, node_type->uwb_address, node_type->node_type);
}

/**
 * @brief Send a node_type message
 * @param chan MAVLink channel to send the message
 *
 * @param uwb_address  
 * @param node_type  
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_node_type_send(mavlink_channel_t chan, uint16_t uwb_address, uint8_t node_type)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_NODE_TYPE_LEN];
    _mav_put_uint16_t(buf, 0, uwb_address);
    _mav_put_uint8_t(buf, 2, node_type);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_NODE_TYPE, buf, MAVLINK_MSG_ID_NODE_TYPE_MIN_LEN, MAVLINK_MSG_ID_NODE_TYPE_LEN, MAVLINK_MSG_ID_NODE_TYPE_CRC);
#else
    mavlink_node_type_t packet;
    packet.uwb_address = uwb_address;
    packet.node_type = node_type;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_NODE_TYPE, (const char *)&packet, MAVLINK_MSG_ID_NODE_TYPE_MIN_LEN, MAVLINK_MSG_ID_NODE_TYPE_LEN, MAVLINK_MSG_ID_NODE_TYPE_CRC);
#endif
}

/**
 * @brief Send a node_type message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_node_type_send_struct(mavlink_channel_t chan, const mavlink_node_type_t* node_type)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_node_type_send(chan, node_type->uwb_address, node_type->node_type);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_NODE_TYPE, (const char *)node_type, MAVLINK_MSG_ID_NODE_TYPE_MIN_LEN, MAVLINK_MSG_ID_NODE_TYPE_LEN, MAVLINK_MSG_ID_NODE_TYPE_CRC);
#endif
}

#if MAVLINK_MSG_ID_NODE_TYPE_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This varient of _send() can be used to save stack space by re-using
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_node_type_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  uint16_t uwb_address, uint8_t node_type)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_uint16_t(buf, 0, uwb_address);
    _mav_put_uint8_t(buf, 2, node_type);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_NODE_TYPE, buf, MAVLINK_MSG_ID_NODE_TYPE_MIN_LEN, MAVLINK_MSG_ID_NODE_TYPE_LEN, MAVLINK_MSG_ID_NODE_TYPE_CRC);
#else
    mavlink_node_type_t *packet = (mavlink_node_type_t *)msgbuf;
    packet->uwb_address = uwb_address;
    packet->node_type = node_type;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_NODE_TYPE, (const char *)packet, MAVLINK_MSG_ID_NODE_TYPE_MIN_LEN, MAVLINK_MSG_ID_NODE_TYPE_LEN, MAVLINK_MSG_ID_NODE_TYPE_CRC);
#endif
}
#endif

#endif

// MESSAGE NODE_TYPE UNPACKING


/**
 * @brief Get field uwb_address from node_type message
 *
 * @return  
 */
static inline uint16_t mavlink_msg_node_type_get_uwb_address(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint16_t(msg,  0);
}

/**
 * @brief Get field node_type from node_type message
 *
 * @return  
 */
static inline uint8_t mavlink_msg_node_type_get_node_type(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  2);
}

/**
 * @brief Decode a node_type message into a struct
 *
 * @param msg The message to decode
 * @param node_type C-struct to decode the message contents into
 */
static inline void mavlink_msg_node_type_decode(const mavlink_message_t* msg, mavlink_node_type_t* node_type)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    node_type->uwb_address = mavlink_msg_node_type_get_uwb_address(msg);
    node_type->node_type = mavlink_msg_node_type_get_node_type(msg);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_NODE_TYPE_LEN? msg->len : MAVLINK_MSG_ID_NODE_TYPE_LEN;
        memset(node_type, 0, MAVLINK_MSG_ID_NODE_TYPE_LEN);
    memcpy(node_type, _MAV_PAYLOAD(msg), len);
#endif
}
