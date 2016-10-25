#!/bin/csh 

rm -rf ita_results_days.csv

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

					echo "32; ${NEURONS}; ${SYNAPSES}; ${WINDOW_SIZE}; ${STATISTICS}; ${CERTAINTY};" `tail -n 1 ita_ind_${NEURONS}_${SYNAPSES}_${WINDOW_SIZE}_${STATISTICS}_${CERTAINTY}/ita_days_32.csv` >>  ita_results_days.csv

					echo "64; ${NEURONS}; ${SYNAPSES}; ${WINDOW_SIZE}; ${STATISTICS}; ${CERTAINTY};" `tail -n 1 ita_ind_${NEURONS}_${SYNAPSES}_${WINDOW_SIZE}_${STATISTICS}_${CERTAINTY}/ita_days_64.csv` >>  ita_results_days.csv

					echo "128; ${NEURONS}; ${SYNAPSES}; ${WINDOW_SIZE}; ${STATISTICS}; ${CERTAINTY};" `tail -n 1 ita_ind_${NEURONS}_${SYNAPSES}_${WINDOW_SIZE}_${STATISTICS}_${CERTAINTY}/ita_days_128.csv` >>  ita_results_days.csv

					echo "256; ${NEURONS}; ${SYNAPSES}; ${WINDOW_SIZE}; ${STATISTICS}; ${CERTAINTY};" `tail -n 1 ita_ind_${NEURONS}_${SYNAPSES}_${WINDOW_SIZE}_${STATISTICS}_${CERTAINTY}/ita_days_256.csv` >>  ita_results_days.csv
				end
			end
		end
	end
end
