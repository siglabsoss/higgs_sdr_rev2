onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate /tb_cs00_top/UUT/resetter_125/RESET_COUNT
add wave -noupdate /tb_cs00_top/UUT/resetter_125/PIPE_LENGTH
add wave -noupdate /tb_cs00_top/UUT/resetter_125/i_rst_n
add wave -noupdate /tb_cs00_top/UUT/resetter_125/i_clk
add wave -noupdate /tb_cs00_top/UUT/resetter_125/i_enable
add wave -noupdate /tb_cs00_top/UUT/resetter_125/o_rst
add wave -noupdate /tb_cs00_top/UUT/resetter_125/count
add wave -noupdate /tb_cs00_top/UUT/resetter_125/sync_rst
add wave -noupdate /tb_cs00_top/UUT/resetter_125/sync_rst_out
add wave -noupdate /tb_cs00_top/UUT/resetter_125/count_rst_n
add wave -noupdate /tb_cs00_top/UUT/resetter_125/gen_rst
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {9805502 ps} 0} {{Cursor 2} {685961000 ps} 0}
quietly wave cursor active 1
configure wave -namecolwidth 404
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
WaveRestoreZoom {9738155 ps} {10268481 ps}
