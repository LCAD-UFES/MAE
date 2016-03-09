#!/bin/csh 

rm -rf inflection_point_results.txt

foreach NEURONS (8 16 32 )
	foreach SYNAPSES ( 128 256 512 1024)

		echo ${NEURONS} ${SYNAPSES} `tail -n 1 inflection_point_${NEURONS}_${SYNAPSES}/inflection_point.test` >>  inflection_point_results.txt

	end
end
