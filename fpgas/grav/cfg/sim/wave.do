onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate /tb_cfg_top/UUT/_cfg_fpga_comm/_fmc_slave/i_sys_clk
add wave -noupdate /tb_cfg_top/UUT/_cfg_fpga_comm/_fmc_slave/i_sys_rst
add wave -noupdate /tb_cfg_top/UUT/_cfg_fpga_comm/_fmc_slave/o_cmd_sel
add wave -noupdate /tb_cfg_top/UUT/_cfg_fpga_comm/_fmc_slave/o_cmd_rd_wr_n
add wave -noupdate /tb_cfg_top/UUT/_cfg_fpga_comm/_fmc_slave/o_cmd_byte_addr
add wave -noupdate /tb_cfg_top/UUT/_cfg_fpga_comm/_fmc_slave/o_cmd_wdata
add wave -noupdate /tb_cfg_top/UUT/_cfg_fpga_comm/_fmc_slave/i_cmd_ack
add wave -noupdate /tb_cfg_top/UUT/_cfg_fpga_comm/_fmc_slave/i_cmd_rdata
add wave -noupdate /tb_cfg_top/UUT/_cfg_fpga_comm/_fmc_slave/o_cmd_timeout
add wave -noupdate /tb_cfg_top/UUT/_cfg_fpga_comm/_fmc_slave/i_fmc_a
add wave -noupdate /tb_cfg_top/UUT/_cfg_fpga_comm/_fmc_slave/i_fmc_d
add wave -noupdate /tb_cfg_top/UUT/_cfg_fpga_comm/_fmc_slave/o_fmc_d
add wave -noupdate /tb_cfg_top/UUT/_cfg_fpga_comm/_fmc_slave/o_fmc_d_high_z
add wave -noupdate /tb_cfg_top/UUT/_cfg_fpga_comm/_fmc_slave/i_fmc_ne1
add wave -noupdate /tb_cfg_top/UUT/_cfg_fpga_comm/_fmc_slave/i_fmc_noe
add wave -noupdate /tb_cfg_top/UUT/_cfg_fpga_comm/_fmc_slave/i_fmc_nwe
add wave -noupdate /tb_cfg_top/UUT/_cfg_fpga_comm/_fmc_slave/o_fmc_nwait
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {39967665 ps} 0}
quietly wave cursor active 1
configure wave -namecolwidth 493
configure wave -valuecolwidth 100
configure wave -justifyvalue left
configure wave -signalnamewidth 0
configure wave -snapdistance 10
configure wave -datasetprefix 0
configure wave -rowmargin 4
configure wave -childrowmargin 2
configure wave -gridoffset 0
configure wave -gridperiod 1
configure wave -griddelta 40
configure wave -timeline 0
configure wave -timelineunits ps
update
WaveRestoreZoom {0 ps} {263257050 ps}
