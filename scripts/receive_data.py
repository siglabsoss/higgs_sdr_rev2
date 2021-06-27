###############################################################################
###############################################################################
# Name: receive_data.py
# Coder: Janson Fang
# Description:
###############################################################################

###############################################################################
# Libraries and Modules
###############################################################################
import struct
import threading
import time
import argparse
import higgs
import numpy as np
import socket as sc
import matplotlib.pyplot as plt
from log import logger
###############################################################################
# Constants
###############################################################################
# 1472 is max buffer size
RX_BUFFER_SIZE = 1472
RX_SOC_TIMEOUT = 1.0
RX_PACKET_COUNT = 178
PACKET_DELAY = 0.005
RX_CHAIN_SAMPLES = 65326
SAVE_DATA_FILE_PATH = r'received_data.csv'
###############################################################################
# Class Definitions
###############################################################################
class ReceiveData:
    def __init__(self, host=higgs.HOST, our_host=higgs.OUR_HOST,
                 rx_data_port=higgs.RX_DATA_PORT, connect_type=sc.SOCK_DGRAM):
        self.higgs_controller = higgs.HiggsController(host=host,
                                                      our_host=our_host,
                                                      rx_data_port=rx_data_port,
                                                      connect_type=connect_type)

    def bind_rx_data_soc(self, timeout):
        self.higgs_controller.bind_rx_data_soc(timeout)

    def receive_packets(self, packet_count, buffer_size, save_data):
        self.receive_data = ()
        packet_received = 0
        self.pass_data_chunk(packet_count*buffer_size/4, PACKET_DELAY)
        for packet in range(packet_count):
            try:
                self.receive_data += self.higgs_controller.get_data(buffer_size)
                packet_received += 1
            except TypeError:
                logger.error('Scheduled packet not received')
        logger.info('%d/%d packets received', packet_received, packet_count)
        if save_data:
            self.plot_data(save_data)

    def test_rx_chain(self, samples, adc=False):
        data_packet = ()
        self.pass_data_chunk(samples, PACKET_DELAY)
        for cycle in range(samples/higgs.ETH_PACKET_SIZE):
            try:
                data_packet += self.higgs_controller.get_data(RX_BUFFER_SIZE)
            except TypeError:
                logger.error('Scheduled packet not received')
        if adc:
            correct_value = tuple(range(samples - 5))
            data_packet = data_packet[5:]
        else:
            correct_value = tuple(range(samples))
        # logger.info(data_packet)
        try:
            assert(data_packet == correct_value)
            logger.info('RX Chain PASSED. A counter of %d values were received',
                        samples - 5 if adc else samples)
        except AssertionError:
            logger.info('RX Chain FAILED. Incorrect counter values received')

            return -1
        except TypeError:
            logger.info('RX Chain FAILED. No counter values received')

            return -1

    def test_adc(self, samples, cmd_delay):
        self.set_adc_counter(False, cmd_delay)
        logger.info('Resetting ADC counter')
        cmd_packet = higgs.RESET_ADC_COUNTER_CMD|\
                     higgs.LED_GPIO_BIT|\
                     higgs.RESET_ADC_COUNTER|\
                     higgs.ADC_COUNTER_BIT
        self.higgs_controller.send_cmd('cs00', cmd_packet, cmd_delay)
        self.capture_adc_data(samples, cmd_delay)
        return self.test_rx_chain(samples, adc=True)

    def set_adc_counter(self, disable_counter, cmd_delay):
        if disable_counter:
            logger.info('Disabling ADC counter')
            cmd_packet = higgs.DISAB_ADC_COUNTER_CMD|\
                         higgs.LED_GPIO_BIT|\
                         higgs.RESET_ADC_COUNTER|\
                         higgs.ADC_COUNTER_BIT
        else:
            logger.info('Enabling ADC counter')
            cmd_packet = higgs.EN_ADC_COUNTER_CMD|\
                         higgs.LED_GPIO_BIT|\
                         higgs.RESET_ADC_COUNTER|\
                         higgs.ADC_COUNTER_BIT

        self.higgs_controller.send_cmd('cs00', cmd_packet, cmd_delay)

    def torture_test(self, samples, iterations, test_type):
        if test_type == 'rx_chain':
            for each_iteration in range(iterations):
                logger.info('Test #%d', each_iteration)
                if self.test_rx_chain(samples):
                    return -1
        elif test_type == 'adc_chain':
            for each_iteration in range(iterations):
                logger.info('Test #%d', each_iteration)
                if self.test_adc(samples, PACKET_DELAY):
                    return -1

    def capture_adc_data(self, packet_count, cmd_delay):
        # raw_input('Pause')
        logger.info('Enabling input DMA of CS00')
        cmd_packet = higgs.DMA_IN_CMD|packet_count
        self.higgs_controller.send_cmd('cs00', cmd_packet, cmd_delay)

    def pass_data_chunk(self, packet_count, cmd_delay):
        logger.info('Enabling data pass through in %d chunks',
                    higgs.ETH_PACKET_SIZE)
        cmd_packet = higgs.DMA_OUT_PACKET_CMD|packet_count
        self.higgs_controller.send_cmd('cs00', cmd_packet, cmd_delay)

    def plot_data(self, save_data):
        rx_signal_raw = self.receive_data
        rx_signal = np.array([])
        for value in rx_signal_raw:
            real = self._signed_value(value&0xffff, 16)
            imag = self._signed_value((value >> 16) & 0xffff, 16)
            rx_signal= np.append(rx_signal, complex(real, imag))
        rx_real = np.real(rx_signal)
        rx_imag = np.imag(rx_signal)
        fft_rx_signal = np.fft.fft(rx_signal)
        rx_mag = np.abs(fft_rx_signal)
        rx_f = np.fft.fftfreq(len(fft_rx_signal), d=1.0/31250000)
        logger.info(rx_signal)
        if save_data != None:
            self._save_recv_data(rx_signal, save_data)
        plt.figure()
        plt.subplot(221)
        plt.plot(rx_f, rx_mag)
        plt.subplot(222)
        plt.plot(rx_real)
        plt.subplot(223)
        plt.plot(rx_imag)
        plt.show()

    def _save_recv_data(self, recv_data, save_data):
        np.savetxt(save_data, recv_data, fmt='%d, %d', delimiter=',')

    def _signed_value(self, value, bit_count):
        if value > 2**(bit_count - 1) - 1:
            value -= 2**bit_count 

        return value
