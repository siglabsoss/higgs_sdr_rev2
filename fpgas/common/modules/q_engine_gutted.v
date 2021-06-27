module q_engine_gutted
  #(
    parameter NO_RISCV = 0
    ) 
   (input wire clk,
    input wire         srst,
    input wire         debugReset,
    //t0_stream
    input wire [31:0]  t0_data,
    input wire         t0_valid,
    output wire        t0_ready,
    //temporary
    output wire [31:0] i0_data,
    output wire        i0_valid,
    input wire         i0_ready,
    output wire        proc_interrupt,

    input wire         sat_detect,
    inout wire [21:0]  gpio,

    //uart
    output wire        io_uart_txd,
    input wire         io_uart_rxd,
    //control and status
    input wire [31:0]  status,
    output wire [31:0] control,

    output [1023:0]    mapmov_mover_active,
    output [31:0]      mapmov_trim_start,
    output [31:0]      mapmov_trim_end,
    output [9:0]       mapmov_pilot_ram_addr,
    output [31:0]      mapmov_pilot_ram_wdata,
    output             mapmov_pilot_ram_we,
    output             mapmov_reset, 
    output [15:0]      mapmov_one_value,
    output [15:0]      mapmov_zero_value,
   
    input              i_ringbus_0,
    output             o_ringbus_0,

    input              i_ringbus_1,
    output             o_ringbus_1,

    //jtag
    input              jtag_tms,
    input              jtag_tdi,
    output             jtag_tdo,
    input              jtag_tck
   
    );
   

   wire                ringbus_0_interrupt_clear;
   wire [31:0]         ringbus_0_write_data;
   wire [31:0]         ringbus_0_write_addr;
   wire                ringbus_0_write_done;
   wire                ringbus_0_write_en;
   wire [31:0]         ringbus_0_read_data;
   wire                ringbus_0_interrupt;
   wire                ringbus_0_rd_of;
   wire                ringbus_0_write_ready;

   wire                ringbus_1_interrupt_clear;
   wire [31:0]         ringbus_1_write_data;
   wire [31:0]         ringbus_1_write_addr;
   wire                ringbus_1_write_done;
   wire                ringbus_1_write_en;
   wire [31:0]         ringbus_1_read_data;
   wire                ringbus_1_interrupt;
   wire                ringbus_1_rd_of;
   wire                ringbus_1_write_ready;


   wire                ringbus_0_buf_empty;
   wire                ringbus_1_buf_empty;

   assign ringbus_0_interrupt_clear=1'b1;
   assign ringbus_1_interrupt_clear=1'b1;

   assign ringbus_0_interrupt = ~ringbus_0_buf_empty;
   assign ringbus_1_interrupt = ~ringbus_1_buf_empty;

   assign io_uart_txd = 1'b1;
   
   ring_bus ring_bus_inst
     (.i_sysclk(clk),         // 125 MHz
      .i_srst(srst),

      .i_wr_data(ringbus_0_write_data),
      .i_wr_addr(ringbus_0_write_addr),
      .o_done_wr(ringbus_0_write_done),
      .i_start_wr(ringbus_0_write_en),
      .o_write_ready(ringbus_0_write_ready),
      
      .o_rd_data(ringbus_0_read_data),
      .o_rd_buf_empty(ringbus_0_buf_empty),
      .o_rd_of(ringbus_0_rd_of),
      .i_clear_flags(ringbus_0_interrupt_clear), //clears all flags

      .o_serial_bus(o_ringbus_0),
      .i_serial_bus(i_ringbus_0)
      );

`ifdef EXTRA_RINGBUS
   ring_bus ring_bus_inst_2
     (.i_sysclk(clk),         // 125 MHz
      .i_srst(srst),

      .i_wr_data(ringbus_1_write_data),
      .i_wr_addr(ringbus_1_write_addr),
      .o_done_wr(ringbus_1_write_done),
      .i_start_wr(ringbus_1_write_en),
      .o_write_ready(ringbus_1_write_ready),
      
      .o_rd_data(ringbus_1_read_data),
      .o_rd_buf_empty(ringbus_1_buf_empty),
      .o_rd_of(ringbus_1_rd_of),
      .i_clear_flags(ringbus_1_interrupt_clear), //clears all flags

      .o_serial_bus(o_ringbus_1),
      .i_serial_bus(i_ringbus_1)
      );
`else // !`ifdef EXTRA_RINGBUS
   assign ringbus_1_buf_empty=1;
`endif //  `ifdef EXTRA_RINGBUS

endmodule
