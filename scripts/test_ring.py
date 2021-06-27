###############################################################################
# Libraries and Modules
###############################################################################
import struct
import time
import argparse
import sys
import higgs
import socket as sc
from log import logger
###############################################################################
# Constants
###############################################################################
DEFAULT_RING_MSG = 0x00dead00
RING_DELAY = 0.005
###############################################################################
# Class Definitions
###############################################################################
class TestRing:
    def __init__(self, host=higgs.HOST, tx_cmd_port=higgs.TX_CMD_PORT,
                 rx_cmd_port=higgs.RX_CMD_PORT, connect_type=sc.SOCK_DGRAM):
        self.higgs_controller = higgs.HiggsController(host=host,
                                                      tx_cmd_port=tx_cmd_port,
                                                      rx_cmd_port=rx_cmd_port,
                                                      connect_type=connect_type)

    # returns 0 for success
    def test_fpga(self, fpga, message):
        logger.info('%s %s', higgs.FPGA_ADDR[fpga], fpga)
        addr = (len(higgs.FPGA_ADDR) - 1) - higgs.FPGA_ADDR[fpga]
        message |= addr
        self.higgs_controller.send_cmd(fpga, message, RING_DELAY)
        return_msg = self.higgs_controller.get_cmd(8)
        timeout_cnt = 0
        while(return_msg is not None and message != return_msg[1]):
            return_msg = self.higgs_controller.get_cmd(8)
            timeout_cnt += 1
            if(timeout_cnt == 20):
              break
        try:
            assert(message == return_msg[1])
            logger.info('%s PASSED: Ringbus test for %s passed',
                        fpga.upper(), fpga.upper())
        except AssertionError:
            logger.info('%s FAILED: Ringbus test for %s failed. ' +\
                        'Received incorrect value %s', fpga.upper(), fpga.upper(), hex(return_msg[1]))
            return higgs.FPGA_ADDR[fpga]*10 + 0
        except TypeError:
            logger.info('%s FAILED: Ringbus test for %s failed. ' +\
                        'No packet received', fpga.upper(), fpga.upper())
            return higgs.FPGA_ADDR[fpga]*10 + 1

        return 0


    def test_all_fpga(self, message, report_fail):
        test_results = ["cs11", "cs01", "cs02", "cs12",
                        "cs22", "cs32",
                        "cs31", "cs21", "cs20"]
        first_fail = None
        fail_fpga = None
        fail_count = 0
        for fpga in test_results:
            ret = self.test_fpga(fpga, message)
            if first_fail is None and ret != 0:
                first_fail = ret
                fail_fpga = fpga
            if ret != 0:
                fail_count += 1

        if first_fail is not None:
            print ""
            print ""
            print "(",fail_count,")", "in total did not pass. The first FPGA was:"
            print " ", fail_fpga, " ret code: ", first_fail
            print ""
            if report_fail:
                sys.exit(first_fail)
        else:
            sys.exit(0)

###############################################################################
# Method Definitions
###############################################################################
def main():
    parser = higgs.create_parser()
    parser.add_argument('-f', '--fpga',
                              help='select which FPGA to test ringbus',
                              nargs='?',
                              type=str)
    parser.add_argument('-a', '--all_fpga',
                              help='test all FPGA ringbus',
                              action='store_true')
    parser.add_argument('-m', '--msg',
                              help='message to send to FPGA via ringbus',
                              nargs='?',
                              type=int,
                              default=DEFAULT_RING_MSG)
    parser.add_argument('-e', '--errexit',
                           action='store_true',
                           help='set flag to return exit 1 if any fpga fails.  this should be default true but it was added late so leaving as default false to be compatible with peoples upenter')

    args = parser.parse_args()
    test_ring = TestRing(host=args.host, tx_cmd_port=args.tx_cmd_port,
                                         rx_cmd_port=args.rx_cmd_port)
    test_ring.higgs_controller.bind_rx_cmd_soc(1.0)

    report_fail = False

    if args.errexit:
        report_fail = True


    if args.fpga != None:
        test_ring.test_fpga(args.fpga, args.msg)
    if args.all_fpga:
        test_ring.test_all_fpga(args.msg, report_fail)
###############################################################################
# Main Script
###############################################################################
if __name__ == "__main__":
    main()