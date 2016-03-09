#!/bin/csh

set results = `find . -name "resultado_*.txt" -print`

foreach result ($results) 
	cat $result | grep Load | awk -F ':' '{printf "%f;", $2}'
	cat $result | grep "Average collision in train" | awk -F ':' '{printf "%f;", $2}'
	cat $result | grep "Average collision in test" | awk -F ':' '{printf "%f;", $2}'
	cat $result | grep "Average test time in ms" | awk -F ':' '{printf "%f;", $2}'
	cat $result | grep Total | awk -F ':' '{printf "%f;", $2}'
	cat $result | grep 10000 | awk -F ' ' '{printf "%f;", $3}'
	echo $result
end


