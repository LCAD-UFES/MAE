#!/bin/csh

foreach NEURONS (8 16 32 )
	foreach SYNAPSES ( 128 256 512 1024)

		rm -rf inflection_point_${NEURONS}_${SYNAPSES}

	end
end


