#!/usr/bin/python3

from pylink.jlink import JLink

jlink = JLink()
for (i, emulator) in enumerate(jlink.connected_emulators()):
    print("Device %02d - SR: %d" % (i, emulator.SerialNumber))