###############################################################################
###############################################################################
# Name: test_nco.py
# Coder: Janson Fang
# Description:
###############################################################################

###############################################################################
# Libraries and Modules
###############################################################################
import higgs
import numpy as np
import socket as sc
from log import logger
###############################################################################
# Constants
###############################################################################
RX_BUFFER_SIZE = 1472
PACKET_DELAY = 0.005
###############################################################################
# Class Definitions
###############################################################################
class NCO:
    def __init__(self, host=higgs.HOST, our_host=higgs.OUR_HOST,
                 rx_data_port=higgs.RX_DATA_PORT, tx_cmd_port=higgs.TX_CMD_PORT,
                 connect_type=sc.SOCK_DGRAM):
        self.higgs_controller = higgs.HiggsController(host=host,
                                                      our_host=our_host,
                                                      rx_data_port=rx_data_port,
                                                      tx_cmd_port=tx_cmd_port,
                                                      connect_type=connect_type)

    def test_nco(self, nco_size):
        receive_data = ()
        packet_received = 0
        cmd_packet = higgs.NCO_TEST_CMD|nco_size
        self.higgs_controller.send_cmd('cs30', cmd_packet, PACKET_DELAY)
        for packet in range(nco_size/higgs.ETH_PACKET_SIZE + 1):
            try:
                receive_data += self.higgs_controller.get_data(RX_BUFFER_SIZE)
                packet_received += 1
            except TypeError:
                logger.error('Scheduled packet not received')
        # Uncomment line to see data received from CS30
        # logger.info(receive_data)
        receive_data = np.array(receive_data[:nco_size])
        self._compare_nco(receive_data)

    def _get_verilated_nco(self):
        verilated_nco = np.array([])
        with open(r'./cs20_out.hex') as nco_data:
            line = nco_data.readline()
            while line:
                verilated_nco = np.append(verilated_nco, int(line, 16))
                line = nco_data.readline()

        return verilated_nco

    def _compare_nco(self, received_nco):
        verilated_nco = self._get_verilated_nco()
        same_nco = np.array_equal(verilated_nco, received_nco)

        if same_nco:
            logger.info('NCO Test PASSED. A NCO of size %s matches with ' +\
                        'verilated results', received_nco.shape[0])
        else:
            try:
                assert(verilated_nco.shape == received_nco.shape)
            except AssertionError:
                logger.info('NCO Test FAILED. Received and verilated NCO ' +\
                            'has different shapes: %s %s', received_nco.shape,
                            verilated_nco.shape)
            else:
                logger.info('NCO Test FAILED. Received and verilated values ' +\
                            'are different')


###############################################################################
# Method Definitions
###############################################################################
def main():
    parser = higgs.create_parser()
    parser.add_argument('-nco', '--test_nco',
                                help='generate and verify NCO of specified' +\
                                     'size',
                                nargs='?',
                                type=int)
    args = parser.parse_args()
    numeric_oscillator = NCO(host=args.host,
                             our_host=args.our_host,
                             rx_data_port=args.rx_data_port,
                             tx_cmd_port=args.tx_cmd_port)
    if args.test_nco != None:
        numeric_oscillator.higgs_controller.bind_rx_data_soc(1.0)
        numeric_oscillator.test_nco(args.test_nco)

###############################################################################
# Main Script
###############################################################################
if __name__ == "__main__":
    main()