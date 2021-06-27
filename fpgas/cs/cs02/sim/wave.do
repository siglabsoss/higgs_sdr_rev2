onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate /tb_cs02_top/UUT/subcarrier_selector/i_dsp_clk
add wave -noupdate /tb_cs02_top/UUT/subcarrier_selector/i_dsp_srst
add wave -noupdate /tb_cs02_top/UUT/subcarrier_selector/i_sc_vld
add wave -noupdate /tb_cs02_top/UUT/subcarrier_selector/o_sc_rdy
add wave -noupdate /tb_cs02_top/UUT/subcarrier_selector/i_sc_inph
add wave -noupdate /tb_cs02_top/UUT/subcarrier_selector/i_sc_quad
add wave -noupdate /tb_cs02_top/UUT/subcarrier_selector/i_sc_mask
add wave -noupdate /tb_cs02_top/UUT/subcarrier_selector/i_txmac_clk
add wave -noupdate /tb_cs02_top/UUT/subcarrier_selector/i_txmac_srst
add wave -noupdate /tb_cs02_top/UUT/subcarrier_selector/o_udp_pktzr_start
add wave -noupdate /tb_cs02_top/UUT/subcarrier_selector/i_udp_pktzr_start_ack
add wave -noupdate /tb_cs02_top/UUT/subcarrier_selector/i_udp_pktzr_byte_rd
add wave -noupdate /tb_cs02_top/UUT/subcarrier_selector/o_udp_pktzr_byte_cnt
add wave -noupdate /tb_cs02_top/UUT/subcarrier_selector/o_udp_pktzr_seq_num
add wave -noupdate /tb_cs02_top/UUT/subcarrier_selector/o_udp_pktzr_meta_data
add wave -noupdate /tb_cs02_top/UUT/subcarrier_selector/o_udp_pktzr_byte_vld
add wave -noupdate /tb_cs02_top/UUT/subcarrier_selector/o_udp_pktzr_byte
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {88128595 ps} 0}
quietly wave cursor active 1
configure wave -namecolwidth 289
configure wave -valuecolwidth 100
configure wave -justifyvalue left
configure wave -signalnamewidth 1
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
WaveRestoreZoom {0 ps} {333506250 ps}
