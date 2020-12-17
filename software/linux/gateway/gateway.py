import paho.mqtt.client as mqtt
from serial import Serial
import threading
import config
import json
import signal
import sys

from mavlink import *

class StoppableThread(threading.Thread):
    """Thread class with a stop() method. The thread itself has to check
    regularly for the stopped() condition."""

    def __init__(self,  *args, **kwargs):
        super(StoppableThread, self).__init__(*args, **kwargs)
        self._stop_event = threading.Event()

    def stop(self):
        self._stop_event.set()

    def stopped(self):
        return self._stop_event.is_set()

class Gateway:
    def __init__(self):
        pass

    def run(self):
        self.thread_mqtt = StoppableThread(target=self.thread_mqtt_func)
        self.thread_gateway = StoppableThread(target=self.thread_serial_func, args=[self.loop_gateway, config.SERIAL_PORT, config.SERIAL_BAUD])
        self.thread_tag = StoppableThread(target=self.thread_serial_func, args=[self.loop_tag, config.SERIAL_TAG_PORT, config.SERIAL_TAG_BAUD])
        try:
            self.thread_mqtt.start()
            self.thread_gateway.start()
            self.thread_tag.start()
        except:
            print("Error: unable to start thread")

    def stop(self):
        self.thread_mqtt.stop()
        self.thread_gateway.stop()
        self.client._sock_close()
        pass

    def on_connect(self, client, userdata, flags, rc):
        print("MQTT Broker connected.")
        self.client.subscribe(config.CMD_TOPIC)

    def on_disconnect(self, client, userdata, rc):
        print("MQTT Broker disconnected.")

    def on_message(self, client, userdata, msg):
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
            try:
                self.mav.location_send(msg["dstsrc"], msg["type"], msg["node"], msg["location_x"], msg["location_y"], msg["location_z"])
            except:
                print("Unable to send downlink msg")
        elif(msg["mavpackettype"] == "ONOFF"):
            try:
                self.mav.onoff_send(msg["dstsrc"], msg["type"], msg["value"])
            except:
                print("Unable to send downlink msg")

    def thread_mqtt_func(self):
        self.client = mqtt.Client()
        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message
        self.client.on_disconnect = self.on_disconnect
        self.client.connect("127.0.0.1", 1883, 60)
        self.client.loop_forever()

    def thread_serial_func(self, loop, port, baud):
        retry_count = 0
        while True:
            try:
                serial = Serial(port, baud)
                mav = MAVLink(serial)
                if (port == "/dev/ttyGateway"):
                    self.mav = mav
                break
            except:
                time.sleep(1)
        while True:
            try:
                if loop(serial, mav):
                    retry_count = 0
            except:
                time.sleep(0.5)
                try:
                    print("Serial connection error: Retrying... " + str(retry_count))
                    retry_count+=1
                    serial = Serial(port, baud)
                except:
                    print("Unable to open port: " + port)

    def loop_gateway(self, serial, mav):
        byte = serial.read()
        try:  
            msg = mav.parse_char(byte)
        except:
            print("Message parse exception: " + serial.name)
            return False
        if(msg is not None):
            try:
                msg = json.loads(msg.to_json())
                print("BLE->NET: %s" % msg)
            except ValueError as e:
                print(e)
                return False
                    
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

            self.client.publish(config.EVT_TOPIC, json.dumps(msg))
        return True

    def loop_tag(self, serial, mav):
        byte = serial.read()
        try:  
            msg = mav.parse_char(byte)
        except:
            print("Message parse exception: " + serial.name)
            return False
        if(msg is not None):
            try:
                msg = json.loads(msg.to_json())
                print("TAG->NET: %s" % msg)
            except ValueError as e:
                print(e)
                return False
            self.client.publish(config.EVT_TOPIC, json.dumps(msg))
        return True

    def signal_int(self, sig, frame):
        self.stop()

if __name__ == "__main__":
    gateway = Gateway()
    gateway.run()
    # with open("logo.txt") as f:
    #     for line in f.readlines():
    #         print(line)
    def signal_int(sig, frame):
        print(" Ctrl+C ---> Stop")
        gateway.signal_int(sig, frame)
        sys.exit(0)
    signal.signal(signal.SIGINT, signal_int)
