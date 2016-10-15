#!/bin/csh

foreach NEURONS ( 1 )
	foreach SYNAPSES ( 256 )
		foreach WINDOW_SIZE ( 10 )
			foreach STATISTICS ( 25 )
				foreach CERTAINTY ( 35. )

					rm -rf ita_ind_${NEURONS}_${SYNAPSES}_${WINDOW_SIZE}_${STATISTICS}_${CERTAINTY}

				end
			end
		end
	end
end




