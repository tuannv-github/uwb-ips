#!/usr/bin/env python

import os
import re
import sys
import time
import queue
import signal
import struct
import logging
import threading
from argparse import ArgumentParser
from binascii import a2b_hex
from distutils.sysconfig import get_python_lib
from serial import Serial, serialutil
from serial.tools.list_ports import comports

class UwbSniffer(object):

    # Various options for pcap files: http://www.tcpdump.org/linktypes.html
    DLT_USER0               = 147
    DLT_IEEE802_15_4_NOFCS  = 230
    DLT_IEEE802_15_4_TAP    = 283

    # Pattern for packets being printed over serial.
    RCV_REGEX = 'received:\s+([0-9a-fA-F]+)\s+power:\s+(-?\d+)\s+lqi:\s+(\d+)\s+time:\s+(-?\d+)'

    TIMER_MAX = 2**32

    def __init__(self):
        self.serial = None
        self.running = threading.Event()
        self.setup_done = threading.Event()

        self.dev = None
        self.channel = None
        self.dlt = None
        self.threads = []

        # Time correction variables.
        self.first_local_timestamp = None
        self.first_sniffer_timestamp = None

    @staticmethod
    def extcap_interfaces():
        """
        Wireshark-related method that returns configuration options.
        :return: string with wireshark-compatible information
        """
        res = []
        res.append("extcap {version=0.0.1}{display=UWB sniffer for 802.15.4}")
        for port in comports():
            res.append ("interface {value=%s}{display=UWB Sniffer for 802.15.4}" % (port.device,) )
        # res.append ("interface {value=/dev/pts/4}{display=UWB Sniffer for 802.15.4}")
        return "\n".join(res)

    @staticmethod
    def extcap_config():
        """
        Wireshark-related method that returns configuration options.
        :return: string with wireshark-compatible information
        """
        args = []
        values = []
        res =[]

        args.append ( (0, '--channel', 'Channel', 'IEEE 802.15.4 channel', 'selector', '{required=true}') )

        for arg in args:
            res.append("arg {number=%d}{call=%s}{display=%s}{tooltip=%s}{type=%s}%s" % arg)

        values = values + [ (0, "%d" % i, "%d" % i, "true" if i == 5 else "false" ) for i in range(1,6) ]
        values += [(0,"7","7","false")]
        for value in values:
            res.append("value {arg=%d}{value=%s}{display=%s}{default=%s}" % value)

        return "\n".join(res)

    @staticmethod
    def extcap_dlts():
        """
        Wireshark-related method that returns configuration options.
        :return: string with wireshark-compatible information
        """
        # Various options for pcap files: http://www.tcpdump.org/linktypes.html
        res = []
        res.append("dlt {number=%d}{name=IEEE802_15_4_NOFCS}{display=IEEE 802.15.4 without FCS}" % UwbSniffer.DLT_IEEE802_15_4_NOFCS)

        return "\n".join(res)

    def extcap_capture(self, dev, channel, fifo):
        if len(self.threads):
            raise RuntimeError("Old threads were not joined properly")

        packet_queue = queue.Queue()
        self.channel = channel
        self.dev = dev
        self.running.set()
        self.dlt = UwbSniffer.DLT_IEEE802_15_4_NOFCS

        self.threads.append(threading.Thread(target=self.serial_thread_func, args=(self.dev, self.channel, packet_queue), name="serial_reader"))
        self.threads.append(threading.Thread(target=self.fifo_writer_thread_func, args=(fifo, packet_queue), name="fifo_writer"))

        for thread in self.threads:
            thread.start()

        logging.info("All thread has been started")
        while self.running.is_set():
            time.sleep(1)

    def serial_thread_func(self, dev, channel, queue):
        """
        Thread responsible for reading from serial port, parsing the output and storing parsed packets into queue.
        """

        logging.info("Thread: serial_thread_func")
        while self.running.is_set():
            try:
                self.serial = Serial(dev, 115200, timeout=1, exclusive=True)
                break
            except Exception as e:
                logging.debug("Can't open serial device: {} reason: {}".format(dev, e))
                time.sleep(0.5)
        logging.info("Serial device: {}".format(dev))

        if self.running.is_set():
            try:
                self.serial.reset_input_buffer()
                self.serial.reset_output_buffer()

                cnt = 0
                while self.running.is_set():
                    self.serial.write(b'channel ' + bytes(str(channel).encode()) + b'\r\n')
                    data = self.serial.readline()
                    if data != b'':
                        if data.decode().rstrip() == "OK":
                            break
                        else:
                            logging.info("Unknown respond: " + data.decode())
                    cnt+=1
                    time.sleep(1)
                logging.info("Channel %s" % (channel))

                cnt = 0
                while self.running.is_set():
                    self.serial.write(b'start\r\n')
                    data = self.serial.readline()
                    if data != b'':
                        if data.decode().rstrip() == "OK":
                            break
                    cnt+=1
                    time.sleep(1)
                logging.info("Start capture")

                buf = b''
                while self.running.is_set():
                    ch = self.serial.read()
                    if ch == b'':
                        continue
                    elif ch != b'\r' and ch != '\n':
                        buf += ch
                    else:
                        m = re.search(self.RCV_REGEX, str(buf))
                        if m:
                            try:
                                packet = a2b_hex(m.group(1)[:])
                            except:
                                buf = b''
                                continue
                            rssi = int(m.group(2))
                            lqi = int(m.group(3))
                            timestamp = int(m.group(4)) & 0xffffffff
                            channel = int(channel)
                            queue.put(self.pcap_packet(packet, self.dlt, channel, rssi, lqi, self.correct_time(timestamp)))
                        buf = b''

            except (serialutil.SerialException, serialutil.SerialTimeoutException) as e:
                logging.error("Cannot communicate with serial device: {} reason: {}".format(dev, e))
            finally:
                if not self.running.is_set():  # Another precaution.
                    self.stop_sig_handler()

    def fifo_writer_thread_func(self, fifo, queue):
        """
        Thread responsible for writing packets into pcap file/fifo from queue.
        """
        logging.info("Thread: fifo_writer_thread_func")
        with open(fifo, 'wb', 0 ) as fh:
            fh.write(self.pcap_header())
            fh.flush()

            while self.running.is_set():
                try:
                    packet = queue.get(block=True, timeout=1)
                    try:
                        fh.write(packet)
                        fh.flush()
                    except IOError:
                        self.stop_sig_handler()
                        pass
                except:
                    if not os.path.exists(fifo):
                        self.stop_sig_handler()
                    pass

    @staticmethod
    def pcap_packet(frame, dlt, channel, rssi, lqi, timestamp):
        """
        Creates pcap packet to be saved in pcap file.
        """
        pcap = bytearray()

        caplength = len(frame)

        if dlt == UwbSniffer.DLT_IEEE802_15_4_TAP:
            caplength += 28
        elif dlt == UwbSniffer.DLT_USER0:
            caplength += 6

        pcap += struct.pack('<L', timestamp // 1000000 ) # Timestamp seconds
        pcap += struct.pack('<L', timestamp % 1000000 ) # Timestamp microseconds
        pcap += struct.pack('<L', caplength ) # Length captured
        pcap += struct.pack('<L', caplength ) # Length in frame

        if dlt == UwbSniffer.DLT_IEEE802_15_4_TAP:
            # Append TLVs according to 802.15.4 TAP specification:
            # https://github.com/jkcko/ieee802.15.4-tap
            pcap += struct.pack('<HH', 0, 28)
            pcap += struct.pack('<HHf', 1, 4, rssi)
            pcap += struct.pack('<HHHH', 3, 3, channel, 0)
            pcap += struct.pack('<HHI', 10, 1, lqi)
        elif dlt == UwbSniffer.DLT_USER0:
            pcap += struct.pack('<H', channel)
            pcap += struct.pack('<h', rssi)
            pcap += struct.pack('<H', lqi)

        pcap += frame

        return pcap

    def pcap_header(self):
        """
        Returns pcap header to be written into pcap file.
        """
        header = bytearray()
        header += struct.pack('<L', int ('a1b2c3d4', 16 ))
        header += struct.pack('<H', 2 )                     # Pcap Major Version
        header += struct.pack('<H', 4 )                     # Pcap Minor Version
        header += struct.pack('<I', int(0))                 # Timezone
        header += struct.pack('<I', int(0))                 # Accuracy of timestamps
        header += struct.pack('<L', int ('000000ff', 16 ))  # Max Length of capture frame
        header += struct.pack('<L', self.dlt) # DLT
        return header

    def correct_time(self, sniffer_timestamp):
        """
        Function responsible for correcting the time reported by the sniffer.
        The sniffer timer has 1us resolution and will overflow after
        approximately every 72 minutes of operation.
        For that reason it is necessary to use the local system timer to find the absolute
        time frame, within which the packet has arrived.

        This method should work as long as the MCU and PC timers don't drift
        from each other by a value of approximately 36 minutes.

        :param sniffer_timestamp: sniffer timestamp in microseconds
        :return: absolute sniffer timestamp in microseconds
        """
        if self.first_local_timestamp is None:
            # First received packets - set the reference time and convert to microseconds.
            self.first_local_timestamp = int(time.time()*(10**6))
            self.first_sniffer_timestamp = sniffer_timestamp
            return self.first_local_timestamp
        else:
            local_timestamp = int(time.time()*(10**6))
            time_difference = local_timestamp - self.first_local_timestamp

            # Absolute sniffer timestamp calculated locally
            absolute_sniffer_timestamp = self.first_sniffer_timestamp + time_difference

            overflow_count = absolute_sniffer_timestamp // self.TIMER_MAX
            timestamp_modulo = absolute_sniffer_timestamp % self.TIMER_MAX

            # Handle the corner case - sniffer timer is about to overflow.
            # Due to drift the locally calculated absolute timestamp reports that the overflow
            # has already occurred. If the modulo of calculated time with added value of half timer period
            # is smaller than the sniffer timestamp, then we decrement the overflow counter.
            #
            # The second corner case is when the sniffer timestamp just overflowed and the value is close to zero.
            # Locally calculated timestamp reports that the overflow hasn't yet occurred. We ensure that this is the
            # case by testing if the sniffer timestamp is less than modulo of calculated timestamp substracted by
            # half of timer period. In this case we increment overflow count.
            if (timestamp_modulo + self.TIMER_MAX//2) < sniffer_timestamp:
                overflow_count -= 1
            elif (timestamp_modulo - self.TIMER_MAX//2) > sniffer_timestamp:
                overflow_count += 1

            return self.first_local_timestamp - self.first_sniffer_timestamp + sniffer_timestamp + overflow_count * self.TIMER_MAX

    def stop_sig_handler(self, *args, **kwargs):
        """
        Function responsible for stopping the sniffer firmware and closing all threads.
        """
        if self.running.is_set():
            self.running.clear()
            alive_threads = []

            for thread in self.threads:
                try:
                    thread.join(timeout=10)
                    if thread.is_alive() is True:
                        logging.error("Failed to stop thread {}".format(thread.name))
                        alive_threads.append(thread)
                except RuntimeError:
                    pass

            logging.info("Stop all thread")
            self.threads = alive_threads

            self.serial.reset_input_buffer()
            self.serial.reset_output_buffer()
            cnt = 0
            while True:
                self.serial.write(b'stop\r\n')
                data = self.serial.readline()
                if data != b'':
                    if data.decode().rstrip() == "OK":
                        break
                logging.info("Stop capturing: %d" % (cnt))
                cnt+=1
                if cnt > 2:
                    logging.warning("Unable to stop capture")
                    break
                time.sleep(1)

            if self.serial is not None:
                if self.serial.is_open is True:
                    self.serial.close()
                self.serial = None
            logging.info("Stopped")
        else:
        #    logging.warning("Asked to stop {} while it was already stopped".format(self))
            pass
    
    def __str__(self):
        return "{} ({}) channel {}".format(type(self).__name__, self.dev, self.channel)

    def __repr__(self):
        return self.__str__()

def parse_args():
    """
    Helper methods to make the standalone script work in console and wireshark.
    """
    parser = ArgumentParser(description="Extcap program for the nRF Sniffer for 802.15.4")

    parser.add_argument("--extcap-interfaces", help="Provide a list of interfaces to capture from", action="store_true")
    parser.add_argument("--extcap-interface", help="Provide a list of interfaces to capture from")
    parser.add_argument("--extcap-dlts", help="Provide a list of dlts for the given interface", action="store_true")
    parser.add_argument("--extcap-config", help="Provide a list of configurations for the given interface", action="store_true")
    parser.add_argument("--capture", help="Start the capture routine", action="store_true" )
    parser.add_argument("--fifo", help="Use together with capture to provide the fifo to dump data to")
    parser.add_argument("--extcap-capture-filter", help="Used together with capture to provide a capture filter")
    parser.add_argument("--extcap-control-in", help="Used to get control messages from toolbar")
    parser.add_argument("--extcap-control-out", help="Used to send control messages to toolbar")

    parser.add_argument("--channel", help="IEEE 802.15.4 capture channel [1-9]")

    result, unknown = parser.parse_known_args()

    if result.capture and not result.extcap_interface:
        parser.error("--extcap-interface is required if --capture is present")

    return result

if __name__ == '__main__':
    args = parse_args()

    logging.basicConfig(format='%(asctime)s [%(levelname)s] %(message)s', level=logging.DEBUG, filename='/tmp/uwb_sniffer.log')
    logging.info("Command argument: " + str(sys.argv))

    uwb_sniffer = UwbSniffer()

    if args.extcap_interfaces:
        print(uwb_sniffer.extcap_interfaces())
    
    if args.extcap_dlts:
        print(uwb_sniffer.extcap_dlts())

    if args.extcap_config:
        print(uwb_sniffer.extcap_config())

    if args.capture and args.fifo:
        signal.signal(signal.SIGINT, uwb_sniffer.stop_sig_handler)
        signal.signal(signal.SIGTERM, uwb_sniffer.stop_sig_handler)
        try:
            uwb_sniffer.extcap_capture(args.extcap_interface, args.channel, args.fifo)
        except KeyboardInterrupt as e:
            uwb_sniffer.stop_sig_handler()
