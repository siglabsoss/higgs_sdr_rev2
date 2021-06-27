add wave -noupdate -divider {CORE IO Signals}
add wave -noupdate -divider {Tx MAC Application Interface}
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/U1_ts_mac_top/txmac_clk
add wave -noupdate -format Literal -radix hexadecimal /test_ts_mac/U1_ts_mac_top/tx_fifodata
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/U1_ts_mac_top/tx_fifoeof
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/U1_ts_mac_top/tx_fifoavail
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/U1_ts_mac_top/tx_fifoempty
add wave -noupdate -format Literal -radix hexadecimal /test_ts_mac/U1_ts_mac_top/tx_sndpaustim
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/U1_ts_mac_top/tx_sndpausreq
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/U1_ts_mac_top/tx_fifoctrl
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/U1_ts_mac_top/tx_macread
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/U1_ts_mac_top/tx_discfrm
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/U1_ts_mac_top/tx_done
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/U1_ts_mac_top/tx_staten
add wave -noupdate -format Literal -radix hexadecimal /test_ts_mac/U1_ts_mac_top/tx_statvec
add wave -noupdate -divider {Rx MAC Aplication Interface}
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/U1_ts_mac_top/rxmac_clk
add wave -noupdate -format Literal -radix hexadecimal /test_ts_mac/U1_ts_mac_top/rx_dbout
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/U1_ts_mac_top/rx_eof
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/U1_ts_mac_top/rx_error
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/U1_ts_mac_top/rx_fifo_error
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/U1_ts_mac_top/rx_write
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/U1_ts_mac_top/rx_stat_en
add wave -noupdate -format Literal -radix hexadecimal /test_ts_mac/U1_ts_mac_top/rx_stat_vector
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/U1_ts_mac_top/rx_fifo_full
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/U1_ts_mac_top/ignore_pkt
add wave -noupdate -divider {CPU Interface Signals}
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/U1_ts_mac_top/hclk
add wave -noupdate -format Literal -radix hexadecimal /test_ts_mac/U1_ts_mac_top/haddr
add wave -noupdate -format Literal -radix hexadecimal /test_ts_mac/U1_ts_mac_top/hdatain
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/U1_ts_mac_top/hcs_n
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/U1_ts_mac_top/hwrite_n
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/U1_ts_mac_top/hread_n
add wave -noupdate -format Literal -radix hexadecimal /test_ts_mac/U1_ts_mac_top/hdataout
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/U1_ts_mac_top/hdataout_en_n
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/U1_ts_mac_top/hready_n
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/U1_ts_mac_top/gbit_en
add wave -noupdate -divider {GMII Signals}
add wave -noupdate -format Literal -radix hexadecimal /test_ts_mac/U1_ts_mac_top/txd
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/U1_ts_mac_top/tx_en
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/U1_ts_mac_top/tx_er
add wave -noupdate -format Literal -radix hexadecimal /test_ts_mac/U1_ts_mac_top/rxd
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/U1_ts_mac_top/rx_dv
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/U1_ts_mac_top/rx_er
add wave -noupdate -divider {FPGA IO Signals}
add wave -noupdate -divider {Clocks and Resets}
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/clk_125
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/sys_clk
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/rxmac_clk_wire
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/txmac_clk_wire
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/hclk
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/reset_n
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/phy_reset_n
add wave -noupdate -divider {GMII Signals}
add wave -noupdate -divider {Transmit}
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/tx_clk
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/gtx_clk
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/tx_en
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/tx_er
add wave -noupdate -format Literal -radix hexadecimal /test_ts_mac/txd
add wave -noupdate -divider {Receive}
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/rx_clk
add wave -noupdate -format Literal -radix hexadecimal /test_ts_mac/rxd
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/rxer
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/rxdv
add wave -noupdate -divider {Register Read Write Interface}
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/pc_clk
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/pc_datain
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/pc_dataout
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/pc_retry
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/pc_error
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/pc_ready
add wave -noupdate -format Logic -radix hexadecimal /test_ts_mac/pc_ack
