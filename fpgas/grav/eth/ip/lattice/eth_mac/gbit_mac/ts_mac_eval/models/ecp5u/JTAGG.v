module JTAGG (
input TCK, TMS, TDI, JTDO2, JTDO1,
output TDO, JTDI, JTCK, JRTI2, JRTI1,
output JSHIFT, JUPDATE, JRSTN, JCE2, JCE1 )
/* synthesis syn_black_box syn_noprune=1 */;  //synthesis syn_black_box
parameter ER1 = "ENABLED";
parameter ER2 = "ENABLED";
endmodule
