#!/bin/csh 

rm -rf face_recog_planar_random_results.txt
rm -rf face_recog_planar_gaussian_results.txt

foreach NEURONS ( 2 4 8 16 32 64 )
       foreach SYNAPSES ( 32 64 128 256  )

		echo ${NEURONS} ${SYNAPSES} `tail -n 1 face_recog_planar_random_${NEURONS}_${SYNAPSES}/face_recog.test` >>  face_recog_planar_random_results.txt

		echo ${NEURONS} ${SYNAPSES} `tail -n 1 face_recog_planar_gaussian_${NEURONS}_${SYNAPSES}/face_recog.test` >>  face_recog_planar_gaussian_results.txt
	end
end
