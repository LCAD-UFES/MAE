#!/bin/csh

foreach NEURONS ( 2 4 8 16 32 64 )
        foreach SYNAPSES ( 32 64 128 256 )

		rm -rf face_recog_without_filters_random_${NEURONS}_${SYNAPSES}

		rm -rf face_recog_without_filters_random_retangular_em_pe_${NEURONS}_${SYNAPSES}

		rm -rf face_recog_without_filters_random_retangular_deitada_${NEURONS}_${SYNAPSES}

		rm -rf face_recog_without_filters_gaussian_${NEURONS}_${SYNAPSES}

		rm -rf face_recog_without_filters_gaussian_retangular_em_pe_${NEURONS}_${SYNAPSES}

		rm -rf face_recog_without_filters_gaussian_retangular_deitada_${NEURONS}_${SYNAPSES}

	end
end


