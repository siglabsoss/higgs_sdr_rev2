#!/bin/bash
while true
do
	sleep $[ ( $RANDOM % 2 )  ]s
	make quickt

	if [ $? -ne 0 ]
	then
		exit
	fi
done

