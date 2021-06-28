
/*
 * Module: eth_mac_wrapper
 * 
 */
 

`default_nettype none 

module eth_mac_wrapper #(
        
    parameter bit [47:0]   P_MAC_ADDR   = 48'h00000000,
    parameter int unsigned P_CFG_CLK_HZ = 10_000_000

) (
    
    /* i_cfg_clk domain signals */
    input         i_cfg_clk,
    input         i_cfg_srst, // reset, synchronous to i_cfg_clk
    output        o_gbit_mode,
    output        o_cfg_done,
    output        o_phy_rst_n,
    
    /* MAC rx clk domain signals */
    output        o_rx_clk125,
    output        o_rx_write,
    output        o_rx_eof,
    output        o_rx_error,       
    output [ 7:0] o_rx_fifodata,
    output        o_rx_stat_en,
    output [31:0] o_rx_stat_vector,
    
    /* MAC tx clk domain signals */
    input         i_tx_clk125,  // has to be 125MHz since we only support Gigabit mode
    input  [ 7:0] i_tx_fifodata,
    input         i_tx_fifoavail,
    input         i_tx_fifoeof,
    input         i_tx_fifoempty,
    output        o_tx_macread,    
    output        o_tx_done,
    output        o_tx_staten,
    output [30:0] o_tx_statvec,
    input         i_tx_sndpausreq,
    input  [15:0] i_tx_sndpaustim,
    
    /* RGMII Signals */
    input         i_rgmii_rxclk,
    input         i_rgmii_rxctrl,
    input  [ 3:0] i_rgmii_rxd,
    output        o_rgmii_txclk,
    output        o_rgmii_txctrl,
    output [ 3:0] o_rgmii_txd
        
);

    logic        hready_n;
    logic        hdataout_en_n;
    logic [ 7:0] hdataout;
    logic        hcs_n;
    logic [ 7:0] haddr;
    logic [ 7:0] hdatain;
    logic        hwrite_n;
    logic        hread_n;

    logic        rxmac_clk;
    logic [ 7:0] rxd;
    logic        rx_dv;
    logic        rx_eof;
    logic        rx_er;
    logic [7:0]  txd;
    logic        tx_en;
    logic        tx_er;


    // MAC config (also resets the PHY so we can avoid configuring the MAC before the PHY is reset)
   
    mac_cfg #(
        .MAC_ADDR       (P_MAC_ADDR),
        .CLK_FREQ_HZ    (P_CFG_CLK_HZ)
        ) mac_cfg (
        .clk            (i_cfg_clk     ), 
        .srst           (i_cfg_srst    ), 
        .hready_n       (hready_n      ), 
        .hdataout_en_n  (hdataout_en_n ), 
        .hdataout       (hdataout      ), 
        .hcs_n          (hcs_n         ), 
        .haddr          (haddr         ), 
        .hdatain        (hdatain       ), 
        .hwrite_n       (hwrite_n      ), 
        .hread_n        (hread_n       ),
        .phy_rst_n      (o_phy_rst_n   ),
        .mac_cfg_done   (o_cfg_done    ) 
    );

    reg loopback_rdata;
    reg loopback_rden;
    reg loopback_rvld;
    // Eth MAC
    
    eth_mac eth_mac (
        .gbit_mac_hclk            (i_cfg_clk        ), 
        .gbit_mac_hcs_n           (hcs_n            ), 
        .gbit_mac_haddr           (haddr            ), 
        .gbit_mac_hdatain         (hdatain          ), 
        .gbit_mac_hdataout        (hdataout         ), 
        .gbit_mac_hdataout_en_n   (hdataout_en_n    ), 
        .gbit_mac_hread_n         (hread_n          ), 
        .gbit_mac_hready_n        (hready_n         ), 
        .gbit_mac_hwrite_n        (hwrite_n         ), 

        .gbit_mac_cpu_if_gbit_en  (o_gbit_mode      ), 
        .gbit_mac_ignore_pkt      (0                ), 
        .gbit_mac_reset_n         (~i_cfg_srst      ), 

        .gbit_mac_rx_dbout        (o_rx_fifodata    ), 
        .gbit_mac_rx_fifo_full    (0                ), // From Eth Mac User Guid (IPUG51): "This signal indicates the Rx FIFO is full and cannot accept any more data. This is an error condition and should never happen."
        .gbit_mac_rx_stat_en      (o_rx_stat_en     ), 
        .gbit_mac_rx_write        (o_rx_write       ), 
        .gbit_mac_rx_stat_vector  (o_rx_stat_vector ), 
        .gbit_mac_rx_eof          (o_rx_eof         ), 
        .gbit_mac_rx_error        (o_rx_error       ), 
        .gbit_mac_rx_fifo_error   (                 ), // Since we never assert rx_fifo_full this signal will never be asserted by the core.
        .gbit_mac_rxmac_clk       (rxmac_clk        ), 
        .gbit_mac_rxd             (rxd              ), 
        .gbit_mac_rx_dv           (rx_dv            ), 
        .gbit_mac_rx_er           (rx_er            ), 

        .gbit_mac_tx_fifodata     (i_tx_fifodata    ), 
        .gbit_mac_tx_sndpaustim   (i_tx_sndpaustim  ), 
        .gbit_mac_tx_statvec      (o_tx_statvec     ), 
        .gbit_mac_tx_discfrm      (                 ), 
        .gbit_mac_tx_done         (o_tx_done        ), 
        .gbit_mac_tx_fifoavail    (i_tx_fifoavail   ), 
        .gbit_mac_tx_fifoctrl     (0                ),  // currently no need to send control packets
        .gbit_mac_tx_fifoempty    (i_tx_fifoempty   ), 
        .gbit_mac_tx_fifoeof      (i_tx_fifoeof     ), 
        .gbit_mac_tx_macread      (o_tx_macread     ), 
        .gbit_mac_tx_sndpausreq   (i_tx_sndpausreq  ),
        .gbit_mac_tx_staten       (o_tx_staten      ), 
        .gbit_mac_txd             (txd              ), 
        .gbit_mac_tx_en           (tx_en            ), 
        .gbit_mac_tx_er           (tx_er            ), 
        .gbit_mac_txmac_clk       (i_tx_clk125      )
    );


    /*pmi_fifo_dc_fwft_v1_0 #(
                           .WR_DEPTH        (32),
                           .WR_DEPTH_AFULL  (UDP_RINGBUS_PACKET_SIZE_WORDS),
                           .WR_WIDTH        (8),
                           .RD_WIDTH        (8),
                           .FAMILY          ("ECP5U"),
                           .IMPLEMENTATION  ("EBR"),
                           .RESET_MODE      ("sync"),
                           .WORD_SWAP       (0),
 `ifndef SIM_MODE
                           .SIM_MODE        (0)
 `else
                           .SIM_MODE        (1)
 `endif
                           ) fifo_dc_rb (
                                         .wrclk           (rxmac_clk),
                                         .wrclk_rst       (sys_clk_srst),
                                         .rdclk           (i_tx_clk125),
                                         .rdclk_rst       (sys_clk_srst),
                                         .wdata           (rxd),
                                         .wren            (rx_dv),
                                         .full            (),
                                         .afull           (),
                                         .rden            (loopback_rden),
                                         .rdata           (loopback_rdata),
                                         .rdata_vld       (loopback_rvld));
