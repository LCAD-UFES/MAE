#!/bin/bash

for myfile in /dados/TLD/TLD/07_motocross/*
do
	if [ -d "$myfile" ]
	then
		echo "$myfile (dir)"
	else
		echo "$myfile"
		convert $myfile -compress None $myfile.pnm
	fi
done
