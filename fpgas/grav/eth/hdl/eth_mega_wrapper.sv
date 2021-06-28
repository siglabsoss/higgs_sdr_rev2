/*
 * Module: eth_mega_wrapper
 * 
 * Big ass wrapper encapsulating all Ethernet related functionality.
 * 
 */


`include "ethernet_support_pkg.sv"
`include "udp_cmd_pkg.sv"
 
`default_nettype none

module eth_mega_wrapper #(
/* verilator lint_off LITENDIAN */
        
    /* ETH MAC WRAPPER PARAMS */
    parameter bit [47:0]                          LOCAL_MAC_ADDR            = 48'h000000000000,
    parameter int unsigned                        CFG_CLK_HZ                = 10_000_000,
    
    /* ETH RX WRAPPER PARAMS */
    parameter int unsigned                        CBUF_MAX_ETH_FRAME_DEPTH  = 8, // Number of maximum sized Ethernet frames that can be simultaneously stored in the circular buffer
    parameter int unsigned                        IPV4_PKT_FIFO_PKT_DEPTH   = 5, // Number of maximum sized IPv4 packets that can be simultaneously stored in the output FIFO
    parameter int unsigned                        ARP_PKT_FIFO_PKT_DEPTH    = 5, // Number of maximum sized ARP packets that can be simultaneously stored in the output FIFO
        
    /* Versha Capture UDP PACKETIZER PARAMS */
    parameter bit [15:0]                          UDP_VC_TX_PORT           = 16'd40001,  // 
    parameter int unsigned                        UDP_VC_ETH_FRAME_BUFFERS = 4,    
    
    /* VD UDP RX STREAM PARAMS */
    parameter bit [15:0]                          UDP_VD_RX_PORT            = 16'd30000, // FPGA listens for ADC data from host on this port
    
    /* RingBus (RB) UDP RX STREAM PARAMS */
    parameter bit [15:0]                          UDP_RB_RX_PORT            = 16'd20000, // FPGA listens for ADC data from host on this port

    /* Ring Bus UDP PACKETIZER PARAMS */
    parameter bit [15:0]                          UDP_RB_TX_PORT           = 16'd10001,  // FPGA sends captured data to this port on the host (i.e. host should listen on this port)
    parameter int unsigned                        UDP_RB_ETH_FRAME_BUFFERS = 4,    
    
    
    parameter int unsigned                        UDP_CMD_ACK_TIMEOUT_CLKS   = 64,
    
    /* ETH TX ARBITER PARAMS */
    parameter int unsigned                        TX_ARB_ETH_FRAME_BUFFERS  = 5,          // Number of Max Size Ethernet II Frames that can be simultaneously buffered
    
    /* MISC PARAMS */
    parameter bit [31:0]                          HOST_IP_ADDR              = {8'd10, 8'd0, 8'd0, 8'd1},
    parameter bit [31:0]                          LOCAL_IP_ADDR             = {8'd10, 8'd0, 8'd0, 8'd2},
    parameter bit                                 SIM_MODE                  = 0
    
)(

// these are here to avoid needing to simulate the Lattice Gbit MAC IP in our eth_top simulation
`ifdef VERILATE_DEF
    input         i_mac_rx_write,
    input         i_mac_rx_eof,
    input  [ 7:0] i_mac_rx_fifodata,
    output [ 7:0] o_mac_tx_fifodata,
    output        o_mac_tx_fifoavail,
    output        o_mac_tx_fifoeof,
    input         i_mac_tx_macread,
`endif
        
    /* ETH MAC WRAPPER PORTS */
    
    input         i_cfg_clk,
    input         i_cfg_srst, // reset, synchronous to i_cfg_clk
    output        o_gbit_mode,
    output        o_cfg_done,

    output        o_rx_clk125,
    input         i_rx_clk125_srst, // used by other modules too
    input         i_tx_clk125,      // has to be 125MHz since we only support Gigabit mode
    input         i_tx_clk125_srst, // used by other module too

`ifndef SIM_MODE
    output        o_phy_rst_n,
    input         i_rgmii_rxclk,    // don't use this clock for any logic other than GMII <---> RGMII.  use rx_clk125 instead.
    input         i_rgmii_rxctrl,
    input  [ 3:0] i_rgmii_rxd,
    output        o_rgmii_txclk,
    output        o_rgmii_txctrl,
    output [ 3:0] o_rgmii_txd,
