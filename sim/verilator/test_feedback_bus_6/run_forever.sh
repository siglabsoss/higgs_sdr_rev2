#!/bin/bash
while true
do
	make quickt

	if [ $? -ne 0 ]
	then
		exit
	fi
done

