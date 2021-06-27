onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate /tb_cs12_top/UUT/mib_slave_wrapper/i_sysclk
add wave -noupdate /tb_cs12_top/UUT/mib_slave_wrapper/i_srst
add wave -noupdate /tb_cs12_top/UUT/mib_slave_wrapper/i_mib_start
add wave -noupdate /tb_cs12_top/UUT/mib_slave_wrapper/i_mib_rd_wr_n
add wave -noupdate /tb_cs12_top/UUT/mib_slave_wrapper/o_mib_slave_ack
add wave -noupdate /tb_cs12_top/UUT/mib_slave_wrapper/mib_dabus
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {4235785 ps} 0}
quietly wave cursor active 1
configure wave -namecolwidth 40
configure wave -valuecolwidth 40
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
WaveRestoreZoom {0 ps} {15398250 ps}
