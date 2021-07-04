`include "configurable_mif_paths.v"


module tb_higgs_top 
  (
      input wire         clk,
      input wire         MIB_MASTER_RESET,



      input wire [31:0]  i_data_adc,
      input wire         i_data_valid_adc,



      // port 30000
        // data bound for cs20
      input wire [31:0]  tx_turnstile_data_in,
      input wire         tx_turnstile_data_last, 
      input wire         tx_turnstile_data_valid,
      output wire        tx_turnstile_data_ready,

      // port 20000
        // data bound for input dma of eth's riscv
      input wire [31:0]  ringbus_in_data,
      input wire         ringbus_in_data_vld,
      output wire        ringbus_in_data_ready,

      // ETH UART
      output wire        snap_eth_io_uart_txd,
      input wire         snap_eth_io_uart_rxd,

      // port 10000
      output wire [31:0] ringbus_out_data,
      output wire        ringbus_out_data_vld,

      input wire         ring_bus_i0_ready,

      output wire        o_data_valid_dac,
`ifndef TB_USE_DAC
`ifdef VERILATE_MULTIPLE_HIGGS
      input wire         i_o_ready_dac,
`else
      output reg         i_o_ready_dac, // an output because we are controlling this from verilog not the tb
`endif
      output wire [31:0] o_data_dac,
`endif

`ifdef TB_USE_CS11
      output wire [31:0] snap_cs11_riscv_out_data,
      output wire        snap_cs11_riscv_out_last,
      output wire        snap_cs11_riscv_out_valid,
      output wire        snap_cs11_riscv_out_ready,

      output wire [31:0] snap_cs11_riscv_in_data,
      output wire        snap_cs11_riscv_in_valid,
      output wire        snap_cs11_riscv_in_ready,
      output wire        snap_cs11_io_uart_txd,
      output wire        snap_cs11_io_uart_rxd,
`endif

`ifdef TB_USE_CS01
      output wire [31:0] snap_cs01_riscv_out_data,
      output wire        snap_cs01_riscv_out_last,
      output wire        snap_cs01_riscv_out_valid,
      output wire        snap_cs01_riscv_out_ready,
      output wire        snap_cs01_io_uart_txd,
      output wire        snap_cs01_io_uart_rxd,
`endif

`ifdef TB_USE_CS31
      output wire [31:0] snap_cs31_riscv_out_data,
      output wire        snap_cs31_riscv_out_last,
      output wire        snap_cs31_riscv_out_valid,
      output wire        snap_cs31_riscv_out_ready,
      output wire        snap_cs31_io_uart_txd,
      output wire        snap_cs31_io_uart_rxd,

      output wire [31:0] snap_cs31_riscv_in_data,
      output wire        snap_cs31_riscv_in_last,
      output wire        snap_cs31_riscv_in_valid,
      output wire        snap_cs31_riscv_in_ready,
`endif

`ifdef TB_USE_CS32
      output wire [31:0] snap_cs32_riscv_out_data,
      output wire        snap_cs32_riscv_out_last, 
      output wire        snap_cs32_riscv_out_valid,
      output wire        snap_cs32_riscv_out_ready,
      output wire        snap_cs32_io_uart_txd,
      output wire        snap_cs32_io_uart_rxd,
`endif

`ifdef TB_USE_CS22
      output wire [31:0] snap_cs22_riscv_out_data,
      output wire        snap_cs22_riscv_out_last,
      output wire        snap_cs22_riscv_out_valid,
      output wire        snap_cs22_riscv_out_ready,

      input wire [31:0]  inject_cs22_riscv_in_data,
      input wire         inject_cs22_riscv_in_last,
      input wire         inject_cs22_riscv_in_valid,
      output wire        inject_cs22_riscv_in_ready,
      output wire        snap_cs22_io_uart_txd,
      output wire        snap_cs22_io_uart_rxd,
`endif

`ifdef TB_USE_CS21
      output wire [31:0] snap_cs21_riscv_out_data,
      output wire        snap_cs21_riscv_out_last,
      output wire        snap_cs21_riscv_out_valid,
      output wire        snap_cs21_riscv_out_ready,

      input wire [31:0]  inject_cs21_riscv_in_data,
      input wire         inject_cs21_riscv_in_last,
      input wire         inject_cs21_riscv_in_valid,
      output wire        inject_cs21_riscv_in_ready,
      output wire        snap_cs21_io_uart_txd,
      output wire        snap_cs21_io_uart_rxd,
`endif

`ifdef TB_USE_CS20
      output wire [31:0] snap_cs20_riscv_out_data,
      output wire        snap_cs20_riscv_out_last,
      output wire        snap_cs20_riscv_out_valid,
      output wire        snap_cs20_riscv_out_ready,
      output wire        snap_cs20_io_uart_txd,
      output wire        snap_cs20_io_uart_rxd,
`endif

`ifdef TB_USE_CS02
      output wire [31:0] snap_cs02_riscv_out_data,
      output wire        snap_cs02_riscv_out_last,
      output wire        snap_cs02_riscv_out_valid,
      output wire        snap_cs02_riscv_out_ready,
      output wire        snap_cs02_io_uart_txd,
      output wire        snap_cs02_io_uart_rxd,
`endif

