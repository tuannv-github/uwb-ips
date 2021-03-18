#!/usr/bin/python3

from pylink.jlink import JLink
import os
import sys

build_template  = "newt build %s"
image_template  = "newt create-image %s 0.0.0.0"
load_template   = 'newt load %s --extrajtagcmd "-select USB=%d"'

def main(argv, snl):
    cmd = build_template % (argv[0])
    os.system(cmd)
    cmd = image_template % (argv[0])
    os.system(cmd)
    jlink = JLink()
    counter = 0
    for emulator in jlink.connected_emulators():
        if snl is None or emulator.SerialNumber in snl:
            counter += 1
            serial_number = emulator.SerialNumber
            app_name = argv[0]
            cmd = load_template % (app_name, serial_number)
            print("--- CMD %d: %s" % (counter, cmd))
            print("Loading program '%s' to %d" % (app_name, serial_number))
            os.system(cmd)

if __name__ == "__main__":
    if len(sys.argv) == 2:
        val = input("You are trying to load program '%s' to all devices. Are you sure (y/n): " % (sys.argv[1]))
        if val == 'y':
            main(sys.argv[1:], None)
    elif len(sys.argv) == 3 and os.path.isfile("sn_" + sys.argv[2]):
        f = open("sn_" + sys.argv[2], 'r')
        snl = []
        for (i, line) in enumerate(f.readlines()):
            try:
                snl.append(float(line))
            except:
                print("Parse error at line %d: %s" % (i+1,line))
        main(sys.argv[1:], snl)
    else:
        print("%s <apps> <sn>" % __file__)