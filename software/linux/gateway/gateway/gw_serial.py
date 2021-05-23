import json
import time
import argparse
from serial import Serial
from serial.serialutil import Timeout

from mavlink import *
from gw_config import *

class GatewaySerial():
    def __init__(self, port, baud):
        print("Serial device: %s:%d" % (args.port, args.baud))
        self.port = port
        self.baud = baud
        self.serial = Serial()
        self.mav = None

    def open_serial(self):
        try:
            self.serial.close()
        except:
            pass
        
        retry_counter = 0
        while True:
            try:
                self.serial = Serial(self.port, self.baud, timeout=1)
                self.mav = MAVLink(self.serial)
                break
            except:
                time.sleep(1)
                retry_counter+=1
                print("Serial connection error: Retrying... %d" % retry_counter)
                pass

    def run(self):
        while True:
            if not self.loop():
                self.open_serial()
    
    def loop(self):
        try:
            byte = self.serial.read()
        except Exception as e:
            print("Message read exception: " + self.port)
            return False

        try:
            msg = self.mav.parse_char(byte)
        except:
            time.sleep(0.1)
            print("Message parse exception: " + self.port)
            return True
        
        if(msg is not None):
            try:
                msg = json.loads(msg.to_json())
                print("MESH->NET: %s" % json.dumps(msg))
            except ValueError as e:
                print(e)
                return False
            except:
                pass

        return True

if __name__ == "__main__":

    # Parse arguments
    parser = argparse.ArgumentParser()
    parser.add_argument("-p", "--port", help="Serial port charater device: /dev/ttyACM0, /dev/ttyUSB0, ...", default=GATEWAY_DEFAULT_PORT)
    parser.add_argument("-b", "--baud", help="Serial port baudrate: 115200, 9600, 57600, ...", type=int, default=GATEWAY_DEFAULT_BAUD)
    args = parser.parse_args()

    # Run the software
    gateway = GatewaySerial(args.port, args.baud)
    gateway.run()