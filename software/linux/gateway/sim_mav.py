from time import sleep
from serial import Serial
import threading
import config
import time
import random

from mavlink import *

serial = Serial(config.SERIAL_PORT_SIM, config.SERIAL_BAUD_SIM)
mav = MAVLink(serial)

NODE_MAX = 16

class Node:
    def __init__(self, a, t, x, y , z):
        self.a = a
        self.x = x
        self.y = y
        self.z = z
        self.t = t

    def getX(self):
        return self.x
    def getY(self):
        return self.y
    def getZ(self):
        return self.z
    def getT(self):
        return self.t
    def getA(self):
        return self.a
    def move(self):
        if(self.t == TAG):
            self.x = random.random()*5
            self.y = random.random()*5
            self.z = random.random()*5

def loop_write():
    i = 0x00
    nodes = []

    node = Node(0, ANCHOR, 0, 1, 2)
    nodes.append(node)

    node = Node(1, ANCHOR, 1, 4, 5)
    nodes.append(node)

    node = Node(2, ANCHOR, 5, 2, 5)
    nodes.append(node)

    node = Node(3, TAG, 0, 1, 2)
    nodes.append(node)

    node = Node(4, TAG, 0, 1, 2)
    nodes.append(node)

    node = Node(5, ANCHOR, 3, 2, 5)
    nodes.append(node)

    while True:
        for node in nodes:
            node.move()
            mav.location_send(node.getA(), STATUS, node.getT(), node.getX(), node.getY(), node.getZ())
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