#!/bin/csh 

rm -rf ita_results.txt

foreach NEURONS ( 1 )
	foreach SYNAPSES ( 256 )
		foreach WINDOW_SIZE ( 10 )
			foreach STATISTICS ( 25 )
				foreach CERTAINTY ( 35. )

					echo ${NEURONS} ${SYNAPSES} ${WINDOW_SIZE} ${STATISTICS} ${CERTAINTY} `tail -n 1 ita_ind_${NEURONS}_${SYNAPSES}_${WINDOW_SIZE}_${STATISTICS}_${CERTAINTY}/ita.test` >>  ita_results.txt

				end
			end
		end
	end
end
