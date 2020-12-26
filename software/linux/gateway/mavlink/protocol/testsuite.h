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




static void mavlink_test_blink(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
    mavlink_status_t *status = mavlink_get_channel_status(MAVLINK_COMM_0);
        if ((status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) && MAVLINK_MSG_ID_BLINK >= 256) {
            return;
        }
#endif
    mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
    mavlink_blink_t packet_in = {
        17235,139
    };
    mavlink_blink_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        packet1.uwb_address = packet_in.uwb_address;
        packet1.role = packet_in.role;
        
        
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
        if (status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) {
           // cope with extensions
           memset(MAVLINK_MSG_ID_BLINK_MIN_LEN + (char *)&packet1, 0, sizeof(packet1)-MAVLINK_MSG_ID_BLINK_MIN_LEN);
        }
#endif
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_blink_encode(system_id, component_id, &msg, &packet1);
    mavlink_msg_blink_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_blink_pack(system_id, component_id, &msg , packet1.uwb_address , packet1.role );
    mavlink_msg_blink_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_blink_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.uwb_address , packet1.role );
    mavlink_msg_blink_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
            comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
    mavlink_msg_blink_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_blink_send(MAVLINK_COMM_1 , packet1.uwb_address , packet1.role );
    mavlink_msg_blink_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
}

static void mavlink_test_ble_mesh(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
    mavlink_status_t *status = mavlink_get_channel_status(MAVLINK_COMM_0);
        if ((status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) && MAVLINK_MSG_ID_BLE_MESH >= 256) {
            return;
        }
#endif
    mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
    mavlink_ble_mesh_t packet_in = {
        17235,17339
    };
    mavlink_ble_mesh_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        packet1.uwb_address = packet_in.uwb_address;
        packet1.mesh_address = packet_in.mesh_address;
        
        
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
        if (status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) {
           // cope with extensions
           memset(MAVLINK_MSG_ID_BLE_MESH_MIN_LEN + (char *)&packet1, 0, sizeof(packet1)-MAVLINK_MSG_ID_BLE_MESH_MIN_LEN);
        }
#endif
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_ble_mesh_encode(system_id, component_id, &msg, &packet1);
    mavlink_msg_ble_mesh_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_ble_mesh_pack(system_id, component_id, &msg , packet1.uwb_address , packet1.mesh_address );
    mavlink_msg_ble_mesh_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_ble_mesh_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.uwb_address , packet1.mesh_address );
    mavlink_msg_ble_mesh_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
            comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
    mavlink_msg_ble_mesh_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_ble_mesh_send(MAVLINK_COMM_1 , packet1.uwb_address , packet1.mesh_address );
    mavlink_msg_ble_mesh_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
}

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
        17.0,45.0,73.0,17859
    };
    mavlink_location_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        packet1.location_x = packet_in.location_x;
        packet1.location_y = packet_in.location_y;
        packet1.location_z = packet_in.location_z;
        packet1.uwb_address = packet_in.uwb_address;
        
        
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
    mavlink_msg_location_pack(system_id, component_id, &msg , packet1.uwb_address , packet1.location_x , packet1.location_y , packet1.location_z );
    mavlink_msg_location_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_location_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.uwb_address , packet1.location_x , packet1.location_y , packet1.location_z );
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
    mavlink_msg_location_send(MAVLINK_COMM_1 , packet1.uwb_address , packet1.location_x , packet1.location_y , packet1.location_z );
    mavlink_msg_location_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
}

