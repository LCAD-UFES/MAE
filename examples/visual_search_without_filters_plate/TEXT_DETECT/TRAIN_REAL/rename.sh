#!/bin/bash

j=1
for i in *.ppm 
do 
	mv "$i" "$j".ppm
	j=$(expr $j + 1)
done

