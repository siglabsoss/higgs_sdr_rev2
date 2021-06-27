module vex_machine_top #(
                         parameter VMEM_DEPTH = 4096,
                         parameter SCALAR_MEM_0 = "scalar0.mif",
                         parameter SCALAR_MEM_1 = "scalar1.mif",
                         parameter SCALAR_MEM_2 = "scalar2.mif",
                         parameter SCALAR_MEM_3 = "scalar3.mif",
                         parameter VMEM0 = "vmem0.mif",
                         parameter VMEM1 = "vmem1.mif",
                         parameter VMEM2 = "vmem2.mif",
                         parameter VMEM3 = "vmem3.mif",
                         parameter VMEM4 = "vmem4.mif",
                         parameter VMEM5 = "vmem5.mif",
                         parameter VMEM6 = "vmem6.mif",
                         parameter VMEM7 = "vmem7.mif",
                         parameter VMEM8 = "vmem8.mif",
                         parameter VMEM9 = "vmem9.mif",
                         parameter VMEM10 = "vmem10.mif",
                         parameter VMEM11 = "vmem11.mif",
                         parameter VMEM12 = "vmem12.mif",
                         parameter VMEM13 = "vmem13.mif",
                         parameter VMEM14 = "vmem14.mif",
                         parameter VMEM15 = "vmem15.mif",
                         parameter NO_RISCV = 0,
                         parameter COUNTER_MODE = 0,
                         // Extra input fifo capacity.  this number will have 8 added to it
                         // so keep taht in mind if setting to a power of 2
                         parameter EXTRA_INPUT_CAPACITY = 0,
                         parameter EXTRA_OUTPUT_CAPACITY = 0,
                         parameter INPUT_VALID_READY_PRIME = 1,  // set to 0 to disable the buffer, and also disable "valid-ready prime"
                         parameter OUTPUT_VALID_READY_PRIME = 1  // should always be set to 1 in hardware
                         )

   (
    input              clk,
    input              reset,
    input              debugReset,

    input wire [31:0]  t0_data,
    input wire         t0_last,
    input wire         t0_valid,
    output wire        t0_ready,


    output wire [31:0] i0_data,
    output wire        i0_last,
    output wire        i0_valid,
    input wire         i0_ready,
    input wire [31:0]  outside_status,
    output wire [31:0] outside_control,

    //uart
    output wire        io_uart_txd,
    input wire         io_uart_rxd,
    
    // "Snap" these to the edge when under verilator
    // this is required due to delayed valid/ready signaling between "off chip" wires
`ifdef VERILATE_DEF
    output wire [31:0] snap_riscv_out_data,
    output wire        snap_riscv_out_last,
    output wire        snap_riscv_out_valid,
    output wire        snap_riscv_out_ready,

    output wire [31:0] snap_riscv_in_data,
    output wire        snap_riscv_in_last,
    output wire        snap_riscv_in_valid,
    output wire        snap_riscv_in_ready,
`endif

    input wire         sat_detect,
    inout [21:0]       gpio,
    input wire         i_ringbus,
    output reg         o_ringbus
    );

   wire [31:0]         riscv_in_data /* synthesis syn_noprune=1 */;
   wire                riscv_in_last /* synthesis syn_noprune=1 */;
   wire                riscv_in_valid /* synthesis syn_noprune=1 */;
   wire                riscv_in_ready /* synthesis syn_noprune=1 */; // output from t0_ready


   wire [31:0]         riscv_out_data /* synthesis syn_noprune=1 */;
   wire                riscv_out_last /* synthesis syn_noprune=1 */;
   wire                riscv_out_valid /* synthesis syn_noprune=1 */;
   wire                riscv_out_ready /* synthesis syn_noprune=1 */;

