#!/usr/bin/python3

import pylink

jlink = pylink.JLink()
for (i, emulator) in enumerate(jlink.connected_emulators()):
    print("Device %02d - SR: %d" % (i, emulator.SerialNumber))