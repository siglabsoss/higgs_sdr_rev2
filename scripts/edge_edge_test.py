import higgs
import sys
import thread

if __name__ == "__main__":
    obj_eth = higgs.HiggsController()

    obj_eth.bind_rx_cmd_soc(1.0)
    obj_eth.bind_rx_data_soc(1.0)

    obj_eth.send_cmd(sys.argv[1], 0x12000000, 0.005)
    obj_eth.send_cmd(sys.argv[1], 0x11000000, 0.005)

    obj_eth.send_cmd(sys.argv[2], 0x12000000, 0.005)
    obj_eth.send_cmd(sys.argv[2], 0x11000000, 0.005)

    obj_eth.send_cmd(sys.argv[1], 0x12000001, 0.005) # dma_out
    obj_eth.send_cmd(sys.argv[2], 0x11000001, 0.005) # dma_in



    if sys.argv[2] == 'eth':
        print("operating differently for eth")
        data = obj_eth.get_data(1472)
        data2 = obj_eth.get_data(1472)
        # as far as I can tell the order coming out here is wrong
        print([hex(x) for x in data])
        print([hex(x) for x in data2])
    else:
        data = hex(obj_eth.get_cmd(8)[1])

        if data[2:6] == 'dead':
            print("edge to edge test between "+ str(sys.argv[1]) + " and " + str(sys.argv[2]) + " failed")
            sys.exit(1)
        elif data[2:6] == 'babe':
            print("edge to edge test between "+ str(sys.argv[1]) + " and " + str(sys.argv[2]) + " is successful")
            sys.exit(0)