`endif
    
    /* DAC UDP STREAM PORTS */
    input         i_dac_buffer_rd,
    output        o_dac_buffer_data_vld,
    output        o_dac_buffer_data_parity,
    output [31:0] o_dac_buffer_data,
    
    /* GRAV CMD UDP PACKETIZER PORTS */
    input         i_cmd_clk,
    input         i_cmd_srst, // synchronous to i_cmd_clk
    intf_cmd      cmd[2**MIB_SEL_BITS],
    
    /* VC UDP PACKETIZER PORTS */
    input         i_vc_pktzr_start,
    output        o_vc_pktzr_start_ack,
    output        o_vc_pktzr_done,
    input         i_vc_pktzr_data_byte_vld,
    input  [ 7:0] i_vc_pktzr_data_byte,
    output        o_vc_pktzr_data_byte_rd,

    
    /* VershaDump UDP STREAM PORTS */
    
    input 		  i_vd_buffer_rd,
    output 		  o_vd_buffer_data_vld,
    output [31:0] o_vd_buffer_data,
    
    /* RingBus UDP STREAM PORTS */
    
    input 		  i_rb_buffer_rd,
    output 		  o_rb_buffer_data_vld,
    output [31:0] o_rb_buffer_data,
    
    /* RingBus UDP PACKETIZER PORTS */
    input         i_rb_pktzr_start,
    output        o_rb_pktzr_start_ack,
    output        o_rb_pktzr_done,
    input         i_rb_pktzr_data_byte_vld,
    input  [ 7:0] i_rb_pktzr_data_byte,
    output        o_rb_pktzr_data_byte_rd,
    
    
    /* STATUS & ERROR REPORTING (Synchronous to i_cmd_clk) */
    output [47:0] o_host_mac, // software needs to update UDP packetizer modules in other FPGAs (e.g. on Copper Suicide) with this once learned so that they can correctly make Ethernet Frames
    output [31:0] o_mac_rx_err_cntr,
    output [31:0] o_mac_rx_crc_err_cntr,
    output [31:0] o_mac_rx_len_chk_err_cntr,
    output [31:0] o_mac_rx_pkt_rx_ok_cntr,
    output [31:0] o_unsupported_eth_type_error_cntr,
    output [31:0] o_unsupported_ipv4_protocol_cntr,
    output [31:0] o_unsupported_dest_port_cntr,

    /* Overflow / Underflow counters for rx packets */
    output [31:0] o_cs20_data_buf_overflow_cntr,

    output [31:0] o_rb_in_buf_overflow_cntr,

    /* Overflow counters for tx packets */
    output        o_cs30_data_buf_afull,
    output [31:0] o_cs30_data_buf_overflow_cntr,
    output [31:0] o_rb_out_buf_overflow_cntr,
    // output [31:0] o_dac_data_udp_seq_num_error_cntr

    // if any of these errors occur you'll need to reset this module to recover and clear the error
    output             eth_frame_circ_buf_overflow,    // FATAL
    output             ipv4_pkt_fifo_overflow,         // FATAL
    output             arp_pkt_fifo_overflow,          // FATAL
    output             udp_pkt_fifo_overflow,          // FATAL
    output             icmp_pkt_fifo_overflow,         // FATAL
    output [0:3]       udp_port_fifo_overflow          // FATAL

    // output [7:0]       snap_cbuf_rd_fsm_state,
    // output [7:0]       snap_cbuf_wr_fsm_state

    // output reg [0:3] rr_arb_grant_mask,
    // output reg [0:3] rr_arb_grant,
    // output reg [0:3] rr_arb_req_raw,
    // output reg [0:3] rr_arb_req_masked
);
    
    /* ETH MAC WRAPPER */

    logic        rx_clk125;
    logic        mac_rx_write;
    logic        mac_rx_eof;
    logic [ 7:0] mac_rx_fifodata;
    logic        mac_rx_error;
    logic        mac_rx_stat_en;
    logic [31:0] mac_rx_stat_vector;
    logic [ 7:0] mac_tx_fifodata;
    logic        mac_tx_fifoavail;
    logic        mac_tx_fifoeof;
    logic        mac_tx_fifoempty;
    logic        mac_tx_macread;    
    logic        mac_tx_done;
    logic        mac_tx_staten;  // currently unused
    logic [30:0] mac_tx_statvec; // currently unused

    logic [31:0] mac_rx_err_cntr         /* synthesis syn_noprune=1 */;
    logic [31:0] mac_rx_crc_err_cntr     /* synthesis syn_noprune=1 */;
    logic [31:0] mac_rx_len_chk_err_cntr /* synthesis syn_noprune=1 */;
    logic [31:0] mac_rx_pkt_rx_ok_cntr   /* synthesis syn_noprune=1 */;
    
    assign o_rx_clk125 = rx_clk125;

    // Ethernet Frame Rx stats (mainly for observation in Reveal Debugger)
//    always_ff @(posedge rx_clk125 or posedge i_cfg_srst) begin
    always_ff @(posedge rx_clk125) begin
        if (i_rx_clk125_srst) begin
            mac_rx_err_cntr         <= '0;
            mac_rx_crc_err_cntr     <= '0;
            mac_rx_len_chk_err_cntr <= '0;
            mac_rx_pkt_rx_ok_cntr   <= '0;
        end else begin
            mac_rx_err_cntr <= (mac_rx_eof & mac_rx_error) ? mac_rx_err_cntr + 1 : mac_rx_err_cntr;
            if (mac_rx_stat_en) begin
                mac_rx_crc_err_cntr     <= (mac_rx_stat_vector[25]) ? mac_rx_crc_err_cntr + 1     : mac_rx_crc_err_cntr;
                mac_rx_len_chk_err_cntr <= (mac_rx_stat_vector[24]) ? mac_rx_len_chk_err_cntr + 1 : mac_rx_len_chk_err_cntr;
                mac_rx_pkt_rx_ok_cntr   <= (mac_rx_stat_vector[23]) ? mac_rx_pkt_rx_ok_cntr + 1   : mac_rx_pkt_rx_ok_cntr;
            end
        end
    end
    
    fifo_cdc #(.SIM_MODE(SIM_MODE)) MAC_RX_ERR_CDC     (.i_wclk(rx_clk125), .i_wclk_rst(i_rx_clk125_srst), .i_wdata_vld(1'b1), .i_wdata(mac_rx_err_cntr),         .i_rclk(i_cmd_clk), .i_rclk_rst(i_cmd_srst), .o_rdata(o_mac_rx_err_cntr));
    fifo_cdc #(.SIM_MODE(SIM_MODE)) MAC_RX_CRC_ERR_CDC (.i_wclk(rx_clk125), .i_wclk_rst(i_rx_clk125_srst), .i_wdata_vld(1'b1), .i_wdata(mac_rx_crc_err_cntr),     .i_rclk(i_cmd_clk), .i_rclk_rst(i_cmd_srst), .o_rdata(o_mac_rx_crc_err_cntr));
    fifo_cdc #(.SIM_MODE(SIM_MODE)) MAC_RX_LEN_ERR_CDC (.i_wclk(rx_clk125), .i_wclk_rst(i_rx_clk125_srst), .i_wdata_vld(1'b1), .i_wdata(mac_rx_len_chk_err_cntr), .i_rclk(i_cmd_clk), .i_rclk_rst(i_cmd_srst), .o_rdata(o_mac_rx_len_chk_err_cntr));
    fifo_cdc #(.SIM_MODE(SIM_MODE)) MAC_RX_OK_CDC      (.i_wclk(rx_clk125), .i_wclk_rst(i_rx_clk125_srst), .i_wdata_vld(1'b1), .i_wdata(mac_rx_pkt_rx_ok_cntr),   .i_rclk(i_cmd_clk), .i_rclk_rst(i_cmd_srst), .o_rdata(o_mac_rx_pkt_rx_ok_cntr));
    
    // this stuff is just to avoid simulating the Lattice Gbit MAC
`ifndef VERILATE_DEF
    eth_mac_wrapper #(
        .P_MAC_ADDR        (LOCAL_MAC_ADDR     ), 
        .P_CFG_CLK_HZ      (CFG_CLK_HZ         )
        ) ETH_MAC_WRAPPER (
        .i_cfg_clk         (i_cfg_clk          ), 
        .i_cfg_srst        (i_cfg_srst         ), 
        .o_gbit_mode       (o_gbit_mode        ), 
        .o_cfg_done        (o_cfg_done         ), 
        .o_phy_rst_n       (o_phy_rst_n        ), 
        .o_rx_clk125       (rx_clk125          ), 
        .o_rx_write        (mac_rx_write       ), 
        .o_rx_eof          (mac_rx_eof         ), 
        .o_rx_error        (                   ), 
        .o_rx_fifodata     (mac_rx_fifodata    ), 
        .o_rx_stat_en      (mac_rx_stat_en     ), 
        .o_rx_stat_vector  (mac_rx_stat_vector ), 
        .i_tx_clk125       (i_tx_clk125        ), 
        .i_tx_fifodata     (mac_tx_fifodata    ), 
        .i_tx_fifoavail    (mac_tx_fifoavail   ), 
        .i_tx_fifoeof      (mac_tx_fifoeof     ), 
        .i_tx_fifoempty    (mac_tx_fifoempty   ), 
        .o_tx_macread      (mac_tx_macread     ), 
        .o_tx_done         (mac_tx_done        ), 
        .o_tx_staten       (mac_tx_staten      ), 
        .o_tx_statvec      (mac_tx_statvec     ), 
        .i_tx_sndpausreq   (0                  ), 
        .i_tx_sndpaustim   (0                  ), 
        .i_rgmii_rxclk     (i_rgmii_rxclk      ), 
        .i_rgmii_rxctrl    (i_rgmii_rxctrl     ), 
        .i_rgmii_rxd       (i_rgmii_rxd        ), 
        .o_rgmii_txclk     (o_rgmii_txclk      ), 
        .o_rgmii_txctrl    (o_rgmii_txctrl     ), 
        .o_rgmii_txd       (o_rgmii_txd        ));
