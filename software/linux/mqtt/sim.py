from serial import Serial
import threading
import config
import time

serial = Serial(config.SERIAL_PORT_SIM, config.SERIAL_BAUD_SIM)

def loop_write():
    while True:
        serial.write(b"STATUS {addr: 0xA01F, type: ANCHOR, location: [0.12345, 0.56789, 0.1234]}\n")
        serial.write(b"STATUS {addr: 0xA02F, type: ANCHOR, location: [0.12345, 0.56789, 0.1234]}\n")
        serial.write(b"STATUS {addr: 0xA03F, type: ANCHOR, location: [0.12345, 0.56789, 0.1234]}\n")
        time.sleep(10)

def loop_read():
    while True:
        line = serial.readline()
        print(line)

thread_write = threading.Thread(target=loop_write)
thread_read = threading.Thread(target=loop_read)

thread_write.start()
thread_read.start()