`ifdef TB_USE_CS12
      output wire [31:0] snap_cs12_riscv_out_data,
      output wire        snap_cs12_riscv_out_last,
      output wire        snap_cs12_riscv_out_valid,
      output wire        snap_cs12_riscv_out_ready,
      output wire        snap_cs12_io_uart_txd,
      output wire        snap_cs12_io_uart_rxd,
`endif

`ifndef TB_USE_ADC
      input wire [31:0]  adc_data_out,
      input wire         adc_data_out_valid,
      output wire        adc_data_out_ready,
`endif

`ifdef ETH_USE_MEGA_WRAPPER
      input              MAC_RX_WRITE,
      input              MAC_RX_EOF,
      input [7:0]        MAC_RX_FIFODATA,
      output             MAC_TX_FIFOAVAIL,
      output [7:0]       MAC_TX_FIFODATA,
      output             MAC_TX_FIFOEOF,
      input              MAC_TX_MACREAD,
`endif

      output wire [31:0] snap_mapmov_in_data,
      output wire        snap_mapmov_in_valid,
      output wire        snap_mapmov_in_ready,

      //
      output wire [31:0] o_data_eth,
      output wire        o_data_valid_eth,
      output wire        o_data_last_eth,

      output wire [31:0] o_rx_data_eth,
      output wire        o_rx_valid_eth,
      input wire         i_rx_ready_eth,

      output             DAC_CTRL_SDIO, // technically an inout, but we currently don't support reads from DAC SIF
      output             DAC_CTRL_SDENN,
      output             DAC_CTRL_SCLK,
      output             DAC_CTRL_RESETN

        // only when we are not using the dac


      );


parameter VERILATE = 1'b1;

   logic [31:0] cs11_out_data;
   logic        cs11_o_data_last;
   logic        cs11_o_data_vld;
   logic        cs11_i_ready;
   logic        cs11_o_ringbus;

   logic [31:0] cs12_out_data;
   logic        cs12_o_data_last;
   logic        cs12_o_data_vld;
   logic        cs12_i_ready;
   logic        cs12_o_ringbus;

   logic [31:0] cs02_out_data;
   logic        cs02_o_data_last;
   logic        cs02_o_data_vld;
   logic        cs02_i_ready;
   logic        cs02_o_ringbus;

   logic        cs01_o_ringbus;

   logic [31:0] cs31_out_data;
   logic        cs31_o_data_last;
   logic        cs31_o_data_vld;
   logic        cs31_i_ready;
   logic        cs31_o_ringbus;

   logic [31:0] cs32_out_data;
   logic        cs32_o_data_last;
   logic        cs32_o_data_vld;
   logic        cs32_i_ready;
   logic        cs32_o_ringbus;

   logic [31:0] cs22_out_data;
   logic        cs22_o_data_last;
   logic        cs22_o_data_vld;
   logic        cs22_i_ready;
   logic        cs22_o_ringbus;

   logic [31:0] cs21_out_data;
   logic        cs21_o_data_last;
   logic        cs21_o_data_vld;
   logic        cs21_i_ready;
   logic        cs21_o_ringbus;

   logic [31:0] cs20_out_data;
   logic        cs20_o_data_last;
   logic        cs20_o_data_vld;
   logic        cs20_i_ready;
   logic        cs20_o_ringbus;

   logic [31:0]   i_data_eth;
   logic          i_data_valid_eth;
   logic          i_data_last_eth;
   logic          o_i_ready_eth;

   logic          eth_o_ringbus;
   logic          eth_i_ringbus;
   // logic [31:0]   o_data_eth;
   // logic       o_data_valid_eth;

   logic          i_o_ready_eth;

   logic          eth_rst;

   logic    [20:0] MIB_AD;


`ifdef TB_USE_DAC
    logic  [31:0]    o_data_dac;
    logic      o_data_valid_dac;
    logic         i_o_ready_dac;
`endif

`ifdef TB_USE_ADC
    wire [31:0] adc_data_out;
    wire  adc_data_out_valid;
`endif
    wire  adc_data_out_parity; // not really used anymore but still wired up



