###############################################################################
###############################################################################
# Name: set_adc.py
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
import numpy as np
import matplotlib.pyplot as plt
from log import logger
###############################################################################
# Constants
###############################################################################
VGA_GAIN = 0 # Corresponds to 26 dB gain
GAIN_ADDR = 0x00000200
VGA_CHANNEL = {'A':0x00010000, 'B':0x00000000}
RX_CHANNEL = 'B'
PACKET_DELAY = 0.005
MAX_ATTEN_CHANNEL_A = 31
MAX_ATTEN_CHANNEL_B = 30
MIN_ATTEN = 0
BLOCK_SIZE = 6
###############################################################################
# Class Definitions
###############################################################################
class SetADC:
    def __init__(self, host=higgs.HOST, our_host=higgs.OUR_HOST,
                 rx_cmd_port=higgs.RX_CMD_PORT, tx_cmd_port=higgs.TX_CMD_PORT,
                 connect_type=sc.SOCK_DGRAM):
        self.higgs_controller = higgs.HiggsController(host=host,
                                                      our_host=our_host,
                                                      rx_cmd_port=rx_cmd_port,
                                                      tx_cmd_port=tx_cmd_port,
                                                      connect_type=connect_type)
        self.attenuation_value = np.arange(0, 32, 2)

    def set_vga_attenuation(self, attenuation, channel):
        cmd_packet = higgs.VGA_GAIN_CMD|\
                           VGA_CHANNEL[channel]|\
                           GAIN_ADDR|\
                           attenuation
        self.higgs_controller.send_cmd('eth', cmd_packet, PACKET_DELAY)
        logger.info('Setting attenuation of Variable Gain Attenuator (VGA) ' +\
                    'to %d at channel %s', attenuation, channel)

    def set_dsa_attenuation(self, attenuation, channel):
        attenuation = self._get_attenuation(attenuation, channel)
        cmd_packet = higgs.DSA_GAIN_CMD|VGA_CHANNEL[channel]|attenuation
        self.higgs_controller.send_cmd('eth', cmd_packet, PACKET_DELAY)
        logger.info('Setting attenuation of Digital Step Attenuator (DSA) ' +\
                    'to %d at channel %s', attenuation, channel)

    def measure_saturation(self, fpga, attenuation, block_size, channel):
        attenuation = self._get_attenuation(attenuation, channel)
        cmd_packet = higgs.SATURATION_RATIO_CMD|attenuation|block_size
        self.higgs_controller.send_cmd(fpga, cmd_packet, PACKET_DELAY)
        saturated_samples = self.higgs_controller.get_cmd(8)
        total_sample = saturated_samples[1]&0xffff
        saturated_count = saturated_samples[1]>>16
        saturated_ratio = float(saturated_count)/float(total_sample)
        logger.info('Saturated samples: %d, ' +\
                    'Total samples %d, ' +\
                    'Saturated ratio %f',
                    saturated_count, total_sample, saturated_ratio)

        return saturated_ratio, total_sample

    def create_sat_map(self, channel):
        power = []
        saturation_ratio = self._get_clipping_curve(BLOCK_SIZE, channel)

        for attenuation in self.attenuation_value:
            power.append(self.estimate_power('cs00', attenuation, channel))

        logger.info(saturation_ratio)
        logger.info(power)
        fig = plt.figure()
        ax1 = fig.add_subplot(111)
        ax2 = ax1.twinx()
        ax1.plot(self.attenuation_value, saturation_ratio, 'g-', marker='o')
        ax2.plot(self.attenuation_value, power, 'b-', marker='x')

        ax1.set_xlabel('Attenuation (dB)')
        ax1.set_ylabel('Clipping Ratio', color='g')
        ax2.set_ylabel('Power', color='b')

        plt.show()

    def create_block_size_map(self, channel):
        block_size_fig = plt.figure(figsize=(16, 10))
        block_size_axes = block_size_fig.add_subplot(111)
        for block_size in range(4, 7):
            saturation_ratio = self._get_clipping_curve(block_size, channel)
            block_size_axes.plot(self.attenuation_value,
                                 saturation_ratio,
                                 marker='o',
                                 label=str(2**(block_size+4)) + ' Block Size')
        handles, labels = block_size_axes.get_legend_handles_labels()
        block_size_axes.legend(handles, labels)
        block_size_axes.set_xlabel('Attenuation (dB)')
        block_size_axes.set_ylabel('Clipping Ratio', color='g')
        plt.show()

    def create_sat_variation_map(self, iteration, channel):
        end_block = 1
        handles = []
        start_pos = np.arange(0, (6-end_block)*16, 6-end_block)
        boxplot_data = np.empty((0, 16))
        clipping_var_fig = plt.figure(figsize=(30, 16))
        clipping_var_axes = clipping_var_fig.add_subplot(111)
        for block_size in range(6, end_block, -1):
            for each_run in range(iteration):
                saturation_ratio = self._get_clipping_curve(block_size, channel)
                boxplot_data = np.append(boxplot_data, [saturation_ratio],
                                                       axis=0)
            boxes = clipping_var_axes.boxplot(boxplot_data,
                                              patch_artist=True,
                                              positions=start_pos+block_size)
            self._set_box_color(boxes, block_size)
            handles.append(boxes['boxes'][0])
        clipping_var_axes.legend(handles, [1024, 512, 256, 128, 64, 32])
        clipping_var_axes.set_ylabel('Clipping Ratio', color='g')
        plt.show()

    def estimate_power(self, fpga, attenuation, channel):
        attenuation = self._get_attenuation(attenuation, channel)
        cmd_packet = higgs.POWER_ESTIMATION_CMD|attenuation
        self.higgs_controller.send_cmd(fpga, cmd_packet, PACKET_DELAY)
        power = self.higgs_controller.get_cmd(8)[1];
        logger.info(power)

        return power

    def test_agc(self):
        cmd_packet = higgs.AGC_TEST_CMD;
        self.higgs_controller.send_cmd('cs00', cmd_packet, PACKET_DELAY)
        attenuation = self.higgs_controller.get_cmd(8)[1];
        logger.info((attenuation>>6)*2)

    def _get_attenuation(self, attenuation, channel):
        if channel == 'A':
            return (attenuation*4)<<8
        else:
            return (attenuation*4)<<3

    def _get_clipping_curve(self, block_size, channel):
        saturation_ratio = []
        for attenuation in self.attenuation_value:
            sat_ratio = self.measure_saturation('cs00',
                                                attenuation,
                                                block_size,
                                                channel)
            saturation_ratio.append(sat_ratio[0])

        return saturation_ratio

    def _set_box_color(self, boxes, block_size):
        box_color = ['indigo', 'violet', 'red',
                     'blue', 'green', 'orange', 'yellow']
        for box in boxes['boxes']:
                box.set(facecolor=box_color[block_size])

