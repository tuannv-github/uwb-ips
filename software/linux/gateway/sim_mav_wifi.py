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

STD_DEVIATION = 0.1

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
    anchors = []
    tags = []

    anchor = Node(0, ANCHOR, 0, 0, 1)
    anchors.append(anchor)

    anchor = Node(1, ANCHOR, 5, 0, 2)
    anchors.append(anchor)

    anchor = Node(2, ANCHOR, 5, 5, 3)
    anchors.append(anchor)

    anchor = Node(3, ANCHOR, 0, 5, 4)
    anchors.append(anchor)

    tag = Node(4, TAG, 0, 1, 0)
    tags.append(tag)
    tag.move()

    while True:
        for tag in tags:
            tag.move()

            anchor0 = anchors[0]
            dist0 = m.sqrt((tag.getX()-anchor0.getX())**2 + (tag.getY()-anchor0.getY())**2 + (tag.getZ()-anchor0.getZ())**2)
            dist0 += (random.random()-0.5)*STD_DEVIATION
            
            anchor1 = anchors[1]
            dist1 = m.sqrt((tag.getX()-anchor1.getX())**2 + (tag.getY()-anchor1.getY())**2 + (tag.getZ()-anchor1.getZ())**2)
            dist1 += (random.random()-0.5)*STD_DEVIATION

            anchor2 = anchors[2]
            dist2 = m.sqrt((tag.getX()-anchor2.getX())**2 + (tag.getY()-anchor2.getY())**2 + (tag.getZ()-anchor2.getZ())**2)
            dist2 += (random.random()-0.5)*STD_DEVIATION

            anchor3 = anchors[3]
            dist3 = m.sqrt((tag.getX()-anchor3.getX())**2 + (tag.getY()-anchor3.getY())**2 + (tag.getZ()-anchor3.getZ())**2)
            dist3 += (random.random()-0.5)*STD_DEVIATION

            mav.tag_send(   anchor0.getA(), anchor0.getX(), anchor0.getY(), anchor0.getZ(), dist0, 
                            anchor1.getA(), anchor1.getX(), anchor1.getY(), anchor1.getZ(), dist1,
                            anchor2.getA(), anchor2.getX(), anchor2.getY(), anchor2.getZ(), dist2,
                            anchor3.getA(), anchor3.getX(), anchor3.getY(), anchor3.getZ(), dist3,
                            tag.getA(), tag.getX(), tag.getY(), tag.getZ())
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