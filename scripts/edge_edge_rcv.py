import higgs
import time
import threading
import socket as sc

class ReceiveCMD:
    def __init__(self, host=higgs.HOST, our_host=higgs.OUR_HOST,
                 rx_data_port=higgs.RX_DATA_PORT, rx_cmd_port= higgs.RX_CMD_PORT,
                 connect_type=sc.SOCK_DGRAM):
        self.higgs_controller = higgs.HiggsController(host=host,
                                                      our_host=our_host,
                                                      rx_data_port=rx_data_port,
                                                      rx_cmd_port=rx_cmd_port,
                                                      connect_type=connect_type)

    def bind_rx_cmd_soc(self, timeout):
        self.higgs_controller.bind_rx_cmd_soc(timeout)

    def _stream_cmd(self, buffer_size):
        while(self.stream_thread_alive):
            try:
                self.data_packet = self.higgs_controller.get_cmd(buffer_size)
            except KeyboardInterrupt:
                self.stream_thread_alive = False

    def stream_cmd_thread(self, buffer_size):
        stream_thread = threading.Thread(target=self._stream_cmd,
                                         args=(buffer_size,))
        self.stream_thread_alive = True
        stream_thread.setDaemon(True)
        self.thread_data_lock = threading.Lock()
        with self.thread_data_lock:
            self.data_packet = None
        stream_thread.start()

    def handle_recv_packet(self):
        action = True
        while(action):
            with self.thread_data_lock:
                if(self.data_packet != None):
                    action = False
                    print hex(self.data_packet[1])

if __name__ == "__main__":
    # obj_eth = higgs.HiggsController()
    # data = obj_eth.get_cmd(4)
    receive_cmd = ReceiveCMD()
    receive_cmd.bind_rx_cmd_soc(1.0)
    receive_cmd.stream_cmd_thread(8)
    receive_cmd.handle_recv_packet()