###############################################################################
# Method Definitions
###############################################################################
def main():
    parser = higgs.create_parser()
    parser.add_argument('-rp', '--receive_packet',
                               help='number of packets to receive from Higgs',
                               nargs='?',
                               type=int)
    parser.add_argument('-s', '--save_data',
                              help='save received data into CSV file',
                              nargs='?',
                              type=str)
    parser.add_argument('-adc', '--test_adc',
                                help='test ADC connection with CS00',
                                nargs='?',
                                type=int)
    parser.add_argument('-dc', '--disable_counter',
                               help='disable ADC counter',
                               action='store_true')
    parser.add_argument('-trx', '--test_rx_chain',
                                help='enable testing of RX chain',
                                nargs='?',
                                type=int)
    parser.add_argument('-to', '--torture_rx',
                                help='execute RX chain test multiple times',
                                nargs='?',
                                type=int)
    parser.add_argument('-toa', '--torture_adc',
                                help='execute ADC chain test multiple times',
                                nargs='?',
                                type=int)
    parser.add_argument('-pd', '--pass_data',
                               help='pass data received from ADC to computer',
                               nargs='?',
                               type=int)
    parser.add_argument('-c', '--adc_capture',
                              help='Data samples to capture from ADC FPGA',
                              nargs='?',
                              type=int)
    parser.add_argument('-bs', '--buffersize',
                               help='buffer size of data when receiving',
                               nargs='?',
                               type=int,
                               default=RX_BUFFER_SIZE)
    parser.add_argument('-t', '--timeout',
                              help='set timeout for receive socket',
                              nargs='?',
                              type=float,
                              default=RX_SOC_TIMEOUT)
    args = parser.parse_args()
    receive_signal = ReceiveData(host=args.host,
                                 our_host=args.our_host,
                                 rx_data_port=args.rx_data_port)
    if args.adc_capture != None:
        receive_signal.capture_adc_data(args.adc_capture, PACKET_DELAY)
    if args.pass_data != None:
        receive_signal.pass_data_chunk(args.pass_data, PACKET_DELAY)
    if args.disable_counter:
        receive_signal.set_adc_counter(args.disable_counter, PACKET_DELAY)
    if args.receive_packet:
        receive_signal.bind_rx_data_soc(args.timeout)
        receive_signal.receive_packets(args.receive_packet,
                                      args.buffersize,
                                      args.save_data)
    if args.test_rx_chain:
        if not args.test_rx_chain%higgs.ETH_PACKET_SIZE:
            receive_signal.bind_rx_data_soc(args.timeout)
            receive_signal.test_rx_chain(args.test_rx_chain)
        else:
            logger.error('Sample size need to be a multiple of %d',
                         higgs.ETH_PACKET_SIZE)
    if args.torture_rx:
        receive_signal.bind_rx_data_soc(args.timeout)
        receive_signal.torture_test(RX_CHAIN_SAMPLES, args.torture_rx,
                                    'rx_chain')
    if args.test_adc:
        if not args.test_adc%higgs.ETH_PACKET_SIZE:
            receive_signal.bind_rx_data_soc(args.timeout)
            receive_signal.test_adc(args.test_adc, PACKET_DELAY)
        else:
            logger.error('Sample size need to be a multiple of %d',
                         higgs.ETH_PACKET_SIZE)
    if args.torture_adc:
        receive_signal.bind_rx_data_soc(args.timeout)
        receive_signal.torture_test(RX_CHAIN_SAMPLES, args.torture_adc,
                                    'adc_chain')

###############################################################################
# Main Script
###############################################################################
if __name__ == "__main__":
    main()