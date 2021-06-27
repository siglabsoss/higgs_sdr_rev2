import struct
import socket
from socket import *
from time import sleep

DAC_DATA_PORT = 50000
IP_ADDR = "192.168.2.3"
PORT = 50000


s = socket(AF_INET, SOCK_DGRAM)

seq_num = 0

dac_data = range(367) # 367 dac IQ sample pairs in one max size UDP packet
dac_data_str = ""

for i in range(len(dac_data)):
    dac_data_str += struct.pack("!I", dac_data[i])

while (1):

    tx_str = struct.pack("!I", seq_num) + dac_data_str
    seq_num += 1
    
    s.sendto(tx_str, (IP_ADDR, PORT))
    sleep(0.001)