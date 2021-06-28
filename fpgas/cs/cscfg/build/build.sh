#!/bin/bash

clear

rm -f build.log

java -jar ../../../../ordt/Ordt.jar -parms ../../../../ordt/ordt_parms.txt -xml ../regs/regs.xml -reglist ../regs/regs.txt -systemverilog ../hdl/regs.sv ../regs/regs.rdl

if [ $? -ne 0 ];
then
    printf "\n\n\n\nORDT REGISTER CREATION FAILED!\n\n\n\n"
    exit 1
fi

set -o pipefail # need this to get the exit code of pnmainc and not the exit code of tee, which will always be 0

pnmainc build.tcl 2>&1 | tee build.log

if [ $? -ne 0 ];
then
    printf "\n\n\n\nBUILD FAILED! "
    pwd
    printf "\n\n\n\n"
    exit 1
fi

tail reports/timing_report.txt

# get rid of annoying ^M characters when viewing in vim
dos2unix build.log
dos2unix build.log

printf "\n\n\n\nBUILD SUCCEEDED!\n\n\n\n"