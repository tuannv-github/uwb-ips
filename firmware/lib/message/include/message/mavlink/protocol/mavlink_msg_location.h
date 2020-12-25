#pragma once
// MESSAGE LOCATION PACKING

#define MAVLINK_MSG_ID_LOCATION 0


typedef struct __mavlink_location_t {
 float location_x; /*<  */
 float location_y; /*<  */
 float location_z; /*<  */
 uint16_t mesh_address; /*<  */
 uint16_t uwb_address; /*<  */
 uint8_t type; /*<  */
 uint8_t node; /*<  */
} mavlink_location_t;

#define MAVLINK_MSG_ID_LOCATION_LEN 18
#define MAVLINK_MSG_ID_LOCATION_MIN_LEN 18
#define MAVLINK_MSG_ID_0_LEN 18
#define MAVLINK_MSG_ID_0_MIN_LEN 18

#define MAVLINK_MSG_ID_LOCATION_CRC 11
#define MAVLINK_MSG_ID_0_CRC 11



#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_LOCATION { \
    0, \
    "LOCATION", \
    7, \
    {  { "mesh_address", NULL, MAVLINK_TYPE_UINT16_T, 0, 12, offsetof(mavlink_location_t, mesh_address) }, \
         { "uwb_address", NULL, MAVLINK_TYPE_UINT16_T, 0, 14, offsetof(mavlink_location_t, uwb_address) }, \
         { "type", NULL, MAVLINK_TYPE_UINT8_T, 0, 16, offsetof(mavlink_location_t, type) }, \
         { "node", NULL, MAVLINK_TYPE_UINT8_T, 0, 17, offsetof(mavlink_location_t, node) }, \
         { "location_x", NULL, MAVLINK_TYPE_FLOAT, 0, 0, offsetof(mavlink_location_t, location_x) }, \
         { "location_y", NULL, MAVLINK_TYPE_FLOAT, 0, 4, offsetof(mavlink_location_t, location_y) }, \
         { "location_z", NULL, MAVLINK_TYPE_FLOAT, 0, 8, offsetof(mavlink_location_t, location_z) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_LOCATION { \
    "LOCATION", \
    7, \
    {  { "mesh_address", NULL, MAVLINK_TYPE_UINT16_T, 0, 12, offsetof(mavlink_location_t, mesh_address) }, \
         { "uwb_address", NULL, MAVLINK_TYPE_UINT16_T, 0, 14, offsetof(mavlink_location_t, uwb_address) }, \
         { "type", NULL, MAVLINK_TYPE_UINT8_T, 0, 16, offsetof(mavlink_location_t, type) }, \
         { "node", NULL, MAVLINK_TYPE_UINT8_T, 0, 17, offsetof(mavlink_location_t, node) }, \
         { "location_x", NULL, MAVLINK_TYPE_FLOAT, 0, 0, offsetof(mavlink_location_t, location_x) }, \
         { "location_y", NULL, MAVLINK_TYPE_FLOAT, 0, 4, offsetof(mavlink_location_t, location_y) }, \
         { "location_z", NULL, MAVLINK_TYPE_FLOAT, 0, 8, offsetof(mavlink_location_t, location_z) }, \
         } \
}
#endif

/**
 * @brief Pack a location message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param mesh_address  
 * @param uwb_address  
 * @param type  
 * @param node  
 * @param location_x  
 * @param location_y  
 * @param location_z  
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_location_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               uint16_t mesh_address, uint16_t uwb_address, uint8_t type, uint8_t node, float location_x, float location_y, float location_z)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_LOCATION_LEN];
    _mav_put_float(buf, 0, location_x);
    _mav_put_float(buf, 4, location_y);
    _mav_put_float(buf, 8, location_z);
    _mav_put_uint16_t(buf, 12, mesh_address);
    _mav_put_uint16_t(buf, 14, uwb_address);
    _mav_put_uint8_t(buf, 16, type);
    _mav_put_uint8_t(buf, 17, node);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_LOCATION_LEN);
#else
    mavlink_location_t packet;
    packet.location_x = location_x;
    packet.location_y = location_y;
    packet.location_z = location_z;
    packet.mesh_address = mesh_address;
    packet.uwb_address = uwb_address;
    packet.type = type;
    packet.node = node;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_LOCATION_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_LOCATION;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_LOCATION_MIN_LEN, MAVLINK_MSG_ID_LOCATION_LEN, MAVLINK_MSG_ID_LOCATION_CRC);
}

/**
 * @brief Pack a location message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param mesh_address  
 * @param uwb_address  
 * @param type  
 * @param node  
 * @param location_x  
 * @param location_y  
 * @param location_z  
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_location_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   uint16_t mesh_address,uint16_t uwb_address,uint8_t type,uint8_t node,float location_x,float location_y,float location_z)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_LOCATION_LEN];
    _mav_put_float(buf, 0, location_x);
    _mav_put_float(buf, 4, location_y);
    _mav_put_float(buf, 8, location_z);
    _mav_put_uint16_t(buf, 12, mesh_address);
    _mav_put_uint16_t(buf, 14, uwb_address);
    _mav_put_uint8_t(buf, 16, type);
    _mav_put_uint8_t(buf, 17, node);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_LOCATION_LEN);
#else
    mavlink_location_t packet;
    packet.location_x = location_x;
    packet.location_y = location_y;
    packet.location_z = location_z;
    packet.mesh_address = mesh_address;
    packet.uwb_address = uwb_address;
    packet.type = type;
    packet.node = node;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_LOCATION_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_LOCATION;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_LOCATION_MIN_LEN, MAVLINK_MSG_ID_LOCATION_LEN, MAVLINK_MSG_ID_LOCATION_CRC);
}

/**
 * @brief Encode a location struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param location C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_location_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_location_t* location)
{
    return mavlink_msg_location_pack(system_id, component_id, msg, location->mesh_address, location->uwb_address, location->type, location->node, location->location_x, location->location_y, location->location_z);
}

/**
 * @brief Encode a location struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param location C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_location_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_location_t* location)
{
    return mavlink_msg_location_pack_chan(system_id, component_id, chan, msg, location->mesh_address, location->uwb_address, location->type, location->node, location->location_x, location->location_y, location->location_z);
}

/**
 * @brief Send a location message
 * @param chan MAVLink channel to send the message
 *
 * @param mesh_address  
 * @param uwb_address  
 * @param type  
 * @param node  
 * @param location_x  
 * @param location_y  
 * @param location_z  
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_location_send(mavlink_channel_t chan, uint16_t mesh_address, uint16_t uwb_address, uint8_t type, uint8_t node, float location_x, float location_y, float location_z)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_LOCATION_LEN];
    _mav_put_float(buf, 0, location_x);
    _mav_put_float(buf, 4, location_y);
    _mav_put_float(buf, 8, location_z);
    _mav_put_uint16_t(buf, 12, mesh_address);
    _mav_put_uint16_t(buf, 14, uwb_address);
    _mav_put_uint8_t(buf, 16, type);
    _mav_put_uint8_t(buf, 17, node);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_LOCATION, buf, MAVLINK_MSG_ID_LOCATION_MIN_LEN, MAVLINK_MSG_ID_LOCATION_LEN, MAVLINK_MSG_ID_LOCATION_CRC);
#else
    mavlink_location_t packet;
    packet.location_x = location_x;
    packet.location_y = location_y;
    packet.location_z = location_z;
    packet.mesh_address = mesh_address;
    packet.uwb_address = uwb_address;
    packet.type = type;
    packet.node = node;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_LOCATION, (const char *)&packet, MAVLINK_MSG_ID_LOCATION_MIN_LEN, MAVLINK_MSG_ID_LOCATION_LEN, MAVLINK_MSG_ID_LOCATION_CRC);
#endif
}

/**
 * @brief Send a location message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_location_send_struct(mavlink_channel_t chan, const mavlink_location_t* location)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_location_send(chan, location->mesh_address, location->uwb_address, location->type, location->node, location->location_x, location->location_y, location->location_z);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_LOCATION, (const char *)location, MAVLINK_MSG_ID_LOCATION_MIN_LEN, MAVLINK_MSG_ID_LOCATION_LEN, MAVLINK_MSG_ID_LOCATION_CRC);
#endif
}

#if MAVLINK_MSG_ID_LOCATION_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This varient of _send() can be used to save stack space by re-using
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_location_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  uint16_t mesh_address, uint16_t uwb_address, uint8_t type, uint8_t node, float location_x, float location_y, float location_z)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_float(buf, 0, location_x);
    _mav_put_float(buf, 4, location_y);
    _mav_put_float(buf, 8, location_z);
    _mav_put_uint16_t(buf, 12, mesh_address);
    _mav_put_uint16_t(buf, 14, uwb_address);
    _mav_put_uint8_t(buf, 16, type);
    _mav_put_uint8_t(buf, 17, node);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_LOCATION, buf, MAVLINK_MSG_ID_LOCATION_MIN_LEN, MAVLINK_MSG_ID_LOCATION_LEN, MAVLINK_MSG_ID_LOCATION_CRC);
#else
    mavlink_location_t *packet = (mavlink_location_t *)msgbuf;
    packet->location_x = location_x;
    packet->location_y = location_y;
    packet->location_z = location_z;
    packet->mesh_address = mesh_address;
    packet->uwb_address = uwb_address;
    packet->type = type;
    packet->node = node;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_LOCATION, (const char *)packet, MAVLINK_MSG_ID_LOCATION_MIN_LEN, MAVLINK_MSG_ID_LOCATION_LEN, MAVLINK_MSG_ID_LOCATION_CRC);
#endif
}
#endif

#endif

// MESSAGE LOCATION UNPACKING


/**
 * @brief Get field mesh_address from location message
 *
 * @return  
 */
static inline uint16_t mavlink_msg_location_get_mesh_address(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint16_t(msg,  12);
}

/**
 * @brief Get field uwb_address from location message
 *
 * @return  
 */
static inline uint16_t mavlink_msg_location_get_uwb_address(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint16_t(msg,  14);
}

/**
 * @brief Get field type from location message
 *
 * @return  
 */
static inline uint8_t mavlink_msg_location_get_type(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  16);
}

/**
 * @brief Get field node from location message
 *
 * @return  
 */
static inline uint8_t mavlink_msg_location_get_node(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  17);
}