`ifdef TB_USE_CS11
   cs11_top #(.VERILATE (VERILATE),
              .SCALAR_MEM_0 (`CS11_SCALAR_0),
              .SCALAR_MEM_1 (`CS11_SCALAR_1),
              .SCALAR_MEM_2 (`CS11_SCALAR_2),
              .SCALAR_MEM_3 (`CS11_SCALAR_3),
              .VMEM0 (`CS11_VMEM0),
              .VMEM1 (`CS11_VMEM1),
              .VMEM2 (`CS11_VMEM2),
              .VMEM3 (`CS11_VMEM3),
              .VMEM4 (`CS11_VMEM4),
              .VMEM5 (`CS11_VMEM5),
              .VMEM6 (`CS11_VMEM6),
              .VMEM7 (`CS11_VMEM7),
              .VMEM8 (`CS11_VMEM8),
              .VMEM9 (`CS11_VMEM9),
              .VMEM10 (`CS11_VMEM10),
              .VMEM11 (`CS11_VMEM11),
              .VMEM12 (`CS11_VMEM12),
              .VMEM13 (`CS11_VMEM13),
              .VMEM14 (`CS11_VMEM14),
              .VMEM15 (`CS11_VMEM15)
)
      cs11_top (
         .CLK                                (clk),
         .FPGA_LED                           (),
         .HS_EAST_OUT                        ({cs11_o_data_vld,cs11_out_data}), // data valid + 32 bit data
         .HS_EAST_OUT_LAST                   (cs11_o_data_last),
         .HS_EAST_IN                         (cs11_i_ready), // ready
         // .LS_WEST_IN                         (eth_rst), // reset coming from eth
         .HS_WEST_IN_RB                      (eth_o_ringbus), // i_ringbus
         .HS_NORTH_OUT_RB                    (cs11_o_ringbus), // o_ringbus
         .HS_WEST_IN                         ({i_data_valid_eth,i_data_eth}), // data valid + 32 bit data
         .HS_WEST_IN_LAST                    (i_data_last_eth),
         .HS_WEST_OUT                        (o_i_ready_eth),  // ready

         .snap_riscv_out_data                (snap_cs11_riscv_out_data),
         .snap_riscv_out_last                (snap_cs11_riscv_out_last),
         .snap_riscv_out_valid               (snap_cs11_riscv_out_valid),
         .snap_riscv_out_ready               (snap_cs11_riscv_out_ready),
         .snap_riscv_in_data                 (snap_cs11_riscv_in_data),
         .snap_riscv_in_valid                (snap_cs11_riscv_in_valid),
         .snap_riscv_in_ready                (snap_cs11_riscv_in_ready),
         .snap_io_uart_txd                   (snap_cs11_io_uart_txd),
         .snap_io_uart_rxd                   (snap_cs11_io_uart_rxd),
         .MIB_MASTER_RESET (MIB_MASTER_RESET)
      );
`else
assign cs11_o_ringbus = 1'b1;
`endif

`ifdef TB_USE_CS12
   cs12_top #(.VERILATE (VERILATE),
              .SCALAR_MEM_0 (`CS12_SCALAR_0),
              .SCALAR_MEM_1 (`CS12_SCALAR_1),
              .SCALAR_MEM_2 (`CS12_SCALAR_2),
              .SCALAR_MEM_3 (`CS12_SCALAR_3),
              .VMEM0 (`CS12_VMEM0),
              .VMEM1 (`CS12_VMEM1),
              .VMEM2 (`CS12_VMEM2),
              .VMEM3 (`CS12_VMEM3),
              .VMEM4 (`CS12_VMEM4),
              .VMEM5 (`CS12_VMEM5),
              .VMEM6 (`CS12_VMEM6),
              .VMEM7 (`CS12_VMEM7),
              .VMEM8 (`CS12_VMEM8),
              .VMEM9 (`CS12_VMEM9),
              .VMEM10 (`CS12_VMEM10),
              .VMEM11 (`CS12_VMEM11),
              .VMEM12 (`CS12_VMEM12),
              .VMEM13 (`CS12_VMEM13),
              .VMEM14 (`CS12_VMEM14),
              .VMEM15 (`CS12_VMEM15))
      cs12_top (
         .CLK                                (clk),
         .FPGA_LED                           (),
         .HS_NORTH_OUT                       ({cs12_o_data_vld,cs12_out_data}), // data valid + 32 bit data
         .HS_NORTH_OUT_LAST                  (cs12_o_data_last),
         .HS_NORTH_IN                        (cs12_i_ready), // ready
         .HS_NORTH_IN_RB                     (cs02_o_ringbus), // i_ringbus
         .HS_SOUTH_OUT_RB                    (cs12_o_ringbus), // o_ringbus
         .HS_WEST_IN                         ({cs11_o_data_vld,cs11_out_data}), // data valid + 32 bit data
         .HS_WEST_IN_LAST                    (cs11_o_data_last),
         .HS_WEST_OUT                        (cs11_i_ready),   // ready

         .snap_riscv_out_data                (snap_cs12_riscv_out_data),
         .snap_riscv_out_last                (snap_cs12_riscv_out_last),
         .snap_riscv_out_valid               (snap_cs12_riscv_out_valid),
         .snap_riscv_out_ready               (snap_cs12_riscv_out_ready),
         .snap_io_uart_txd                   (snap_cs12_io_uart_txd),
         .snap_io_uart_rxd                   (snap_cs12_io_uart_rxd),
         .MIB_MASTER_RESET (MIB_MASTER_RESET)
      );
`else
assign cs12_o_ringbus = 1'b1;
`endif

