from serial import Serial
import threading
import config
import time

serial = Serial(config.SERIAL_PORT_SIM, config.SERIAL_BAUD_SIM)

NODE_MAX = 10

def loop_write():
    address = 0
    location_x = 123.456
    location_y = 99.2222
    location_z = 789.123
    cnt = 0
    while True:
        if(address < NODE_MAX/2):
            str = '{"type": "STATUS", "msg": {"address": "%s", "type": "ANCHOR", "location": [%f, %f, %f]}}\n' % (address, location_x, location_y, location_z)
            serial.write(bytes(str, 'utf-8'))
        else:
            str = '{"type": "STATUS", "msg": {"address": "%s", "type": "TAG", "location": [%f, %f, %f]}}\n' % (address, location_x, location_y, location_z)
            serial.write(bytes(str, 'utf-8'))
        time.sleep(0.05)
        location_x += 1
        location_y += 2
        location_z += 10
        cnt += 1
        if(not (cnt%20)):
            address += 1
            address = address % NODE_MAX

def loop_read():
    while True:
        line = serial.readline()
        print(line)

thread_write = threading.Thread(target=loop_write)
thread_read = threading.Thread(target=loop_read)

thread_write.start()
thread_read.start()