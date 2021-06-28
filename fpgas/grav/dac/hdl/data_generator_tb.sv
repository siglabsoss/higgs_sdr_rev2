module data_generator_tb;

	logic clk;
	logic rstf;
	logic [31:0] i_data;
	logic i_valid;
	logic i_ready;

	data_generator dg_0(
		.i_data(i_data),
		.i_valid(i_valid),
		.i_ready(i_ready),
		.clk(clk),
		.rstf(rstf)
	);


	assign #5ns clk = rstf?~clk:0;

	initial begin
		//clk=0;
		i_ready=1;
		rstf=0;
		#25ns;
		rstf=1;
		#10000;
		$finish;
	end


endmodule // data_generator_tb	
