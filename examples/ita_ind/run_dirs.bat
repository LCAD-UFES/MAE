#!/bin/csh

foreach NEURONS ( 1 )
	foreach SYNAPSES ( 256 )
		foreach WINDOW_SIZE ( 10 )
			foreach STATISTICS ( 25 )
				foreach CERTAINTY ( 35. )

					cd ita_ind_${NEURONS}_${SYNAPSES}_${WINDOW_SIZE}_${STATISTICS}_${CERTAINTY}
					echo ita_ind_${NEURONS}_${SYNAPSES}_${WINDOW_SIZE}_${STATISTICS}_${CERTAINTY}
					./ita_ind.sh
					cd ..

				end
			end
		end
	end
end
