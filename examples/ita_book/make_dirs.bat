#!/bin/csh

#foreach NEURONS ( 1 )
#	foreach SYNAPSES ( 32 )
#		foreach WINDOW_SIZE ( 10 )
#			foreach STATISTICS ( 32 )
#				foreach CERTAINTY ( 15. )
foreach NEURONS ( 1 3 5 9 17 33 )
	foreach SYNAPSES ( 32 64 128 256 )
		foreach WINDOW_SIZE ( 5 10 15 20 )
#			foreach STATISTICS ( 3 5 10 15 20 )
			foreach STATISTICS ( 1 )
				foreach CERTAINTY ( 35. 45. 55. 65. 75. )

					cd ita_ind_conf_${NEURONS}_${SYNAPSES}_${WINDOW_SIZE}_${STATISTICS}_${CERTAINTY}
					make clean		
					make -f Makefile.no_interface
					cd ..
				end
			end
		end
	end
end