`ifdef TB_USE_CS02
   cs02_top #(.VERILATE (VERILATE),
              .SCALAR_MEM_0 (`CS02_SCALAR_0),
              .SCALAR_MEM_1 (`CS02_SCALAR_1),
              .SCALAR_MEM_2 (`CS02_SCALAR_2),
              .SCALAR_MEM_3 (`CS02_SCALAR_3),
              .VMEM0 (`CS02_VMEM0),
              .VMEM1 (`CS02_VMEM1),
              .VMEM2 (`CS02_VMEM2),
              .VMEM3 (`CS02_VMEM3),
              .VMEM4 (`CS02_VMEM4),
              .VMEM5 (`CS02_VMEM5),
              .VMEM6 (`CS02_VMEM6),
              .VMEM7 (`CS02_VMEM7),
              .VMEM8 (`CS02_VMEM8),
              .VMEM9 (`CS02_VMEM9),
              .VMEM10 (`CS02_VMEM10),
              .VMEM11 (`CS02_VMEM11),
              .VMEM12 (`CS02_VMEM12),
              .VMEM13 (`CS02_VMEM13),
              .VMEM14 (`CS02_VMEM14),
              .VMEM15 (`CS02_VMEM15))
      cs02_top (
         .CLK                                (clk),
         .FPGA_LED                           (),
         .HS_WEST_OUT                        ({cs02_o_data_vld,cs02_out_data}), // data valid + 32 bit data
         .HS_WEST_OUT_LAST                   (cs02_o_data_last),
         .HS_WEST_IN                         (cs02_i_ready), // ready
         .HS_WEST_IN_RB                      (cs01_o_ringbus), // i_ringbus
         .HS_SOUTH_OUT_RB                    (cs02_o_ringbus), // o_ringbus
         .HS_SOUTH_IN                        ({cs12_o_data_vld,cs12_out_data}), // data valid + 32 bit data
         .HS_SOUTH_IN_LAST                   (cs12_o_data_last),
         .HS_SOUTH_OUT                       (cs12_i_ready),   // ready

         .snap_riscv_out_data                (snap_cs02_riscv_out_data),
         .snap_riscv_out_last                (snap_cs02_riscv_out_last),
         .snap_riscv_out_valid               (snap_cs02_riscv_out_valid),
         .snap_riscv_out_ready               (snap_cs02_riscv_out_ready),
         .snap_io_uart_txd                   (snap_cs02_io_uart_txd),
         .snap_io_uart_rxd                   (snap_cs02_io_uart_rxd),
         .MIB_MASTER_RESET (MIB_MASTER_RESET)
      );
`else
assign cs02_o_ringbus = 1'b1;
`endif

`ifdef TB_USE_CS01
   cs01_top #(.VERILATE (VERILATE),
              .SCALAR_MEM_0 (`CS01_SCALAR_0),
              .SCALAR_MEM_1 (`CS01_SCALAR_1),
              .SCALAR_MEM_2 (`CS01_SCALAR_2),
              .SCALAR_MEM_3 (`CS01_SCALAR_3),
              .VMEM0 (`CS01_VMEM0),
              .VMEM1 (`CS01_VMEM1),
              .VMEM2 (`CS01_VMEM2),
              .VMEM3 (`CS01_VMEM3),
              .VMEM4 (`CS01_VMEM4),
              .VMEM5 (`CS01_VMEM5),
              .VMEM6 (`CS01_VMEM6),
              .VMEM7 (`CS01_VMEM7),
              .VMEM8 (`CS01_VMEM8),
              .VMEM9 (`CS01_VMEM9),
              .VMEM10 (`CS01_VMEM10),
              .VMEM11 (`CS01_VMEM11),
              .VMEM12 (`CS01_VMEM12),
              .VMEM13 (`CS01_VMEM13),
              .VMEM14 (`CS01_VMEM14),
              .VMEM15 (`CS01_VMEM15))
      cs01_top (
         .CLK                                (clk),
         .FPGA_LED                           (),
         .HS_WEST_OUT                        ({o_data_valid_dac,o_data_dac}), // data valid + 32 bit data

         .HS_WEST_IN                         (i_o_ready_dac), // ready
         .HS_SOUTH_IN_RB                     (cs11_o_ringbus), // i_ringbus
         .HS_EAST_OUT_RB                     (cs01_o_ringbus), // o_ringbus
         .HS_EAST_IN                         ({cs02_o_data_vld,cs02_out_data}), // data valid + 32 bit data
         .HS_EAST_IN_LAST                    (cs02_o_data_last),
         .HS_EAST_OUT                        (cs02_i_ready),   // ready

         .snap_riscv_out_data                (snap_cs01_riscv_out_data),
         .snap_riscv_out_valid               (snap_cs01_riscv_out_valid),
         .snap_riscv_out_ready               (snap_cs01_riscv_out_ready),
         .snap_io_uart_txd                   (snap_cs01_io_uart_txd),
         .snap_io_uart_rxd                   (snap_cs01_io_uart_rxd),
         // .snap_riscv_in_data                 (snap_cs01_riscv_in_data),
         // .snap_riscv_in_valid                (snap_cs01_riscv_in_valid),
         // .snap_riscv_in_ready                (snap_cs01_riscv_in_ready),

         .MIB_MASTER_RESET (MIB_MASTER_RESET)
      );
`else
assign cs01_o_ringbus = 1'b1;
`endif

