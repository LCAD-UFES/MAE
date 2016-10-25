#!/bin/csh

#foreach NEURONS ( 1 )
#	foreach SYNAPSES ( 32 )
#		foreach WINDOW_SIZE ( 10 )
#			foreach STATISTICS ( 32 )
#				foreach CERTAINTY ( 15. )
foreach NEURONS ( 1 2 4 8 16 32 )
	foreach SYNAPSES ( 32 64 128 256 )
		foreach WINDOW_SIZE ( 5 10 15 20 )
			foreach STATISTICS ( 1 2 4 8 16 32 64 )
				foreach CERTAINTY ( 10. 15. 20. 25. 30. 35. )

					cd ita_ind_${NEURONS}_${SYNAPSES}_${WINDOW_SIZE}_${STATISTICS}_${CERTAINTY}
					echo ita_ind_${NEURONS}_${SYNAPSES}_${WINDOW_SIZE}_${STATISTICS}_${CERTAINTY}
					./ita_ind.sh
					cd ..

				end
			end
		end
	end
end
