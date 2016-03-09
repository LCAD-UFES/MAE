#!/bin/csh

foreach NEURONS ( 2 4 8 16 32 64 )
	foreach SYNAPSES ( 32 64 128 256 )

		cd face_recog_planar_random_${NEURONS}_${SYNAPSES}
		echo face_recog_planar_random_${NEURONS}_${SYNAPSES}
		qsub face_recog.sh
		cd ..

		cd face_recog_planar_gaussian_${NEURONS}_${SYNAPSES}
		echo face_recog_planar_gaussian_${NEURONS}_${SYNAPSES}
		qsub face_recog.sh
		cd ..
	end
end
