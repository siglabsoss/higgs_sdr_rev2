#!/usr/local/bin/wish

cd C:/FPGA/gravitinolink/experiments/tempy_temp/eth_mac/gbit_mac
lappend auto_path "C:/lscc/diamond/3.9_x64/tcltk/lib/ipwidgets/ispipbuilder/../runproc"
package require runcmd

if [runCmd "\"C:/lscc/diamond/3.9_x64/ispfpga/bin/nt64/ngdbuild\" -dt -a ecp5u -d LFE5U-85F -p \"C:/lscc/diamond/3.9_x64/ispfpga/sa5p00/data\" -p \".\" \"gbit_mac.ngo\" \"gbit_mac.ngd\""] {
   return
} else {
   vwait done
   if [checkResult $done] {
    return
   }
}
