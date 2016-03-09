#!/bin/csh

foreach CHANNEL ( RED GREEN BLUE )
	cd "traffic_sign_$CHANNEL"	

	set results = `find . -name "*$CHANNEL.txt" -print`

	foreach result ($results) 
		cat $result | grep Load | awk -F ':' '{printf "%f;", $2}'
		cat $result | grep "Average collision in train" | awk -F ':' '{printf "%f;", $2}'
		cat $result | grep "Average collision in test" | awk -F ':' '{printf "%f;", $2}'
		cat $result | grep "Average test time in ms" | awk -F ':' '{printf "%f;", $2}'
		cat $result | grep Total | awk -F ':' '{printf "%f;", $2}'
		cat $result | grep 12630 | awk -F ' ' '{printf "%f;", $3}'
		echo $result
    	end

	cd ..
end
