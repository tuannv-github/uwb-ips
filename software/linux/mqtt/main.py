import paho.mqtt.client as mqtt
from serial import Serial
import threading
import config

serial = Serial(config.SERIAL_PORT, config.SERIAL_BAUD)
client = mqtt.Client()

def on_connect(client, userdata, flags, rc):
    client.subscribe(config.LOCATION_CMD_TOPIC)

def on_message(client, userdata, msg):
    print("NET->BLE: " + msg.payload.decode("utf-8"))
    serial.write(msg.payload + b'\n')

client.on_connect = on_connect
client.on_message = on_message
client.connect("127.0.0.1", 1883, 60)

def loop_mqtt():
    client.loop_forever()

def loop_serial():
    while True:
        byteline = serial.readline()
        if(len(byteline) < 2): 
            continue
        if(byteline[-2] == b'r'):
            line  = byteline[:-2].decode("utf-8")
        else:
            line  = byteline[:-1].decode("utf-8")
        print("BLE->NET: " + line)
        client.publish(config.LOCATION_EVT_TOPIC, line)

thread_mqtt = threading.Thread(target=loop_mqtt)
thread_serial = threading.Thread(target=loop_serial)

try:
    thread_mqtt.start()
    thread_serial.start()
except:
   print("Error: unable to start thread")