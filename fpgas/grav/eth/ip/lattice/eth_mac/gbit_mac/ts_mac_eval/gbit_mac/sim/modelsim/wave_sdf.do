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