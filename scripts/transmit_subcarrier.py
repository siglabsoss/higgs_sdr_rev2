###############################################################################
# Libraries and Modules
###############################################################################
import struct
import time
import argparse
import higgs
import socket as sc
from log import logger
import sys
sys.path.insert(0,"../../python-osi")
from sigmath import *
import fcntl

###############################################################################
# Constants
###############################################################################
CMD_DELAY = 0.005
###############################################################################
# Class Definitions
###############################################################################
class TransmitData:
    def __init__(self, host=higgs.HOST, cmd_port=higgs.TX_CMD_PORT,
                 data_port=higgs.TX_DATA_PORT, connect_type=sc.SOCK_DGRAM):
        self.higgs = higgs.HiggsController(host=host,
                                                      tx_cmd_port=cmd_port,
                                                      tx_data_port=data_port,
                                                      connect_type=connect_type)

        # bind sockets
        self.higgs.bind_rx_cmd_soc(None)
        # self.higgs.bind_rx_data_soc(1.0) # uncomment if we want to get "adc" data

        # setup sockets
        fcntl.fcntl(self.higgs.rx_cmd_sock, fcntl.F_SETFL, os.O_NONBLOCK)

        # for generated counter data
        self.gen_counter = 0

    def get_data(self, filepath):
        try:
            with open(filepath) as datafile:
                self.signal = []
                line = datafile.readline()
                while line:
                    value = int(line.split(',')[0], 16)
                    self.signal.append(value)
                    line = datafile.readline()
        except IOError:
            logger.info('Input data file does not exist')

    def setup_subcarrier(self):
        self.data = []
        for i in range(1024):
            self.data.append(i)

    def gen_next(self, bytess):
        words = bytess / 4
        gen = range(self.gen_counter, self.gen_counter+words)

        self.gen_counter += words

        return self.get_packet(gen)


    def transmit(self):
        self.chunk = 256
        # p = self.get_packet(self.data[0:self.chunk])
        p = self.gen_next(self.chunk)

        self.enabled_subcarriers = 256
        self.start_adjustment = -0.05  # in seconds, negative means we send more data
        self.cp_percentage = 0.25

        self.bytes_per_sample = 4
        self.bits_per_symbol = 2  # aka bits per subcarrier
        self.bits_per_byte = 8
        self.words_per_fft = self.enabled_subcarriers / (32.0/self.bits_per_symbol)
        self.start = time.time()
        self.fs = 31.25E6 #/ (1024*(1+self.cp_percentage) / 8)
        self.fs *= 1.001   # Over send data, we will back off when we hit "almost full" below
        self.tx_byte_count = 0

        self.bytes_per_second = (self.fs / (1024*(1+self.cp_percentage))) * (self.words_per_fft*4)

        # print("self.bytes_per_fft", self.bytes_per_fft)
        print("self.words_per_fft", self.words_per_fft)
        print("self.bytes_per_second", self.bytes_per_second)
        # print(self.bytes_per_second)
        # sys.exit(0)

        self.tx_packet_data_size = self.chunk * self.bytes_per_sample

        self.one_shot = False


        while(1):
            
            target_bytes = (time.time() - (self.start+self.start_adjustment)) * self.bytes_per_second

            if self.one_shot is False:
                if target_bytes >= self.tx_byte_count + self.tx_packet_data_size:
                    self.higgs.tx_data_sock.sendto(p, self.higgs.tx_data_addr)
                    self.tx_byte_count += len(p)
                    # print(target_bytes)
                    # print("(sending)")
                    # self.one_shot = True
                    p = self.gen_next(self.chunk)
                    # print a

            # if target_bytes >= self.tx

            buf = nonblock_socket(self.higgs.rx_cmd_sock, 8)
            if buf is not None:
                (seq,) = struct.unpack('<I', buf[0:4])
                data = buf[4:]
                (word,) = struct.unpack('<I', data[0:4])

                if((word & 0xFF000000) == 0x0C000000):
                    print("ERROR offset detected (" + hex(word) + ")")

                elif((word & 0xFF000000) == 0x0B000000):
                    fill_low = word & 0xFF
                    fill_high = (word>>8) & 0xFF
                    fill_flags = word & 0x000F0000
                    fill_underflow = word & 0x00010000
                    fill_overflow  = word & 0x00020000

                    if(fill_high > 56):
                        print "delay", self.start_adjustment
                        # adding to this number will slow us down
                        self.start_adjustment += 0.001
                    
                    ss = hex(word) + " - " + str(fill_low) + " - "
                    if fill_underflow:
                        ss += " (underflow)"
                    if fill_overflow:
                        ss += " (overflow)"

                    print(ss)
                    print(fill_low)
                    print(fill_high)
                else:
                    print(hex(word) + " " + str(word))
        # blah
        print("exiting ")


                # print(len(buf))

    def get_packet(self, din):
        args = ['<%dI' % len(din)]
        args += din
        tups = tuple(args)
        udp_packet = struct.pack(*tups)
        return udp_packet
        # print(udp_packet)
        # except KeyError:
        #     logger.info('Selected FPGA not within ringbus protocol')
        # self.tx_cmd_sock.sendto(udp_packet, self.tx_cmd_addr)
        # logger.info('Packet sent: %s, %s', hex(FPGA_ADDR[fpga]), hex(cmd))

        # cmd_packet = higgs.DMA_IN_CMD|len(self.signal)
        # self.higgs.send_cmd('cs20', cmd_packet, CMD_DELAY)
        # self.higgs.send_data(self.data)
        # cmd_packet = higgs.DMA_IN_CMD|len(self.signal)
        # self.higgs.send_cmd('cs10', cmd_packet, CMD_DELAY)
        # cmd_packet = higgs.DMA_OUT_CMD|len(self.signal)
        # self.higgs.send_data('cs20', cmd_packet, CMD_DELAY)

###############################################################################
# Method Definitions
###############################################################################
def main():
    parser = higgs.create_parser()
    # parser.add_argument('-tx', '--send_signal',
    #                            action='store_true',
    #                            help='send new signal to transmit')
    # parser.add_argument('-fp', '--filepath',
    #                            help='path of transmit data file',
    #                            type=str,
    #                            default=r'./libs/datapath/symbol.csv')
    args = parser.parse_args()

    c = TransmitData()
    c.setup_subcarrier()
    c.transmit()

###############################################################################
# Main Script
###############################################################################
if __name__ == "__main__":
    main()