`ifdef VERILATE_DEF
   assign snap_riscv_out_data = riscv_out_data;
   assign snap_riscv_out_last = riscv_out_last;
   assign snap_riscv_out_valid = riscv_out_valid;
   assign snap_riscv_out_ready = riscv_out_ready;
   assign snap_riscv_in_data = riscv_in_data;
   assign snap_riscv_in_last = riscv_in_last;
   assign snap_riscv_in_valid = riscv_in_valid;
   assign snap_riscv_in_ready = riscv_in_ready;
`endif

   wire [31:0]         control;
   wire [31:0]         status;
   


   wire         out_temp_ready;


   //
   //  These two fifos allow for "valid ready prime" operation
   //  this is modified form of "valid read" operation where the
   //  ready signal has several clock cycles of delay
   //  this means that the receiving end must accept MORE data
   //  after putting ready low.  aka ready goes low early, but we still accept data
   //

   localparam EXTRA_CAPACITY_UPPER_MARGIN = 8;
   localparam EXTRA_CAPACITY_LOWER_MARGIN = 2;





    generate 
        if (INPUT_VALID_READY_PRIME == 1) begin

            reg [31:0]          temp_data;
            reg                 temp_last;
            reg                 temp_valid;
            wire                temp_ready;
            reg                 temp_ready_delay;

            always @(posedge clk) begin
                temp_data <= t0_data;
                temp_last <= t0_last;
                temp_valid <= t0_valid;
                temp_ready_delay <= temp_ready;
            end


   fwft_sc_fifo #(
                  .DEPTH        (EXTRA_CAPACITY_UPPER_MARGIN+EXTRA_INPUT_CAPACITY), // number of locations in the fifo
                  .WIDTH        (32 +1       ), // address width

                  .ALMOST_FULL  (EXTRA_CAPACITY_LOWER_MARGIN+EXTRA_INPUT_CAPACITY) // number of locations for afull to be active
                  ) in_buffer (
                               .clk          (clk         ),
                               .rst          (reset    ),
                               .wren         (temp_valid && temp_ready_delay),
                               .wdata        ({temp_last,temp_data}       ),
                               .o_afull_n    (t0_ready),
                               .o_afull_n_d  (temp_ready),
                               .rden         (riscv_in_ready),
                               .rdata        ({riscv_in_last, riscv_in_data}),
                               .rdata_vld    (riscv_in_valid));

        end else begin
            assign riscv_in_data = t0_data;
            assign riscv_in_last = t0_last;
            assign riscv_in_valid = t0_valid;
            assign t0_ready = riscv_in_ready;
        end
    endgenerate


    generate 
        if (OUTPUT_VALID_READY_PRIME == 1) begin
            
            reg                 out_buffer_read;

            always @(posedge clk) begin
                out_buffer_read <= i0_ready;
            end


   fwft_sc_fifo #(
                  .DEPTH        (EXTRA_CAPACITY_UPPER_MARGIN+EXTRA_OUTPUT_CAPACITY), // number of locations in the fifo
                  .WIDTH        (32 +1     ), // address width

                  .ALMOST_FULL  (EXTRA_CAPACITY_LOWER_MARGIN+EXTRA_OUTPUT_CAPACITY) // number of locations for afull to be active
                  ) out_buffer (
                                .clk          (clk),
                                .rst          (reset),
                                .wren         (riscv_out_valid && riscv_out_ready),
                                .wdata        ({riscv_out_last, riscv_out_data}),
                                .full          (),
                                .o_afull_n      (riscv_out_ready),
                                .rden         (out_buffer_read),
                                .rdata        ({i0_last, i0_data}),
                                .rdata_vld    (i0_valid));
        end else begin
            assign i0_data = riscv_out_data;
            assign i0_last = riscv_out_last;
            assign i0_valid = riscv_out_valid;
            assign riscv_out_ready = i0_ready;
        end
    endgenerate




   reg          in_ringbus;
   wire         out_ringbus;
   always @(posedge clk) begin
      in_ringbus <= i_ringbus;
      o_ringbus <= out_ringbus;
   end

   q_engine #(
              .VMEM_DEPTH (VMEM_DEPTH),
              .SCALAR_MEM_0 (SCALAR_MEM_0),
              .SCALAR_MEM_1 (SCALAR_MEM_1),
              .SCALAR_MEM_2 (SCALAR_MEM_2),
              .SCALAR_MEM_3 (SCALAR_MEM_3),
              .VMEM0 (VMEM0),
              .VMEM1 (VMEM1),
              .VMEM2 (VMEM2),
              .VMEM3 (VMEM3),
              .VMEM4 (VMEM4),
              .VMEM5 (VMEM5),
              .VMEM6 (VMEM6),
              .VMEM7 (VMEM7),
              .VMEM8 (VMEM8),
              .VMEM9 (VMEM9),
              .VMEM10 (VMEM10),
              .VMEM11 (VMEM11),
              .VMEM12 (VMEM12),
              .VMEM13 (VMEM13),
              .VMEM14 (VMEM14),
              .VMEM15 (VMEM15),
              .NO_RISCV (NO_RISCV)
              )
   q_engine_inst (
                  .clk                                (clk),
                  .srst                               (reset),
                  .debugReset                         (reset),

                  .t0_data                            (riscv_in_data),
                  .t0_last                            (riscv_in_last),
                  .t0_valid                           (riscv_in_valid),
                  .t0_ready                           (riscv_in_ready),

                  .i0_data                            (riscv_out_data),
                  .i0_last                            (riscv_out_last),
                  .i0_valid                           (riscv_out_valid),
                  .i0_ready                           (riscv_out_ready),

                  // uart
                  .io_uart_txd                        (io_uart_txd),
                  .io_uart_rxd                        (io_uart_rxd),

                  .proc_interrupt                     (),
                  .sat_detect                                                     (sat_detect),
                  .status(status),
                  .control(control),

                  .gpio                               (gpio),
                  .i_ringbus_0                            (in_ringbus),
                  .o_ringbus_0                            (out_ringbus)
                  );



   wire    riscv_in_err;
   wire    riscv_out_err;
   // 
   assign  riscv_in_err = riscv_in_valid && (~t0_ready);
   assign  riscv_out_err = (~i0_valid) && riscv_out_ready;


   generate 
      if (COUNTER_MODE == 0) begin
         assign status = outside_status;
         assign outside_control = control;
      end
   endgenerate

   generate 
      if (COUNTER_MODE == 1) begin
         counter counter (
                          .clk(clk),.reset(reset),
                          .clr(control[0]), .en(riscv_out_err),
                          .out(status)
                          );
      end
   endgenerate

   generate 
      if (COUNTER_MODE == 2) begin
         counter counter (
                          .clk(clk),.reset(reset),
                          .clr(control[0]), .en(riscv_in_err),
                          .out(status)
                          );
      end
   endgenerate

