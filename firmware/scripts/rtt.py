#!/usr/bin/env python

import pylink
import threading
from subprocess import PIPE, run

BASE_PORT = 20000
jlink = pylink.JLink()

def os_cmd(command):
    result = run(command, stdout=PIPE, stderr=PIPE, universal_newlines=True, shell=True)
    return result.stdout

cmd_template = '''JLinkGDBServer -device NRF52 -if swd \
-endian little -port %d -swoport %d -telnetport %d -RTTTelnetport %d \
-vd -noir -strict -timeout 0 -select USB=%d -nohalt'''

def run_cmd(serial_number, idx):
    cmd = cmd_template % (BASE_PORT + 4*idx, BASE_PORT + 4*idx + 1, BASE_PORT + 4*idx +2, BASE_PORT + 4*idx + 3, serial_number)
    print(str(serial_number) + ": JLinkRTTClient -RTTTelnetPort " + str(BASE_PORT + 4*idx + 3))
    os_cmd(cmd)

for (i, emulator) in enumerate(jlink.connected_emulators()):
    thread = threading.Thread(target=run_cmd, args=(emulator.SerialNumber, i))
    thread.start()