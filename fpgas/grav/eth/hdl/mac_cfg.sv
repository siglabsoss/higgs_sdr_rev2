/*
 * Module: mac_cfg
 * 
 * Configures the Lattice Tri-Speed Ethernet MAC
 * 
 * TODO: Add module documentation
 * TODO: Add MAC register read capability (low priority) 
 * 
 */
module mac_cfg #(

    parameter [47:0] MAC_ADDR     = 48'hAC_DE_48_00_00_80,
    parameter        CLK_FREQ_HZ  = 25_800_000 // frequency of clk input in Hertz
        
) (

    input            clk,
    input            srst,
    input            hready_n,
    input            hdataout_en_n,
    input      [7:0] hdataout,
    output reg       hcs_n,
    output reg [7:0] haddr,
    output reg [7:0] hdatain,
    output reg       hwrite_n,
    output reg       hread_n,
    output reg       phy_rst_n,
    output reg       mac_cfg_done
);
    
    
    localparam [7:0] MAC_MODE_REG_0      = 8'h0e;
    localparam [7:0] MAC_TX_RX_CTL_REG_0 = 8'hc2; // rx broadcast, drop control, discard Rx FCS 
    
    localparam PHY_RST_CNTR_BITS = $clog2(int'(15e-3 * CLK_FREQ_HZ)); // phy needs to be reset for at least 10ms, so we reset for 15ms

    logic [PHY_RST_CNTR_BITS-1:0] phy_rst_cntr;
    
    assign hread_n = 1; // don't currently support read and we probably need to any time soon
    

    // phy reset

    always_ff @(posedge clk)
    begin
        if (srst) begin
            phy_rst_cntr <= {PHY_RST_CNTR_BITS{1'b0}};
            phy_rst_n    <= 0;
        end else begin
            if (phy_rst_cntr != {PHY_RST_CNTR_BITS{1'b1}}) begin
                phy_rst_cntr <= phy_rst_cntr + 1;
                phy_rst_n    <= 0;
            end else begin
                phy_rst_cntr <= phy_rst_cntr;
                phy_rst_n    <= 1;
            end
        end
    end     

    // program MAC control registers (wait for phy reset to finish)
    
    always_ff @(posedge clk) begin
        if (srst | ~phy_rst_n) begin
            haddr        <= 8'h0b;
            hcs_n        <= 1;
            hwrite_n     <= 1;
            mac_cfg_done <= 0;
        end else begin
        
            /* defaults */
            hcs_n    <= 1;
            hwrite_n <= 1;
        
            case (haddr)
                
                8'h0b: begin
                    if (hready_n) begin
                        hcs_n    <= 0;
                        hwrite_n <= 0;
                        hdatain  <= MAC_ADDR[47:40];
                    end else begin
                        haddr <= 8'h0a;
                    end                
                end
                
                8'h0a: begin
                    if (hready_n) begin
                        hcs_n    <= 0;
                        hwrite_n <= 0;
                        hdatain  <= MAC_ADDR[39:32];
                    end else begin
                        haddr <= 8'h0d;
                    end                    
                end
                
                8'h0d: begin
                    if (hready_n) begin
                        hcs_n    <= 0;
                        hwrite_n <= 0;
                        hdatain  <= MAC_ADDR[31:24];
                    end else begin
                        haddr <= 8'h0c;
                    end                    
                end
                
                8'h0c: begin
                    if (hready_n) begin
                        hcs_n    <= 0;
                        hwrite_n <= 0;
                        hdatain  <= MAC_ADDR[23:16];
                    end else begin
                        haddr <= 8'h0f;
                    end                    
                end
                
                8'h0f: begin
                    if (hready_n) begin
                        hcs_n    <= 0;
                        hwrite_n <= 0;
                        hdatain  <= MAC_ADDR[15:8];
                    end else begin
                        haddr <= 8'h0e;
                    end                    
                end
                
                8'h0e: begin
                    if (hready_n) begin
                        hcs_n    <= 0;
                        hwrite_n <= 0;
                        hdatain  <= MAC_ADDR[7:0];
                    end else begin
                        haddr <= 8'h02;
                    end                    
                end
                
                8'h02: begin
                    if (hready_n) begin
                        hcs_n    <= 0;
                        hwrite_n <= 0;
                        hdatain  <= MAC_TX_RX_CTL_REG_0;
                    end else begin
                        haddr <= 8'h00;
                    end                    
                end
                
                8'h00: begin
                    if (hready_n) begin
                        hcs_n    <= 0;
                        hwrite_n <= 0;
                        hdatain  <= MAC_MODE_REG_0;
                    end else begin
                        haddr <= 8'hff;
                    end                    
                end
                
                8'hff: begin
                    haddr        <= 8'hff;
                    hcs_n        <= 1;
                    hwrite_n     <= 1;
                    mac_cfg_done <= 1;
                end
                
                default: begin
                    haddr        <= 8'h0b;
                    hcs_n        <= 1;
                    hwrite_n     <= 1;
                    mac_cfg_done <= 0;
                end
                
            endcase
        end
    end    


endmodule