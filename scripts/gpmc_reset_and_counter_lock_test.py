import struct
import socket
from socket import *
from time import sleep

IP_ADDR = "192.168.2.2"

GRAV_FPGA_CMD_RX_PORT = 60000
GRAV_FPGA_CMD_TX_PORT = 60001
CS_FPGA_CMD_RX_PORT = 60002
CS_FPGA_CMD_TX_PORT = 60003

GRAV_FPGA_CMD_IP_PORT = (IP_ADDR, GRAV_FPGA_CMD_RX_PORT)
CS_FPGA_CMD_IP_PORT = (IP_ADDR, CS_FPGA_CMD_RX_PORT)


grav = socket(AF_INET, SOCK_DGRAM)
grav.bind(("", GRAV_FPGA_CMD_TX_PORT))
grav.settimeout(1)

cs = socket(AF_INET, SOCK_DGRAM)
cs.bind(("", CS_FPGA_CMD_TX_PORT))
cs.settimeout(1)

grav_wr_req0 = struct.pack("<IBII", 0, 0x00, 0x01000008, 0x1) # generate software reset strobe
grav_wr_req1 = struct.pack("<IBII", 1, 0x00, 0x0100000c, 0x1) # generate software counter lock strobe

grav_rd_req0 = struct.pack("<IBI", 2, 0x01, 0x01100008) # read DAC test counter lock value
grav_rd_req1 = struct.pack("<IBI", 3, 0x01, 0x01200008) # read ADC test counter lock value

cs_rd_req0 = struct.pack("<IBI", 0, 0x01, 0x01c00008) # read CS30 test counter lock value 
cs_rd_req1 = struct.pack("<IBI", 1, 0x01, 0x01000008) # read CS00 test counter lock value 
cs_rd_req2 = struct.pack("<IBI", 2, 0x01, 0x01e00008) # read CS32 test counter lock value 


loop_cnt = 0

# reset the board
#grav.sendto(grav_wr_req0, (IP_ADDR, GRAV_FPGA_CMD_RX_PORT)) # reset
#print (struct.unpack("<IBBI", grav.recv(1500)))
#sleep(2)

while (True):

    if loop_cnt == 0:
        # reset the board
        grav.sendto(grav_wr_req0, (IP_ADDR, GRAV_FPGA_CMD_RX_PORT)) # reset
        print (struct.unpack("<IBBI", grav.recv(1500)))
        sleep(1)

    grav.sendto(grav_wr_req1, (IP_ADDR, GRAV_FPGA_CMD_RX_PORT)) # lock counters
#    print (struct.unpack("<IBBI", grav.recv(1500)))
    grav.recv(1500)

    grav.sendto(grav_rd_req0, (IP_ADDR, GRAV_FPGA_CMD_RX_PORT)) # read dac locked count
    (seq_num, cmd, ack_nack, dac_cnt) = (struct.unpack("<IBBI", grav.recv(1500)))
#    print (struct.unpack("<IBBI", grav.recv(1500)))
    grav.sendto(grav_rd_req1, (IP_ADDR, GRAV_FPGA_CMD_RX_PORT)) # read adc locked count
    (seq_num, cmd, ack_nack, adc_cnt) = (struct.unpack("<IBBI", grav.recv(1500)))
#    print (struct.unpack("<IBBI", grav.recv(1500)))
    cs.sendto(cs_rd_req0, (IP_ADDR, CS_FPGA_CMD_RX_PORT)) # read cs30 locked count
    (seq_num, cmd, ack_nack, cs30_cnt) = (struct.unpack("<IBBI", cs.recv(1500)))
#    print (struct.unpack("<IBBI", cs.recv(1500)))
    cs.sendto(cs_rd_req1, (IP_ADDR, CS_FPGA_CMD_RX_PORT)) # read cs00 locked count
    (seq_num, cmd, ack_nack, cs00_cnt) = (struct.unpack("<IBBI", cs.recv(1500)))

    cs.sendto(cs_rd_req2, (IP_ADDR, CS_FPGA_CMD_RX_PORT)) # read cs00 locked count
    (seq_num, cmd, ack_nack, cs32_cnt) = (struct.unpack("<IBBI", cs.recv(1500)))
    
    print ("DAC Count: "        + str(dac_cnt).ljust(10)  + 
           " ADC Count: "       + str(adc_cnt).ljust(10)  + 
           " CS30 Count: "      + str(cs30_cnt).ljust(10) + 
           " CS00 Count: "      + str(cs00_cnt).ljust(10) + 
           " CS32 Count: "      + str(cs32_cnt).ljust(10) + 
           " DAC-ADC Delta: "   + str(abs((dac_cnt - adc_cnt))).ljust(3) +
           " DAC-CS30 Delta: "  + str(abs((dac_cnt - cs30_cnt))).ljust(3) + 
           " DAC-CS00 Delta: "  + str(abs((dac_cnt - cs00_cnt))).ljust(3) +
           " DAC-CS32 Delta: "  + str(abs((dac_cnt - cs32_cnt))).ljust(3))

    sleep(1)
    
    loop_cnt = loop_cnt + 1
    
    if loop_cnt == 10:
        loop_cnt = 0