`else
    assign o_gbit_mode        = 1;
    assign o_rx_clk125        = i_tx_clk125;
    assign mac_rx_write       = i_mac_rx_write;
    assign mac_rx_fifodata    = i_mac_rx_fifodata;
    assign mac_rx_eof         = i_mac_rx_eof;
    assign o_mac_tx_fifoavail = mac_tx_fifoavail;
    assign o_mac_tx_fifoeof   = mac_tx_fifoeof;
    assign o_mac_tx_fifodata  = mac_tx_fifodata;
    assign mac_tx_macread     = i_mac_tx_macread;
    assign mac_tx_macread     = mac_tx_fifoavail;
    assign rx_clk125          = i_tx_clk125;
`endif
    
    
    /* ETH RX WRAPPER */


    /* 
     * NOTE: These localparams work together to define what udp ports to listen to and which ports correspond to which outputs of the udp_pkt_router module
     * 
     * BE CAREFUL CHANGING THESE! 
     * if you change NUM_RX_PORTS, also change udp_port_fifo_overflow
     */
    localparam int unsigned                      UDP_NUM_RX_PORTS         = 4; // we currently have grav udp command, cs udp command, and udp ADC data
    localparam bit [0:UDP_NUM_RX_PORTS-1] [15:0] UDP_RX_PORTS             = {UDP_VD_RX_PORT, UDP_RB_RX_PORT}; // => UDP Commands come out of udp_pkt_router port 0, DAC data out of port 1
//    localparam int unsigned                      PORT_FIFO_INDEX_GRAV_CMD = 0;
//    localparam int unsigned                      PORT_FIFO_INDEX_DAC      = 1;

`ifndef VERILATE_DEF
    localparam int unsigned                      PORT_FIFO_INDEX_VD  	  = 2;
    localparam int unsigned                      PORT_FIFO_INDEX_RB  	  = 3;
`else
    localparam int unsigned                      PORT_FIFO_INDEX_VD       = 1;
    localparam int unsigned                      PORT_FIFO_INDEX_RB       = 0;
