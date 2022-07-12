#!/bin/bash
while true
do
	sleep $[ ( $RANDOM % 3 )  ]s
	make quickt

	if [ $? -ne 0 ]
	then
		exit
	fi
done