wire sat_detect;
logic [35:34] P2A_DDR_IN;
`ifdef TB_USE_CS31
   cs31_top #(.VERILATE (VERILATE),
              .SCALAR_MEM_0 (`CS31_SCALAR_0),
              .SCALAR_MEM_1 (`CS31_SCALAR_1),
              .SCALAR_MEM_2 (`CS31_SCALAR_2),
              .SCALAR_MEM_3 (`CS31_SCALAR_3),
              .VMEM0 (`CS31_VMEM0),
              .VMEM1 (`CS31_VMEM1),
              .VMEM2 (`CS31_VMEM2),
              .VMEM3 (`CS31_VMEM3),
              .VMEM4 (`CS31_VMEM4),
              .VMEM5 (`CS31_VMEM5),
              .VMEM6 (`CS31_VMEM6),
              .VMEM7 (`CS31_VMEM7),
              .VMEM8 (`CS31_VMEM8),
              .VMEM9 (`CS31_VMEM9),
              .VMEM10 (`CS31_VMEM10),
              .VMEM11 (`CS31_VMEM11),
              .VMEM12 (`CS31_VMEM12),
              .VMEM13 (`CS31_VMEM13),
              .VMEM14 (`CS31_VMEM14),
              .VMEM15 (`CS31_VMEM15))
      cs31_top (
         .CLK                                (clk),
         .FPGA_LED                           (),
         .HS_EAST_OUT                        ({cs31_o_data_vld,cs31_out_data}), // data valid + 32 bit data
         .HS_EAST_OUT_LAST                   (cs31_o_data_last),
         .HS_EAST_IN                         (cs31_i_ready), // ready
         .HS_EAST_IN_RB                      (cs32_o_ringbus), // i_ringbus
         .HS_NORTH_OUT_RB                    (cs31_o_ringbus), // o_ringbus
         .HS_WEST_OUT                        (P2A_DDR_IN),
         .HS_WEST_IN                         ({adc_data_out_valid, adc_data_out}), // data valid + 32 bit data
         .HS_WEST_IN_SAT                     (sat_detect),
         .snap_riscv_out_data                (snap_cs31_riscv_out_data),
         .snap_riscv_out_last                (snap_cs31_riscv_out_last),
         .snap_riscv_out_valid               (snap_cs31_riscv_out_valid),
         .snap_riscv_out_ready               (snap_cs31_riscv_out_ready),
         .snap_io_uart_txd                   (snap_cs31_io_uart_txd),
         .snap_io_uart_rxd                   (snap_cs31_io_uart_rxd),
         .snap_riscv_in_data                 (snap_cs31_riscv_in_data),
         .snap_riscv_in_last                 (snap_cs31_riscv_in_last),
         .snap_riscv_in_valid                (snap_cs31_riscv_in_valid),
         .snap_riscv_in_ready                (snap_cs31_riscv_in_ready),

         .MIB_MASTER_RESET (MIB_MASTER_RESET)
      );
`else
assign cs31_o_ringbus = 1'b1;
assign P2A_DDR_IN[34] = 1'b1; // always ready to adc in odd case that we DO have adc but no cs00
`endif

`ifdef TB_USE_CS32
   cs32_top #(.VERILATE (VERILATE),
              .SCALAR_MEM_0 (`CS32_SCALAR_0),
              .SCALAR_MEM_1 (`CS32_SCALAR_1),
              .SCALAR_MEM_2 (`CS32_SCALAR_2),
              .SCALAR_MEM_3 (`CS32_SCALAR_3),
              .VMEM0 (`CS32_VMEM0),
              .VMEM1 (`CS32_VMEM1),
              .VMEM2 (`CS32_VMEM2),
              .VMEM3 (`CS32_VMEM3),
              .VMEM4 (`CS32_VMEM4),
              .VMEM5 (`CS32_VMEM5),
              .VMEM6 (`CS32_VMEM6),
              .VMEM7 (`CS32_VMEM7),
              .VMEM8 (`CS32_VMEM8),
              .VMEM9 (`CS32_VMEM9),
              .VMEM10 (`CS32_VMEM10),
              .VMEM11 (`CS32_VMEM11),
              .VMEM12 (`CS32_VMEM12),
              .VMEM13 (`CS32_VMEM13),
              .VMEM14 (`CS32_VMEM14),
              .VMEM15 (`CS32_VMEM15))
      cs32_top (
         .CLK           (clk),
         .FPGA_LED         (),
`ifndef CS32_NO_RISCV
         .HS_NORTH_OUT                       ({cs32_o_data_vld,cs32_out_data}), // data valid + 32 bit data
         .HS_NORTH_OUT_LAST                  (cs32_o_data_last),
         .HS_NORTH_IN                        (cs32_i_ready), // ready
`else
         .HS_NORTH_OUT                       (),
         .HS_NORTH_OUT_LAST                  (),
         .HS_NORTH_IN                        (),
`endif
         .HS_NORTH_IN_RB                     (cs22_o_ringbus), // i_ringbus
         .HS_WEST_OUT_RB                     (cs32_o_ringbus), // o_ringbus
         .HS_WEST_IN                         ({cs31_o_data_vld,cs31_out_data}), // data valid + 32 bit data
         .HS_WEST_IN_LAST                    (cs31_o_data_last),
         .HS_WEST_OUT                        (cs31_i_ready),   // ready

         .snap_riscv_out_data                (snap_cs32_riscv_out_data),
         .snap_riscv_out_last                (snap_cs32_riscv_out_last),
         .snap_riscv_out_valid               (snap_cs32_riscv_out_valid),
         .snap_riscv_out_ready               (snap_cs32_riscv_out_ready),
         .snap_io_uart_txd                   (snap_cs32_io_uart_txd),
         .snap_io_uart_rxd                   (snap_cs32_io_uart_rxd),
         // .snap_riscv_in_data                 (snap_cs32_riscv_in_data),
         // .snap_riscv_in_valid                (snap_cs32_riscv_in_valid),
         // .snap_riscv_in_ready                (snap_cs32_riscv_in_ready),

         .MIB_MASTER_RESET (MIB_MASTER_RESET)
      );

`ifdef CS32_NO_RISCV
`ifdef TB_USE_CS22
assign inject_cs22_riscv_in_ready = cs32_i_ready;
assign cs32_o_data_vld = inject_cs22_riscv_in_valid;
assign cs32_out_data = inject_cs22_riscv_in_data;
assign cs32_o_data_last = inject_cs22_riscv_in_last;
`endif
`endif

`else
assign cs32_o_ringbus = 1'b1;
`endif




`ifdef TB_USE_CS22
   cs22_top #(.VERILATE (VERILATE),
              .SCALAR_MEM_0 (`CS22_SCALAR_0),
              .SCALAR_MEM_1 (`CS22_SCALAR_1),
              .SCALAR_MEM_2 (`CS22_SCALAR_2),
              .SCALAR_MEM_3 (`CS22_SCALAR_3),
              .VMEM0 (`CS22_VMEM0),
              .VMEM1 (`CS22_VMEM1),
              .VMEM2 (`CS22_VMEM2),
              .VMEM3 (`CS22_VMEM3),
              .VMEM4 (`CS22_VMEM4),
              .VMEM5 (`CS22_VMEM5),
              .VMEM6 (`CS22_VMEM6),
              .VMEM7 (`CS22_VMEM7),
              .VMEM8 (`CS22_VMEM8),
              .VMEM9 (`CS22_VMEM9),
              .VMEM10 (`CS22_VMEM10),
              .VMEM11 (`CS22_VMEM11),
              .VMEM12 (`CS22_VMEM12),
              .VMEM13 (`CS22_VMEM13),
              .VMEM14 (`CS22_VMEM14),
              .VMEM15 (`CS22_VMEM15))
      cs22_top (
         .CLK                                (clk),
         .FPGA_LED                           (),
`ifndef CS22_NO_RISCV
         .HS_WEST_OUT                        ({cs22_o_data_vld,cs22_out_data}), // data valid + 32 bit data
         .HS_WEST_OUT_LAST                   (cs22_o_data_last),
         .HS_WEST_IN                         (cs22_i_ready), // ready
`else
         .HS_WEST_OUT                        (),
         .HS_WEST_OUT_LAST                   (),
         .HS_WEST_IN                         (),
`endif

         .HS_NORTH_IN_RB                     (cs12_o_ringbus), // i_ringbus
         .HS_SOUTH_OUT_RB                    (cs22_o_ringbus), // o_ringbus
         .HS_SOUTH_IN                        ({cs32_o_data_vld,cs32_out_data}), // data valid + 32 bit data
         .HS_SOUTH_IN_LAST                   (cs32_o_data_last),
         .HS_SOUTH_OUT                       (cs32_i_ready),   // ready

         .snap_riscv_out_data                (snap_cs22_riscv_out_data),
         .snap_riscv_out_last                (snap_cs22_riscv_out_last),
         .snap_riscv_out_valid               (snap_cs22_riscv_out_valid),
         .snap_riscv_out_ready               (snap_cs22_riscv_out_ready),
         .snap_io_uart_txd                   (snap_cs22_io_uart_txd),
         .snap_io_uart_rxd                   (snap_cs22_io_uart_rxd),
         // .snap_riscv_in_data                 (snap_cs22_riscv_in_data),
         // .snap_riscv_in_valid                (snap_cs22_riscv_in_valid),
         // .snap_riscv_in_ready                (snap_cs22_riscv_in_ready),

         .MIB_MASTER_RESET (MIB_MASTER_RESET)
      );

`ifdef CS22_NO_RISCV
assign inject_cs21_riscv_in_ready = cs22_i_ready;
assign cs22_o_data_vld = inject_cs21_riscv_in_valid;
assign cs22_out_data = inject_cs21_riscv_in_data;
assign cs22_o_data_last = inject_cs21_riscv_in_last;
`endif

`else
assign cs22_o_ringbus = 1'b1;

`endif

`ifdef TB_USE_CS21
   cs21_top #(.VERILATE (VERILATE),
              .SCALAR_MEM_0 (`CS21_SCALAR_0),
              .SCALAR_MEM_1 (`CS21_SCALAR_1),
              .SCALAR_MEM_2 (`CS21_SCALAR_2),
              .SCALAR_MEM_3 (`CS21_SCALAR_3),
              .VMEM0 (`CS21_VMEM0),
              .VMEM1 (`CS21_VMEM1),
              .VMEM2 (`CS21_VMEM2),
              .VMEM3 (`CS21_VMEM3),
              .VMEM4 (`CS21_VMEM4),
              .VMEM5 (`CS21_VMEM5),
              .VMEM6 (`CS21_VMEM6),
              .VMEM7 (`CS21_VMEM7),
              .VMEM8 (`CS21_VMEM8),
              .VMEM9 (`CS21_VMEM9),
              .VMEM10 (`CS21_VMEM10),
              .VMEM11 (`CS21_VMEM11),
              .VMEM12 (`CS21_VMEM12),
              .VMEM13 (`CS21_VMEM13),
              .VMEM14 (`CS21_VMEM14),
              .VMEM15 (`CS21_VMEM15))
      cs21_top (
         .CLK                                (clk),
         .FPGA_LED                           (),
         .HS_WEST_OUT                        ({cs21_o_data_vld,cs21_out_data}), // data valid + 32 bit data
         .HS_WEST_OUT_LAST                   (cs21_o_data_last),
         .HS_WEST_IN                         (cs21_i_ready), // ready
         .HS_SOUTH_IN_RB                     (cs31_o_ringbus), // i_ringbus
         .HS_WEST_OUT_RB                     (cs21_o_ringbus), // o_ringbus
         .HS_EAST_IN                         ({cs22_o_data_vld,cs22_out_data}), // data valid + 32 bit data
         .HS_EAST_IN_LAST                    (cs22_o_data_last),
         .HS_EAST_OUT                        (cs22_i_ready),   // ready

         .snap_riscv_out_data                (snap_cs21_riscv_out_data),
         .snap_riscv_out_last                (snap_cs21_riscv_out_last),
         .snap_riscv_out_valid               (snap_cs21_riscv_out_valid),
         .snap_riscv_out_ready               (snap_cs21_riscv_out_ready),
         .snap_io_uart_txd                   (snap_cs21_io_uart_txd),
         .snap_io_uart_rxd                   (snap_cs21_io_uart_rxd),
         .MIB_MASTER_RESET (MIB_MASTER_RESET)
      );
