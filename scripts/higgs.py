###############################################################################
###############################################################################
# Name: higgs.py
# Coder: Janson Fang
# Description:
###############################################################################

###############################################################################
# Libraries and Modules
###############################################################################
import argparse
import time
import struct
import socket as sc
from log import logger
###############################################################################
# Constants
###############################################################################
HOST = '10.2.2.2'
OUR_HOST = '10.2.2.1'
RX_CMD_PORT = 10001
TX_CMD_PORT = 20000
TX_DATA_PORT = 30000
RX_DATA_PORT = 40001
TEST_CMD = 0x00000000
RING_TEST_CMD = 0x00000000
BOOTLOADER_CMD = 0x01000000
DMA_IN_CMD = 0x02000000
TURNSTILE_CMD = 0x03000000
CONFIG_DAC_CMD = 0x04000000
TX_CHANNEL_CMD = 0x05000000
DMA_OUT_CMD = 0x06000000
VGA_GAIN_CMD = 0x07000000
PASS_DATA_CMD = 0x08000000
DMA_OUT_PACKET_CMD = 0x0A000000
RESET_ADC_COUNTER_CMD = 0x0C000000
EN_ADC_COUNTER_CMD = 0x0D000000
DISAB_ADC_COUNTER_CMD = 0x0E000000
DSA_GAIN_CMD = 0x0F000000
ETH_TEST_CMD = 0x10000000
EDGE_EDGE_IN = 0x11000000
EDGE_EDGE_OUT = 0x12000000
SATURATION_RATIO_CMD = 0x13000000
DISABLE_DAC_CMD = 0x14000000
EXTENDED_SR_CMD = 0x15000000
EXTENDED_EXECUTE_CMD = 0x16000000
DMA_FLUSH_RESET_CMD = 0x17000000
SYNCHRONIZATION_CMD = 0x18000000
TX_CFO_LOWER_CMD = 0x20000000
TX_CFO_UPPER_CMD = 0x21000000
POWER_ESTIMATION_CMD = 0x22000000
AGC_TEST_CMD = 0x23000000
NCO_TEST_CMD = 0x27000000
TX_CHANNEL_A_BIT = 0x010000
TX_CHANNEL_B_BIT = 0x008000
LED_GPIO_BIT = 0x200000
ADC_COUNTER_BIT = 0x100000
RESET_ADC_COUNTER = 0x080000
ETH_PACKET_SIZE = 367
FPGA_ADDR = {"eth": 0, "cs11":1, "cs01":2, "cs02":3, "cs12":4,  "cs22":5,
             "cs32":6, "cs31":7, "cs21":8, "cs20":9}
###############################################################################
# Method Definitions
###############################################################################
def create_parser():
    parser = argparse.ArgumentParser()
    parser.add_argument('-tcp', '--tx_cmd_port',
                                help='specify transmit command port of Higgs',
                                nargs='?',
                                type=int,
                                default=TX_CMD_PORT)
    parser.add_argument('-rcp', '--rx_cmd_port',
                                help='specify receive command port of Higgs',
                                nargs='?',
                                type=int,
                                default=RX_CMD_PORT)
    parser.add_argument('-tdp', '--tx_data_port',
                                help='specify transmit data port of Higgs',
                                nargs='?',
                                type=int,
                                default=TX_DATA_PORT)
    parser.add_argument('-rdp', '--rx_data_port',
                                help='specify receive data port of Higgs',
                                nargs='?',
                                type=int,
                                default=RX_DATA_PORT)
    parser.add_argument('-oht', '--our_host',
                                help='specify IP address of your machine',
                                nargs='?',
                                type=str,
                                default=OUR_HOST)
    parser.add_argument('-ht', '--host',
                               help='specify IP address of Higgs i.e 10.2.2.2',
                               nargs='?',
                               type=str,
                               default=HOST)

    return parser
###############################################################################
# Class Definitions
###############################################################################
class HiggsController:
    def __init__(self, host=HOST, our_host=OUR_HOST,
                                  tx_cmd_port=TX_CMD_PORT,
                                  rx_cmd_port=RX_CMD_PORT,
                                  tx_data_port=TX_DATA_PORT,
                                  rx_data_port=RX_DATA_PORT,
                                  connect_type=sc.SOCK_DGRAM):
        self.host = host
        self.our_host = our_host
        self.tx_cmd_port = tx_cmd_port
        self.rx_cmd_port = rx_cmd_port
        self.tx_data_port = tx_data_port
        self.rx_data_port = rx_data_port
        self.connect_type = connect_type
        self.tx_cmd_addr = (host, self.tx_cmd_port)
        self.rx_cmd_addr = (our_host, self.rx_cmd_port)
        self.tx_data_addr = (host, self.tx_data_port)
        self.rx_data_addr = (our_host, self.rx_data_port)
        self.tx_cmd_sock = sc.socket(sc.AF_INET, self.connect_type)
        self.rx_cmd_sock = sc.socket(sc.AF_INET, self.connect_type)
        self.tx_data_sock = sc.socket(sc.AF_INET, self.connect_type)
        self.rx_data_sock = sc.socket(sc.AF_INET, self.connect_type)

    def send_cmd(self, fpga, cmd, delay):
        time.sleep(delay)
        try:
            udp_packet = struct.pack('<II', FPGA_ADDR[fpga], cmd)
        except KeyError:
            logger.info('Selected FPGA not within ringbus protocol')
        self.tx_cmd_sock.sendto(udp_packet, self.tx_cmd_addr)
        logger.info('Packet sent: %s, %s', hex(FPGA_ADDR[fpga]), hex(cmd))

    def send_data(self, data, delay, packet_size):
        for index in range(0, len(data), packet_size):
            time.sleep(delay)
            packet = data[index:index+packet_size]
            data_packet = struct.pack('<%dI' % len(packet), *packet)
            self.tx_data_sock.sendto(data_packet, self.tx_data_addr)
            logger.info('%d packets sent', len(packet))
        logger.info('Total packets sent: %d', len(data))

    def get_data(self, buffer_size):
        try:
            buf, addr = self.rx_data_sock.recvfrom(buffer_size)
            data = struct.unpack('<%sI' % str(buffer_size/4), buf)

            return data[1:]
        except sc.timeout:
            logger.error('TIMEOUT: No packet received')

    def get_cmd(self, buffer_size):
        try:
            buf, addr = self.rx_cmd_sock.recvfrom(buffer_size)
            data = struct.unpack('<%sI' % str(buffer_size/4), buf)

            return data
        except sc.timeout:
            logger.info('No packet received')

    def bind_rx_data_soc(self, timeout):
        self.rx_data_sock.bind(self.rx_data_addr)
        self.rx_data_sock.settimeout(timeout)

    def bind_rx_cmd_soc(self, timeout):
        self.rx_cmd_sock.bind(self.rx_cmd_addr)
        self.rx_cmd_sock.settimeout(timeout)