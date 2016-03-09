#!/bin/bash

for myfile in /dados/TLD/TLD/09_carchase/*
do
	if [ -d "$myfile" ]
	then
		echo "$myfile (dir)"
	else
		echo "$myfile"
		convert $myfile -compress None $myfile.pnm
	fi
done
