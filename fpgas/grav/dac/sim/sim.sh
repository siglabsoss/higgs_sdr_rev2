#!/bin/bash

clear

rm -f *.log

$ACTIVEHDLBIN/vsimsa -l console.log -do ./sim.do 
