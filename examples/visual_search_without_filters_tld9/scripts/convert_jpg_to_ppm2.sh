#!/bin/bash

for myfile in /dados/vot2014/*
do
	for myfile2 in $myfile/*
	do
		if [ -d "$myfile2" ]
		then
			echo "$myfile2 (dir)"
		else
			echo "$myfile2"
			convert $myfile2 -compress None $myfile2.pnm
		fi
	done
done
