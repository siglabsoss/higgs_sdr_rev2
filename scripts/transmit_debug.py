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

        # setup sockets
        fcntl.fcntl(self.higgs.rx_cmd_sock, fcntl.F_SETFL, os.O_NONBLOCK)

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

    def transmit(self):
        enabled_subcarriers = 8
        chunk = 365
        fs = 31.25E6
        fft_size = 1024
        cp = 1.0  # 1.0 is 100%
        fft_time = (1+cp) * (fft_size/fs)
        time_per_chunk = (chunk / enabled_subcarriers) * fft_time  # in seconds
        delay_per_packet = time_per_chunk * 0.90  # shrink this down
        p = self.get_packet(self.data[0:chunk])


        if True:
            self.higgs.tx_data_sock.sendto(p, self.higgs.tx_data_addr)
            time.sleep(delay_per_packet)

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