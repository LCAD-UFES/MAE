#!/bin/csh 

rm -rf face_recog_without_filters_random_results.txt
rm -rf face_recog_without_filters_gaussian_results.txt

foreach NEURONS ( 2 4 8 16 32 64 )
       foreach SYNAPSES ( 32 64 128 256  )

		echo ${NEURONS} ${SYNAPSES} `tail -n 1 face_recog_without_filters_random_${NEURONS}_${SYNAPSES}/face_recog.test` >>  face_recog_without_filters_random_results.txt

		echo ${NEURONS} ${SYNAPSES} `tail -n 1 face_recog_without_filters_random_retangular_em_pe_${NEURONS}_${SYNAPSES}/face_recog.test` >>  face_recog_without_filters_random_results.txt

		echo ${NEURONS} ${SYNAPSES} `tail -n 1 face_recog_without_filters_random_retangular_deitada_${NEURONS}_${SYNAPSES}/face_recog.test` >>  face_recog_without_filters_random_results.txt

		echo ${NEURONS} ${SYNAPSES} `tail -n 1 face_recog_without_filters_gaussian_${NEURONS}_${SYNAPSES}/face_recog.test` >>  face_recog_without_filters_gaussian_results.txt

		echo ${NEURONS} ${SYNAPSES} `tail -n 1 face_recog_without_filters_gaussian_retangular_em_pe_${NEURONS}_${SYNAPSES}/face_recog.test` >>  face_recog_without_filters_gaussian_results.txt

		echo ${NEURONS} ${SYNAPSES} `tail -n 1 face_recog_without_filters_gaussian_retangular_deitada_${NEURONS}_${SYNAPSES}/face_recog.test` >>  face_recog_without_filters_gaussian_results.txt

	end
end