static void mavlink_test_location_reduced(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
    mavlink_status_t *status = mavlink_get_channel_status(MAVLINK_COMM_0);
        if ((status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) && MAVLINK_MSG_ID_LOCATION_REDUCED >= 256) {
            return;
        }
#endif
    mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
    mavlink_location_reduced_t packet_in = {
        17.0,45.0,17651
    };
    mavlink_location_reduced_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        packet1.location_x = packet_in.location_x;
        packet1.location_y = packet_in.location_y;
        packet1.uwb_address = packet_in.uwb_address;
        
        
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
        if (status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) {
           // cope with extensions
           memset(MAVLINK_MSG_ID_LOCATION_REDUCED_MIN_LEN + (char *)&packet1, 0, sizeof(packet1)-MAVLINK_MSG_ID_LOCATION_REDUCED_MIN_LEN);
        }
#endif
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_location_reduced_encode(system_id, component_id, &msg, &packet1);
    mavlink_msg_location_reduced_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_location_reduced_pack(system_id, component_id, &msg , packet1.uwb_address , packet1.location_x , packet1.location_y );
    mavlink_msg_location_reduced_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_location_reduced_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.uwb_address , packet1.location_x , packet1.location_y );
    mavlink_msg_location_reduced_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
            comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
    mavlink_msg_location_reduced_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_location_reduced_send(MAVLINK_COMM_1 , packet1.uwb_address , packet1.location_x , packet1.location_y );
    mavlink_msg_location_reduced_decode(last_msg, &packet2);
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
        17235,139
    };
    mavlink_onoff_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        packet1.uwb_address = packet_in.uwb_address;
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
    mavlink_msg_onoff_pack(system_id, component_id, &msg , packet1.uwb_address , packet1.value );
    mavlink_msg_onoff_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_onoff_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.uwb_address , packet1.value );
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
    mavlink_msg_onoff_send(MAVLINK_COMM_1 , packet1.uwb_address , packet1.value );
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
        17.0,17443,17547
    };
    mavlink_distance_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        packet1.distance = packet_in.distance;
        packet1.uwb_address = packet_in.uwb_address;
        packet1.anchor = packet_in.anchor;
        
        
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
    mavlink_msg_distance_pack(system_id, component_id, &msg , packet1.uwb_address , packet1.anchor , packet1.distance );
    mavlink_msg_distance_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_distance_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.uwb_address , packet1.anchor , packet1.distance );
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
    mavlink_msg_distance_send(MAVLINK_COMM_1 , packet1.uwb_address , packet1.anchor , packet1.distance );
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
        963497464,17443,17547
    };
    mavlink_tof_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        packet1.tof = packet_in.tof;
        packet1.uwb_address = packet_in.uwb_address;
        packet1.anchor = packet_in.anchor;
        
        
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
    mavlink_msg_tof_pack(system_id, component_id, &msg , packet1.uwb_address , packet1.anchor , packet1.tof );
    mavlink_msg_tof_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_tof_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.uwb_address , packet1.anchor , packet1.tof );
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
    mavlink_msg_tof_send(MAVLINK_COMM_1 , packet1.uwb_address , packet1.anchor , packet1.tof );
    mavlink_msg_tof_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
}

static void mavlink_test_slot(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
    mavlink_status_t *status = mavlink_get_channel_status(MAVLINK_COMM_0);
        if ((status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) && MAVLINK_MSG_ID_SLOT >= 256) {
            return;
        }
#endif
    mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
    mavlink_slot_t packet_in = {
        17235,139
    };
    mavlink_slot_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        packet1.uwb_address = packet_in.uwb_address;
        packet1.slot = packet_in.slot;
        
        
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
        if (status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) {
           // cope with extensions
           memset(MAVLINK_MSG_ID_SLOT_MIN_LEN + (char *)&packet1, 0, sizeof(packet1)-MAVLINK_MSG_ID_SLOT_MIN_LEN);
        }
#endif
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_slot_encode(system_id, component_id, &msg, &packet1);
    mavlink_msg_slot_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_slot_pack(system_id, component_id, &msg , packet1.uwb_address , packet1.slot );
    mavlink_msg_slot_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_slot_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.uwb_address , packet1.slot );
    mavlink_msg_slot_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
            comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
    mavlink_msg_slot_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_slot_send(MAVLINK_COMM_1 , packet1.uwb_address , packet1.slot );
    mavlink_msg_slot_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
}

