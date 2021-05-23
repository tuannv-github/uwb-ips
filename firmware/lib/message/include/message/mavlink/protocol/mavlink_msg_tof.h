#pragma once
// MESSAGE TOF PACKING

#define MAVLINK_MSG_ID_TOF 25


typedef struct __mavlink_tof_t {
 int32_t node_id; /*<  */
 float x; /*<  */
 float y; /*<  */
 float z; /*<  */
 int32_t tof; /*<  */
} mavlink_tof_t;

#define MAVLINK_MSG_ID_TOF_LEN 20
#define MAVLINK_MSG_ID_TOF_MIN_LEN 20
#define MAVLINK_MSG_ID_25_LEN 20
#define MAVLINK_MSG_ID_25_MIN_LEN 20

#define MAVLINK_MSG_ID_TOF_CRC 178
#define MAVLINK_MSG_ID_25_CRC 178



#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_TOF { \
    25, \
    "TOF", \
    5, \
    {  { "node_id", NULL, MAVLINK_TYPE_INT32_T, 0, 0, offsetof(mavlink_tof_t, node_id) }, \
         { "x", NULL, MAVLINK_TYPE_FLOAT, 0, 4, offsetof(mavlink_tof_t, x) }, \
         { "y", NULL, MAVLINK_TYPE_FLOAT, 0, 8, offsetof(mavlink_tof_t, y) }, \
         { "z", NULL, MAVLINK_TYPE_FLOAT, 0, 12, offsetof(mavlink_tof_t, z) }, \
         { "tof", NULL, MAVLINK_TYPE_INT32_T, 0, 16, offsetof(mavlink_tof_t, tof) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_TOF { \
    "TOF", \
    5, \
    {  { "node_id", NULL, MAVLINK_TYPE_INT32_T, 0, 0, offsetof(mavlink_tof_t, node_id) }, \
         { "x", NULL, MAVLINK_TYPE_FLOAT, 0, 4, offsetof(mavlink_tof_t, x) }, \
         { "y", NULL, MAVLINK_TYPE_FLOAT, 0, 8, offsetof(mavlink_tof_t, y) }, \
         { "z", NULL, MAVLINK_TYPE_FLOAT, 0, 12, offsetof(mavlink_tof_t, z) }, \
         { "tof", NULL, MAVLINK_TYPE_INT32_T, 0, 16, offsetof(mavlink_tof_t, tof) }, \
         } \
}
#endif

/**
 * @brief Pack a tof message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param node_id  
 * @param x  
 * @param y  
 * @param z  
 * @param tof  
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_tof_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               int32_t node_id, float x, float y, float z, int32_t tof)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_TOF_LEN];
    _mav_put_int32_t(buf, 0, node_id);
    _mav_put_float(buf, 4, x);
    _mav_put_float(buf, 8, y);
    _mav_put_float(buf, 12, z);
    _mav_put_int32_t(buf, 16, tof);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_TOF_LEN);
#else
    mavlink_tof_t packet;
    packet.node_id = node_id;
    packet.x = x;
    packet.y = y;
    packet.z = z;
    packet.tof = tof;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_TOF_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_TOF;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_TOF_MIN_LEN, MAVLINK_MSG_ID_TOF_LEN, MAVLINK_MSG_ID_TOF_CRC);
}

/**
 * @brief Pack a tof message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param node_id  
 * @param x  
 * @param y  
 * @param z  
 * @param tof  
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_tof_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   int32_t node_id,float x,float y,float z,int32_t tof)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_TOF_LEN];
    _mav_put_int32_t(buf, 0, node_id);
    _mav_put_float(buf, 4, x);
    _mav_put_float(buf, 8, y);
    _mav_put_float(buf, 12, z);
    _mav_put_int32_t(buf, 16, tof);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_TOF_LEN);
#else
    mavlink_tof_t packet;
    packet.node_id = node_id;
    packet.x = x;
    packet.y = y;
    packet.z = z;
    packet.tof = tof;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_TOF_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_TOF;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_TOF_MIN_LEN, MAVLINK_MSG_ID_TOF_LEN, MAVLINK_MSG_ID_TOF_CRC);
}

/**
 * @brief Encode a tof struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param tof C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_tof_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_tof_t* tof)
{
    return mavlink_msg_tof_pack(system_id, component_id, msg, tof->node_id, tof->x, tof->y, tof->z, tof->tof);
}

/**
 * @brief Encode a tof struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param tof C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_tof_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_tof_t* tof)
{
    return mavlink_msg_tof_pack_chan(system_id, component_id, chan, msg, tof->node_id, tof->x, tof->y, tof->z, tof->tof);
}

/**
 * @brief Send a tof message
 * @param chan MAVLink channel to send the message
 *
 * @param node_id  
 * @param x  
 * @param y  
 * @param z  
 * @param tof  
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_tof_send(mavlink_channel_t chan, int32_t node_id, float x, float y, float z, int32_t tof)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_TOF_LEN];
    _mav_put_int32_t(buf, 0, node_id);
    _mav_put_float(buf, 4, x);
    _mav_put_float(buf, 8, y);
    _mav_put_float(buf, 12, z);
    _mav_put_int32_t(buf, 16, tof);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_TOF, buf, MAVLINK_MSG_ID_TOF_MIN_LEN, MAVLINK_MSG_ID_TOF_LEN, MAVLINK_MSG_ID_TOF_CRC);
#else
    mavlink_tof_t packet;
    packet.node_id = node_id;
    packet.x = x;
    packet.y = y;
    packet.z = z;
    packet.tof = tof;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_TOF, (const char *)&packet, MAVLINK_MSG_ID_TOF_MIN_LEN, MAVLINK_MSG_ID_TOF_LEN, MAVLINK_MSG_ID_TOF_CRC);
#endif
}

/**
 * @brief Send a tof message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_tof_send_struct(mavlink_channel_t chan, const mavlink_tof_t* tof)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_tof_send(chan, tof->node_id, tof->x, tof->y, tof->z, tof->tof);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_TOF, (const char *)tof, MAVLINK_MSG_ID_TOF_MIN_LEN, MAVLINK_MSG_ID_TOF_LEN, MAVLINK_MSG_ID_TOF_CRC);
#endif
}

#if MAVLINK_MSG_ID_TOF_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This varient of _send() can be used to save stack space by re-using
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_tof_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  int32_t node_id, float x, float y, float z, int32_t tof)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_int32_t(buf, 0, node_id);
    _mav_put_float(buf, 4, x);
    _mav_put_float(buf, 8, y);
    _mav_put_float(buf, 12, z);
    _mav_put_int32_t(buf, 16, tof);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_TOF, buf, MAVLINK_MSG_ID_TOF_MIN_LEN, MAVLINK_MSG_ID_TOF_LEN, MAVLINK_MSG_ID_TOF_CRC);
#else
    mavlink_tof_t *packet = (mavlink_tof_t *)msgbuf;
    packet->node_id = node_id;
    packet->x = x;
    packet->y = y;
    packet->z = z;
    packet->tof = tof;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_TOF, (const char *)packet, MAVLINK_MSG_ID_TOF_MIN_LEN, MAVLINK_MSG_ID_TOF_LEN, MAVLINK_MSG_ID_TOF_CRC);
#endif
}
#endif

#endif

// MESSAGE TOF UNPACKING


/**
 * @brief Get field node_id from tof message
 *
 * @return  
 */
static inline int32_t mavlink_msg_tof_get_node_id(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  0);
}

/**
 * @brief Get field x from tof message
 *
 * @return  
 */
static inline float mavlink_msg_tof_get_x(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  4);
}

/**
 * @brief Get field y from tof message
 *
 * @return  
 */
static inline float mavlink_msg_tof_get_y(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  8);
}

/**
 * @brief Get field z from tof message
 *
 * @return  
 */
static inline float mavlink_msg_tof_get_z(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  12);
}

/**
 * @brief Get field tof from tof message
 *
 * @return  
 */
static inline int32_t mavlink_msg_tof_get_tof(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  16);
}

/**
 * @brief Decode a tof message into a struct
 *
 * @param msg The message to decode
 * @param tof C-struct to decode the message contents into
 */
static inline void mavlink_msg_tof_decode(const mavlink_message_t* msg, mavlink_tof_t* tof)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    tof->node_id = mavlink_msg_tof_get_node_id(msg);
    tof->x = mavlink_msg_tof_get_x(msg);
    tof->y = mavlink_msg_tof_get_y(msg);
    tof->z = mavlink_msg_tof_get_z(msg);
    tof->tof = mavlink_msg_tof_get_tof(msg);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_TOF_LEN? msg->len : MAVLINK_MSG_ID_TOF_LEN;
        memset(tof, 0, MAVLINK_MSG_ID_TOF_LEN);
    memcpy(tof, _MAV_PAYLOAD(msg), len);
#endif
}
