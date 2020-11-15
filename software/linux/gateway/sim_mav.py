from serial import Serial
import threading
import config
import time

from mavlink import *

serial = Serial(config.SERIAL_PORT_SIM, config.SERIAL_BAUD_SIM)
mav = MAVLink(serial)

NODE_MAX = 16

def loop_write():
    i = 0x00
    location_x = 123.456
    location_y = 99.2222
    location_z = 789.123
    while True:
        if(i > NODE_MAX/2):
            mav.location_send(i, STATUS, ANCHOR, location_x, location_y, location_z)
        else:
            mav.location_send(i, STATUS, TAG, location_x, location_y, location_z)
        mav.onoff_send(i, STATUS, 1)
        time.sleep(0.1)

        i+=1
        i %= NODE_MAX
        location_x += 1
        location_y += 2
        location_z += 10

def loop_read():
    while True:
        byte = serial.read()
        msg = mav.parse_char(byte)
        if(msg is not None):
            print(msg.to_json())

thread_write = threading.Thread(target=loop_write)
thread_read = threading.Thread(target=loop_read)

thread_write.start()
thread_read.start()