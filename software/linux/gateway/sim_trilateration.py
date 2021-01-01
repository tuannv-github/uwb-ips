from time import sleep
from serial import Serial
import threading
import config
import time
import random
import math as m

from mavlink import *

serial = Serial(config.SERIAL_PORT_SIM, config.SERIAL_BAUD_SIM)
mav = MAVLink(serial)

STD_DEVIATION = 0.5

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
            # self.z = random.random()*5

def loop_write():
    i = 0x00
    anchors = []
    tags = []

    anchor = Node(0, ANCHOR, 0, 0, 0)
    anchors.append(anchor)

    anchor = Node(1, ANCHOR, 5, 0, 0)
    anchors.append(anchor)

    anchor = Node(2, ANCHOR, 5, 5, 0)
    anchors.append(anchor)

    anchor = Node(3, ANCHOR, 0, 5, 0)
    anchors.append(anchor)

    tag = Node(4, TAG, 0, 1, 0)
    tags.append(tag)
    tag.move()

    while True:
        for anchor in anchors:
            mav.location_send(anchor.getA(), STATUS, anchor.getT(), anchor.getX(), anchor.getY(), anchor.getZ())
            time.sleep(0.5)
        for tag in tags:
            tag.move()

            anchor = anchors[0]
            dist = m.sqrt((tag.getX()-anchor.getX())**2 + (tag.getY()-anchor.getY())**2 + (tag.getZ()-anchor.getZ())**2)
            dist += (random.random()-0.5)*STD_DEVIATION
            mav.distance_send(STATUS, tag.getA(), anchor.getA(), dist)
            
            anchor = anchors[1]
            dist = m.sqrt((tag.getX()-anchor.getX())**2 + (tag.getY()-anchor.getY())**2 + (tag.getZ()-anchor.getZ())**2)
            dist += (random.random()-0.5)*STD_DEVIATION
            mav.distance_send(STATUS, tag.getA(), anchor.getA(), dist)

            anchor = anchors[2]
            dist = m.sqrt((tag.getX()-anchor.getX())**2 + (tag.getY()-anchor.getY())**2 + (tag.getZ()-anchor.getZ())**2)
            dist += (random.random()-0.5)*STD_DEVIATION
            mav.distance_send(STATUS, tag.getA(), anchor.getA(), dist)

            anchor = anchors[3]
            dist = m.sqrt((tag.getX()-anchor.getX())**2 + (tag.getY()-anchor.getY())**2 + (tag.getZ()-anchor.getZ())**2)
            dist += (random.random()-0.5)*STD_DEVIATION
            mav.distance_send(STATUS, tag.getA(), anchor.getA(), dist)

            mav.location_send(tag.getA(), STATUS, tag.getT(), tag.getX(), tag.getY(), tag.getZ())
            time.sleep(0.5)  

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