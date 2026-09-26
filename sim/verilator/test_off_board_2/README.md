# Purpose

Trying to put eq only through mapmov in effort to solve half_clock issues.

Trying to verilate a pmi_fifo_dc_fwft_v1_0

May need hand edited version of cs20 options, but this version tests pmi_fifo_dc_fwft_v1_0(DELAYED_BACK_PRESSURE)



The idea is to modify in_buffer on vex_machine_top.sv from:
  .wren         (temp_valid && temp_ready_delay),
to
  .wren         (temp_valid),

but we need valid to not chill at high when rden is low.