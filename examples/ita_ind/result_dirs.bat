#!/bin/csh 

rm -rf ita_results_days_64.txt

foreach NEURONS ( 1 2 4 8 16 32 )
	foreach SYNAPSES ( 32 64 128 256 )
		foreach WINDOW_SIZE ( 5 10 15 20 )
			foreach STATISTICS ( 1 2 4 8 16 32 64 )
				foreach CERTAINTY ( 10. 15. 20. 25. 30. 35. )

					echo ${NEURONS} ${SYNAPSES} ${WINDOW_SIZE} ${STATISTICS} ${CERTAINTY} `tail -n 1 ita_ind_${NEURONS}_${SYNAPSES}_${WINDOW_SIZE}_${STATISTICS}_${CERTAINTY}/ita_days_64.test` >>  ita_results_days_64.txt

				end
			end
		end
	end
end
