
###############################################################################
# Libraries and Modules
###############################################################################
import sys
import struct
import threading
import time
import argparse
import higgs
import numpy as np
from numpy.fft import fft, fftshift
import socket as sc
import matplotlib.pyplot as plt
from log import logger
from bitstring import BitArray
###############################################################################
# Constants
###############################################################################
# 1472 is max buffer size
RX_BUFFER_SIZE = 1472
RX_SOC_TIMEOUT = 1.0
RX_PACKET_COUNT = 178
PACKET_DELAY = 0.005
ETH_PACKET_SIZE = 367
RX_CHAIN_SAMPLES = 65326
SAVE_DATA_FILE_PATH = r'received_data.csv'
###############################################################################
# Class Definitions
###############################################################################

# I hate this
def nplotfft(rf, fs = 1, title=None, newfig=True, peaks=False, peaksHzSeparation=1, peaksFloat=1.1):
    fig = None

    if newfig is True:
        fig = nplotfigure()

    if title is not None:
        plt.title(title)

    N = len(rf)

    X = fftshift(fft(rf)/N)
    absbins = 2*np.abs(X)
    df = fs/N
    f = np.linspace(-fs/2, fs/2-df, N)

    plt.semilogy(f, absbins)
    plt.xlabel('Frequency (in hertz)')
    plt.ylabel('Magnitude Response')


    if peaks:
        peaksres = sig_peaks(X, f, peaks, peaksHzSeparation)
        if type(newfig) is type(True):
            ax = fig.add_subplot(111)
        else:
            ax = newfig


        for pk in peaksres:

            maxidx = pk
            maxval = absbins[pk]

            lbl = s_('hz:', f[maxidx])

            ax.annotate(lbl, xy=(f[maxidx], maxval), xytext=(f[maxidx], maxval * peaksFloat),
                        arrowprops=dict(facecolor='black'),
                        )

    return fig



class ReceiveData:
    def __init__(self, host=higgs.HOST, our_host=higgs.OUR_HOST,
                 rx_data_port=higgs.RX_DATA_PORT, connect_type=sc.SOCK_DGRAM):
        self.higgs_controller = higgs.HiggsController(host=host,
                                                      our_host=our_host,
                                                      rx_data_port=rx_data_port,
                                                      connect_type=connect_type)

    def plot_data(self, data_source):
        rx_signal = data_source
        # for value in rx_signal_raw:
        #     real = self._signed_value(value&0xffff, 16)
        #     imag = self._signed_value((value >> 16) & 0xffff, 16)
        #     rx_signal= np.append(rx_signal, complex(real, imag))
        rx_real = np.real(rx_signal)
        rx_imag = np.imag(rx_signal)
        fft_rx_signal = np.fft.fft(rx_signal)
        rx_mag = np.abs(fft_rx_signal)
        rx_f = np.fft.fftfreq(len(fft_rx_signal), d=1.0/31250000)
        # logger.info(rx_signal)
        plt.figure()
        plt.subplot(221)
        plt.plot(rx_f, rx_mag)
        plt.subplot(222)
        plt.plot(rx_real)
        plt.subplot(223)
        plt.plot(rx_imag)
        plt.subplot(224)
        plt.plot(rx_real)
        plt.plot(rx_imag)

        plt.figure()
        nplotfft(rx_signal, newfig=False)


        # plt.show()

    def ncplot(self, rf, title=None):

        plt.figure()

        if title is not None:
            plt.title(title)

        plt.plot(np.real(rf))
        plt.plot(np.imag(rf), 'r')

    def nplotqam(self, rf, title=None):
        plt.figure()

        if title is not None:
            plt.title(title)

        plt.plot(np.real(rf), np.imag(rf), '.b', alpha=0.6)

    def _save_recv_data(self, recv_data, save_data):
        np.savetxt(save_data, recv_data, fmt='%d, %d', delimiter=',')

    def _load_recv_data(self, path):
        return np.loadtxt(path, delimiter=',').view(complex)

    def _load_hex_file(self, filepath):
        with open(filepath) as bootprogram:
            lines = bootprogram.readlines()

        words = [int(l,16) for l in lines]

        rf = []
        for w in words:
            real = self._signed_value(w&0xffff, 16)
            imag = self._signed_value((w >> 16) & 0xffff, 16)
            rf.append(np.complex(real,imag))
            # print "r", real, " i ", imag
            # print rf[0]
            # sys.exit(0)
        return rf

        # return np.loadtxt(path, delimiter=',').view(complex)

    def _signed_value(self, value, bit_count):
        if value > 2**(bit_count - 1) - 1:
            value -= 2**bit_count 

        return value
