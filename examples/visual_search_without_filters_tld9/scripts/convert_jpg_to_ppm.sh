#!/bin/bash

for myfile in /dados/OnlineObjectTracking/Liquor/img/*
do
	if [ -d "$myfile" ]
	then
		echo "$myfile (dir)"
	else
		echo "$myfile"
		convert $myfile -compress None $myfile.pnm
	fi
done



