import struct
import socket
from socket import *
from time import sleep

IP_ADDR = "192.168.2.2"
FPGA_CMD_RX_PORT = 60002
FPGA_CMD_TX_PORT = 60003

#FPGA_CMD_RX_PORT = 60000
#FPGA_CMD_TX_PORT = 60001


s = socket(AF_INET, SOCK_DGRAM)
s.bind(("", FPGA_CMD_TX_PORT))
s.settimeout(1)

seq_num = 0

#reg_write_req0 = struct.pack("<IBII", 0, 0x00, 0x01c00004, 0x12345678) # ETH FPGA internal scratch reg 0
reg_write_req0 = struct.pack("<IBII", 0, 0x00, 0x0080000c, 0x4) 
#reg_write_req1 = struct.pack("<IBII", 1, 0x00, 0x00000004, 0xdeadbeef) # ETH FPGA internal scratch reg 1

reg_read_req0 = struct.pack("<IBI", 2, 0x01, 0x01c00000)
reg_read_req1 = struct.pack("<IBI", 3, 0x01, 0x01d00000)
reg_read_req2 = struct.pack("<IBI", 4, 0x01, 0x01e00000)
reg_read_req3 = struct.pack("<IBI", 5, 0x01, 0x01f00000)

#reg_read_req4 = struct.pack("<IBI", 6, 0x01, 0x01400000)
#reg_read_req5 = struct.pack("<IBI", 7, 0x01, 0x01500000)
#reg_read_req6 = struct.pack("<IBI", 8, 0x01, 0x00800000)
#reg_read_req7 = struct.pack("<IBI", 9, 0x01, 0x01900000)

#reg_read_req0 = struct.pack("<IBI", 2, 0x01, 0x00000000)
#reg_read_req1 = struct.pack("<IBI", 3, 0x01, 0x00000004)
#reg_read_req2 = struct.pack("<IBI", 4, 0x01, 0x00000008)
#reg_read_req3 = struct.pack("<IBI", 5, 0x01, 0x0000000c)
 
cnt = 0
     
while (True):
    input('hit enter:')
    s.sendto(reg_write_req0, (IP_ADDR, FPGA_CMD_RX_PORT))
    print (struct.unpack("<IBBI", s.recv(1500)))
#    reg_read_req0 = struct.pack("<IBI", cnt, 0x01, 0x00000000)
#    cnt = cnt + 1
    #s.sendto(reg_write_req0, (IP_ADDR, FPGA_CMD_RX_PORT))
    #s.sendto(reg_write_req1, (IP_ADDR, FPGA_CMD_RX_PORT))
#    s.sendto(reg_read_req0, (IP_ADDR, FPGA_CMD_RX_PORT))
#    print (struct.unpack("<IBBI", s.recv(1500)))
#    s.sendto(reg_read_req1, (IP_ADDR, FPGA_CMD_RX_PORT))
#    print (struct.unpack("<IBBI", s.recv(1500)))
#    s.sendto(reg_read_req2, (IP_ADDR, FPGA_CMD_RX_PORT))
#    print (struct.unpack("<IBBI", s.recv(1500)))
#    s.sendto(reg_read_req3, (IP_ADDR, FPGA_CMD_RX_PORT))
#    print (struct.unpack("<IBBI", s.recv(1500)))
#    s.sendto(reg_read_req4, (IP_ADDR, FPGA_CMD_RX_PORT))
#    print (struct.unpack("<IBBI", s.recv(1500)))
#    s.sendto(reg_read_req5, (IP_ADDR, FPGA_CMD_RX_PORT))
#    print (struct.unpack("<IBBI", s.recv(1500)))
#    s.sendto(reg_read_req6, (IP_ADDR, FPGA_CMD_RX_PORT))
#    print (struct.unpack("<IBBI", s.recv(1500)))
#    s.sendto(reg_read_req7, (IP_ADDR, FPGA_CMD_RX_PORT))
#    print (struct.unpack("<IBBI", s.recv(1500)))

#adc_reg_write_req = struct.pack("<IBII", 4, 0x00, 0x01230008, 0x00000000)
#s.sendto(adc_reg_write_req, (IP_ADDR, FPGA_CMD_RX_PORT))
#print struct.unpack("<IBBI", s.recv(1500))
#
#second_amp_reg_write_req = struct.pack("<IBII", 5, 0x00, 0x01220000, 0x00000055)
#s.sendto(second_amp_reg_write_req, (IP_ADDR, FPGA_CMD_RX_PORT))
#print struct.unpack("<IBBI", s.recv(1500))
#
#dac_dsp_reg_write_req = struct.pack("<IBII", 6, 0x00, 0x01130000, 0x00000000)
#s.sendto(dac_dsp_reg_write_req, (IP_ADDR, FPGA_CMD_RX_PORT))
#print struct.unpack("<IBBI", s.recv(1500))