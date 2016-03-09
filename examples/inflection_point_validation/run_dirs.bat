#!/bin/csh

foreach NEURONS (8 16 32 )
	foreach SYNAPSES ( 128 256 512 1024)

		cd inflection_point_${NEURONS}_${SYNAPSES}
		echo inflection_point_${NEURONS}_${SYNAPSES}
		qsub inflection_point.sh
		cd ..

	end
end