endmodule

module counter #(
                 parameter EXTRA_DELAY_REG = 3
                 ) (
                    input              clk, reset, clr, en,
                    output wire [31:0] out
                    );

   wire                                enable;
   reg [EXTRA_DELAY_REG-1:0]           en_reg;
   reg [31:0]                          count;

   
   generate
      if (EXTRA_DELAY_REG == 0) begin
         assign enable = en;
      end else begin
         always @(posedge clk) en_reg <= {en_reg[EXTRA_DELAY_REG-2:0],en};
         assign enable = en_reg[EXTRA_DELAY_REG-1];
      end
   endgenerate

   always @(posedge clk) begin
      if(reset) begin
         count <= 0;
      end 
      else begin
         if (enable) begin
            count <= count + 1;
         end

         if (clr) begin
            count <= 0;
         end 
      end
   end
   
   assign out = count;

endmodule

/* Example C Code
 void check_error_counter(void) {
 // CSR_WRITE(CS_CONTROL, err_state);
 unsigned int riscv_status;
 CSR_READ(CS_STATUS, riscv_status);
 unsigned int now;

 if( riscv_status > 0 ) {
 // set LED
 // CSR_SET_BITS(GPIO_WRITE, LED_GPIO_BIT);
 CSR_WRITE(CS_CONTROL, 0x1); // Send counter reset
 CSR_WRITE(CS_CONTROL, 0x0);
 // CSR_READ(TIMER_VALUE, now);
 // last_reset = now + 125000000;
 ring_block_send_eth_debug(riscv_status);
 }
 
 // pet_error_handler();
 }*/
