/*
 * Module: tb_eth_top
 * 
 * TODO: Add module documentation
 */

 
module tb_eth_top;
 
    /* DUT SIGNALS */

    logic FPGA2_CLK;
    logic LED_D4;
    logic LED_D13;

    logic UDP_PKT_SOP; 
    logic UDP_PKT_EOP; 
    logic [7:0] UDP_PKT_BYTE; 
    logic UDP_PKT_AVAIL; 
    logic UDP_PKT_RD = 0;
    logic [33:0]  P1A_DDR = '0; // 33 = ADC Data Valid, 32 = Parity, 31:16 = ADC Sample 0, 15:0 = ADC Sample 1
    wire  [33:0]  P1B_DDR;

    logic              dac_fifo_wren = 0;
    logic              dac_fifo_rden = 0;
    logic              dac_fifo_full;
    logic [ 7:0]       dac_fifo_wdata;
    logic [31:0]       dac_fifo_rdata;
    logic              dac_fifo_rdata_vld;

    
    /* TEST BENCH SIGNALS */    
    
    logic [15:0] tb_sample_i = '0;
    logic [15:0] tb_sample_q = '0;

    
    /*
     * 
     * CLOCK GENERATION
     * 
     */
    
    initial begin
        FPGA2_CLK = 0;
        forever #4ns FPGA2_CLK = ~FPGA2_CLK;
    end
    
    
    /*
     * 
     * STIMULUS
     * 
     */
    
    // mimic continuous stream of periodic ADC data
    initial begin
        
        repeat (100) @(posedge FPGA2_CLK);
        
        forever begin
            @(posedge FPGA2_CLK);
            P1A_DDR[33]   <= 1;
            P1A_DDR[32]   <= ^{tb_sample_i, tb_sample_q};
            P1A_DDR[31:0] <= {tb_sample_i, tb_sample_q};
            tb_sample_i   <= tb_sample_i + 1;
            tb_sample_q   <= tb_sample_q + 1;
            @(posedge FPGA2_CLK);
            P1A_DDR[33] <= 0;
            repeat (1249) @(posedge FPGA2_CLK); // 100K samples/sec = 1250 125MHz clocks
        end
    end
    
    // mimic reading of fifo_dc by the ethernet mac
    initial begin
        
        forever begin
            @(posedge FPGA2_CLK);
            if (UDP_PKT_AVAIL) begin
                UDP_PKT_RD <= 1;
                repeat (6) @(posedge FPGA2_CLK); // I believe the ethernet MAC reads the first 6 bytes then drop the read signal for a clock or two before reading the rest of the packet
                UDP_PKT_RD <= 0;
                @(posedge FPGA2_CLK);
                UDP_PKT_RD <= 1;
                while (~UDP_PKT_EOP) begin
                    @(posedge FPGA2_CLK);
                end
                UDP_PKT_RD <= 0;
            end
        end
    end
    
    initial begin
        
        bit [31:0] dac_data = 0;
        
        @(negedge dac_fifo.wrclk_srst);
        @(posedge dac_fifo.wrclk);
        while (~dac_fifo_full) begin
            @(posedge dac_fifo.wrclk);
            dac_fifo_wren <= 1;
            dac_fifo_wdata <= dac_data[7:0];
            @(posedge dac_fifo.wrclk);
            dac_fifo_wdata <= dac_data[15:8];
            @(posedge dac_fifo.wrclk);
            dac_fifo_wdata <= dac_data[23:16];
            @(posedge dac_fifo.wrclk);
            dac_fifo_wdata <= dac_data[31:24];
            dac_data <= dac_data + 1;
        end
        @(posedge dac_fifo.wrclk);
        dac_fifo_wren <= 0;
    end
    
    initial begin
        @(negedge dac_fifo.rdclk_srst);
        while (~dac_fifo_full) begin
            @(posedge dac_fifo.rdclk);
        end
        @(posedge dac_fifo.rdclk);
        dac_fifo_rden <= 0;
        @(posedge dac_fifo.rdclk);
        dac_fifo_rden <= 1;
        repeat (2) @(posedge dac_fifo.rdclk);
        dac_fifo_rden <= 0;
        @(posedge dac_fifo.rdclk);
        dac_fifo_rden <= 1;
        @(posedge dac_fifo.rdclk);
        dac_fifo_rden <= 0;
        @(posedge dac_fifo.rdclk);
        dac_fifo_rden <= 1;
        @(posedge dac_fifo.rdclk);
        dac_fifo_rden <= 0;
        @(posedge dac_fifo.rdclk);
        dac_fifo_rden <= 1;
        repeat (10) @(posedge dac_fifo.rdclk);
        dac_fifo_rden <= 0;
        repeat (10) @(posedge dac_fifo.rdclk);
        dac_fifo_rden <= 1;
        repeat (50) @(posedge dac_fifo.rdclk);
        dac_fifo_rden <= 0;
        repeat (10) @(posedge dac_fifo.rdclk);
        dac_fifo_rden <= 1;
        
        while (~dac_fifo.fifo_empty) begin
            @(posedge dac_fifo.rdclk);
        end
        @(posedge dac_fifo.rdclk);
        dac_fifo_rden <= 0;
    end
    
    initial begin
        bit [31:0] expected_data = 0;
        forever begin
            @(posedge dac_fifo.rdclk);
            if (dac_fifo_rdata_vld & dac_fifo_rden) begin
                if (dac_fifo_rdata != expected_data) begin
                    $fatal(0, "Error! DAC FIFO gave incorrect data output. Expected %d, Received %d", expected_data, dac_fifo_rdata);
                end
                expected_data <= expected_data + 1;
            end
        end
    end
    
    // prevents the simulation from running forever
    initial begin
        #10ms;
        $finish();
    end

    GSR     GSR_INST (.GSR (tb_gsr));
    PUR     PUR_INST (.PUR (tb_pur));
    eth_top DUT (.*);
        
    /*
     * Shoe-horn testing of DAC fwft FIFO
     */
    
    pmi_fifo_dc_fwft dac_fifo (
        .wrclk        (DUT.clk125),
        .wrclk_srst   (DUT.clk125_srst),
        .rdclk        (DUT.clk125),
        .rdclk_srst   (DUT.clk125_srst),
        .wren         (dac_fifo_wren),
        .rden         (dac_fifo_rden),
        .wdata        (dac_fifo_wdata),
        .full         (dac_fifo_full),
        .rdata        (dac_fifo_rdata),
        .rdata_vld    (dac_fifo_rdata_vld));

endmodule


