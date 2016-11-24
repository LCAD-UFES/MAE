#!/bin/csh

#foreach NEURONS ( 1 )
#	foreach SYNAPSES ( 32 )
#		foreach WINDOW_SIZE ( 10 )
#			foreach STATISTICS ( 32 )
#				foreach CERTAINTY ( 15. )
foreach NEURONS ( 1 3 5 9 17 33 )
	foreach SYNAPSES ( 32 64 128 256 )
		foreach WINDOW_SIZE ( 4 6 8 10 )
			#foreach STATISTICS ( 0 1 2 4 )
				#foreach CERTAINTY ( 10. 15. 20. 25. 30. 35. )
			foreach STATISTICS ( 0 )
				foreach CERTAINTY ( 55. )

					cd ita_ind_${NEURONS}_${SYNAPSES}_${WINDOW_SIZE}_${STATISTICS}_${CERTAINTY}
					make clean		
					make -f Makefile.no_interface
					cd ..
				end
			end
		end
	end
end