`endif

    logic [7:0] arp_pkt_byte;
    logic       arp_pkt_byte_vld;
    logic       arp_pkt_last_byte;
    logic       arp_pkt_byte_rd;
    
    logic [0:UDP_NUM_RX_PORTS-1]       udp_rx_port_last_byte;
    logic [0:UDP_NUM_RX_PORTS-1]       udp_rx_port_byte_vld;
    logic [0:UDP_NUM_RX_PORTS-1] [7:0] udp_rx_port_byte;
    logic [0:UDP_NUM_RX_PORTS-1]       udp_rx_port_byte_rd;

    // these error status signals get asserted for one clock if their respective error occurs.  you need external logic to keep track of these errors if desired.
    logic                              eth_rx_frame_long_frame_error;
    logic                              eth_rx_frame_short_frame_error;
    logic                              eth_rx_frame_ipg_error;
    logic                              eth_rx_frame_crc_error;
    logic                              eth_rx_frame_vlan_error;
    logic                              unsupported_eth_type_error;
    logic                              unsupported_ipv4_protocol;
    logic                              unsupported_dest_port;
    logic [31:0]                       unsupported_eth_type_error_cntr;
    logic [31:0]                       unsupported_ipv4_protocol_cntr;
    logic [31:0]                       unsupported_dest_port_cntr;
    
    always_ff @(posedge rx_clk125) begin
        if (i_rx_clk125_srst) begin
            unsupported_eth_type_error_cntr <= '0;
            unsupported_ipv4_protocol_cntr  <= '0;
            unsupported_dest_port_cntr      <= '0;
        end else begin
            unsupported_eth_type_error_cntr <= (unsupported_eth_type_error) ? unsupported_eth_type_error_cntr + 1 : unsupported_eth_type_error_cntr;
            unsupported_ipv4_protocol_cntr  <= (unsupported_ipv4_protocol)  ? unsupported_ipv4_protocol_cntr  + 1 : unsupported_ipv4_protocol_cntr;
            unsupported_dest_port_cntr      <= (unsupported_dest_port)      ? unsupported_dest_port_cntr      + 1 : unsupported_dest_port_cntr;
        end
    end

    fifo_cdc #(.SIM_MODE(SIM_MODE)) ETH_TYPE_ERR_CDC   (.i_wclk(rx_clk125), .i_wclk_rst(i_rx_clk125_srst), .i_wdata_vld(1'b1), .i_wdata(unsupported_eth_type_error_cntr), .i_rclk(i_cmd_clk), .i_rclk_rst(i_cmd_srst), .o_rdata(o_unsupported_eth_type_error_cntr));
    fifo_cdc #(.SIM_MODE(SIM_MODE)) IPV4_PROTO_ERR_CDC (.i_wclk(rx_clk125), .i_wclk_rst(i_rx_clk125_srst), .i_wdata_vld(1'b1), .i_wdata(unsupported_ipv4_protocol_cntr),  .i_rclk(i_cmd_clk), .i_rclk_rst(i_cmd_srst), .o_rdata(o_unsupported_ipv4_protocol_cntr));
    fifo_cdc #(.SIM_MODE(SIM_MODE)) DEST_PORT_ERR_CDC  (.i_wclk(rx_clk125), .i_wclk_rst(i_rx_clk125_srst), .i_wdata_vld(1'b1), .i_wdata(unsupported_dest_port_cntr),      .i_rclk(i_cmd_clk), .i_rclk_rst(i_cmd_srst), .o_rdata(o_unsupported_dest_port_cntr));
    
    eth_rx_wrapper #(
        .CBUF_MAX_ETH_FRAME_DEPTH       (CBUF_MAX_ETH_FRAME_DEPTH       ), 
        .IPV4_PKT_FIFO_PKT_DEPTH        (IPV4_PKT_FIFO_PKT_DEPTH        ), 
        .ARP_PKT_FIFO_PKT_DEPTH         (ARP_PKT_FIFO_PKT_DEPTH         ), 
        .UDP_NUM_RX_PORTS               (UDP_NUM_RX_PORTS               ), 
        .UDP_RX_PORTS                   (UDP_RX_PORTS                   ) 
        ) ETH_RX_WRAPPER (
        .i_rxmac_clk                    (rx_clk125                      ),
        .i_rxmac_srst                   (i_rx_clk125_srst               ), 
        .i_rx_write                     (mac_rx_write                   ), 
        .i_rx_eof                       (mac_rx_eof                     ), 
        .i_rx_error                     (mac_rx_error                   ), 
        .i_rx_dbout                     (mac_rx_fifodata                ), 
        .i_rx_stat_en                   (mac_rx_stat_en                 ),
        .i_rx_stat_vector               (mac_rx_stat_vector             ),
        .o_arp_pkt_byte                 (arp_pkt_byte                   ),
        .o_arp_pkt_byte_vld             (arp_pkt_byte_vld               ),
        .o_arp_pkt_last_byte            (arp_pkt_last_byte              ),
        .i_arp_pkt_byte_rd              (arp_pkt_byte_rd                ),
        .o_port_last_byte               (udp_rx_port_last_byte          ), 
        .o_port_byte_vld                (udp_rx_port_byte_vld           ), 
        .o_port_byte                    (udp_rx_port_byte               ), 
        .i_port_byte_rd                 (udp_rx_port_byte_rd            ), 
        .o_eth_frame_circ_buf_overflow  (eth_frame_circ_buf_overflow    ),  // FATAL
        .o_ipv4_pkt_fifo_overflow       (ipv4_pkt_fifo_overflow         ),  // FATAL
        .o_arp_pkt_fifo_overflow        (arp_pkt_fifo_overflow          ),  // FATAL
        .o_udp_pkt_fifo_overflow        (udp_pkt_fifo_overflow          ),  // FATAL
        .o_icmp_pkt_fifo_overflow       (icmp_pkt_fifo_overflow         ),  // FATAL
        .o_port_fifo_overflow           (udp_port_fifo_overflow         ),  // FATAL
        .o_eth_frame_long_frame_error   (eth_rx_frame_long_frame_error  ), 
        .o_eth_frame_short_frame_error  (eth_rx_frame_short_frame_error ), 
        .o_eth_frame_ipg_error          (eth_rx_frame_ipg_error         ), 
        .o_eth_frame_crc_error          (eth_rx_frame_crc_error         ), 
        .o_eth_frame_vlan_error         (eth_rx_frame_vlan_error        ), 
        .o_unsupported_eth_type_error   (unsupported_eth_type_error     ), 
        .o_unsupported_ipv4_protocol    (unsupported_ipv4_protocol      ), 
        .o_unsupported_dest_port        (unsupported_dest_port          )
        // .snap_cbuf_rd_fsm_state         (snap_cbuf_rd_fsm_state         ),
        // .snap_cbuf_wr_fsm_state         (snap_cbuf_wr_fsm_state         )
    );
    
    
    /* MAC TX ARBITER */

    // TODO: Add PING support
    localparam int unsigned TX_ARB_NUM_INPUTS = 4; // Currently we have ADC data udp_packetizer, the udp_packetizer in udp_cmd, the udp_packetizer in udp_cmd in CS FPGA, and ARP reply 
    
    
//    localparam int unsigned TX_ARB_INDEX_GRAV_CMD = 0;
    localparam int unsigned TX_ARB_INDEX_ARP      = 1;
    localparam int unsigned TX_ARB_INDEX_VC  	  = 2;
    localparam int unsigned TX_ARB_INDEX_RB  	  = 3;
    
    logic [0:TX_ARB_NUM_INPUTS-1] [7:0] tx_src_byte;
    logic [0:TX_ARB_NUM_INPUTS-1]       tx_src_byte_vld;
    logic [0:TX_ARB_NUM_INPUTS-1]       tx_src_last_byte;
    logic [0:TX_ARB_NUM_INPUTS-1]       tx_src_byte_rd;
    
    logic eth_tx_frame_fifo_overflow;
    
    mac_tx_arbiter #(
        .NUM_INPUTS                 (TX_ARB_NUM_INPUTS           ), 
        .MAX_ETH_FRAME_BUFFERS      (TX_ARB_ETH_FRAME_BUFFERS    ), 
        .FAMILY                     ("ECP5U"                     ), 
        .IMPLEMENTATION             ("EBR"                       )
        ) mac_tx_arbiter (
        .i_txmac_clk                (i_tx_clk125                 ), 
        .i_txmac_srst               (i_tx_clk125_srst            ), 
        .i_src_byte                 (tx_src_byte                 ), 
        .i_src_byte_vld             (tx_src_byte_vld             ), 
        .i_src_last_byte            (tx_src_last_byte            ), 
        .o_src_byte_rd              (tx_src_byte_rd              ), 
        .i_tx_macread               (mac_tx_macread              ), 
        .o_tx_fifoavail             (mac_tx_fifoavail            ), 
        .o_tx_fifoeof               (mac_tx_fifoeof              ), 
        .o_tx_fifoempty             (mac_tx_fifoempty            ), 
        .o_tx_fifodata              (mac_tx_fifodata             ), 
        .o_eth_frame_fifo_overflow  (eth_tx_frame_fifo_overflow  )

        // .rr_arb_grant_mask          (rr_arb_grant_mask),
        // .rr_arb_grant               (rr_arb_grant),
        // .rr_arb_req_raw             (rr_arb_req_raw),
        // .rr_arb_req_masked          (rr_arb_req_masked)



        );
   


    /* ARP REPLY */
    
    logic [47:0] host_mac_tx; // _tx because this signal is synchronous to the i_tx_clk125/i_txmac_clk domain
    logic [47:0] host_mac_tx_learned;
    logic        host_mac_tx_vld;
    
    assign host_mac_tx = (host_mac_tx_vld) ? host_mac_tx_learned : 48'hffffffffffff; // just broadcast until we learn the host's actual MAC address
    
    fifo_cdc #(.WIDTH(48), .SIM_MODE(SIM_MODE)) HOST_MAC_CDC (.i_wclk(i_tx_clk125), .i_wclk_rst(i_tx_clk125_srst), .i_wdata_vld(1'b1), .i_wdata(host_mac_tx), .i_rclk(i_cmd_clk), .i_rclk_rst(i_cmd_srst), .o_rdata(o_host_mac));
    
    arp_reply #(
        .LOCAL_MAC                (LOCAL_MAC_ADDR                     ), 
        .LOCAL_IP                 (LOCAL_IP_ADDR                      ), 
        .FAMILY                   ("ECP5U"                            ), 
        .SIM_MODE                 (SIM_MODE                           )
        ) arp_reply (
        .i_rxmac_clk              (rx_clk125                          ), 
        .i_rxmac_srst             (i_rx_clk125_srst                   ), 
        .i_arp_pkt_byte           (arp_pkt_byte                       ), 
        .i_arp_pkt_byte_vld       (arp_pkt_byte_vld                   ), 
        .i_arp_pkt_last_byte      (arp_pkt_last_byte                  ), 
        .o_arp_pkt_byte_rd        (arp_pkt_byte_rd                    ), 
        .i_txmac_clk              (i_tx_clk125                        ), 
        .i_txmac_srst             (i_tx_clk125_srst                   ), 
        .o_eth_eof                (tx_src_last_byte[TX_ARB_INDEX_ARP] ), 
        .o_eth_byte_vld           (tx_src_byte_vld[TX_ARB_INDEX_ARP]  ), 
        .o_eth_byte               (tx_src_byte[TX_ARB_INDEX_ARP]      ), 
        .i_eth_byte_rd            (tx_src_byte_rd[TX_ARB_INDEX_ARP]   ), 
        .o_host_mac_tx            (host_mac_tx_learned                ),
        .o_host_mac_tx_vld        (host_mac_tx_vld                    ),
        .o_arp_pkt_wrong_ip_addr  (), 
        .o_arp_pkt_bad_htype      (), 
        .o_arp_pkt_bad_ptype      (), 
        .o_arp_pkt_bad_hlen       (), 
        .o_arp_pkt_bad_plen       (), 
        .o_arp_pkt_bad_oper       (), 
        .o_arp_pkt_short          ()); 
    
    
    /* UDP COMMAND AND CONTROL */
    
    logic runt_grav_udp_cmd;
    


    /* DAC DATA BUFFER */
    
    // localparam int unsigned DAC_SAMPLE_BYTES          = 4; // 16-bit I, 16-bit Q
    // localparam int unsigned DAC_DATA_BUF_SAMPLE_DEPTH = 16384;
    
    // logic dac_data_buf_afull;
    // connect these to udp_rx_stream_buffer
    logic cs20_data_buf_overflow;
    logic cs20_data_buf_underflow;
    logic rb_in_buf_overflow;
    logic rb_in_buf_underflow;
    // logic dac_data_udp_seq_num_error;

    logic [31:0] cs20_data_buf_overflow_cntr;
    logic [31:0] rb_in_buf_overflow_cntr;
    // logic [31:0] dac_data_udp_seq_num_error_cntr;

    
    /*
     * This always block is on the ethernet RX clock.
     * We acumulate any errors into counters in the ethernet clock domain
     */
    always_ff @(posedge rx_clk125) begin
        if (i_rx_clk125_srst) begin
            cs20_data_buf_overflow_cntr      <= '0;
            rb_in_buf_overflow_cntr      <= '0;
        end else begin
            cs20_data_buf_overflow_cntr      <= (cs20_data_buf_overflow)      ? cs20_data_buf_overflow_cntr      + 1 : cs20_data_buf_overflow_cntr     ;
            rb_in_buf_overflow_cntr      <= (rb_in_buf_overflow)      ? rb_in_buf_overflow_cntr      + 1 : rb_in_buf_overflow_cntr     ;
        end
    end

    /*
     * This clock domain crossing fifo allows us to read the error counters on the sys clock
     * domain
     */

    fifo_cdc #(
        .SIM_MODE(SIM_MODE)) CS20_DATA_BUF_OVERFLOW_CDC  (
        .i_wclk(rx_clk125),
        .i_wclk_rst(i_rx_clk125_srst),
        .i_wdata_vld(1'b1),
        .i_wdata(cs20_data_buf_overflow_cntr     ),
        .i_rclk(i_cmd_clk),
        .i_rclk_rst(i_cmd_srst),
        .o_rdata(o_cs20_data_buf_overflow_cntr     ));
    
    fifo_cdc #(
        .SIM_MODE(SIM_MODE)) RB_in_BUF_OVERFLOW_CDC  (
        .i_wclk(rx_clk125),
        .i_wclk_rst(i_rx_clk125_srst),
        .i_wdata_vld(1'b1),
        .i_wdata(rb_in_buf_overflow_cntr     ), 
        .i_rclk(i_cmd_clk),
        .i_rclk_rst(i_cmd_srst),
        .o_rdata(o_rb_in_buf_overflow_cntr     ));


    // connect these to udp_packetizer
    logic cs30_data_buf_overflow;
    // logic cs30_data_buf_overflow;
    logic rb_out_buf_overflow;
    // logic rb_out_buf_underflow;
    // logic dac_data_udp_seq_num_error;

    logic [31:0] cs30_data_buf_overflow_cntr;
    // logic [31:0] cs30_data_buf_underflow_cntr;
    logic [31:0] rb_out_buf_overflow_cntr;
    // logic [31:0] rb_out_buf_underflow_cntr;


    /*
     * This always block is on the ethernet TX clock.
     */
    always_ff @(posedge i_tx_clk125) begin
        if (i_tx_clk125_srst) begin
            cs30_data_buf_overflow_cntr      <= '0;
            // cs30_data_buf_underflow_cntr     <= '0;
            rb_out_buf_overflow_cntr      <= '0;
            // rb_out_buf_underflow_cntr     <= '0;
        end else begin
            cs30_data_buf_overflow_cntr      <= (cs30_data_buf_overflow)      ? cs30_data_buf_overflow_cntr      + 1 : cs30_data_buf_overflow_cntr     ;
            // cs30_data_buf_underflow_cntr     <= (cs30_data_buf_underflow)     ? cs30_data_buf_underflow_cntr     + 1 : cs30_data_buf_underflow_cntr    ;
            rb_out_buf_overflow_cntr      <= (rb_out_buf_overflow)      ? rb_out_buf_overflow_cntr      + 1 : rb_out_buf_overflow_cntr     ;
            // rb_out_buf_underflow_cntr     <= (rb_out_buf_underflow)     ? rb_out_buf_underflow_cntr     + 1 : rb_out_buf_underflow_cntr    ;
        end
    end

    fifo_cdc #(.SIM_MODE(SIM_MODE)) CS30_DATA_BUF_OVERFLOW_CDC  (
        .i_wclk(i_tx_clk125),
        .i_wclk_rst(i_tx_clk125_srst),
        .i_wdata_vld(1'b1),
        .i_wdata(cs30_data_buf_overflow_cntr     ),
        .i_rclk(i_cmd_clk),
        .i_rclk_rst(i_cmd_srst),
        .o_rdata(o_cs30_data_buf_overflow_cntr     ));

    fifo_cdc #(
         .SIM_MODE(SIM_MODE)
        ,.DEPTH(4)
        ,.WIDTH(1)
    ) CS30_DATA_AFULL_CDC  (
        .i_wclk(i_tx_clk125),
        .i_wclk_rst(i_tx_clk125_srst),
        .i_wdata_vld(1'b1),
        .i_wdata(cs30_data_buf_overflow),
        .i_rclk(i_cmd_clk),
        .i_rclk_rst(i_cmd_srst),
        .o_rdata(o_cs30_data_buf_afull     ));






    fifo_cdc #(.SIM_MODE(SIM_MODE)) RB_OUT_BUF_OVERFLOW_CDC  (
        .i_wclk(i_tx_clk125),
        .i_wclk_rst(i_tx_clk125_srst),
        .i_wdata_vld(1'b1),
        .i_wdata(rb_out_buf_overflow_cntr     ),
        .i_rclk(i_cmd_clk),
        .i_rclk_rst(i_cmd_srst),
        .o_rdata(o_rb_out_buf_overflow_cntr     ));


    localparam int unsigned CS20_DATA_SAMPLE_BYTES          = 4; // 16-bit I, 16-bit Q
    localparam int unsigned CS20_DATA_DATA_BUF_SAMPLE_DEPTH = 64;
    
    /*
     * 
     * Forwarded off chip to cs20
     * 
     */ 

    udp_rx_stream_buffer #(
    		.RD_WIDTH                   (8*CS20_DATA_SAMPLE_BYTES),
    		.RD_DEPTH                   (CS20_DATA_DATA_BUF_SAMPLE_DEPTH),
    		.FAMILY                     ("ECP5U"),
    		.IMPLEMENTATION             ("EBR"),
    		.RESET_MODE                 ("sync"),
    		.BIG_ENDIAN_FMT             (0),
    		.SEQ_NUM_PRSNT              (0),
    		.SIM_MODE                   (SIM_MODE)
    	) CS20_DATA_BUF (
    		.i_rxmac_clk                (rx_clk125                                  ),
    		.i_rxmac_srst               (i_rx_clk125_srst                           ),
    		.o_udp_port_fifo_rd         (udp_rx_port_byte_rd[PORT_FIFO_INDEX_VD]   ),
    		.i_udp_port_fifo_byte_vld   (udp_rx_port_byte_vld[PORT_FIFO_INDEX_VD]  ),
    		.i_udp_port_fifo_last_byte  (udp_rx_port_last_byte[PORT_FIFO_INDEX_VD] ),
    		.i_udp_port_fifo_byte       (udp_rx_port_byte[PORT_FIFO_INDEX_VD]      ),
    		.o_buffer_afull             (                         ),
    		.o_buffer_overflow          (cs20_data_buf_overflow                    ),
    		.i_sys_clk                  (i_tx_clk125                                ),
    		.i_sys_srst                 (i_tx_clk125_srst                           ),
    		.i_buffer_rd                (i_vd_buffer_rd                            ),
    		.o_buffer_data_vld          (o_vd_buffer_data_vld                      ),
    		.o_buffer_data_parity       (                   ),
    		.o_buffer_data              (o_vd_buffer_data                          ),
    		.o_buffer_underflow         (cs20_data_buf_underflow                     ),
    		.o_udp_seq_num_error        (                 ));         
    
    /*
     * 
     * Ring Bus receving data from software
     * 
     */ 
    localparam int unsigned RingBus_SAMPLE_BYTES          = 4; // 16-bit I, 16-bit Q
    localparam int unsigned RingBus_DATA_BUF_SAMPLE_DEPTH = 64;
    
    // FIXME: if logic is all setup, this can be cut a lot
    udp_rx_stream_buffer #(
    		.RD_WIDTH                   (8*RingBus_SAMPLE_BYTES),
    		.RD_DEPTH                   (RingBus_DATA_BUF_SAMPLE_DEPTH),
    		.FAMILY                     ("ECP5U"),
    		.IMPLEMENTATION             ("EBR"),
    		.RESET_MODE                 ("sync"),
    		.BIG_ENDIAN_FMT             (0),
    		.SEQ_NUM_PRSNT              (0),
    		.SIM_MODE                   (SIM_MODE)
    	) RingBus_DATA_BUF (
    		.i_rxmac_clk                (rx_clk125                                  ),
    		.i_rxmac_srst               (i_rx_clk125_srst                           ),
    		.o_udp_port_fifo_rd         (udp_rx_port_byte_rd[PORT_FIFO_INDEX_RB]   ),
    		.i_udp_port_fifo_byte_vld   (udp_rx_port_byte_vld[PORT_FIFO_INDEX_RB]  ),
    		.i_udp_port_fifo_last_byte  (udp_rx_port_last_byte[PORT_FIFO_INDEX_RB] ),
    		.i_udp_port_fifo_byte       (udp_rx_port_byte[PORT_FIFO_INDEX_RB]      ),
    		.o_buffer_afull             (                         ),
    		.o_buffer_overflow          (rb_in_buf_overflow                      ),
    		.i_sys_clk                  (i_tx_clk125                                ),
    		.i_sys_srst                 (i_tx_clk125_srst                           ),
    		.i_buffer_rd                (i_rb_buffer_rd                            ),
    		.o_buffer_data_vld          (o_rb_buffer_data_vld                      ),
    		.o_buffer_data_parity       (                   ),
    		.o_buffer_data              (o_rb_buffer_data                          ),
    		.o_buffer_underflow         (rb_in_buf_underflow                     ),
    		.o_udp_seq_num_error        (                 ));         
    
    /*
     * 
     * Logic to send captured data from VS to the host
     * 
     */ 
    localparam int unsigned UDP_RB_SEQ_NUM_BYTES   = 4; // this should match the number of bytes in each Ring Bus sample, otherwise the below udp_packetizer configuration won't work and will need to be redone.
    logic [(8*UDP_RB_SEQ_NUM_BYTES)-1:0] rb_udp_seq_num;
    
    always_ff @(posedge i_tx_clk125) begin
    	if (i_tx_clk125_srst) begin
    		rb_udp_seq_num                   <= '0;
    	end else begin
    		if (o_rb_pktzr_start_ack) begin
    			rb_udp_seq_num <= rb_udp_seq_num + 1;
    		end
    	end
    end
 	
    /*
     *  Ringbus data leaving higgs, destined for pc
     */
    udp_packetizer #(
    		.NUM_ETH_FRAME_BUFFERS    (UDP_RB_ETH_FRAME_BUFFERS			  ), 
    		.SEQ_NUM_BYTES            (UDP_RB_SEQ_NUM_BYTES				  ), 
    		.SEQ_NUM_LITTLE_ENDIAN    (1								  ), 
    		.META_DATA_BYTES          (1								  ), 
    		.META_DATA_LITTLE_ENDIAN  (1								  ), 
    		.GBIT_MAC_DIRECT_MODE     (0								  )
    	) udp_packetizer_RB (
    		.i_txmac_clk              (i_tx_clk125             			  ), 
    		.i_txmac_srst             (i_tx_clk125_srst        			  ), 
    		.i_start                  (i_rb_pktzr_start                   ),
    		.o_start_ack              (o_rb_pktzr_start_ack               ),
    		.o_done                   (o_rb_pktzr_done                    ), 
    		.i_data_byte_vld          (i_rb_pktzr_data_byte_vld           ),
    		.i_data_byte              (i_rb_pktzr_data_byte               ),
    		.o_data_byte_rd           (o_rb_pktzr_data_byte_rd            ),
    		.i_dest_mac               (host_mac_tx						  ), 
    		.i_dest_ip                (HOST_IP_ADDR               		  ), 
    		.i_dest_port              (UDP_RB_TX_PORT             		  ), 
    		.i_src_mac                (LOCAL_MAC_ADDR              		  ), 
    		.i_src_ip                 (LOCAL_IP_ADDR                 	  ), 
    		.i_src_port               (UDP_RB_TX_PORT+1              	  ), 
    		.i_udp_payload_bytes      (UDP_RINGBUS_PAYLOAD_BYTES + UDP_RB_SEQ_NUM_BYTES),
    		.i_seq_num_prsnt          (1         						  ), 
    		.i_seq_num                (rb_udp_seq_num 					  ), 
    		.i_meta_data_prsnt        (0								  ), 
    		.i_meta_data              (0                      			  ), 
    		.o_eth_avail              (tx_src_byte_vld[TX_ARB_INDEX_RB]   ), // note: use this for request to mac_tx_arbiter so that we don't make a request until the full Ethernet II Frame is ready
    		.o_eth_eof                (tx_src_last_byte[TX_ARB_INDEX_RB]  ), 
    		.o_eth_byte_vld           (                                   ), 
    		.o_eth_byte               (tx_src_byte[TX_ARB_INDEX_RB]       ), 
            .i_eth_byte_rd            (tx_src_byte_rd[TX_ARB_INDEX_RB]    ),
            .o_fifo_full              (                                   ),
            .o_fifo_afull             (rb_out_buf_overflow                )); // wire up to afull because the logic inside will not procude when afull is high, meaning this is the same as full
    
    localparam int unsigned UDP_VC_SEQ_NUM_BYTES   = 4; // this should match the number of bytes in each VC sample, otherwise the below udp_packetizer configuration won't work and will need to be redone.
    logic [(8*UDP_VC_SEQ_NUM_BYTES)-1:0] vc_udp_seq_num;
    
    always_ff @(posedge i_tx_clk125) begin
        if (i_tx_clk125_srst) begin
            vc_udp_seq_num                   <= '0;
        end else begin
            if (o_vc_pktzr_start_ack) begin
                vc_udp_seq_num <= vc_udp_seq_num + 1;
            end
        end
    end
    
    /*
     *  Data coming from CS30
     */
    
    udp_packetizer #(
            .NUM_ETH_FRAME_BUFFERS    (UDP_VC_ETH_FRAME_BUFFERS           ), 
            .SEQ_NUM_BYTES            (UDP_VC_SEQ_NUM_BYTES               ), 
            .SEQ_NUM_LITTLE_ENDIAN    (1                                  ), 
            .META_DATA_BYTES          (1                                  ), 
            .META_DATA_LITTLE_ENDIAN  (1                                  ), 
            .GBIT_MAC_DIRECT_MODE     (0                                  )
        ) udp_packetizer_VC (
            .i_txmac_clk              (i_tx_clk125                        ), 
            .i_txmac_srst             (i_tx_clk125_srst                   ), 
            .i_start                  (i_vc_pktzr_start                   ),
            .o_start_ack              (o_vc_pktzr_start_ack               ),
            .o_done                   (o_vc_pktzr_done                    ), 
            .i_data_byte_vld          (i_vc_pktzr_data_byte_vld           ),
            .i_data_byte              (i_vc_pktzr_data_byte               ),
            .o_data_byte_rd           (o_vc_pktzr_data_byte_rd            ),
            .i_dest_mac               (host_mac_tx                        ), 
            .i_dest_ip                (HOST_IP_ADDR                       ), 
            .i_dest_port              (UDP_VC_TX_PORT                     ), 
            .i_src_mac                (LOCAL_MAC_ADDR                     ), 
            .i_src_ip                 (LOCAL_IP_ADDR                      ), 
            .i_src_port               (UDP_VC_TX_PORT+1                   ), 
            .i_udp_payload_bytes      (UDP_PAYLOAD_MAX_BYTES              ), 
            .i_seq_num_prsnt          (1                                  ), 
            .i_seq_num                (vc_udp_seq_num                     ), 
            .i_meta_data_prsnt        (0                                  ), 
            .i_meta_data              (0                                  ), 
            .o_eth_avail              (tx_src_byte_vld[TX_ARB_INDEX_VC]   ), // note: use this for request to mac_tx_arbiter so that we don't make a request until the full Ethernet II Frame is ready
            .o_eth_eof                (tx_src_last_byte[TX_ARB_INDEX_VC]  ), 
            .o_eth_byte_vld           (                                   ), 
            .o_eth_byte               (tx_src_byte[TX_ARB_INDEX_VC]       ), 
            .i_eth_byte_rd            (tx_src_byte_rd[TX_ARB_INDEX_VC]    ),
            .o_fifo_full              (                                   ),
            .o_fifo_afull             (cs30_data_buf_overflow             ));
    
    
/* verilator lint_on LITENDIAN */
endmodule

`default_nettype wire
