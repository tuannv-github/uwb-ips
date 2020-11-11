import paho.mqtt.client as mqtt
from serial import Serial
import threading
import config
import json

from mavlink import *

serial = Serial(config.SERIAL_PORT, config.SERIAL_BAUD)
mav = MAVLink(serial)
client = mqtt.Client()

def on_connect(client, userdata, flags, rc):
    client.subscribe(config.CMD_TOPIC)

def on_message(client, userdata, msg):
    print("NET->BLE: " + config.CMD_TOPIC + ": " + msg.payload.decode("utf-8"))

    try:
        msg = json.loads(msg.payload.decode("utf-8"))
    except ValueError as e:
        print(e)
        return

    if(msg["mavpackettype"] == "LOCATION"):
        mav.location_send(msg["dstsrc"], msg["msg_type"], msg["location_x"], msg["location_y"], msg["location_z"])
    elif(msg["mavpackettype"] == "ONOFF"):
        mav.location_send(msg["dstsrc"], msg["msg_type"], msg["value"])
    
client.on_connect = on_connect
client.on_message = on_message
client.connect("127.0.0.1", 1883, 60)

def loop_mqtt():
    client.loop_forever()

def loop_serial():
    while True:
        byte = serial.read()
        msg = mav.parse_char(byte)
        if(msg is not None):
            client.publish(config.EVT_TOPIC, str(msg.to_json()))

thread_mqtt = threading.Thread(target=loop_mqtt)
thread_serial = threading.Thread(target=loop_serial)

try:
    thread_mqtt.start()
    thread_serial.start()
except:
   print("Error: unable to start thread")