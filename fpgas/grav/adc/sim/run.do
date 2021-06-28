#!/bin/sh

# Create the Work Library
vlib work

# Compile the SystemVerilog Packages
vlog -work work -sv ../hdl/pkg_adc_regmap.sv

# Compile the SystemVerilog Interfaces
vlog -work work -sv ../../../libs/ip-library/interfaces/cmd_interface/hdl/intf_cmd.sv

# Compile the local SystemVerilog Modules
vlog -work work -sv tb_adc_dsp.sv
vlog -work work -sv ../hdl/adc_dsp.sv

# Compile the IP Library SystemVerilog Modules
file copy -force ../../../libs/ip-library/downconverter/hdl/ddc_sincos.mif ./ddc_sincos.mif
vlog -work work -sv ../../../libs/ip-library/downconverter/hdl/downconverter.sv
vlog -work work -sv ../../../libs/ip-library/downconverter/hdl/ddc_ddsx2.sv
vlog -work work -sv ../../../libs/ip-library/downconverter/hdl/ddc_hb_cascade.sv
vlog -work work -sv ../../../libs/ip-library/downconverter/hdl/ddc_hb_decim_fir_h1.sv
vlog -work work -sv ../../../libs/ip-library/downconverter/hdl/ddc_hb_decim_fir_h2.sv
vlog -work work -sv ../../../libs/ip-library/downconverter/hdl/ddc_hb_decim_firx2_h0.sv

vlog -work work -sv ../../../libs/ip-library/cic_decimator/hdl/cic_decimator.sv
vlog -work work -sv ../../../libs/ip-library/cic_decimator/hdl/cic_decim_comb.sv
vlog -work work -sv ../../../libs/ip-library/cic_decimator/hdl/cic_decim_compfir.sv
vlog -work work -sv ../../../libs/ip-library/cic_decimator/hdl/cic_decim_integrator.sv
vlog -work work -sv ../../../libs/ip-library/cic_decimator/hdl/cic_decim_stages.sv

vlog -work work -sv ../../../libs/ip-library/rx_channel_modulator/hdl/rx_channel_modulator.sv
vlog -work work -sv ../../../libs/ip-library/rx_channel_modulator/hdl/rx_chmod_dds.sv

# Execute the simulation
vsim -t 1ps -L pmi_work -L ecp5u_vlg -voptargs=+acc tb_adc_dsp

log -r /*

do wave.do

run -all
