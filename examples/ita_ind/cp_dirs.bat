#!/bin/csh

foreach NEURONS ( 1 )
	foreach SYNAPSES ( 256 )
		foreach WINDOW_SIZE ( 10 )
			foreach STATISTICS ( 25 )
				foreach CERTAINTY ( 35. )

					cp -r ita_ind ita_ind_${NEURONS}_${SYNAPSES}_${WINDOW_SIZE}_${STATISTICS}_${CERTAINTY}
					cd ita_ind_${NEURONS}_${SYNAPSES}_${WINDOW_SIZE}_${STATISTICS}_${CERTAINTY}
					sed -r "s/const OUT_HEIGHT(.*?)= (.*?);/const OUT_HEIGHT = $NEURONS;/" ../ita_ind/ita.con | sed -r "s/const SYNAPSES(.*?)= (.*?);/const SYNAPSES = $SYNAPSES;/" | sed -r "s/const INPUT_HEIGHT(.*?)= (.*?);/const INPUT_HEIGHT = $WINDOW_SIZE;/" | sed -r "s/const STATISTICS(.*?)= (.*?);/const STATISTICS = $STATISTICS;/" | sed -r "s/const CERTAINTY(.*?)= (.*?);/const CERTAINTY = $CERTAINTY;/" > ita.con
					cd ..
				end
			end
		end
	end
end


