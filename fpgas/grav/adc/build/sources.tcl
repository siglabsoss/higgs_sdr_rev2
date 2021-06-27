set macro_search_path "./tmp;" ; # every diamond_sources.tcl script should append the paths to any IP/pre-compiled .ngo files they need
set syn_cmd_line_args "set_option -include_path ./tmp;"; # every diamond_sources.tcl script should append the paths to any Memory initialization files they need

#
# TOP LEVEL PROJECT LOCAL SOURCES & IP
#

prj_src add ../hdl/adc_top.sv
prj_src add ../hdl/pkg_adc_regmap.sv
prj_src add ../hdl/adc_dsp.sv
prj_src add ../hdl/adc_data_rx.sv
prj_src add ../hdl/vga_ctrl.sv
prj_src add ../hdl/regs.sv


#
# EXTERNAL MODULES & IP
#

# COMMON FPGA MODULES AND IP

set com_ip_path "../../../common/ip";
set com_mod_path "../../../common/modules";

prj_src add $com_ip_path/lattice/sys_pll/sys_pll.v
prj_src add $com_mod_path/core_reset.sv
prj_src add $com_mod_path/core_top.sv
prj_src add $com_mod_path/mib_cdc.sv


# IP LIBRARY MODULES AND IP

set ip_lib_path "../../../../libs/ip-library";

source $ip_lib_path/mib_bus/diamond_sources.tcl;                                               # mib_slave
source $ip_lib_path/interfaces/cmd_interface/diamond_sources.tcl;                              # command bus interface definition
source $ip_lib_path/lattice_support/fifos/pmi_fifo_sc_fwft_v1_0/diamond_sources.tcl;           # pmi_fifo_sc_fwft
source $ip_lib_path/lattice_support/fifos/pmi_fifo_dc_fwft_v1_0/diamond_sources.tcl;           # pmi_fifo_dc_fwft
source $ip_lib_path/clock_shift/diamond_sources.tcl;                                           # clock shift module for mib clock deskew
source $ip_lib_path/downconverter/diamond_sources.tcl;                                         # downconverter
source $ip_lib_path/rx_channel_modulator/diamond_sources.tcl;                                  # rx channel modulator
source $ip_lib_path/cic_decimator/diamond_sources.tcl;                                         # cic decimator

prj_impl option {include path} $ip_lib_path/lattice_support/gbit_mac/packages

prj_strgy set_value "bd_macro_search_path=$macro_search_path"; # needed in order for Translate to find IP core ngo files (since we remove all sources and only add back the edif file in build.tcl)
prj_strgy set_value "syn_cmdline_args=$syn_cmd_line_args";     # primarily needed in order for Synplify to find memory initialization files 
