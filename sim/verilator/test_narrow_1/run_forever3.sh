#!/bin/bash
while true
do
	sleep $[ ( $RANDOM % 2 )  ]s
	TEST_SELECT=3 make quickt

	if [ $? -ne 0 ]
	then
		exit
	fi
done

