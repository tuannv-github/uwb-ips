#pragma once
// MESSAGE EVT_ACCEL_RAW PACKING

#define MAVLINK_MSG_ID_EVT_ACCEL_RAW 11


typedef struct __mavlink_evt_accel_raw_t {
 float acc_x; /*<  Accel X*/
 float acc_y; /*<  Accel Y*/
 float acc_z; /*<  Accel Z*/
} mavlink_evt_accel_raw_t;

#define MAVLINK_MSG_ID_EVT_ACCEL_RAW_LEN 12
#define MAVLINK_MSG_ID_EVT_ACCEL_RAW_MIN_LEN 12
#define MAVLINK_MSG_ID_11_LEN 12
#define MAVLINK_MSG_ID_11_MIN_LEN 12

#define MAVLINK_MSG_ID_EVT_ACCEL_RAW_CRC 117
#define MAVLINK_MSG_ID_11_CRC 117



#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_EVT_ACCEL_RAW { \
    11, \
    "EVT_ACCEL_RAW", \
    3, \
    {  { "acc_x", NULL, MAVLINK_TYPE_FLOAT, 0, 0, offsetof(mavlink_evt_accel_raw_t, acc_x) }, \
         { "acc_y", NULL, MAVLINK_TYPE_FLOAT, 0, 4, offsetof(mavlink_evt_accel_raw_t, acc_y) }, \
         { "acc_z", NULL, MAVLINK_TYPE_FLOAT, 0, 8, offsetof(mavlink_evt_accel_raw_t, acc_z) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_EVT_ACCEL_RAW { \
    "EVT_ACCEL_RAW", \
    3, \
    {  { "acc_x", NULL, MAVLINK_TYPE_FLOAT, 0, 0, offsetof(mavlink_evt_accel_raw_t, acc_x) }, \
         { "acc_y", NULL, MAVLINK_TYPE_FLOAT, 0, 4, offsetof(mavlink_evt_accel_raw_t, acc_y) }, \
         { "acc_z", NULL, MAVLINK_TYPE_FLOAT, 0, 8, offsetof(mavlink_evt_accel_raw_t, acc_z) }, \
         } \
}
#endif

/**
 * @brief Pack a evt_accel_raw message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param acc_x  Accel X
 * @param acc_y  Accel Y
 * @param acc_z  Accel Z
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_evt_accel_raw_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               float acc_x, float acc_y, float acc_z)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_EVT_ACCEL_RAW_LEN];
    _mav_put_float(buf, 0, acc_x);
    _mav_put_float(buf, 4, acc_y);
    _mav_put_float(buf, 8, acc_z);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_EVT_ACCEL_RAW_LEN);
#else
    mavlink_evt_accel_raw_t packet;
    packet.acc_x = acc_x;
    packet.acc_y = acc_y;
    packet.acc_z = acc_z;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_EVT_ACCEL_RAW_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_EVT_ACCEL_RAW;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_EVT_ACCEL_RAW_MIN_LEN, MAVLINK_MSG_ID_EVT_ACCEL_RAW_LEN, MAVLINK_MSG_ID_EVT_ACCEL_RAW_CRC);
}

/**
 * @brief Pack a evt_accel_raw message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param acc_x  Accel X
 * @param acc_y  Accel Y
 * @param acc_z  Accel Z
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_evt_accel_raw_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   float acc_x,float acc_y,float acc_z)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_EVT_ACCEL_RAW_LEN];
    _mav_put_float(buf, 0, acc_x);
    _mav_put_float(buf, 4, acc_y);
    _mav_put_float(buf, 8, acc_z);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_EVT_ACCEL_RAW_LEN);
#else
    mavlink_evt_accel_raw_t packet;
    packet.acc_x = acc_x;
    packet.acc_y = acc_y;
    packet.acc_z = acc_z;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_EVT_ACCEL_RAW_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_EVT_ACCEL_RAW;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_EVT_ACCEL_RAW_MIN_LEN, MAVLINK_MSG_ID_EVT_ACCEL_RAW_LEN, MAVLINK_MSG_ID_EVT_ACCEL_RAW_CRC);
}

/**
 * @brief Encode a evt_accel_raw struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param evt_accel_raw C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_evt_accel_raw_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_evt_accel_raw_t* evt_accel_raw)
{
    return mavlink_msg_evt_accel_raw_pack(system_id, component_id, msg, evt_accel_raw->acc_x, evt_accel_raw->acc_y, evt_accel_raw->acc_z);
}

/**
 * @brief Encode a evt_accel_raw struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param evt_accel_raw C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_evt_accel_raw_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_evt_accel_raw_t* evt_accel_raw)
{
    return mavlink_msg_evt_accel_raw_pack_chan(system_id, component_id, chan, msg, evt_accel_raw->acc_x, evt_accel_raw->acc_y, evt_accel_raw->acc_z);
}

/**
 * @brief Send a evt_accel_raw message
 * @param chan MAVLink channel to send the message
 *
 * @param acc_x  Accel X
 * @param acc_y  Accel Y
 * @param acc_z  Accel Z
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_evt_accel_raw_send(mavlink_channel_t chan, float acc_x, float acc_y, float acc_z)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_EVT_ACCEL_RAW_LEN];
    _mav_put_float(buf, 0, acc_x);
    _mav_put_float(buf, 4, acc_y);
    _mav_put_float(buf, 8, acc_z);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_EVT_ACCEL_RAW, buf, MAVLINK_MSG_ID_EVT_ACCEL_RAW_MIN_LEN, MAVLINK_MSG_ID_EVT_ACCEL_RAW_LEN, MAVLINK_MSG_ID_EVT_ACCEL_RAW_CRC);
#else
    mavlink_evt_accel_raw_t packet;
    packet.acc_x = acc_x;
    packet.acc_y = acc_y;
    packet.acc_z = acc_z;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_EVT_ACCEL_RAW, (const char *)&packet, MAVLINK_MSG_ID_EVT_ACCEL_RAW_MIN_LEN, MAVLINK_MSG_ID_EVT_ACCEL_RAW_LEN, MAVLINK_MSG_ID_EVT_ACCEL_RAW_CRC);
#endif
}

/**
 * @brief Send a evt_accel_raw message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_evt_accel_raw_send_struct(mavlink_channel_t chan, const mavlink_evt_accel_raw_t* evt_accel_raw)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_evt_accel_raw_send(chan, evt_accel_raw->acc_x, evt_accel_raw->acc_y, evt_accel_raw->acc_z);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_EVT_ACCEL_RAW, (const char *)evt_accel_raw, MAVLINK_MSG_ID_EVT_ACCEL_RAW_MIN_LEN, MAVLINK_MSG_ID_EVT_ACCEL_RAW_LEN, MAVLINK_MSG_ID_EVT_ACCEL_RAW_CRC);
#endif
}

#if MAVLINK_MSG_ID_EVT_ACCEL_RAW_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This varient of _send() can be used to save stack space by re-using
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_evt_accel_raw_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  float acc_x, float acc_y, float acc_z)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_float(buf, 0, acc_x);
    _mav_put_float(buf, 4, acc_y);
    _mav_put_float(buf, 8, acc_z);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_EVT_ACCEL_RAW, buf, MAVLINK_MSG_ID_EVT_ACCEL_RAW_MIN_LEN, MAVLINK_MSG_ID_EVT_ACCEL_RAW_LEN, MAVLINK_MSG_ID_EVT_ACCEL_RAW_CRC);
#else
    mavlink_evt_accel_raw_t *packet = (mavlink_evt_accel_raw_t *)msgbuf;
    packet->acc_x = acc_x;
    packet->acc_y = acc_y;
    packet->acc_z = acc_z;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_EVT_ACCEL_RAW, (const char *)packet, MAVLINK_MSG_ID_EVT_ACCEL_RAW_MIN_LEN, MAVLINK_MSG_ID_EVT_ACCEL_RAW_LEN, MAVLINK_MSG_ID_EVT_ACCEL_RAW_CRC);
#endif
}
#endif

#endif

// MESSAGE EVT_ACCEL_RAW UNPACKING


/**
 * @brief Get field acc_x from evt_accel_raw message
 *
 * @return  Accel X
 */
static inline float mavlink_msg_evt_accel_raw_get_acc_x(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  0);
}

/**
 * @brief Get field acc_y from evt_accel_raw message
 *
 * @return  Accel Y
 */
static inline float mavlink_msg_evt_accel_raw_get_acc_y(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  4);
}

/**
 * @brief Get field acc_z from evt_accel_raw message
 *
 * @return  Accel Z
 */
static inline float mavlink_msg_evt_accel_raw_get_acc_z(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  8);
}

/**
 * @brief Decode a evt_accel_raw message into a struct
 *
 * @param msg The message to decode
 * @param evt_accel_raw C-struct to decode the message contents into
 */
static inline void mavlink_msg_evt_accel_raw_decode(const mavlink_message_t* msg, mavlink_evt_accel_raw_t* evt_accel_raw)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    evt_accel_raw->acc_x = mavlink_msg_evt_accel_raw_get_acc_x(msg);
    evt_accel_raw->acc_y = mavlink_msg_evt_accel_raw_get_acc_y(msg);
    evt_accel_raw->acc_z = mavlink_msg_evt_accel_raw_get_acc_z(msg);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_EVT_ACCEL_RAW_LEN? msg->len : MAVLINK_MSG_ID_EVT_ACCEL_RAW_LEN;
        memset(evt_accel_raw, 0, MAVLINK_MSG_ID_EVT_ACCEL_RAW_LEN);
    memcpy(evt_accel_raw, _MAV_PAYLOAD(msg), len);
#endif
}
