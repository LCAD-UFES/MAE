#!/bin/csh

foreach NEURONS ( 2 4 8 16 32 64 )
	foreach SYNAPSES ( 32 64 128 256 )

		cp -r face_recog_planar_random face_recog_planar_random_${NEURONS}_${SYNAPSES}
		cd face_recog_planar_random_${NEURONS}_${SYNAPSES}
		sed -r "s/const OUT_WIDTH(.*?)= (.*?);/const OUT_WIDTH = $NEURONS;/"  ../face_recog_planar_random/face_recog.con | sed -r "s/const OUT_HEIGHT(.*?)= (.*?);/const OUT_HEIGHT = $NEURONS;/" | sed -r "s/const SYNAPSES(.*?)= (.*?);/const SYNAPSES = $SYNAPSES;/" > face_recog.con
		cd ..

		cp -r face_recog_planar_gaussian face_recog_planar_gaussian_${NEURONS}_${SYNAPSES}		
		cd face_recog_planar_gaussian_${NEURONS}_${SYNAPSES}
		sed -r "s/const OUT_WIDTH(.*?)= (.*?);/const OUT_WIDTH = $NEURONS;/"  ../face_recog_planar_gaussian/face_recog.con | sed -r "s/const OUT_HEIGHT(.*?)= (.*?);/const OUT_HEIGHT = $NEURONS;/" | sed -r "s/const SYNAPSES(.*?)= (.*?);/const SYNAPSES = $SYNAPSES;/" > face_recog.con
		cd ..
	end
end