###############################################################################
# Method Definitions
###############################################################################
def main():
    parser = higgs.create_parser()
    parser.add_argument('-csv', '--csv_file_path',
                              help='path to a .csv file',
                              nargs='?',
                              type=str)
    parser.add_argument('-hex', '--hex_file_path',
                              help='path to a .hex file',
                              nargs='?',
                              type=str)
    
    parser.add_argument('-scn', '--subcarriers_normal',
                              help='specify subcarrier. ordered as 0th in file is 0th',
                              nargs='+')

    parser.add_argument('-scr', '--subcarriers_reversed',
                              help='specify subcarrier. values will be subtracted from 1024 to compensate for our transmit/receiver subcarrier numbering discrepancies',
                              nargs='+')

    parser.add_argument('-rotd', '--rotations',
                              help='degree rotations',
                              nargs='+')

    parser.add_argument('-cut', '--sample_cut',
                              help='how many SAMPLES to cut off the front',
                              nargs='?',
                              type=int)

    # parser.add_argument('-t', '--transform',
    #                           help='fixme',
    #                           action='store_true')

    args = parser.parse_args()
    receivedata = ReceiveData(host=args.host,
                                 our_host=args.our_host,
                                 rx_data_port=args.rx_data_port)


    if args.csv_file_path is not None and args.hex_file_path is not None:
        print "Don't pass both -csv and -hex at the same time!"
        print ""
        parser.print_usage()
        sys.exit(1)

    from_file = None

    if args.csv_file_path is not None:
        from_file = receivedata._load_recv_data(args.csv_file_path)

    if args.hex_file_path is not None:
        from_file = receivedata._load_hex_file(args.hex_file_path)

    if from_file is not None:

        sc_int = None
        print "Considering Subcarriers",
        if args.subcarriers_normal is not None:
            # convert from strings (given by argparse) to ints
            sc_int = [int(x,10) for x in args.subcarriers_normal]
            print sc_int
        elif args.subcarriers_reversed is not None:
            sc_int = [1024-int(x,10) for x in args.subcarriers_reversed]
            print "(Reversed) ", sc_int
        else:
            sc_int = None

        
        if sc_int is None:
            receivedata.plot_data(from_file)
        else:
            if args.rotations is not None:
                if len(args.rotations) != len(sc_int):
                    print "If passing -sc and -rotd together, they must be the same length"
                    sys.exit(1)

                rot_rad = [np.deg2rad(int(x,10)) for x in  args.rotations]

            for i,sc in enumerate(sc_int):
                rf = from_file[sc::1024]
                if args.rotations is not None:
                    rot_single = rot_rad[i]
                    rf = np.array(rf) * np.exp(1j*rot_single)

                if args.sample_cut is not None:
                    print "Cutting off first", args.sample_cut, "samples"
                    rf = rf[args.sample_cut:]

                # if args.transform:
                #     print "transforming"
                #     rf = np.vectorize(complex)(np.imag(rf), np.real(rf))
                #     # rf = np.complex(np.imag(rf), )

                # print rf
                titl = "sc " + str(sc)
                if args.rotations is not None:
                    titl += " rot: " + str(args.rotations[i])
                # if args.transform:
                #     titl += " (transformed)"
                receivedata.ncplot(rf, titl)
                receivedata.nplotqam(rf, titl)

        plt.show()
    else:
        parser.print_usage()


###############################################################################
# Main Script
###############################################################################
if __name__ == "__main__":
    main()