static void mavlink_test_tag(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
    mavlink_status_t *status = mavlink_get_channel_status(MAVLINK_COMM_0);
        if ((status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) && MAVLINK_MSG_ID_TAG >= 256) {
            return;
        }
#endif
    mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t i;
    mavlink_tag_t packet_in = {
        17.0,45.0,73.0,101.0,129.0,157.0,185.0,213.0,241.0,269.0,297.0,325.0,353.0,381.0,409.0,437.0,465.0,493.0,521.0,21187,21291,21395,21499,21603
    };
    mavlink_tag_t packet1, packet2;
        memset(&packet1, 0, sizeof(packet1));
        packet1.a0x = packet_in.a0x;
        packet1.a0y = packet_in.a0y;
        packet1.a0z = packet_in.a0z;
        packet1.a0r = packet_in.a0r;
        packet1.a1x = packet_in.a1x;
        packet1.a1y = packet_in.a1y;
        packet1.a1z = packet_in.a1z;
        packet1.a1r = packet_in.a1r;
        packet1.a2x = packet_in.a2x;
        packet1.a2y = packet_in.a2y;
        packet1.a2z = packet_in.a2z;
        packet1.a2r = packet_in.a2r;
        packet1.a3x = packet_in.a3x;
        packet1.a3y = packet_in.a3y;
        packet1.a3z = packet_in.a3z;
        packet1.a3r = packet_in.a3r;
        packet1.t0x = packet_in.t0x;
        packet1.t0y = packet_in.t0y;
        packet1.t0z = packet_in.t0z;
        packet1.a0 = packet_in.a0;
        packet1.a1 = packet_in.a1;
        packet1.a2 = packet_in.a2;
        packet1.a3 = packet_in.a3;
        packet1.t0 = packet_in.t0;
        
        
#ifdef MAVLINK_STATUS_FLAG_OUT_MAVLINK1
        if (status->flags & MAVLINK_STATUS_FLAG_OUT_MAVLINK1) {
           // cope with extensions
           memset(MAVLINK_MSG_ID_TAG_MIN_LEN + (char *)&packet1, 0, sizeof(packet1)-MAVLINK_MSG_ID_TAG_MIN_LEN);
        }
#endif
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_tag_encode(system_id, component_id, &msg, &packet1);
    mavlink_msg_tag_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_tag_pack(system_id, component_id, &msg , packet1.a0 , packet1.a0x , packet1.a0y , packet1.a0z , packet1.a0r , packet1.a1 , packet1.a1x , packet1.a1y , packet1.a1z , packet1.a1r , packet1.a2 , packet1.a2x , packet1.a2y , packet1.a2z , packet1.a2r , packet1.a3 , packet1.a3x , packet1.a3y , packet1.a3z , packet1.a3r , packet1.t0 , packet1.t0x , packet1.t0y , packet1.t0z );
    mavlink_msg_tag_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_tag_pack_chan(system_id, component_id, MAVLINK_COMM_0, &msg , packet1.a0 , packet1.a0x , packet1.a0y , packet1.a0z , packet1.a0r , packet1.a1 , packet1.a1x , packet1.a1y , packet1.a1z , packet1.a1r , packet1.a2 , packet1.a2x , packet1.a2y , packet1.a2z , packet1.a2r , packet1.a3 , packet1.a3x , packet1.a3y , packet1.a3z , packet1.a3r , packet1.t0 , packet1.t0x , packet1.t0y , packet1.t0z );
    mavlink_msg_tag_decode(&msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);

        memset(&packet2, 0, sizeof(packet2));
        mavlink_msg_to_send_buffer(buffer, &msg);
        for (i=0; i<mavlink_msg_get_send_buffer_length(&msg); i++) {
            comm_send_ch(MAVLINK_COMM_0, buffer[i]);
        }
    mavlink_msg_tag_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
        
        memset(&packet2, 0, sizeof(packet2));
    mavlink_msg_tag_send(MAVLINK_COMM_1 , packet1.a0 , packet1.a0x , packet1.a0y , packet1.a0z , packet1.a0r , packet1.a1 , packet1.a1x , packet1.a1y , packet1.a1z , packet1.a1r , packet1.a2 , packet1.a2x , packet1.a2y , packet1.a2z , packet1.a2r , packet1.a3 , packet1.a3x , packet1.a3y , packet1.a3z , packet1.a3r , packet1.t0 , packet1.t0x , packet1.t0y , packet1.t0z );
    mavlink_msg_tag_decode(last_msg, &packet2);
        MAVLINK_ASSERT(memcmp(&packet1, &packet2, sizeof(packet1)) == 0);
}

static void mavlink_test_protocol(uint8_t system_id, uint8_t component_id, mavlink_message_t *last_msg)
{
    mavlink_test_blink(system_id, component_id, last_msg);
    mavlink_test_ble_mesh(system_id, component_id, last_msg);
    mavlink_test_location(system_id, component_id, last_msg);
    mavlink_test_location_reduced(system_id, component_id, last_msg);
    mavlink_test_onoff(system_id, component_id, last_msg);
    mavlink_test_distance(system_id, component_id, last_msg);
    mavlink_test_tof(system_id, component_id, last_msg);
    mavlink_test_slot(system_id, component_id, last_msg);
    mavlink_test_tag(system_id, component_id, last_msg);
}

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // PROTOCOL_TESTSUITE_H
