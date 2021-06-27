import higgs

if __name__ == "__main__":
    obj_eth = higgs.HiggsController()
    obj_eth.send_cmd("cs00", 0x12000000, 0.005)
    obj_eth.send_cmd("cs00", 0x11000000, 0.005)

    obj_eth.send_cmd("cs01", 0x12000000, 0.005)
    obj_eth.send_cmd("cs01", 0x11000000, 0.005)

    obj_eth.send_cmd("cs11", 0x12000000, 0.005)
    obj_eth.send_cmd("cs11", 0x11000000, 0.005)

    obj_eth.send_cmd("cs21", 0x12000000, 0.005)
    obj_eth.send_cmd("cs21", 0x11000000, 0.005)

    obj_eth.send_cmd("cs31", 0x12000000, 0.005)
    obj_eth.send_cmd("cs31", 0x11000000, 0.005)

    obj_eth.send_cmd("cs30", 0x12000000, 0.005)
    obj_eth.send_cmd("cs30", 0x11000000, 0.005)

    obj_eth.send_cmd("cs20", 0x12000000, 0.005)
    obj_eth.send_cmd("cs20", 0x11000000, 0.005)

    obj_eth.send_cmd("cs10", 0x12000000, 0.005)
    obj_eth.send_cmd("cs10", 0x11000000, 0.005)

    # obj_eth.send_cmd("cs00", 0x12000001, 0.005)
    # obj_eth.send_cmd("cs01", 0x11000001, 0.005)

    # data = obj_eth.get_cmd(4)
    # print data
