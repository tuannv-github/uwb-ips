from serial import Serial
import threading

import signal
import sys

serial = Serial("/dev/ttyACM0", 115200, timeout=1.0)

f_send = open("file_tx.txt", "r")
f_recv = open("file_rx.txt", "w")

do_exit = False

def loop_write():
    for line in f_send.readlines():
        print(line[:-1])
        serial.write(line.encode('utf-8'))

def loop_read():
    while not do_exit:
        line = serial.readline()
        if line:
            f_recv.write(line.decode("utf-8"))

thread_write = threading.Thread(target=loop_write)
thread_read = threading.Thread(target=loop_read)

thread_write.start()
thread_read.start()

def signal_handler(sig, frame):
    global do_exit
    print('You pressed Ctrl+C!')
    f_send.close()
    f_recv.close()
    do_exit = True
    sys.exit(0)

signal.signal(signal.SIGINT, signal_handler)
signal.pause()