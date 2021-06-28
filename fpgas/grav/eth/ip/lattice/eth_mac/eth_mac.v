/* synthesis translate_off*/
`define SBP_SIMULATION
/* synthesis translate_on*/
`ifndef SBP_SIMULATION
`define SBP_SYNTHESIS
`endif

//
// Verific Verilog Description of module eth_mac
//
module eth_mac (gbit_mac_haddr, gbit_mac_hdatain, gbit_mac_hdataout, gbit_mac_rx_dbout, 
            gbit_mac_rx_stat_vector, gbit_mac_rxd, gbit_mac_tx_fifodata, 
            gbit_mac_tx_sndpaustim, gbit_mac_tx_statvec, gbit_mac_txd, 
            gbit_mac_cpu_if_gbit_en, gbit_mac_hclk, gbit_mac_hcs_n, gbit_mac_hdataout_en_n, 
            gbit_mac_hread_n, gbit_mac_hready_n, gbit_mac_hwrite_n, gbit_mac_ignore_pkt, 
            gbit_mac_reset_n, gbit_mac_rx_dv, gbit_mac_rx_eof, gbit_mac_rx_er, 
            gbit_mac_rx_error, gbit_mac_rx_fifo_error, gbit_mac_rx_fifo_full, 
            gbit_mac_rx_stat_en, gbit_mac_rx_write, gbit_mac_rxmac_clk, 
            gbit_mac_tx_discfrm, gbit_mac_tx_done, gbit_mac_tx_en, gbit_mac_tx_er, 
            gbit_mac_tx_fifoavail, gbit_mac_tx_fifoctrl, gbit_mac_tx_fifoempty, 
            gbit_mac_tx_fifoeof, gbit_mac_tx_macread, gbit_mac_tx_sndpausreq, 
            gbit_mac_tx_staten, gbit_mac_txmac_clk) /* synthesis sbp_module=true */ ;
    input [7:0]gbit_mac_haddr;
    input [7:0]gbit_mac_hdatain;
    output [7:0]gbit_mac_hdataout;
    output [7:0]gbit_mac_rx_dbout;
    output [31:0]gbit_mac_rx_stat_vector;
    input [7:0]gbit_mac_rxd;
    input [7:0]gbit_mac_tx_fifodata;
    input [15:0]gbit_mac_tx_sndpaustim;
    output [30:0]gbit_mac_tx_statvec;
    output [7:0]gbit_mac_txd;
    output gbit_mac_cpu_if_gbit_en;
    input gbit_mac_hclk;
    input gbit_mac_hcs_n;
    output gbit_mac_hdataout_en_n;
    input gbit_mac_hread_n;
    output gbit_mac_hready_n;
    input gbit_mac_hwrite_n;
    input gbit_mac_ignore_pkt;
    input gbit_mac_reset_n;
    input gbit_mac_rx_dv;
    output gbit_mac_rx_eof;
    input gbit_mac_rx_er;
    output gbit_mac_rx_error;
    output gbit_mac_rx_fifo_error;
    input gbit_mac_rx_fifo_full;
    output gbit_mac_rx_stat_en;
    output gbit_mac_rx_write;
    input gbit_mac_rxmac_clk;
    output gbit_mac_tx_discfrm;
    output gbit_mac_tx_done;
    output gbit_mac_tx_en;
    output gbit_mac_tx_er;
    input gbit_mac_tx_fifoavail;
    input gbit_mac_tx_fifoctrl;
    input gbit_mac_tx_fifoempty;
    input gbit_mac_tx_fifoeof;
    output gbit_mac_tx_macread;
    input gbit_mac_tx_sndpausreq;
    output gbit_mac_tx_staten;
    input gbit_mac_txmac_clk;
    
    
    gbit_mac gbit_mac_inst (.haddr({gbit_mac_haddr}), .hdatain({gbit_mac_hdatain}), 
            .hdataout({gbit_mac_hdataout}), .rx_dbout({gbit_mac_rx_dbout}), 
            .rx_stat_vector({gbit_mac_rx_stat_vector}), .rxd({gbit_mac_rxd}), 
            .tx_fifodata({gbit_mac_tx_fifodata}), .tx_sndpaustim({gbit_mac_tx_sndpaustim}), 
            .tx_statvec({gbit_mac_tx_statvec}), .txd({gbit_mac_txd}), .cpu_if_gbit_en(gbit_mac_cpu_if_gbit_en), 
            .hclk(gbit_mac_hclk), .hcs_n(gbit_mac_hcs_n), .hdataout_en_n(gbit_mac_hdataout_en_n), 
            .hread_n(gbit_mac_hread_n), .hready_n(gbit_mac_hready_n), .hwrite_n(gbit_mac_hwrite_n), 
            .ignore_pkt(gbit_mac_ignore_pkt), .reset_n(gbit_mac_reset_n), 
            .rx_dv(gbit_mac_rx_dv), .rx_eof(gbit_mac_rx_eof), .rx_er(gbit_mac_rx_er), 
            .rx_error(gbit_mac_rx_error), .rx_fifo_error(gbit_mac_rx_fifo_error), 
            .rx_fifo_full(gbit_mac_rx_fifo_full), .rx_stat_en(gbit_mac_rx_stat_en), 
            .rx_write(gbit_mac_rx_write), .rxmac_clk(gbit_mac_rxmac_clk), 
            .tx_discfrm(gbit_mac_tx_discfrm), .tx_done(gbit_mac_tx_done), 
            .tx_en(gbit_mac_tx_en), .tx_er(gbit_mac_tx_er), .tx_fifoavail(gbit_mac_tx_fifoavail), 
            .tx_fifoctrl(gbit_mac_tx_fifoctrl), .tx_fifoempty(gbit_mac_tx_fifoempty), 
            .tx_fifoeof(gbit_mac_tx_fifoeof), .tx_macread(gbit_mac_tx_macread), 
            .tx_sndpausreq(gbit_mac_tx_sndpausreq), .tx_staten(gbit_mac_tx_staten), 
            .txmac_clk(gbit_mac_txmac_clk));
    
endmodule

