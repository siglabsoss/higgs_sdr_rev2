###############################################################################
# Libraries and Modules
###############################################################################
import struct
import time
import argparse
import socket as sc
from log import logger
from itertools import chain, izip
###############################################################################
# Constants
###############################################################################
PACKET_DELAY = 0.001
HOST = '10.2.2.2'
OURHOST = '10.2.2.1'
PORT = 20000
RXPORT = 10001
RING_BOOTLOAD_CMD = 0x01000000
TX_CFO_LOWER_CMD =      (0x20000000)
TX_CFO_UPPER_CMD   =    (0x21000000)

FPGA_ADDR = {'eth':0, 'cs20':1, 'cs10':2, 'cs00':3, 'cs01':4, 'cs11':5,
             'cs21':6, 'cs31':7, 'cs30':8}
BOOTLOADER_ADDRESS_CUTOFF = 0x7bfc
BOOTLOADER_LARGEST_STACK = 0x7900
###############################################################################
# Class Definitions
###############################################################################
class TestEth:
    def __init__(self, host=HOST, port=PORT, rxport=RXPORT, delay=0, connect_type=sc.SOCK_DGRAM):
        self.host = host
        self.port = port
        self.rxport = rxport
        self.addr = (host, port)
        self.connect_type = connect_type
        self.program_opcode = []
        self.delay = delay

    def create_client(self):
        '''Creates a UDP client socket'''
        self.sock = sc.socket(sc.AF_INET, self.connect_type)
        # self.rxsock = sc.socket(sc.AF_INET, self.connect_type)
        # self.rxsock.bind((OURHOST, self.rxport))
        # self.rxsock.settimeout(1.0)

    def _send_dual_packet(self, d0, d1, delay):
        time.sleep(delay)
        udp_packet = struct.pack('<II', d0, d1)
        self.sock.sendto(udp_packet, self.addr)
        logger.info('Packet sent: %s %s', hex(d0), hex(d1))

    def get_message(self):
        ''' waits for a single rinbgus packet
        Returns:
            32 bit sequence number, 32 bit data

        '''
        buf, addr = self.rxsock.recvfrom(8)

        sup = struct.unpack('<II', buf)
        return sup

    def set_cfo(self, val):

        sign = val < 0

        val = int(abs(val))

        lower = val & 0xffff;
        upper = (val>>16) & 0xffff;
        if sign:
            upper |= 0x010000

        self._send_dual_packet(2, TX_CFO_LOWER_CMD | lower, 0.0)
        self._send_dual_packet(2, TX_CFO_UPPER_CMD | upper, 0.2)

        logger.info("set CFO to " + str(val))

    def set_phase(self, val):

        val = int(val)

        # val = int(abs(val))

        lower = val & 0xffff;
        upper = (val>>16) & 0xffff;
        upper |= 0x020000

        self._send_dual_packet(2, TX_CFO_LOWER_CMD | lower, 0.0)
        self._send_dual_packet(2, TX_CFO_UPPER_CMD | upper, 0.2)

        logger.info("added Phase to " + str(val))





###############################################################################
# Method Definitions
###############################################################################
def main():
    '''Command line program use to bootload selected FPGA'''
    parser = argparse.ArgumentParser(description='Bootload FPGA')
    # parser.add_argument('fpga', help='specify which FPGA to bootload', type=str)
    parser.add_argument('-p', '--port',
                              help='specify port number of Higgs i.e 20000',
                              nargs='?',
                              type=int,
                              default=PORT)
    parser.add_argument('-rp', '--rxport',
                              help='specify rx port number on our nic',
                              nargs='?',
                              type=int,
                              default=RXPORT)
    parser.add_argument('-ht', '--host',
                               help='specify IP address of Higgs i.e 10.2.2.2',
                               nargs='?',
                               type=str,
                               default=HOST)
    parser.add_argument('-pd', '--packetdelay',
                               help='specify time delay (secs) between packets',
                               nargs='?',
                               type=float,
                               default=PACKET_DELAY)
    parser.add_argument('-cfo', '--cfohz',
                               help='specify time delay (secs) between packets',
                               nargs='?',
                               type=float,
                               default=PACKET_DELAY)
    parser.add_argument('-phase', '--phasedelta',
                               help='specify relative phase',
                               nargs='?',
                               type=float,
                               default=None)

    args = parser.parse_args()
    logger.info('Testing at %s port %d',  args.host, args.port)
    fpga = TestEth(args.host, args.port, args.rxport, args.packetdelay)
    fpga.create_client()
    print "phase", args.phasedelta
    if args.phasedelta:
        fpga.set_phase(args.phasedelta)
    else:
        fpga.set_cfo(args.cfohz)

###############################################################################
# Main Script
###############################################################################
if __name__ == "__main__":
    main()
