###############################################################################
###############################################################################
# Name: bootload.py
# Coder: Janson Fang
# Description:
#   This is a command line script used to bootload new main programs in selected
# FPGAs in Higgs. This script was created as a way to write new programs in
# FPGAs without needing to recompile bitfiles. To bootload CS20, execute the
# following line from higgs_sdr_rev2:
#
#   python scripts/bootload.py cs20
#
# You can specify the port number, host, input file type, input filename, and
# packet delay. If optional arguements are not provided, the folowing default
# values will be used:
#
# HOST = 10.2.2.2
# PORT = 20000
# PACKET_DELAY = 0.001
# FILETYPE = hex
# FILEPATH = fpgas/$(CS_OR_GRAV)/fpga/build/tmp/$(FPGA)_top.$(FILETYPE)
#
# Please note the input file will be retrieved from the build/tmp folder of
# specified FPGA if not input file is provided
###############################################################################

###############################################################################
# Libraries and Modules
###############################################################################
from __future__ import print_function
import struct
import time
import argparse
import higgs
import socket as sc
import sys
from log import logger
from itertools import chain, izip
###############################################################################
# Constants
###############################################################################
PACKET_DELAY = 0.00000
# BOOTLOADER_ADDRESS_CUTOFF = 0x7bfc # not used
BOOTLOADER_LARGEST_STACK = 0x7900

# important, anything above this there is a jump in address to move to vmem
BOOTLOADER_ADDRESS_START = 0x7c00
V_MEM_START = 0x40000
V_MEM_LEN = 0x40000

