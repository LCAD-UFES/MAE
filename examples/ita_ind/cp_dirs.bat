#!/bin/csh

foreach NEURONS ( 1 2 4 8 16 32 )
	foreach SYNAPSES ( 32 64 128 256 )
		foreach WINDOW_SIZE ( 5 10 15 20 )
			foreach STATISTICS ( 1 2 4 8 16 32 64 )
				foreach CERTAINTY ( 10. 15. 20. 25. 30. 35. )

					cp -r ita_ind ita_ind_${NEURONS}_${SYNAPSES}_${WINDOW_SIZE}_${STATISTICS}_${CERTAINTY}
					cd ita_ind_${NEURONS}_${SYNAPSES}_${WINDOW_SIZE}_${STATISTICS}_${CERTAINTY}
					sed -r "s/const OUT_HEIGHT(.*?)= (.*?);/const OUT_HEIGHT = $NEURONS;/" ../ita_ind/ita.con | sed -r "s/const SYNAPSES(.*?)= (.*?);/const SYNAPSES = $SYNAPSES;/" | sed -r "s/const INPUT_HEIGHT(.*?)= (.*?);/const INPUT_HEIGHT = $WINDOW_SIZE;/" | sed -r "s/const STATISTICS(.*?)= (.*?);/const STATISTICS = $STATISTICS;/" | sed -r "s/const CERTAINTY(.*?)= (.*?);/const CERTAINTY = $CERTAINTY;/" > ita.con
					cd ..
				end
			end
		end
	end
end


