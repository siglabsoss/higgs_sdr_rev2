module mapper_memory
  #(parameter DEPTH=8,
    parameter MEMINIT = "qam16.mif")
   (input wire [$clog2(DEPTH)-1:0] t_addr,
    input wire [31:0]  t_data,
    input              t_we,
    input              t_valid,
    output             t_ready,
    output wire [31:0] i_data,
    output reg         i_valid,
    input wire         i_ready,
    input              clk, rstf);

   wire                re;

   spram
     #(.DEPTH(DEPTH),
       .MEMINIT(MEMINIT))
   spram_inst
     (.addr(t_addr),
      .din(t_data),
      .dout(i_data),
      .we(t_we),
      .re(re),
      .clk(clk)
      );
   
   
   assign t_ready = ~i_valid | i_ready;
   assign re = (~t_we && t_valid) & t_ready;

   always  @(posedge clk or negedge rstf) begin
      if(~rstf) 
        i_valid <=0;
      else
        i_valid <= (~t_we && t_valid) | ~t_ready;
   end

endmodule // mapper_memory

/* verilator lint_off DECLFILENAME */
module spram 
  #(parameter DEPTH = 8,
    parameter MEMINIT = "mem.mif")
   (input wire [$clog2(DEPTH)-1:0] addr,
    input wire [31:0] din,
    input wire        we,
    input wire        re,
    output reg [31:0] dout,
    input             clk
    );

   reg [31:0]         mem [DEPTH-1:0]; /* synthesis syn_ramstyle = block_ram */
   
   always @(posedge clk) begin
      if(re)
        dout <= mem[addr];
      if(we) begin
         mem[addr] <= din;
         dout <= din;
      end
   end

   initial begin
      $readmemh(MEMINIT,mem);
   end
endmodule