###############################################################################
# Class Definitions
###############################################################################
class Bootloader:
    '''Bootload new programs in specified Higgs FPGAs
    
    Convert hex or csv file of a new program into 32 bit packets and sent via a
    UDP socket connection to Higgs

    Attributes:
        host (int): Host address of Higgs. Default is 10.2.2.2
        port (str): Port number of Higgs. Default is 20000
        connection_type (sc): Connection type to establish with Higgs. Default
        is UDP connection
    '''
    def __init__(self, host=higgs.HOST, port=higgs.TX_CMD_PORT,
                                        connect_type=sc.SOCK_DGRAM):
        self.host = host
        self.port = port
        self.addr = (host, port)
        self.connect_type = connect_type
        self.program_opcode = []
        self.dump_to_file = False  # If true, no packets are sent
        self.print_bytes = True

    def create_client(self):
        '''Creates a UDP client socket'''
        self.sock = sc.socket(sc.AF_INET, self.connect_type)

    def get_opcode(self, fpga, filetype, filepath, skipvmem):
        '''Retrieve opcode of new program to bootload to selected FPGA

        Args:
            fpga (str): Name of FPGA to get opcode ex. cs20
            filetype (str): File type of bootloader ex. csv or hex
            filepath (str): File path of hex or csv file of bootloader program  
        '''
        cs_or_grav = 'cs' if fpga[0:2] == 'cs' else 'grav'
        if filepath == None:
            filepath = 'fpgas/' + cs_or_grav + '/' + fpga + \
                       '/build/tmp/' + fpga + '_top.' + filetype
        try:
            with open(filepath) as bootprogram:
                self.program_opcode = []
                line = bootprogram.readline()

                if filetype == 'hex':
                    # start with full array of zeros
                    self.program_opcode = [0] * ((V_MEM_START+V_MEM_LEN)/4)

                self._parse_hex_reset()
                while line:
                    if(filetype == 'csv'):
                        self._parse_csv(fpga, line)
                    elif(filetype == 'hex'):
                        self._parse_hex(fpga, line)
                    line = bootprogram.readline()

                if filetype == 'hex':
                    # self._trim_right()

                    # right now bootloader has a small problem that is
                    # we don't know where the data ends and where the stack begins on the
                    # (soon to be overwritten) running program
                    # if we set this too low we risk not including compile time dmem values
                    # if we set it too high we will overwrite the active stack and crash the board
                    if skipvmem:
                        self._trim_hard(BOOTLOADER_LARGEST_STACK/4)
                    else:
                        # hacked bootloader makes a seam in memory which is undone by vex when bootloading
                        self._cut_glue_vmem()
                        self._vmem_trim_right()
                    # self._print_opcodes()

        except IOError:
            logger.info('Bootloading program ' + filepath + ' does not exist')


    def bootload_fpga_dual(self, fpga, delay):
        print_progress = not self.print_bytes
        # dont need this in the file
        if not self.dump_to_file:
            # always start with some zeros to eth directly
            for _ in range(4):
                self._send_dual_packet(higgs.FPGA_ADDR['eth'], 0, delay)
                self._send_dual_packet(higgs.FPGA_ADDR[fpga], (0x17000000) | 0x1 | 0x2 | 0x4, delay*4) # does this work?

        # dont write the size message for file outputs
        if not self.dump_to_file:
            self._init_bootloader(fpga, delay)

        # long time after first packet to give a slow polling program a chance
        if not self.dump_to_file:
            time.sleep(1.0)

        # number of ringbus messages to send in a single udp packet
        # note that the ringbus address is repeated for every byte so this number should not be more than udpmaxlen / 2
        # (at the moment this method breaks dump_to_file support)
        chunk = 4

        # math for printing, 1/thresh is number of prints
        progress_thresh = 1.0/10.0
        progress_last_p = 0.0

        


        llen = len(self.program_opcode)
        fpga_ttl = higgs.FPGA_ADDR[fpga]

        pp = lambda a : print("Bootloading ({})  {:0.2f}%".format(fpga_ttl,a))

        # printing
        if print_progress:
            print("")
            pp(0)
            sys.stdout.flush()

        for i in range(0,llen, chunk):
            opcodes = self.program_opcode[i:i+chunk]
            self._send_variable_packet(fpga_ttl, opcodes, delay)
            if print_progress:
                progress_at = i/(1.0*llen)
                if progress_at > (progress_last_p + progress_thresh):
                    pp(progress_at*100.0)
                    sys.stdout.flush()
                    progress_last_p += progress_thresh

        if print_progress:
            pp(100)
            print("")
            sys.stdout.flush()


        if not self.dump_to_file:
            logger.info('%d packets sent', llen)

        # right now eth requires the incoming DMAs to be cleared up after a bootload
        # we send 0x00000000 which is ignored by eth right now
        if fpga == 'eth' and not self.dump_to_file:
            logger.info('4 extra packets to help eth')
            time.sleep(1.0)
            for _ in range(4):
                self._send_dual_packet(fpga_ttl, 0, delay)
            


    def _init_bootloader(self, fpga, delay):
        filetype = "hex"
        '''Initialize Higgs to prepare bootloading sequence

        Initializing Higgs include telling Higgs which FPGA to bootload and the
        size of bootloader

        Args:
            fpga (str): Name of FPGA to bootload
            delay (float): Time delay between sending each packet
        '''
        # Setting destination FPGA to bootload
        # self._send_packet(FPGA_ADDR[fpga], PACKET_DELAY)
        if filetype == "hex":
            program_len = len(self.program_opcode)
        else:
            program_len = len(self.program_opcode)/2

        # Setting initial command to identify program size
        init_data = higgs.BOOTLOADER_CMD|program_len
        # self._send_packet(init_data, PACKET_DELAY)
        self._send_dual_packet(higgs.FPGA_ADDR[fpga], init_data, delay)
        logger.info('Finished initializing bootloader')


    def _send_dual_packet(self, d0, d1, delay):

        if self.dump_to_file:
            self.fid.write(hex(d1)[2:])
            self.fid.write('\n')
        else:
            time.sleep(delay)
            udp_packet = struct.pack('<II', d0, d1)
            self.sock.sendto(udp_packet, self.addr)
            if self.print_bytes:
                logger.info('Packet sent: %s %s', hex(d0), hex(d1))

    def _send_variable_packet(self, addr, ops, delay):
        time.sleep(delay)
        args = []

        opslen = len(ops)

        # we build arguments to the function in a list
        # and then call apply() to struct.pack (instead of a normal function call)
        args.append('<' + ('I' * (opslen*2) ) )

        for op in ops:
            args.append(addr)
            args.append(op)

        udp_packet = apply(struct.pack, args)
        hex_str = ''.join(["{0:#0{1}x}".format(x,10)[2:] for x in ops])

        self.sock.sendto(udp_packet, self.addr)
        if self.print_bytes:
            logger.info('Packet sent: (%s) %s', addr, hex_str)


    def _send_quad_packet(self, d0, d1, d2, d3, delay):

        if self.dump_to_file:
            self.fid.write(hex(d1)[2:])
            self.fid.write('\n')
        else:
            time.sleep(delay)
            udp_packet = struct.pack('<IIII', d0, d1, d2, d3)
            self.sock.sendto(udp_packet, self.addr)
            logger.info('Packet sent: %s %s %s %s', hex(d0), hex(d1), hex(d2), hex(d3))

    def _setup_csv_dump(self, fname):

        # print(fname)
        self.fid = open(fname, 'w')
        self.dump_to_file = True
        # sys.exit(0)
        pass

    def _send_packet(self, data, delay):
        '''Send UDP packet to Higgs
        
        Args:
            data (int): A 32-bit integer of data
            delay (float): Time delay between sending each packet
        '''
        time.sleep(delay)
        udp_packet = struct.pack('<I', data)
        self.sock.sendto(udp_packet, self.addr)
        logger.info('Packet sent: %s', hex(data))

    def _parse_csv(self, fpga, line):
        '''Method to parse bootloader program in CSV form
        
        Args:
            fpga (str): Name of FPGA to parse opcode from CSV
            line (str): A line of opcode
        '''
        opcode = int(line.split(',')[0], 16)
        self.program_opcode.append(higgs.FPGA_ADDR[fpga])
        self.program_opcode.append(opcode)


    def _parse_hex_reset(self):
        '''We have hex files that have an address "roll over" event
        We need _parse_hex to keep state, and this resets that state

        :return:
        '''
        self.parse_hex_upper = 0


    def _parse_hex(self, fpga, line):
        '''Method to parse bootloader program in HEX form

        Args:
            fpga (str): Name of FPGA to parse opcode from HEX
            line (str): A line of Intel Hex code
        '''
        line = line[1:]
        bytecount = int(line[0:2], 16)
        address = int(line[2:6], 16) + self.parse_hex_upper
        rec_type = int(line[6:8], 16)
        if rec_type == 2:
            # we need to catch this record type for upper addresses (vmem)
            new_base = int(line[8:8+4], 16)
            self.parse_hex_upper = new_base*16
            print("new base ", self.parse_hex_upper)
        if rec_type == 0:
            data_out = line[8:(8+2*(bytecount))]
            for index in range(0, len(data_out), 8):
                opcode = int(data_out[index + 6:index + 8] +\
                             data_out[index + 4:index + 6] +\
                             data_out[index + 2: index+ 4] +\
                             data_out[index: index + 2], 16)

                array_index = (address / 4) + (index / 8)
                # print("index", index, "address", address, "calcaddr", hex(array_index*4), "op", hex(opcode))
                # print("  " + line[2:6])

                self.program_opcode[array_index] = opcode
                # print hex(array_index*4), ":", hex(opcode)

                # self.program_opcode.append(FPGA_ADDR[fpga])
                # self.program_opcode.append(opcode)


    # trying a change where board fills vmem with zeros near time of bootload
    # and then we only bootload until the last non zero element
    # We can't trim below dmem boundary because we do NOT fill zeros in DMEM due to bootloader running out of it
    def _vmem_trim_right(self):
        last_zero = None
        for i, opcode in reversed(list(enumerate(self.program_opcode))):
            # print "i", i, "opcode", opcode
            if opcode == 0:
                last_zero = i
            else:
                break

            # force output array to always be 7936 or longer
            if i <= (BOOTLOADER_ADDRESS_START/4):
                last_zero = i
                break

        if last_zero is not None:
            del self.program_opcode[last_zero:]


    def _cut_glue_vmem(self):
        self.program_opcode = self.program_opcode[0:(BOOTLOADER_ADDRESS_START/4)] + self.program_opcode[(V_MEM_START/4):((V_MEM_START+V_MEM_LEN)/4)]

    def _trim_hard(self, maxx):
        del self.program_opcode[maxx:]

    # unused
    def _trim_right(self):
        last_zero = -1
        for i,opcode in reversed(list(enumerate(self.program_opcode))):
            # print "i", i, "opcode", opcode
            if opcode == 0:
                last_zero = i
            else:
                break
        # print "found last zero", last_zero
        # print self.program_opcode[0:last_zero]
        del self.program_opcode[last_zero:]

    def _print_opcodes(self):
        for array_index,opcode in enumerate(self.program_opcode):
            # print hex(array_index*4), ":", hex(opcode)
            print(hex(opcode))

    def _interleave_ttl(self, fpga):
        listb = [higgs.FPGA_ADDR[fpga]] * len(self.program_opcode)
        inter = list(chain.from_iterable(izip(listb, self.program_opcode)))
        self.program_opcode = inter

        # print self.program_opcode
        pass

