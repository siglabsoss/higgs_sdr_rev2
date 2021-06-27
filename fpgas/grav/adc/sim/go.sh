#!/bin/sh

# Create the Work Library
vlib work

# Compile the SystemVerilog Packages
vlog -work work -gSIMULATION -sv2k12 -dbg ../hdl/pkg_adc_regmap.sv

# Compile the SystemVerilog Interfaces
vlog -work work -sv2k12 -dbg ../../../libs/ip-library/interfaces/cmd_interface/hdl/intf_cmd.sv

# Compile the local SystemVerilog Modules
vlog -work work -sv2k12 -dbg tb_adc_dsp.sv
vlog -work work -sv2k12 -dbg ../hdl/adc_dsp.sv

# Compile the IP Library SystemVerilog Modules
cp ../../../libs/ip-library/downconverter/hdl/ddc_sincos.mif ./ddc_sincos.mif
vlog -work work -sv2k12 -dbg ../../../libs/ip-library/downconverter/hdl/downconverter.sv
vlog -work work -sv2k12 -dbg ../../../libs/ip-library/downconverter/hdl/ddc_ddsx2.sv
vlog -work work -sv2k12 -dbg ../../../libs/ip-library/downconverter/hdl/ddc_hb_cascade.sv
vlog -work work -sv2k12 -dbg ../../../libs/ip-library/downconverter/hdl/ddc_hb_decim_fir_h1.sv
vlog -work work -sv2k12 -dbg ../../../libs/ip-library/downconverter/hdl/ddc_hb_decim_fir_h2.sv
vlog -work work -sv2k12 -dbg ../../../libs/ip-library/downconverter/hdl/ddc_hb_decim_firx2_h0.sv

vlog -work work -sv2k12 -dbg ../../../libs/ip-library/cic_decimator/hdl/cic_decimator.sv
vlog -work work -sv2k12 -dbg ../../../libs/ip-library/cic_decimator/hdl/cic_decim_comb.sv
vlog -work work -sv2k12 -dbg ../../../libs/ip-library/cic_decimator/hdl/cic_decim_compfir.sv
vlog -work work -sv2k12 -dbg ../../../libs/ip-library/cic_decimator/hdl/cic_decim_integrator.sv
vlog -work work -sv2k12 -dbg ../../../libs/ip-library/cic_decimator/hdl/cic_decim_stages.sv

vlog -work work -sv2k12 -dbg ../../../libs/ip-library/rx_channel_modulator/hdl/rx_channel_modulator.sv
vlog -work work -sv2k12 -dbg ../../../libs/ip-library/rx_channel_modulator/hdl/rx_chmod_dds.sv

# Execute the simulation
vsimsa -do dosim.do
