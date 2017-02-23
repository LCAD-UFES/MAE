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

					cp -r src ita_ind_conf_${NEURONS}_${SYNAPSES}_${WINDOW_SIZE}_${STATISTICS}_${CERTAINTY}
					cd ita_ind_conf_${NEURONS}_${SYNAPSES}_${WINDOW_SIZE}_${STATISTICS}_${CERTAINTY}
					sed -r "s/const OUT_HEIGHT(.*?)= (.*?);/const OUT_HEIGHT = $NEURONS;/" ../src/ita.con | sed -r "s/const SYNAPSES(.*?)= (.*?);/const SYNAPSES = $SYNAPSES;/" | sed -r "s/const INPUT_HEIGHT(.*?)= (.*?);/const INPUT_HEIGHT = $WINDOW_SIZE;/" | sed -r "s/const STATISTICS(.*?)= (.*?);/const STATISTICS = $STATISTICS;/" | sed -r "s/const CERTAINTY(.*?)= (.*?);/const CERTAINTY = $CERTAINTY;/" > ita.con
					cd ..
				end
			end
		end
	end
end
