/*
 * Brief: Total that FMC module on MCU can access is 64MB x 4 Banks.
 * for simplicity lets take the conservative approach.
 *
 * @blame: tbags@
 *
 */

module fmc_slave_memory #(
    parameter DATA_BITS  =   		 32, // Data bus size.
    parameter ADDR_BITS  =    	     12  // in terms of bits.
    )

(
    // system inputs
    input i_sys_clk,    // Clock
    input i_sys_rst, 	// System reset.

    // External command Interface
    intf_cmd.slave mem_cmd


);

reg signed [DATA_BITS-1:0] [(1<<ADDR_BITS)-1:0] ram_mem; // memory
reg signed [DATA_BITS-1:0] mem_r_reg;
reg signed [DATA_BITS-1:0] mem_r_reg_d;

enum {
    IDLE,
    READ,
    WRITE,
    ACK
} mem_fsm;


reg cmd_ack;

// I'm following https://github.com/siglabsoss/ip-library/blob/master/interfaces/cmd_interface/doc/cmd_bus_timing.jpg
// for mimicing memory behavior, lol.
assign mem_cmd.rdata = cmd_ack ? mem_r_reg_d : {DATA_BITS{1'bz}};
assign mem_cmd.ack = cmd_ack;

    always_ff @(posedge i_sys_clk) begin
        mem_r_reg_d <= mem_r_reg;
    end


    always_ff @(posedge i_sys_clk) begin : MEMORY_FSM
        if(i_sys_rst) begin
            mem_r_reg <= {DATA_BITS{1'b0}};
            cmd_ack <= 1'b0;
            mem_fsm <= IDLE;
        end else begin
            cmd_ack <= 1'b0;
            mem_r_reg <= {DATA_BITS{1'b0}};
            case(mem_fsm)
                IDLE: begin
                    if (mem_cmd.sel) begin
                        if (mem_cmd.rd_wr_n) begin
                            mem_r_reg <= ram_mem[(mem_cmd.byte_addr & ((1<<ADDR_BITS)-1))];
                            mem_fsm <= READ;
                        end else begin
                            mem_r_reg <= mem_cmd.wdata; // done so that we may emulate a row buffer hit/miss.
                            mem_fsm <= WRITE;
                        end
                    end else begin
                        mem_fsm <=  IDLE;
                    end
                end

                READ : begin
                    cmd_ack <= 1'b1;
                    mem_fsm <= IDLE;
                end

                WRITE: begin
                    cmd_ack <= 1'b1;
                    ram_mem[mem_cmd.byte_addr] <= mem_r_reg;
                    mem_fsm <= IDLE;
                end

                default: begin
                    mem_fsm <= IDLE; 
                    cmd_ack <= 1'b1;
                end


            endcase // mem_fsm
        end
    end




endmodule