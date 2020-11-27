import paho.mqtt.client as mqtt
import json

DISTANCE_M = "1.5"
FILE_NAME = "distance/" + DISTANCE_M

client = mqtt.Client()
f = open(FILE_NAME, "w")
count = 0

def on_connect(client, userdata, flags, rc):
    client.subscribe("topic_evt")

def on_message(client, userdata, msg):
    global count
    msg = json.loads(msg.payload.decode("utf-8"))
    if(msg["mavpackettype"] == 'TOF'):
        f.write(str(msg["tof"]) + "\n")
        count+=1
        print(str(count) + ": " + str(msg))

client.on_connect = on_connect
client.on_message = on_message
client.connect("127.0.0.1", 1883, 60)
client.loop_forever()