`else
assign cs21_o_ringbus = 1'b1;
`endif

`ifdef TB_USE_CS20
   cs20_top #(.VERILATE (VERILATE),
              .SCALAR_MEM_0 (`CS20_SCALAR_0),
              .SCALAR_MEM_1 (`CS20_SCALAR_1),
              .SCALAR_MEM_2 (`CS20_SCALAR_2),
              .SCALAR_MEM_3 (`CS20_SCALAR_3),
              .VMEM0 (`CS20_VMEM0),
              .VMEM1 (`CS20_VMEM1),
              .VMEM2 (`CS20_VMEM2),
              .VMEM3 (`CS20_VMEM3),
              .VMEM4 (`CS20_VMEM4),
              .VMEM5 (`CS20_VMEM5),
              .VMEM6 (`CS20_VMEM6),
              .VMEM7 (`CS20_VMEM7),
              .VMEM8 (`CS20_VMEM8),
              .VMEM9 (`CS20_VMEM9),
              .VMEM10 (`CS20_VMEM10),
              .VMEM11 (`CS20_VMEM11),
              .VMEM12 (`CS20_VMEM12),
              .VMEM13 (`CS20_VMEM13),
              .VMEM14 (`CS20_VMEM14),
              .VMEM15 (`CS20_VMEM15))
      cs20_top (
         .CLK                                (clk),
         .FPGA_LED                           (),
         .HS_NORTH_OUT                       ({cs20_o_data_vld,cs20_out_data}), // data valid + 32 bit data
         .HS_NORTH_OUT_LAST                  (cs20_o_data_last),
         .HS_NORTH_IN                        (cs20_i_ready), // ready
         .HS_EAST_IN_RB                      (cs21_o_ringbus), // i_ringbus
         .HS_NORTH_OUT_RB                    (cs20_o_ringbus), // o_ringbus
         .HS_EAST_IN                         ({cs21_o_data_vld,cs21_out_data}), // data valid + 32 bit data
         .HS_EAST_IN_LAST                    (cs21_o_data_last),
         .HS_EAST_OUT                        (cs21_i_ready),   // ready

         .snap_riscv_out_data                (snap_cs20_riscv_out_data),
         .snap_riscv_out_last                (snap_cs20_riscv_out_last),
         .snap_riscv_out_valid               (snap_cs20_riscv_out_valid),
         .snap_riscv_out_ready               (snap_cs20_riscv_out_ready),
         .snap_io_uart_txd                   (snap_cs20_io_uart_txd),
         .snap_io_uart_rxd                   (snap_cs20_io_uart_rxd),
         // .snap_riscv_in_data                 (snap_cs20_riscv_in_data),
         // .snap_riscv_in_valid                (snap_cs20_riscv_in_valid),
         // .snap_riscv_in_ready                (snap_cs20_riscv_in_ready),

         .MIB_MASTER_RESET (MIB_MASTER_RESET)
      );
`else
assign cs20_o_ringbus = 1'b1;
`endif

   eth_top #(.VERILATE (VERILATE),
            .SCALAR_MEM_0 (`ETH_SCALAR_0),
            .SCALAR_MEM_1 (`ETH_SCALAR_1),
            .SCALAR_MEM_2 (`ETH_SCALAR_2),
            .SCALAR_MEM_3 (`ETH_SCALAR_3),
            .VMEM0 (`ETH_VMEM0),
            .VMEM1 (`ETH_VMEM1),
            .VMEM2 (`ETH_VMEM2),
            .VMEM3 (`ETH_VMEM3),
            .VMEM4 (`ETH_VMEM4),
            .VMEM5 (`ETH_VMEM5),
            .VMEM6 (`ETH_VMEM6),
            .VMEM7 (`ETH_VMEM7),
            .VMEM8 (`ETH_VMEM8),
            .VMEM9 (`ETH_VMEM9),
            .VMEM10 (`ETH_VMEM10),
            .VMEM11 (`ETH_VMEM11),
            .VMEM12 (`ETH_VMEM12),
            .VMEM13 (`ETH_VMEM13),
            .VMEM14 (`ETH_VMEM14),
            .VMEM15 (`ETH_VMEM15)

         )
      eth_top (
         .CLK           (clk),

         .LED_D4           (),
         .LED_D12       (),

         // .P1A_DDR_IN      ({o_data_valid_eth,o_data_eth}),
         // .P1A_DDR_OUT     (i_o_ready_eth),

            // connection with cs20
         // .P1B_DDR_OUT     ({i_data_valid_eth,i_data_eth}),
         // .P1B_DDR_IN      (o_i_ready_eth),

         // .P1B_SDR_OUT     (eth_rst),
         .HS_SOUTH_IN        ({cs20_o_data_vld,cs20_out_data}),
         .HS_SOUTH_OUT       (cs20_i_ready),
         .HS_EAST_OUT        ({i_data_valid_eth,i_data_eth}),
         .HS_EAST_IN         (o_i_ready_eth),
         .HS_SOUTH_IN_RB     (cs20_o_ringbus),
         .HS_EAST_OUT_RB     (eth_o_ringbus),
         .UART_TX            (snap_eth_io_uart_txd),
         .UART_RX            (snap_eth_io_uart_rxd),
         // .MIB_AD          (MIB_AD),

         .MIB_MASTER_RESET(MIB_MASTER_RESET),

            .snap_mapmov_in_data(snap_mapmov_in_data),
            .snap_mapmov_in_valid(snap_mapmov_in_valid),
            .snap_mapmov_in_ready(snap_mapmov_in_ready),

               .split_fb_data(o_rx_data_eth),
               .split_fb_valid(o_rx_valid_eth),
               .split_fb_ready(i_rx_ready_eth),
`ifdef ETH_USE_MEGA_WRAPPER
            .MAC_RX_WRITE(MAC_RX_WRITE),
            .MAC_RX_EOF(MAC_RX_EOF),
            .MAC_RX_FIFODATA(MAC_RX_FIFODATA),
            .MAC_TX_FIFOAVAIL(MAC_TX_FIFOAVAIL),
            .MAC_TX_FIFODATA(MAC_TX_FIFODATA),
            .MAC_TX_FIFOEOF(MAC_TX_FIFOEOF),
            .MAC_TX_MACREAD(MAC_TX_MACREAD),
`endif


         // port 30000
            // data bound for cs20
         .tx_turnstile_data_in     (tx_turnstile_data_in),
         .tx_turnstile_data_valid  (tx_turnstile_data_valid),
            .tx_turnstile_data_ready  (tx_turnstile_data_ready),

         // port 20000
            // data bound for input dma of eth's riscv
            // this is a ringbus bytes which eth will internally convert
            // and omit a ringbus message to it's partners. aka "bridge" udp->ringbus
         .ringbus_in_data          (ringbus_in_data),
         .ringbus_in_data_vld      (ringbus_in_data_vld),
            .ringbus_in_data_ready    (ringbus_in_data_ready),

         // port 10000
         .ringbus_out_data         (ringbus_out_data),
         .ringbus_out_data_vld     (ringbus_out_data_vld),

         .ring_bus_i0_ready        (ring_bus_i0_ready)

         );



   cfg_top #(.VERILATE (VERILATE))
       cfg_top (
         .CFG_CLK          (clk),
         .MIB_AD           (MIB_AD[20:17]),
         .DAC_CTRL_SDIO    (DAC_CTRL_SDIO),
         .DAC_CTRL_SDENN   (DAC_CTRL_SDENN),
         .DAC_CTRL_SCLK    (DAC_CTRL_SCLK),
         .DAC_CTRL_RESETN  (DAC_CTRL_RESETN)

         );


`ifdef TB_USE_DAC
    dac_top #(.VERILATE (VERILATE))
       dac_top (
             .FPGA0_CLK     (clk),
             .MIB_MASTER_RESET(MIB_MASTER_RESET),
             .HS_EAST_IN    ({o_data_valid_dac,o_data_dac}),
             .HS_EAST_OUT   (i_o_ready_dac)
             // .MIB_AD        (MIB_AD[16:15])
             );
`else


`ifndef VERILATE_MULTIPLE_HIGGS
// fake counter when dac FPGA is disabled,
// gives CS10 the ready signal every 4th clock cycle
reg [3:0] fake_dac_counter;

always_ff @(posedge clk) begin
    if (MIB_MASTER_RESET) begin
        fake_dac_counter <= 0;
        i_o_ready_dac <= 0;
    end else begin

        i_o_ready_dac <= 0;
        

        if(fake_dac_counter == 4'h0) begin
            fake_dac_counter <= 4'h1;
        end

        if(fake_dac_counter == 4'h1) begin
           fake_dac_counter <= 4'h2;
        end

        if(fake_dac_counter == 4'h2) begin
           fake_dac_counter <= 4'h3;
        end

        if(fake_dac_counter == 4'h3) begin
           fake_dac_counter <= 4'h0;
           i_o_ready_dac <= 1;
        end
    end
end
`endif


`endif


`ifdef TB_USE_ADC
   adc_top #(.VERILATE (VERILATE))
       adc_top (
             .FPGA1_CLK     (clk),
             .MIB_MASTER_RESET(MIB_MASTER_RESET),

             .i_data_adc      (i_data_adc),
             .i_data_valid_adc(i_data_valid_adc),
             .P2A_DDR_IN    (P2A_DDR_IN),

             .P2A_DDR_OUT     ({adc_data_out_valid, adc_data_out_parity, adc_data_out}),

             .MIB_AD        (MIB_AD[16:15]),
             .P2A_DDR_out   (sat_detect)
             );
`else
   // when adc is not present, TB controls input data.
    assign adc_data_out_parity = 'h0;
    assign adc_data_out_ready = P2A_DDR_IN[34];
    // assign adc_data_out_valid = 'h0;
    // assign adc_data_out = 'h0;
`endif

endmodule
