#!/bin/csh

foreach NEURONS (8 16 32 )
	foreach SYNAPSES ( 128 256 512 1024)

		cd inflection_point_${NEURONS}_${SYNAPSES}
		make clean		
		make -f Makefile.no_interface
		cd ..

	end
end
