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
PACKET_DELAY = 1
DAC_CONFIG = [0x00019d, 0x028082, 0x07ffff, 0x1f4444, 0x201100, 0x241c00,
              0x1b0800, 0x037001]
###############################################################################
# Class Definitions
###############################################################################
class ConfigDAC:
    def __init__(self, host=higgs.HOST, port=higgs.TX_CMD_PORT,
                                        connect_type=sc.SOCK_DGRAM):
        self.host = host
        self.port = port
        self.addr = (host, port)
        self.connect_type = connect_type
        self.TURNMASK = 0x07000000

    def create_client(self):
        self.sock = sc.socket(sc.AF_INET, sc.SOCK_DGRAM)

    def send_cmd(self, data, delay):
        time.sleep(delay)
        udp_packet = struct.pack('<II', higgs.FPGA_ADDR['eth'], data)
        self.sock.sendto(udp_packet, self.addr)
        logger.info('Packet sent: 0x0, %s', hex(data))

    def send_cmd_cs10(self, data, delay = 0.0):
        time.sleep(delay)
        udp_packet = struct.pack('<II', higgs.FPGA_ADDR['cs10'], data)
        self.sock.sendto(udp_packet, self.addr)
        logger.info('Packet sent: 0x0, %s', hex(data))

    def set_tx_channel(self, channel, delay):
        channel |= higgs.TX_CHANNEL_CMD
        self.send_cmd(channel, delay)
        select_channel = channel == higgs.TX_CHANNEL_CMD|higgs.TX_CHANNEL_A_BIT
        select_channel = 'A' if select_channel else 'B'
        logger.info('Set TX channel to %s', select_channel)

    def pa(self):

        self.send_cmd_cs10(self.TURNMASK | 3)
        self.send_cmd_cs10(self.TURNMASK | 4, 2.0)

###############################################################################
# Method Definitions
###############################################################################
def main():
    parser = argparse.ArgumentParser(description='Configure FPGA')
    parser.add_argument('-dm', '--dac_module',
                               action='store_true',
                               help='set flag to send DAC config values')
    parser.add_argument('-tx', '--tx_channel',
                               help='set transmit channel to A or B',
                               type=str,
                               choices=['a', 'b'])
    parser.add_argument('-p', '--port',
                              help='specify port number of Higgs i.e 20000',
                              nargs='?',
                              type=int,
                              default=higgs.TX_CMD_PORT)
    parser.add_argument('-ht', '--host',
                               help='specify IP address of Higgs i.e 10.2.2.2',
                               nargs='?',
                               type=str,
                               default=higgs.HOST)
    parser.add_argument('-pd', '--packetdelay',
                               help='specify time delay (secs) between packets',
                               nargs='?',
                               type=float,
                               default=PACKET_DELAY)
    args = parser.parse_args()
    config_dac = ConfigDAC(args.host, args.port)
    config_dac.create_client()

    config_dac.pa()
    # if args.dac_module:
    #     for config in DAC_CONFIG:
    #         config |= higgs.CONFIG_DAC_CMD
    #         config_dac.send_cmd(config, args.packetdelay)
    # if args.tx_channel:
    #     config_dac.set_tx_channel(higgs.TX_CHANNEL_A_BIT, args.packetdelay)
###############################################################################
# Main Script
###############################################################################
if __name__ == "__main__":
    main()