###############################################################################
# Method Definitions
###############################################################################
def main():
    '''Command line program use to bootload selected FPGA'''
    parser = argparse.ArgumentParser(description='Bootload FPGA')
    parser.add_argument('fpga', help='specify which FPGA to bootload', type=str)
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
    parser.add_argument('-ft', '--filetype',
                               help='specify hex or csv file with opcode',
                               nargs='?',
                               type=str,
                               default='hex')
    parser.add_argument('-fp', '--filepath',
                               help='specify filepath of bootloader program',
                               nargs='?',
                               type=str,
                               default=None)
    parser.add_argument('-pd', '--packetdelay',
                               help='specify time delay (secs) between packets',
                               nargs='?',
                               type=float,
                               default=PACKET_DELAY)
    parser.add_argument('-dcf', '--dumpcsvfile',
                                help='if specified, dump to this .csv file and do not send packets',
                                nargs='?',
                                type=str,
                                default=None)
    parser.add_argument('-sv', '--skip-vmem',
                               action='store_true',
                               help='Skip bootloading vmem section, this will break programs that rely on vmem')

    parser.add_argument('-q', '--quiet',
                               help='don\'t print every byte sent',
                               action='store_true')
    args = parser.parse_args()
    logger.info('Bootloading %s at %s port %d', args.fpga, args.host, args.port)
    fpga = Bootloader(args.host, args.port)
    if(args.quiet):
        print("Quiet Bootload...")
        fpga.print_bytes = False
    fpga.create_client()
    fpga.get_opcode(args.fpga, args.filetype, args.filepath, args.skip_vmem)
    if(len(fpga.program_opcode) > 0):
        if(args.filetype=="hex"):
            if(args.dumpcsvfile):
                fpga._setup_csv_dump(args.dumpcsvfile)
            fpga.bootload_fpga_dual(args.fpga, args.packetdelay)
    else:
        logger.info('Bootloader program does not exist, or missing values for arguments')
###############################################################################
# Main Script
###############################################################################
if __name__ == "__main__":
    main()