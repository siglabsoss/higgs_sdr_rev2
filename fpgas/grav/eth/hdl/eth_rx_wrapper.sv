/**
 * Module: eth_rx_wrapper
 * 
 * Wraps all the Lattice Gbit MAC Rx support blocks to make the top level instantiation cleaner.
 * 
 */

 /* verilator lint_off LITENDIAN */

`default_nettype none

module eth_rx_wrapper #(
        
    parameter int unsigned                      CBUF_MAX_ETH_FRAME_DEPTH = 8, // Number of maximum sized Ethernet frames that can be simultaneously stored in the circular buffer
    parameter int unsigned                      IPV4_PKT_FIFO_PKT_DEPTH  = 5, // Number of maximum sized IPv4 packets that can be simultaneously stored in the output FIFO
    parameter int unsigned                      ARP_PKT_FIFO_PKT_DEPTH   = 5,  // Number of maximum sized ARP packets that can be simultaneously stored in the output FIFO
    parameter int unsigned                      UDP_NUM_RX_PORTS         = 4,
    parameter bit [0:UDP_NUM_RX_PORTS-1] [15:0] UDP_RX_PORTS             = {16'd30000, 16'd20000} // i.e. port 50,000 would correspond to router output interface 0

)(

    /* LATTICE Gbit ETH MAC RX INTERFACE */
    input        i_rxmac_clk,
    input        i_rxmac_srst, // synchronous to i_rxmac_clk
    input        i_rx_write,
    input        i_rx_eof,
    input        i_rx_error,
    input [ 7:0] i_rx_dbout,
    input        i_rx_stat_en,
    input [31:0] i_rx_stat_vector,
    
    /* ARP OUTPUT PACKET INTERFACE */
    output [7:0] o_arp_pkt_byte,
    output       o_arp_pkt_byte_vld,
    output       o_arp_pkt_last_byte,
    input        i_arp_pkt_byte_rd,

    /* UDP ROUTER OUTPUT INTERFACES */    
    output [0:UDP_NUM_RX_PORTS-1]       o_port_last_byte, 
    output [0:UDP_NUM_RX_PORTS-1]       o_port_byte_vld,
    output [0:UDP_NUM_RX_PORTS-1] [7:0] o_port_byte, 
    input  [0:UDP_NUM_RX_PORTS-1]       i_port_byte_rd,

    /* ERROR REPORTING */

    // IF ANY OF THESE ERRORS OCCUR YOU'LL NEED TO RESET THIS MODULE TO RECOVER AND CLEAR THE ERROR
    output o_eth_frame_circ_buf_overflow, 
    output o_ipv4_pkt_fifo_overflow,
    output o_arp_pkt_fifo_overflow,
    output o_udp_pkt_fifo_overflow,
    output o_icmp_pkt_fifo_overflow,
    output [UDP_NUM_RX_PORTS-1:0]  o_port_fifo_overflow,
    // THESE ERROR STATUS SIGNALS GET ASSERTED FOR ONE CLOCK IF THEIR RESPECTIVE ERROR OCCURS.  YOU NEED EXTERNAL LOGIC TO KEEP TRACK OF THESE ERRORS IF DESIRED.
    output o_eth_frame_long_frame_error,
    output o_eth_frame_short_frame_error,
    output o_eth_frame_ipg_error,
    output o_eth_frame_crc_error,
    output o_eth_frame_vlan_error,
    output o_unsupported_eth_type_error,
    output o_unsupported_ipv4_protocol,
    output o_unsupported_dest_port

    // output [7:0] snap_cbuf_rd_fsm_state,
    // output [7:0] snap_cbuf_wr_fsm_state
        
);

    /* IPv4 OUTPUT PACKET INTERFACE */
    logic [7:0] o_ipv4_pkt_byte;
    logic       o_ipv4_pkt_byte_vld;
    logic       o_ipv4_pkt_last_byte;
    logic       i_ipv4_pkt_byte_rd;
    
    /* UDP OUTPUT PACKET INTERFACE */
    logic [7:0] o_udp_pkt_byte;
    logic       o_udp_pkt_byte_vld;
    logic       o_udp_pkt_last_byte;
    logic       i_udp_pkt_byte_rd;
    
    /* ICMP OUTPUT PACKET INTERFACE */
    logic [7:0] o_icmp_pkt_byte;
    logic       o_icmp_pkt_byte_vld;
    logic       o_icmp_pkt_last_byte;
    logic       i_icmp_pkt_byte_rd = 1'b1;

    
    eth_frame_router #(
        .CBUF_MAX_ETH_FRAME_DEPTH       (CBUF_MAX_ETH_FRAME_DEPTH      ), 
        .IPV4_PKT_FIFO_PKT_DEPTH        (IPV4_PKT_FIFO_PKT_DEPTH       ), 
        .ARP_PKT_FIFO_PKT_DEPTH         (ARP_PKT_FIFO_PKT_DEPTH        )
    ) eth_frame_router (.*);

    
    ipv4_pkt_router ipv4_pkt_router (
        .i_ipv4_pkt_byte              (o_ipv4_pkt_byte             ), 
        .i_ipv4_pkt_byte_vld          (o_ipv4_pkt_byte_vld         ), 
        .i_ipv4_pkt_last_byte         (o_ipv4_pkt_last_byte        ), 
        .o_ipv4_pkt_byte_rd           (i_ipv4_pkt_byte_rd          ), 
        .*);
    
    udp_pkt_router #(
        .P_NUM_PORTS              (UDP_NUM_RX_PORTS             ), 
        .P_PORTS                  (UDP_RX_PORTS                 )
        ) udp_pkt_router (
        .o_port_last_byte         (o_port_last_byte        ), 
        .o_port_byte_vld          (o_port_byte_vld         ), 
        .o_port_byte              (o_port_byte             ), 
        .i_port_byte_rd           (i_port_byte_rd          ), 
        .i_udp_pkt_byte           (o_udp_pkt_byte          ), 
        .i_udp_pkt_byte_vld       (o_udp_pkt_byte_vld      ), 
        .i_udp_pkt_last_byte      (o_udp_pkt_last_byte     ), 
        .o_udp_pkt_byte_rd        (i_udp_pkt_byte_rd       ), 
        .*);

endmodule

`default_nettype wire

/* verilator lint_on LITENDIAN */
