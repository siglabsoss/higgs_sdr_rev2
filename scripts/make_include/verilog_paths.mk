# including file must set these
# Q_ENGINE_REPO=
# IP_LIBRARY_REPO=
# RISCV_BASEBAND_REPO=
# HIGGS_ROOT=

ifndef HIGGS_ROOT
$(error Please define HIGGS_ROOT before including scripts/make_include/verilog_paths.mk)
endif
ifndef IP_LIBRARY_REPO
$(error Please define IP_LIBRARY_REPO before including scripts/make_include/verilog_paths.mk)
endif


HIGS_TB_TOP_ONLY=\
$(HIGGS_ROOT)/sim/hdl/tb_higgs_top.sv

# use this when you want to include your own top when using higgs
HIGGS_TB_NO_TOP_VERILOG=\
$(IP_LIBRARY_REPO)/interfaces/cmd_interface/hdl/intf_cmd.sv \
$(IP_LIBRARY_REPO)/interfaces/cmd_interface/hdl/cmd_master_example.sv \
$(IP_LIBRARY_REPO)/interfaces/cmd_interface/hdl/cmd_slave_example.sv \
$(HIGGS_ROOT)/fpgas/cs/cs20/hdl/cs20_top.sv \
$(HIGGS_ROOT)/fpgas/cs/cs01/hdl/cs01_top.sv \
$(HIGGS_ROOT)/fpgas/cs/cs11/hdl/cs11_top.sv \
$(HIGGS_ROOT)/fpgas/cs/cs21/hdl/cs21_top.sv \
$(HIGGS_ROOT)/fpgas/cs/cs31/hdl/cs31_top.sv \
$(HIGGS_ROOT)/fpgas/cs/cs32/hdl/cs32_top.sv \
$(HIGGS_ROOT)/fpgas/cs/cs22/hdl/cs22_top.sv \
$(HIGGS_ROOT)/fpgas/cs/cs12/hdl/cs12_top.sv \
$(HIGGS_ROOT)/fpgas/cs/cs02/hdl/cs02_top.sv \
$(HIGGS_ROOT)/fpgas/grav/eth/hdl/eth_top.sv \
$(HIGGS_ROOT)/fpgas/grav/dac/hdl/dac_top.sv \
$(HIGGS_ROOT)/fpgas/grav/adc/hdl/adc_top.sv \
$(HIGGS_ROOT)/fpgas/grav/cfg/hdl/cfg_top.sv \
$(HIGGS_ROOT)/fpgas/packages/higgs_sdr_global_pkg.sv \
$(HIGGS_ROOT)/fpgas/common/modules/core_top.sv \
$(HIGGS_ROOT)/fpgas/common/modules/vex_machine_top.v \
$(HIGGS_ROOT)/fpgas/common/modules/vex_machine_top_gutted.v \
$(HIGGS_ROOT)/fpgas/common/modules/vex_machine_top_d_engine.v \
$(HIGGS_ROOT)/fpgas/common/modules/q_engine_gutted.v \
$(HIGGS_ROOT)/fpgas/common/modules/core_reset.sv \
$(HIGGS_ROOT)/fpgas/common/modules/cmd_cdc.sv \
$(HIGGS_ROOT)/fpgas/common/modules/eb2a/eb2a_ctrl.v \
$(HIGGS_ROOT)/fpgas/common/modules/eb2a/eb2a_data.v \
$(HIGGS_ROOT)/fpgas/common/modules/eb2a/eb2a.v \
$(HIGGS_ROOT)/fpgas/common/modules/fb_eq_join/rtl/fb_eq_join.sv \
$(HIGGS_ROOT)/fpgas/common/modules/fb_eq_split/rtl/fb_eq_split.sv \
$(HIGGS_ROOT)/fpgas/common/modules/mapper_mover/mapper_mover.sv \
$(HIGGS_ROOT)/fpgas/common/modules/mapper_mover/qam8_mapper.sv \
$(HIGGS_ROOT)/fpgas/common/modules/mapper_mover/qam16_mapper.sv \
$(HIGGS_ROOT)/fpgas/common/modules/mapper_mover/qam32_mapper.sv \
$(HIGGS_ROOT)/fpgas/common/modules/mapper_mover/qam64_mapper.sv \
$(HIGGS_ROOT)/fpgas/common/modules/mapper_mover/mapper_memory.sv \
$(HIGGS_ROOT)/fpgas/common/modules/demapper/demapper.sv \
$(HIGGS_ROOT)/fpgas/common/modules/width_convert/width_32_8.sv \
$(HIGGS_ROOT)/fpgas/common/modules/width_convert/width_8_32.sv \
$(HIGGS_ROOT)/fpgas/common/modules/off_board/half_cdc.sv \
$(HIGGS_ROOT)/fpgas/grav/eth/hdl/eth_mega_wrapper.sv \
$(HIGGS_ROOT)/fpgas/grav/eth/hdl/fifo_cdc.sv \
$(HIGGS_ROOT)/fpgas/grav/eth/hdl/mac_cfg.sv \
$(HIGGS_ROOT)/fpgas/grav/eth/hdl/eth_rx_wrapper.sv \
$(IP_LIBRARY_REPO)/lattice_support/gbit_mac/modules/arp_reply/hdl/arp_reply.sv \
$(IP_LIBRARY_REPO)/lattice_support/gbit_mac/modules/eth_frame_router/hdl/eth_frame_router.sv \
$(IP_LIBRARY_REPO)/lattice_support/gbit_mac/modules/ipv4_pkt_router/hdl/ipv4_pkt_router.sv \
$(IP_LIBRARY_REPO)/lattice_support/gbit_mac/modules/udp_pkt_router/hdl/udp_pkt_router.sv \
$(IP_LIBRARY_REPO)/lattice_support/gbit_mac/modules/mac_tx_arbiter/hdl/mac_tx_arbiter.sv \
$(IP_LIBRARY_REPO)/lattice_support/gbit_mac/modules/udp_rx_stream_buffer/hdl/udp_rx_stream_buffer.sv \
$(IP_LIBRARY_REPO)/lattice_support/gbit_mac/modules/udp_packetizer/hdl/udp_packetizer.sv \
$(IP_LIBRARY_REPO)/lattice_support/fifos/pmi_fifo_sc_fwft_v1_0/hdl/pmi_fifo_sc_fwft_v1_0.sv \
$(IP_LIBRARY_REPO)/lattice_support/fifos/pmi_fifo_dc_fwft_v1_0/hdl/pmi_fifo_dc_fwft_v1_0.sv \
$(IP_LIBRARY_REPO)/mib_bus/hdl/mib_master.sv \
$(IP_LIBRARY_REPO)/mib_bus/hdl/mib_slave.sv \
$(IP_LIBRARY_REPO)/mib_bus/hdl/mib_master_wrapper.sv \
$(IP_LIBRARY_REPO)/mib_bus/hdl/mib_slave_wrapper.sv \
$(IP_LIBRARY_REPO)/graviton_ti_cfg/hdl/graviton_ti_cfg.sv \
$(IP_LIBRARY_REPO)/graviton_ti_cfg/hdl/ti_sif.sv \
$(IP_LIBRARY_REPO)/n25q_qspi_reader/hdl/n25q_qspi_reader.sv \
$(IP_LIBRARY_REPO)/ecp5_slave_serial_programmer/hdl/ecp5_slave_serial_programmer.sv \
$(IP_LIBRARY_REPO)/upconverter/hdl/duc_fixed_dds.sv \
$(IP_LIBRARY_REPO)/upconverter/hdl/duc_hb_cascade.sv \
$(IP_LIBRARY_REPO)/upconverter/hdl/duc_hb_interp_fir_h0.sv \
$(IP_LIBRARY_REPO)/upconverter/hdl/duc_skid.sv \
$(IP_LIBRARY_REPO)/upconverter/hdl/upconverter.sv \
$(IP_LIBRARY_REPO)/downconverter/hdl/downconverter.sv \
$(IP_LIBRARY_REPO)/downconverter/hdl/fixed_ddsx2.sv \
$(IP_LIBRARY_REPO)/downconverter/hdl/ddc_hb_cascade.sv \
$(IP_LIBRARY_REPO)/downconverter/hdl/ddc_hb_decim_fir_h1.sv \
$(IP_LIBRARY_REPO)/downconverter/hdl/ddc_hb_decim_fir_h2.sv \
$(IP_LIBRARY_REPO)/downconverter/hdl/ddc_hb_decim_firx2_h0.sv \


include $(HIGGS_ROOT)/scripts/make_include/disable_tracing.mk


# use this when you want the default paths for all files in higgs
HIGGS_TB_ALL_VERILOG=\
$(DISABLE_TRACING_PATHS) \
$(HIGS_TB_TOP_ONLY) \
$(HIGGS_TB_NO_TOP_VERILOG)

HIGGS_TB_TOP=tb_higgs_top
