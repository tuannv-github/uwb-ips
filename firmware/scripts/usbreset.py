#!/usr/bin/env python

from usb.core import find as finddev

devices = finddev(find_all=1, idVendor=0x1366)
for dev in devices:
    try:
        dev.reset()
    except:
        pass