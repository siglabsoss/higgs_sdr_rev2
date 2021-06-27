set macro_search_path "./tmp;" ; # every diamond_sources.tcl script should append the paths to any IP/pre-compiled .ngo files they need
set syn_cmd_line_args "set_option -include_path ./tmp;"; # every diamond_sources.tcl script should append the paths to any Memory initialization files they need

#
# TOP LEVEL PROJECT LOCAL SOURCES & IP
#

prj_src add ../hdl/cs21_top.sv

prj_impl option {include path} ../../../packages

#
# EXTERNAL MODULES & IP
#

# COMMON FPGA MODULES AND IP

set com_ip_path "../../../common/ip";
set com_mod_path "../../../common/modules";

prj_src add $com_ip_path/lattice/sys_pll/sys_pll.v
prj_src add $com_mod_path/core_top.sv
prj_src add $com_mod_path/core_reset.sv
prj_src add $com_mod_path/mib_cdc.sv
prj_src add $com_mod_path/piper.sv
#prj_src add $com_mod_path/eb1.5.sv
# prj_src add $com_mod_path/vex_machine_top.v
prj_src add $com_mod_path/vex_machine_top_d_engine.v



# IP LIBRARY MODULES AND IP

set ip_lib_path "../../../../libs/ip-library";

set q_engine_path "../../../../libs/q-engine";
set d_engine_path "../../../../libs/d-engine";
set riscv_baseband_path "../../../../libs/riscv-baseband";
set datapath_path "../../../../libs/datapath";

source $ip_lib_path/mib_bus/diamond_sources.tcl;                                               # mib_master
source $ip_lib_path/interfaces/cmd_interface/diamond_sources.tcl;                              # command bus interface definition
source $ip_lib_path/lattice_support/fifos/pmi_fifo_sc_fwft_v1_0/diamond_sources.tcl;           # pmi_fifo_sc_fwft
source $ip_lib_path/lattice_support/fifos/pmi_fifo_dc_fwft_v1_0/diamond_sources.tcl;           # pmi_fifo_dc_fwft
prj_src add $ip_lib_path/clock_shift/hdl/clock_shift.sv;                                       # clock shift module for mib clock deskew
source $ip_lib_path/fwft_fifos/sc_fifo/diamond_sources.tcl; 				       # fwft_fifo

source $q_engine_path/diamond_sources.tcl; 				       			# q-engine
source $d_engine_path/diamond_sources.tcl; 				       			# d-engine

source $riscv_baseband_path/diamond_sources.tcl; 				       		# riscv_baseband_path

source $datapath_path/diamond_sources.tcl; 				       		# datapath_path

prj_impl option -append {include path} $ip_lib_path/lattice_support/gbit_mac/packages

prj_strgy set_value "bd_macro_search_path=$macro_search_path"; # needed in order for Translate to find IP core ngo files (since we remove all sources and only add back the edif file in build.tcl)
prj_strgy set_value "syn_cmdline_args=$syn_cmd_line_args";     # primarily needed in order for Synplify to find memory initialization files 
