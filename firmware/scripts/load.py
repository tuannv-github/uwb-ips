import pylink
import os
import sys

build_template  = "newt build %s"
image_template  = "newt create-image %s 0.0.0.0"
load_template   = 'newt load %s --extrajtagcmd "-select USB=%d"'

def run_cmd(serial_number, app):
    global load_template
    cmd = load_template % (app, serial_number)
    print("--- CMD: " + cmd)
    print("Loading " + app + " to:", str(serial_number))
    os.system(cmd)

def main(argv):
    print(argv[0])
    cmd = build_template % (argv[0])
    os.system(cmd)
    cmd = image_template % (argv[0])
    os.system(cmd)
    jlink = pylink.JLink()
    for (i, emulator) in enumerate(jlink.connected_emulators()):
        run_cmd(emulator.SerialNumber, argv[0])

if __name__ == "__main__":
    main(sys.argv[1:])