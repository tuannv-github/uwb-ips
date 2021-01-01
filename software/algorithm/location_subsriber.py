import paho.mqtt.client as mqtt
import json

X = str(18*0.6)
Y = "2.4"
Z = "-3"

FILE_NAME = "location/" + X + "_" + Y + "_" + Z

client = mqtt.Client()
f = open(FILE_NAME, "w")
count = 0

def on_connect(client, userdata, flags, rc):
    client.subscribe("topic_evt")

def on_message(client, userdata, msg):
    global count
    msg = json.loads(msg.payload.decode("utf-8"))
    if msg["mavpackettype"] == 'TAG':
        string = ""
        string += str(msg["t0x"]) + " "
        string += str(msg["t0y"]) + " "
        string += str(msg["t0z"]) + " "
        f.write(string + "\n")
        count+=1
        print(str(count) + ": " + string)

client.on_connect = on_connect
client.on_message = on_message
client.connect("127.0.0.1", 1883, 60)
client.loop_forever()