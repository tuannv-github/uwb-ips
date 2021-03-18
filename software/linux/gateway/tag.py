from serial import Serial
import signal
import sys

from mavlink import *

PORT = "/dev/ttyTag"
BAUD = 115200
class TAG:
    def __init__(self):
        self.stop = False

    def run(self):
        retry_count = 0
        while not self.stop:
            try:
                serial = Serial(PORT, BAUD)
                mav = MAVLink(serial)
                self.mav = mav
                break
            except:
                time.sleep(1)
        while not self.stop:
            try:
                if self.loop_tag(serial, self.mav):
                    retry_count = 0
            except:
                time.sleep(0.5)
                retry_count+=1
                if(retry_count < 10):
                    print("Serial connection error: Retrying... " + str(retry_count))
                try:
                    serial = Serial(PORT, BAUD)
                    mav = MAVLink(serial)
                    self.mav = mav
                except:
                    if(retry_count < 10):
                        print("Unable to open port: " + "/dev/ttyTag")

    def loop_tag(self, serial, mav):
        try:
            byte = serial.read()
        except:
            return False
        try:  
            msg = mav.parse_char(byte)
        except:
            print("Message parse exception: " + serial.name)
            return False
        if(msg is not None):
            print(msg.to_json())
        return True

    def stop_now(self):
        self.stop = True

if __name__ == "__main__":
    tag = TAG()
    def signal_int(sig, frame):
        tag.stop_now()
        print(" Ctrl+C ---> Stop")
        sys.exit(0)
    signal.signal(signal.SIGINT, signal_int)
    tag.run()

