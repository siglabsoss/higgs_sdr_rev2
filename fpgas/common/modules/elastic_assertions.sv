module elastic_assertions#(parameter DWIDTH=32)
   (input wire [DWIDTH-1:0] data, 
    input wire valid, 
    input wire ready,
    input wire clk, rstf);

   logic [DWIDTH-1:0] prev_data,prev2_data = 0;
   logic       data_stable = 0;
   logic       valid_stable = 0;
   logic       ready_stable = 0;
   
   logic       prev_valid, prev2_valid;
   logic       prev_ready, prev2_ready;
   
   
   always @(posedge clk)
     if(rstf) begin
        prev_data <= data;
        prev2_data <= prev_data;
        prev_valid <= valid;
        prev2_valid <= prev_valid;
        
        prev_ready <= ready;
        prev2_ready <= prev_ready;
        
        data_stable <= (prev_data == data);
        valid_stable <= (prev_valid == valid);

        ready_stable <= (prev_ready == ready);
        
        
        if(prev2_valid == 1 && prev_valid == 1 && prev_ready == 0 && ready == 0) 
          assert(data_stable) else
            $error("Unstable data");
           
        if(prev_valid == 1 && prev_ready == 0)
          assert(valid) else
            $error("Valid went low without ready begin high");

        if(prev_ready == 1 && prev_valid == 0)
          assert(ready) else
            $error("Ready went low without valid being high");
        
        //assert(valid && !ready && data_stable) else
        //$display("error");
        
         //else $warning("Data changed when valid was high and ready was low");
//        assert(!valid && ready && $stable(ready))
  //        else $warning("Ready went low when valid was low");
     end

endmodule
