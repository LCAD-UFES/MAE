#!/bin/csh

foreach NEURONS (8 16 32 )
	foreach SYNAPSES ( 128 256 512 1024)

		cp -r inflection_point inflection_point_${NEURONS}_${SYNAPSES}
		cd inflection_point_${NEURONS}_${SYNAPSES}
		sed -r "s/const OUT_WIDTH(.*?)= (.*?);/const OUT_WIDTH = $NEURONS;/"  ../inflection_point/inflection_point.con | sed -r "s/const OUT_HEIGHT(.*?)= (.*?);/const OUT_HEIGHT = $NEURONS;/" | sed -r "s/const SYNAPSES(.*?)= (.*?);/const SYNAPSES = $SYNAPSES;/" > inflection_point.con
		cd ..
	end
end