###############################################################################
# Method Definitions
###############################################################################
def main():
    parser = higgs.create_parser()
    parser.add_argument('-vga', '--attenuate_vga',
                                help='set attenuation value of VGA',
                                nargs='?',
                                type=int)
    parser.add_argument('-dsa', '--attenuate_dsa',
                                help='set attenuation value of DSA',
                                nargs='?',
                                type=int)
    parser.add_argument('-sat', '--saturation',
                                help='return a ratio of saturated samples. ' +\
                                     'Set attenuation value',
                                nargs='?',
                                type=int)
    parser.add_argument('-var', '--variation_map',
                                help='create a graph highlighting the ' +\
                                     'variations in saturation vs gain',
                                nargs='?',
                                type=int)
    parser.add_argument('-map', '--saturation_map',
                                help='create a graph of saturation ratio ' +\
                                     'versus gain',
                                action='store_true')
    parser.add_argument('-agc', '--test_agc',
                               help='test if AGC is properly working',
                               action='store_true')
    parser.add_argument('-bs', '--block_size',
                               help='create a saturation graph using ' +\
                                    'different block size',
                               action='store_true')
    parser.add_argument('-p', '--power_estimation',
                              help='estimate power of signal',
                              nargs='?',
                              type=int)
    parser.add_argument('-c', '--channel',
                              help='set receive channel',
                              nargs='?',
                              type=str,
                              choices=['A', 'B'],
                              default=RX_CHANNEL)
    args = parser.parse_args()
    set_adc = SetADC(host=args.host, our_host=args.our_host,
                     rx_cmd_port=args.rx_cmd_port, tx_cmd_port=args.tx_cmd_port)
    if args.attenuate_vga != None:
        set_adc.set_vga_attenuation(args.attenuate_vga, args.channel)
    if args.attenuate_dsa != None:
        atten_val = args.attenuate_dsa
        if args.channel == 'A':
            correct_range = (MAX_ATTEN_CHANNEL_A >= atten_val >= MIN_ATTEN)
            if correct_range and not (atten_val%1):
                set_adc.set_dsa_attenuation(atten_val, args.channel)
            else:
                message = 'Attenuation must be between 0 - 31dB in steps '
                message += 'of 1dB for Channel A'
                logger.info(message)
        elif args.channel == 'B':
            correct_range = (MAX_ATTEN_CHANNEL_B >= atten_val >= MIN_ATTEN)
            if correct_range and not(atten_val%2):
                set_adc.set_dsa_attenuation(atten_val, args.channel)
            else:
                message = 'Attenuation must be between 0 - 30dB in steps '
                message += 'of 2dB for Channel B'
                logger.info(message)
    if args.saturation != None:
        correct_range = (MAX_ATTEN >= args.saturation >= MIN_ATTEN)
        if correct_range and not(args.saturation%2):
            set_adc.higgs_controller.bind_rx_cmd_soc(1.0)
            set_adc.measure_saturation('cs00',
                                       args.saturation,
                                       BLOCK_SIZE,
                                       args.channel)
        else:
            logger.info('Attenuation must be between 0 - 30dB in steps of 2dB')
    if args.saturation_map:
        set_adc.higgs_controller.bind_rx_cmd_soc(1.0)
        set_adc.create_sat_map(args.channel)
    if args.block_size:
        set_adc.higgs_controller.bind_rx_cmd_soc(1.0)
        set_adc.create_block_size_map(args.channel)
    if args.test_agc:
        set_adc.higgs_controller.bind_rx_cmd_soc(1.0)
        set_adc.test_agc()
    if args.variation_map != None:
        set_adc.higgs_controller.bind_rx_cmd_soc(1.0)
        set_adc.create_sat_variation_map(args.variation_map, args.channel)
    if args.power_estimation != None:
        correct_range = (MAX_ATTEN >= args.power_estimation >= MIN_ATTEN)
        if correct_range and not(args.power_estimation%2):
            set_adc.higgs_controller.bind_rx_cmd_soc(1.0)
            set_adc.estimate_power('cs00', args.power_estimation, args.channel)

###############################################################################
# Main Script
###############################################################################
if __name__ == "__main__":
    main()