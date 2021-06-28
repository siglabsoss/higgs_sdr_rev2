module fb_eq_join
  #(
  parameter DROP_AFTER = 1024
  )(
   input wire [31:0] t_fb_data,
   input wire        t_fb_valid,
   input             t_fb_last,
   output reg        t_fb_ready,

   input wire [31:0] t_eq_data,
   input wire        t_eq_valid,
   input wire        t_eq_last,
   output reg        t_eq_ready,

   output reg [31:0] i_data,
   output reg        i_valid,
   output reg        i_last,
   input wire        i_ready,

   input wire        clk, rstf
   );

/// Joints two feedback bus messages
/// Actually a generic interface that joins them based on last signal
/// no actual feedback bus parsing goes on
///
/// q/n_sel are setup such that 0 means we are feeing an EQ (loop)
/// and 1 means we are feeding from the PC (output of mapmov)
///
/// n_blocked is only true if PC is in the middle of a transaction, and a loop
/// comes in, and is blocked (not the other way around)

   reg               q_active, n_active;
   reg               q_sel, n_sel; // 0=EQ (loop), 1=FB (pc)
   reg [31:0]        q_cnt, n_cnt;
   reg               q_blocked, n_blocked;
   reg [31:0]        q_stalcnt, n_stalcnt;
   reg               q_dumping, n_dumping;
   reg               q_stallovr, n_stallovr;

   reg  h_one;
   

   always @(*) begin
      
      i_data = !q_active ? 0:
               q_sel ? t_fb_data:t_eq_data;
      i_valid = !q_active ? 0:
                q_sel ? t_fb_valid:t_eq_valid;

      i_last = !q_active ? 0:
               q_sel ? t_fb_last:t_eq_last;
      
      // 1 is pc
      t_fb_ready = !q_active ? 0:
                   q_sel ? i_ready:0;

      // 0 is loop
      t_eq_ready = !q_active ? (q_dumping):
                   (~q_sel) ? (i_ready):
                    (q_dumping);

      // original ready condition
      // t_eq_ready = !q_active ? 0:
                   // q_sel ? 0:i_ready;

      // n_active goes low if our output is valid ready and last
      // meaning we sent the last word of the packet
      // on the next clock cycle, if q_active is low and one of our inputs is valid
      // we move to set its ready, however if we are dumping we don't want to do that
      n_active = (i_valid & i_last & i_ready) ? 0:
                 (~q_dumping & ~q_active & (t_fb_valid | t_eq_valid)) ? 1:q_active;

      // can't switch n_sel over to mode 0 if dumping
      // hit this when were blocked and dumping, then pc finished before
      // loop finished
      n_sel = (~q_active & t_eq_valid & ~q_dumping) ? 0:
                 (~q_active & t_fb_valid) ? 1:q_sel;

      n_cnt = (i_valid & i_ready) ? q_cnt + 1:q_cnt;

      n_blocked = (q_active & q_sel) & t_eq_valid;

      h_one = q_dumping && ~q_stallovr;

      n_stalcnt = (n_blocked && ~n_stallovr) ? (q_stalcnt + 1) : 
                ((q_dumping && ~n_stallovr) ? q_stalcnt : 0) ;

      n_stallovr = (t_eq_valid && t_eq_last);

      n_dumping = (q_stalcnt > DROP_AFTER) ? ~n_stallovr : 0;
      
   end

   always @(posedge clk) begin
      if(~rstf) begin
         q_active <= 0;
         q_sel <= 0;
         q_cnt <= 0;
         q_blocked <= 0;
         q_stalcnt <= 0;
         q_dumping <= 0;
         q_stallovr <= 0;
      end
      else begin
         q_active <= n_active;
         q_sel <= n_sel;
         q_cnt <= n_cnt;
         q_blocked <= n_blocked;
         q_stalcnt <= n_stalcnt;
         q_dumping <= n_dumping;
         q_stallovr <= n_stallovr;
      end
   end

endmodule

