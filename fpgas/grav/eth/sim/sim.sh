#!/bin/bash
clear

rm -f *.log

vsimsa -l console.log -do ./sim.do 
