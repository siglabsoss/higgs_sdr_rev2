//-----------------------------------------------------
// Design Name : cscfg_cmd
// File Name   : cscfg_cmd.sv
// Authors	   : 
// Modified    : 
// Function    : 
//-----------------------------------------------------

`default_nettype none

module cscfg_cmd #(
    parameter logic [7:0]   FPGA_UID        = 0,
    parameter int unsigned  CMD_DATA_BITS   = 32       
)(

    input wire logic                     i_sysclk,
    input wire logic                     i_srst_n,
    input wire logic [CMD_DATA_BITS-1:0] i_status,
    intf_cmd.slave                       cmd
);
    
    logic [CMD_DATA_BITS-1:0] reg1;
    
    always_ff @(posedge i_sysclk) begin
        
        if(~i_srst_n) begin
            cmd.ack   <= 0;
        end else begin
            
            /* defaults */
            cmd.ack <= 0;
            
            if (cmd.sel) begin

                cmd.ack <= 1;
                if (cmd.rd_wr_n) begin
                    case (cmd.byte_addr)
                        
                        0: begin
                            cmd.rdata <= {i_status, 4'b0001 , FPGA_UID};
                        end
                        
                        4: begin
                            cmd.rdata <= reg1;
                        end
                        
                        default: begin
                            cmd.rdata <= 32'hDEADBEEF; 
                        end
                        
                    endcase
                end else begin
                    case (cmd.byte_addr)
                        
                        4: begin
                            reg1 <= cmd.wdata;
                        end
                        
                        default: begin
                            // do nothing
                        end
                        
                    endcase                
                end
            end
        end
    end
        
endmodule

`default_nettype wire