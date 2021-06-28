onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate /tb_cs01_top/UUT/rx_turnstile/rx_scheduling_counters_inst/FRAME_SIZE
add wave -noupdate /tb_cs01_top/UUT/rx_turnstile/rx_scheduling_counters_inst/FC_SIZE
add wave -noupdate /tb_cs01_top/UUT/rx_turnstile/rx_scheduling_counters_inst/SC_SIZE
add wave -noupdate /tb_cs01_top/UUT/rx_turnstile/rx_scheduling_counters_inst/i_valid
add wave -noupdate /tb_cs01_top/UUT/rx_turnstile/rx_scheduling_counters_inst/o_aligned_sample_counter
add wave -noupdate /tb_cs01_top/UUT/rx_turnstile/rx_scheduling_counters_inst/o_aligned_frame_counter
add wave -noupdate /tb_cs01_top/UUT/rx_turnstile/rx_scheduling_counters_inst/o_unaligned_sample_counter
add wave -noupdate /tb_cs01_top/UUT/rx_turnstile/rx_scheduling_counters_inst/o_unaligned_frame_counter
add wave -noupdate /tb_cs01_top/UUT/rx_turnstile/rx_scheduling_counters_inst/i_target
add wave -noupdate /tb_cs01_top/UUT/rx_turnstile/rx_scheduling_counters_inst/i_target_valid
add wave -noupdate /tb_cs01_top/UUT/rx_turnstile/rx_scheduling_counters_inst/i_clock
add wave -noupdate /tb_cs01_top/UUT/rx_turnstile/rx_scheduling_counters_inst/i_reset
add wave -noupdate -radix unsigned /tb_cs01_top/UUT/rx_turnstile/rx_scheduling_counters_inst/target_reg0
add wave -noupdate -radix unsigned /tb_cs01_top/UUT/rx_turnstile/rx_scheduling_counters_inst/target_reg1
add wave -noupdate /tb_cs01_top/UUT/rx_turnstile/rx_scheduling_counters_inst/target_clear
add wave -noupdate /tb_cs01_top/UUT/rx_turnstile/rx_scheduling_counters_inst/hit_the_target
add wave -noupdate /tb_cs01_top/UUT/rx_turnstile/rx_scheduling_counters_inst/half_period_passed
add wave -noupdate /tb_cs01_top/UUT/rx_turnstile/rx_scheduling_counters_inst/aligned_sample_counter
add wave -noupdate /tb_cs01_top/UUT/rx_turnstile/rx_scheduling_counters_inst/aligned_frame_counter
add wave -noupdate /tb_cs01_top/UUT/rx_turnstile/rx_scheduling_counters_inst/unaligned_sample_counter
add wave -noupdate /tb_cs01_top/UUT/rx_turnstile/rx_scheduling_counters_inst/unaligned_frame_counter
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {79920179 ps} 0} {{Cursor 2} {4508831882 ps} 0}
quietly wave cursor active 2
configure wave -namecolwidth 477
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
WaveRestoreZoom {4508831502 ps} {4508833079 ps}
