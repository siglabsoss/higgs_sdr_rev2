//Verilog instantiation template

eth_mac _inst (.gbit_mac_haddr(), .gbit_mac_hdatain(), .gbit_mac_hdataout(), 
        .gbit_mac_rx_dbout(), .gbit_mac_rx_stat_vector(), .gbit_mac_rxd(), 
        .gbit_mac_tx_fifodata(), .gbit_mac_tx_sndpaustim(), .gbit_mac_tx_statvec(), 
        .gbit_mac_txd(), .gbit_mac_cpu_if_gbit_en(), .gbit_mac_hclk(), 
        .gbit_mac_hcs_n(), .gbit_mac_hdataout_en_n(), .gbit_mac_hread_n(), 
        .gbit_mac_hready_n(), .gbit_mac_hwrite_n(), .gbit_mac_ignore_pkt(), 
        .gbit_mac_reset_n(), .gbit_mac_rx_dv(), .gbit_mac_rx_eof(), .gbit_mac_rx_er(), 
        .gbit_mac_rx_error(), .gbit_mac_rx_fifo_error(), .gbit_mac_rx_fifo_full(), 
        .gbit_mac_rx_stat_en(), .gbit_mac_rx_write(), .gbit_mac_rxmac_clk(), 
        .gbit_mac_tx_discfrm(), .gbit_mac_tx_done(), .gbit_mac_tx_en(), 
        .gbit_mac_tx_er(), .gbit_mac_tx_fifoavail(), .gbit_mac_tx_fifoctrl(), 
        .gbit_mac_tx_fifoempty(), .gbit_mac_tx_fifoeof(), .gbit_mac_tx_macread(), 
        .gbit_mac_tx_sndpausreq(), .gbit_mac_tx_staten(), .gbit_mac_txmac_clk());