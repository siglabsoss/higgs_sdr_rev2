#!/bin/bash
while true
do
	sleep $[ ( $RANDOM % 2 )  ]s
	nice -n 20 make quickt

	if [ $? -ne 0 ]
	then
		exit
	fi
done

