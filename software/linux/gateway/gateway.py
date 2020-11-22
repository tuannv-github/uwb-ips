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
    print("NET->BLE: %s" % (msg.payload.decode("utf-8")))
    try:
        msg = json.loads(msg.payload.decode("utf-8"))
    except ValueError as e:
        print(e)
        return

    if(msg["type"] == "GET"): 
        msg["type"] = GET
    elif(msg["type"] == "SET"):
        msg["type"] = SET
    elif(msg["type"] == "SET_UNACK"):
        msg["type"] = SET_UNACK
    elif(msg["type"] == "STATUS"):
        msg["type"] = STATUS
    else:
        return

    if(msg["mavpackettype"] == "LOCATION"):
        if(msg["node"] == "ANCHOR"): 
            msg["node"] = ANCHOR
        elif(msg["node"] == "TAG"):
            msg["node"] = TAG
        mav.location_send(msg["dstsrc"], msg["type"], msg["node"], msg["location_x"], msg["location_y"], msg["location_z"])
    elif(msg["mavpackettype"] == "ONOFF"):
        mav.onoff_send(msg["dstsrc"], msg["type"], msg["value"])

client.on_connect = on_connect
client.on_message = on_message
client.connect("127.0.0.1", 1883, 60)

def loop_mqtt():
    client.loop_forever()

def loop_serial():
    while True:
        byte = serial.read()
        try:  
            msg = mav.parse_char(byte)
        except:
            print("Message parse exception")
            continue
        if(msg is not None):
            print("BLE->NET: %s" % (msg.to_json()))
            try:
                msg = json.loads(msg.to_json())
            except ValueError as e:
                print(e)
                return
            
            if(msg["type"] == GET): 
                msg["type"] = "GET"
            elif(msg["type"] == SET):
                msg["type"] = "SET"
            elif(msg["type"] == SET_UNACK):
                msg["type"] = "SET_UNACK"
            elif(msg["type"] == STATUS):
                msg["type"] = "STATUS"
            
            try:
                if(msg["node"] == ANCHOR):
                    msg["node"] = "ANCHOR"
                elif(msg["node"] == TAG):
                    msg["node"] = "TAG"
            except:
                pass

            client.publish(config.EVT_TOPIC, json.dumps(msg))

thread_mqtt = threading.Thread(target=loop_mqtt)
thread_serial = threading.Thread(target=loop_serial)

try:
    thread_mqtt.start()
    thread_serial.start()
except:
   print("Error: unable to start thread")