onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate /tb_sys/_cs12_top/val_valid
add wave -noupdate /tb_sys/_cs12_top/_validator/o_data
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {3710614 ps} 0}
quietly wave cursor active 1
configure wave -namecolwidth 339
configure wave -valuecolwidth 183
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
WaveRestoreZoom {0 ps} {20454630 ps}
