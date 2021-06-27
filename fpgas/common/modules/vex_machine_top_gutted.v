module vex_machine_top_gutted 
  #(
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
    parameter EXTRA_OUTPUT_CAPACITY = 0
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

    // Our input dma will always be ready, this allows
    // for upstream fpga to behave normally
    assign t0_ready = 1;

   reg                 in_ringbus;
   wire                out_ringbus;
   always @(posedge clk) begin
      in_ringbus <= i_ringbus;
      o_ringbus <= out_ringbus;
   end

   q_engine_gutted 
     #(
       .NO_RISCV (NO_RISCV)
       )
   q_engine_inst 
     (
      .clk                                (clk),
      .srst                               (reset),
      .debugReset                         (reset),

      .t0_data                            (),
      .t0_valid                           (),
      .t0_ready                           (),

      .i0_data                            (),
      .i0_valid                           (),
      .i0_ready                           (),

      //uart
      .io_uart_txd                         (io_uart_txd),
      .io_uart_rxd                         (io_uart_rxd),
      

      .proc_interrupt                     (),
      .sat_detect                                                     (),
      .status(),
      .control(),

      .gpio                               (),
      .i_ringbus_0                            (in_ringbus),
      .o_ringbus_0                            (out_ringbus)
      );

endmodule
