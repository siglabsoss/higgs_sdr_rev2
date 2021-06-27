###############################################################################
###############################################################################
# Name: transmit_data.py
# Coder: Janson Fang
# Description:
###############################################################################

###############################################################################
# Libraries and Modules
###############################################################################
import struct
import time
import argparse
import higgs
import socket as sc
from log import logger
###############################################################################
# Constants
###############################################################################
CMD_DELAY = 0.005
DATA_DELAY = 0.005
DATA_PACKET_SIZE = 366
###############################################################################
# Class Definitions
###############################################################################
class TransmitData:
    def __init__(self, host=higgs.HOST, cmd_port=higgs.TX_CMD_PORT,
                 data_port=higgs.TX_DATA_PORT, connect_type=sc.SOCK_DGRAM):
        self.higgs_controller = higgs.HiggsController(host=host,
                                                      tx_cmd_port=cmd_port,
                                                      tx_data_port=data_port,
                                                      connect_type=connect_type)

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

    def transmit_data(self, data_delay, cmd_delay, packet_size):
        cmd_packet = higgs.DMA_IN_CMD|len(self.signal)
        self.higgs_controller.send_cmd('cs20', cmd_packet, cmd_delay)
        self.higgs_controller.send_data(self.signal, data_delay, packet_size)
        cmd_packet = higgs.DMA_IN_CMD|len(self.signal)
        self.higgs_controller.send_cmd('cs10', cmd_packet, cmd_delay)
        cmd_packet = higgs.DMA_OUT_CMD|len(self.signal)
        self.higgs_controller.send_cmd('cs20', cmd_packet, cmd_delay)

###############################################################################
# Method Definitions
###############################################################################
def main():
    parser = higgs.create_parser()
    parser.add_argument('-tx', '--send_signal',
                               action='store_true',
                               help='send new signal to transmit')
    parser.add_argument('-fp', '--filepath',
                               help='path of transmit data file',
                               type=str,
                               default=r'./libs/datapath/symbol.csv')
    parser.add_argument('-dd', '--datadelay',
                               help='specify time delay in (secs) between data',
                               nargs='?',
                               type=float,
                               default=DATA_DELAY)
    parser.add_argument('-cd', '--cmddelay',
                               help='specify time delay in (secs) between cmd',
                               nargs='?',
                               type=float,
                               default=CMD_DELAY)
    parser.add_argument('-ps', '--packetsize',
                               help='specify packet size of data',
                               nargs='?',
                               type=int,
                               default=DATA_PACKET_SIZE)
    args = parser.parse_args()
    if args.send_signal:
        transmit_signal = TransmitData(host=args.host,
                                       cmd_port=args.tx_cmd_port,
                                       data_port=args.tx_data_port)
        transmit_signal.get_data(args.filepath)
        transmit_signal.transmit_data(args.datadelay,
                                      args.cmddelay,
                                      args.packetsize)

###############################################################################
# Main Script
###############################################################################
if __name__ == "__main__":
    main()