/**
 * @brief Get field location_x from location message
 *
 * @return  
 */
static inline float mavlink_msg_location_get_location_x(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  0);
}

/**
 * @brief Get field location_y from location message
 *
 * @return  
 */
static inline float mavlink_msg_location_get_location_y(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  4);
}

/**
 * @brief Get field location_z from location message
 *
 * @return  
 */
static inline float mavlink_msg_location_get_location_z(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  8);
}

/**
 * @brief Decode a location message into a struct
 *
 * @param msg The message to decode
 * @param location C-struct to decode the message contents into
 */
static inline void mavlink_msg_location_decode(const mavlink_message_t* msg, mavlink_location_t* location)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    location->location_x = mavlink_msg_location_get_location_x(msg);
    location->location_y = mavlink_msg_location_get_location_y(msg);
    location->location_z = mavlink_msg_location_get_location_z(msg);
    location->mesh_address = mavlink_msg_location_get_mesh_address(msg);
    location->uwb_address = mavlink_msg_location_get_uwb_address(msg);
    location->type = mavlink_msg_location_get_type(msg);
    location->node = mavlink_msg_location_get_node(msg);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_LOCATION_LEN? msg->len : MAVLINK_MSG_ID_LOCATION_LEN;
        memset(location, 0, MAVLINK_MSG_ID_LOCATION_LEN);
    memcpy(location, _MAV_PAYLOAD(msg), len);
#endif
}
