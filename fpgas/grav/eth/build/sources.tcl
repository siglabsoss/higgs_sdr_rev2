set macro_search_path "./tmp;" ; # every diamond_sources.tcl script should append the paths to any IP/pre-compiled .ngo files they need
set syn_cmd_line_args "set_option -include_path ./tmp;"; # every diamond_sources.tcl script should append the paths to any Memory initialization files they need

#
# TOP LEVEL PROJECT LOCAL SOURCES & IP
#

prj_src add ../hdl/DDR.v
prj_src add ../hdl/eth_rx_wrapper.sv
prj_src add ../hdl/eth_mac_wrapper.sv
prj_src add ../hdl/eth_mega_wrapper.sv
prj_src add ../hdl/eth_dsp_counters.sv
prj_src add ../hdl/mdio_setconfig.sv
prj_src add ../hdl/eth_top.sv
prj_src add ../hdl/mac_cfg.sv
prj_src add ../hdl/cs_udp_cmd_rx_tx.sv
prj_src add ../hdl/rgmii2gmii.v
prj_src add ../hdl/rf_rx_eth_frame_rx.sv
prj_src add ../hdl/regs.sv
prj_src add ../hdl/fifo_cdc.sv

prj_src add ../ip/lattice/ddr_input/ddr_input.v
prj_src add ../ip/lattice/eth_mac/eth_mac.v
prj_src add ../ip/lattice/eth_mac/gbit_mac/gbit_mac_bb.v

append macro_search_path "../ip/lattice/eth_mac/gbit_mac;" ; # add directory of pre-compiled ethernet mac IP .ngo to macro search path

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
prj_src add $com_mod_path/cmd_cdc.sv
#prj_src add $com_mod_path/eb1.5.sv
prj_src add $com_mod_path/fb_eq_split/rtl/fb_eq_split.sv
prj_src add $com_mod_path/fb_eq_join/rtl/fb_eq_join.sv
prj_src add $com_mod_path/mapper_mover/mapper_mover.sv
prj_src add $com_mod_path/mapper_mover/qam8_mapper.sv
prj_src add $com_mod_path/mapper_mover/qam16_mapper.sv
prj_src add $com_mod_path/mapper_mover/qam32_mapper.sv
prj_src add $com_mod_path/mapper_mover/qam64_mapper.sv
prj_src add $com_mod_path/mapper_mover/mapper_memory.sv
source $com_mod_path/eb2a/diamond_sources.tcl; #eb2a module


# IP LIBRARY MODULES AND IP

set ip_lib_path "../../../../libs/ip-library";
set q_engine_path "../../../../libs/q-engine";
set riscv_baseband_path "../../../../libs/riscv-baseband";
set datapath_path "../../../../libs/datapath";

# Including memories

# file copy -force $riscv_baseband_path/scalar0.mif ./tmp/scalar0.mif
# file copy -force $riscv_baseband_path/scalar1.mif ./tmp/scalar1.mif
# file copy -force $riscv_baseband_path/scalar2.mif ./tmp/scalar2.mif
# file copy -force $riscv_baseband_path/scalar3.mif ./tmp/scalar3.mif

#source $com_mod_path/eb2a/diamond_sources.tcl
source $ip_lib_path/mib_bus/diamond_sources.tcl;                                               # mib_master
source $ip_lib_path/interfaces/cmd_interface/diamond_sources.tcl;                              # command bus interface definition
source $ip_lib_path/lattice_support/fifos/pmi_fifo_dc_fwft_v1_0/diamond_sources.tcl;           # pmi_fifo_dc_fwft
source $ip_lib_path/lattice_support/fifos/pmi_fifo_sc_fwft_v1_0/diamond_sources.tcl;           # pmi_fifo_sc_fwft
source $ip_lib_path/lattice_support/gbit_mac/modules/eth_frame_router/diamond_sources.tcl;     # eth_frame_router
source $ip_lib_path/lattice_support/gbit_mac/modules/arp_reply/diamond_sources.tcl;            # arp_reply
source $ip_lib_path/lattice_support/gbit_mac/modules/ipv4_pkt_router/diamond_sources.tcl;      # ipv4_pkt_router
source $ip_lib_path/lattice_support/gbit_mac/modules/mac_tx_arbiter/diamond_sources.tcl;       # mac_tx_arbiter
source $ip_lib_path/lattice_support/gbit_mac/modules/udp_cmd/diamond_sources.tcl;              # udp_cmd
source $ip_lib_path/lattice_support/gbit_mac/modules/udp_packetizer/diamond_sources.tcl;       # udp_packetizer
source $ip_lib_path/lattice_support/gbit_mac/modules/udp_pkt_router/diamond_sources.tcl;       # udp_pkt_router
source $ip_lib_path/lattice_support/gbit_mac/modules/udp_rx_stream_buffer/diamond_sources.tcl; # udp_rx_stream_buffer

source $ip_lib_path/fwft_fifos/sc_fifo/diamond_sources.tcl; 				       # fwft_fifo

source $q_engine_path/diamond_sources.tcl; 				       			# q-engine

source $riscv_baseband_path/diamond_sources_for_eth.tcl; 				       		# riscv_baseband_path
#source ../../../libs/ip-library/ddr3_fifo/diamond_sources.tcl;      # ddr FIFO

source $datapath_path/diamond_sources.tcl; 		     # datapath_path

prj_impl option -append {include path} $ip_lib_path/lattice_support/gbit_mac/packages

prj_strgy set_value "bd_macro_search_path=$macro_search_path"; # needed in order for Translate to find IP core ngo files (since we remove all sources and only add back the edif file in build.tcl)
prj_strgy set_value "syn_cmdline_args=$syn_cmd_line_args";     # primarily needed in order for Synplify to find memory initialization files 
