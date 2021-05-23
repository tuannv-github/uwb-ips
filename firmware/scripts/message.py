import argparse

class GatewaySerial():
    def __init__(self, port='/dev/ttyGateway', baud=115200):
        pass
    
if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-p", "--port", help="Serial port charater device: /dev/ttyACM0, /dev/ttyUSB0, ...")
    parser.add_argument("-b", "--baud", help="Serial port baudrate: 115200, 9600, 57600, ...")
    args = parser.parse_args()
    print("%s %d" % (args.port, args.baud))
    gateway = GatewaySerial()
    gateway.run()