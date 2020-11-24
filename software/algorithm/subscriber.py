import paho.mqtt.client as mqtt
import json

client = mqtt.Client()

def on_connect(client, userdata, flags, rc):
    client.subscribe("topic_evt")

def on_message(client, userdata, msg):
    msg = json.loads(msg.payload.decode("utf-8"))
    if(msg["mavpackettype"] == "TOF"):
        if(msg["anchor"] == 3999):
            print(msg)

client.on_connect = on_connect
client.on_message = on_message
client.connect("127.0.0.1", 1883, 60)
client.loop_forever()