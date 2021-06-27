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
        self.rxsock = sc.socket(sc.AF_INET, self.connect_type)
        self.rxsock.bind((OURHOST, self.rxport))
        self.rxsock.settimeout(9999.0)

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

    def run_test(self):

        # self._send_dual_packet(0, 0x10000001, self.delay)

        # expected = range(0x10000000, 0x1000000a)

        while(1):
            (s, data) = self.get_message()
            print(hex(data))
            
            # assert(data == expected[idx])

        # logger.info('')
        # logger.info('Received 10 correct packets: Eth is up and alive')

        # self._send_dual_packet(0, 0x10000002, self.delay)

        # for _ in range(1):
        #     (s, data) = self.get_message()
        #     # print(hex(data))
        #     assert(data == 0xdeadbeef)

        # logger.info('')
        # logger.info("Eth sent a ringbus to itself, all ringbus forwarding ok")




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
    args = parser.parse_args()
    logger.info('Testing at %s port %d',  args.host, args.port)
    fpga = TestEth(args.host, args.port, args.rxport, args.packetdelay)
    fpga.create_client()
    fpga.run_test()

###############################################################################
# Main Script
###############################################################################
if __name__ == "__main__":
    main()