*/
    
    // RGMII-to-GMII
    
    rgmii2gmii rgmii2gmii (
        .rstn        (~i_cfg_srst), 
//        .rstn        (1'b1), 
        
        // GMII
        .rx_clk_out  (rxmac_clk         ), 
        .rxd         (rxd               ), 
        .rx_dv       (rx_dv             ), 
        .rx_er       (rx_er             ), 
        .txd         (txd               ), 
        .tx_en       (tx_en             ), 
        .tx_er       (tx_er             ),
        
        // RGMII
        .tx_clk      (i_tx_clk125       ), 
        .rx_clk      (i_rgmii_rxclk     ), 
        .rd          (i_rgmii_rxd       ), 
        .rx_ctl      (i_rgmii_rxctrl    ),
        .td          (o_rgmii_txd       ), 
        .tx_ctl      (o_rgmii_txctrl    )
    );
    //loopback
    /*rgmii2gmii rgmii2gmii (
        .rstn        (~i_cfg_srst), 
//        .rstn        (1'b1), 
        
        // GMII 
        .rx_clk_out  (rxmac_clk         ), 
        .rxd         (rxd               ), 
        .rx_dv       (rx_dv             ), 
        .rx_er       (rx_er             ), 
        .txd         (rxd               ), 
        .tx_en       (rx_dv             ), 
        .tx_er       (rx_er             ),
        
        // RGMII 
        .tx_clk      (i_tx_clk125     ), 
        .rx_clk      (i_rgmii_rxclk     ), 
        .rd          (i_rgmii_rxd       ), 
        .rx_ctl      (i_rgmii_rxctrl    ),
        .td          (o_rgmii_txd       ), 
        .tx_ctl      (o_rgmii_txctrl    )
    );*/
    
    assign o_rx_clk125   = rxmac_clk;
    assign o_rgmii_txclk = i_tx_clk125;
    
endmodule

`default_nettype wire 