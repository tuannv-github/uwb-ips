from serial import Serial
import threading
import config
import time

from mavlink import *

serial = Serial(config.SERIAL_PORT_SIM, config.SERIAL_BAUD_SIM)
mav = MAVLink(serial)

def loop_write():
    while True:
        mav.location_send(0xFAAB, MSG_GET, 0.123, 987.123, 999.456)
        time.sleep(1)

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