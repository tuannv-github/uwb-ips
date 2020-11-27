/** @file
 *    @brief MAVLink comm protocol testsuite generated from protocol.xml
 *    @see http://qgroundcontrol.org/mavlink/
 */
#pragma once
#ifndef PROTOCOL_TESTSUITE_H
#define PROTOCOL_TESTSUITE_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MAVLINK_TEST_ALL
#define MAVLINK_TEST_ALL

static void mavlink_test_protocol(uint8_t, uint8_t, mavlink_message_t *last_msg);

static void mavlink_test_all(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{

    mavlink_test_protocol(system_id, component_id, last_msg);
}
#endif




static void mavlink_test_location(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
    mavlink_status_t *status = mavlink_get_channel_status(MAVLINK_COMM_0);
        if ((status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) && MAVLINK_MSG_ID_LOCATION >= 256) {
            return;
        }
#endif
    mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
    mavlink_location_t packet_in = {
        17.0,45.0,73.0,17859,175,242
    };
    mavlink_location_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        packet1.location_x = packet_in.location_x;
        packet1.location_y = packet_in.location_y;
        packet1.location_z = packet_in.location_z;
        packet1.dstsrc = packet_in.dstsrc;
        packet1.type = packet_in.type;
        packet1.node = packet_in.node;
        
        
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
        if (status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) {
           // cope with extensions
           memset(MAVLINK_MSG_ID_LOCATION_MIN_LEN + (char *)&packet1, 0, sizeof(packet1)-MAVLINK_MSG_ID_LOCATION_MIN_LEN);
        }
#endif
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_location_encode(system_id, component_id, &msg, &packet1);
    mavlink_msg_location_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_location_pack(system_id, component_id, &msg , packet1.dstsrc , packet1.type , packet1.node , packet1.location_x , packet1.location_y , packet1.location_z );
    mavlink_msg_location_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_location_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.dstsrc , packet1.type , packet1.node , packet1.location_x , packet1.location_y , packet1.location_z );
    mavlink_msg_location_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
            comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
    mavlink_msg_location_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_location_send(MAVLINK_COMM_1 , packet1.dstsrc , packet1.type , packet1.node , packet1.location_x , packet1.location_y , packet1.location_z );
    mavlink_msg_location_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
}

static void mavlink_test_onoff(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
    mavlink_status_t *status = mavlink_get_channel_status(MAVLINK_COMM_0);
        if ((status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) && MAVLINK_MSG_ID_ONOFF >= 256) {
            return;
        }
#endif
    mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
    mavlink_onoff_t packet_in = {
        17235,139,206
    };
    mavlink_onoff_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        packet1.dstsrc = packet_in.dstsrc;
        packet1.type = packet_in.type;
        packet1.value = packet_in.value;
        
        
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
        if (status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) {
           // cope with extensions
           memset(MAVLINK_MSG_ID_ONOFF_MIN_LEN + (char *)&packet1, 0, sizeof(packet1)-MAVLINK_MSG_ID_ONOFF_MIN_LEN);
        }
#endif
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_onoff_encode(system_id, component_id, &msg, &packet1);
    mavlink_msg_onoff_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_onoff_pack(system_id, component_id, &msg , packet1.dstsrc , packet1.type , packet1.value );
    mavlink_msg_onoff_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_onoff_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.dstsrc , packet1.type , packet1.value );
    mavlink_msg_onoff_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
            comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
    mavlink_msg_onoff_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_onoff_send(MAVLINK_COMM_1 , packet1.dstsrc , packet1.type , packet1.value );
    mavlink_msg_onoff_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
}

static void mavlink_test_distance(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
    mavlink_status_t *status = mavlink_get_channel_status(MAVLINK_COMM_0);
        if ((status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) && MAVLINK_MSG_ID_DISTANCE >= 256) {
            return;
        }
#endif
    mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
    mavlink_distance_t packet_in = {
        17.0,17443,17547,29
    };
    mavlink_distance_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        packet1.distance = packet_in.distance;
        packet1.tag = packet_in.tag;
        packet1.anchor = packet_in.anchor;
        packet1.type = packet_in.type;
        
        
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
        if (status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) {
           // cope with extensions
           memset(MAVLINK_MSG_ID_DISTANCE_MIN_LEN + (char *)&packet1, 0, sizeof(packet1)-MAVLINK_MSG_ID_DISTANCE_MIN_LEN);
        }
#endif
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_distance_encode(system_id, component_id, &msg, &packet1);
    mavlink_msg_distance_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_distance_pack(system_id, component_id, &msg , packet1.type , packet1.tag , packet1.anchor , packet1.distance );
    mavlink_msg_distance_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_distance_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.type , packet1.tag , packet1.anchor , packet1.distance );
    mavlink_msg_distance_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
            comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
    mavlink_msg_distance_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_distance_send(MAVLINK_COMM_1 , packet1.type , packet1.tag , packet1.anchor , packet1.distance );
    mavlink_msg_distance_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
}

static void mavlink_test_tof(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
    mavlink_status_t *status = mavlink_get_channel_status(MAVLINK_COMM_0);
        if ((status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) && MAVLINK_MSG_ID_TOF >= 256) {
            return;
        }
#endif
    mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
    mavlink_tof_t packet_in = {
        963497464,17443,17547,29
    };
    mavlink_tof_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        packet1.tof = packet_in.tof;
        packet1.tag = packet_in.tag;
        packet1.anchor = packet_in.anchor;
        packet1.type = packet_in.type;
        
        
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
        if (status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) {
           // cope with extensions
           memset(MAVLINK_MSG_ID_TOF_MIN_LEN + (char *)&packet1, 0, sizeof(packet1)-MAVLINK_MSG_ID_TOF_MIN_LEN);
        }
#endif
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_tof_encode(system_id, component_id, &msg, &packet1);
    mavlink_msg_tof_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_tof_pack(system_id, component_id, &msg , packet1.type , packet1.tag , packet1.anchor , packet1.tof );
    mavlink_msg_tof_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_tof_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.type , packet1.tag , packet1.anchor , packet1.tof );
    mavlink_msg_tof_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
            comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
    mavlink_msg_tof_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_tof_send(MAVLINK_COMM_1 , packet1.type , packet1.tag , packet1.anchor , packet1.tof );
    mavlink_msg_tof_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
}

static void mavlink_test_protocol(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
    mavlink_test_location(system_id, component_id, last_msg);
    mavlink_test_onoff(system_id, component_id, last_msg);
    mavlink_test_distance(system_id, component_id, last_msg);
    mavlink_test_tof(system_id, component_id, last_msg);
}

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // PROTOCOL_TESTSUITE_H
