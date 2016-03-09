#!/bin/csh

foreach NEURONS ( 2 4 8 16 32 64 )
	foreach SYNAPSES ( 32 64 128 256 )

		cp -r face_recog_without_filters_random face_recog_without_filters_random_${NEURONS}_${SYNAPSES}
		cd face_recog_without_filters_random_${NEURONS}_${SYNAPSES}
		sed -r "s/const OUT_WIDTH(.*?)= (.*?);/const OUT_WIDTH = $NEURONS;/"  ../face_recog_without_filters_random/face_recog.con | sed -r "s/const OUT_HEIGHT(.*?)= (.*?);/const OUT_HEIGHT = $NEURONS;/" | sed -r "s/const SYNAPSES(.*?)= (.*?);/const SYNAPSES = $SYNAPSES;/" > face_recog.con
		cd ..

		cp -r face_recog_without_filters_random_retangular_em_pe face_recog_without_filters_random_retangular_em_pe_${NEURONS}_${SYNAPSES}
		cd face_recog_without_filters_random_retangular_em_pe_${NEURONS}_${SYNAPSES}
		sed -r "s/const OUT_WIDTH(.*?)= (.*?);/const OUT_WIDTH = $NEURONS;/"  ../face_recog_without_filters_random_retangular_em_pe/face_recog.con | sed -r "s/const OUT_HEIGHT(.*?)= (.*?);/const OUT_HEIGHT = $NEURONS;/" | sed -r "s/const SYNAPSES(.*?)= (.*?);/const SYNAPSES = $SYNAPSES;/" > face_recog.con
		cd ..

		cp -r face_recog_without_filters_random_retangular_deitada face_recog_without_filters_random_retangular_deitada_${NEURONS}_${SYNAPSES}
		cd face_recog_without_filters_random_retangular_deitada_${NEURONS}_${SYNAPSES}
		sed -r "s/const OUT_WIDTH(.*?)= (.*?);/const OUT_WIDTH = $NEURONS;/"  ../face_recog_without_filters_random_retangular_deitada/face_recog.con | sed -r "s/const OUT_HEIGHT(.*?)= (.*?);/const OUT_HEIGHT = $NEURONS;/" | sed -r "s/const SYNAPSES(.*?)= (.*?);/const SYNAPSES = $SYNAPSES;/" > face_recog.con
		cd ..

		cp -r face_recog_without_filters_gaussian face_recog_without_filters_gaussian_${NEURONS}_${SYNAPSES}		
		cd face_recog_without_filters_gaussian_${NEURONS}_${SYNAPSES}
		sed -r "s/const OUT_WIDTH(.*?)= (.*?);/const OUT_WIDTH = $NEURONS;/"  ../face_recog_without_filters_gaussian/face_recog.con | sed -r "s/const OUT_HEIGHT(.*?)= (.*?);/const OUT_HEIGHT = $NEURONS;/" | sed -r "s/const SYNAPSES(.*?)= (.*?);/const SYNAPSES = $SYNAPSES;/" > face_recog.con
		cd ..

		cp -r face_recog_without_filters_gaussian_retangular_em_pe face_recog_without_filters_gaussian_retangular_em_pe_${NEURONS}_${SYNAPSES}
		cd face_recog_without_filters_gaussian_retangular_em_pe_${NEURONS}_${SYNAPSES}
		sed -r "s/const OUT_WIDTH(.*?)= (.*?);/const OUT_WIDTH = $NEURONS;/"  ../face_recog_without_filters_gaussian_retangular_em_pe/face_recog.con | sed -r "s/const OUT_HEIGHT(.*?)= (.*?);/const OUT_HEIGHT = $NEURONS;/" | sed -r "s/const SYNAPSES(.*?)= (.*?);/const SYNAPSES = $SYNAPSES;/" > face_recog.con
		cd ..

		cp -r face_recog_without_filters_gaussian_retangular_deitada face_recog_without_filters_gaussian_retangular_deitada_${NEURONS}_${SYNAPSES}
		cd face_recog_without_filters_gaussian_retangular_deitada_${NEURONS}_${SYNAPSES}
		sed -r "s/const OUT_WIDTH(.*?)= (.*?);/const OUT_WIDTH = $NEURONS;/"  ../face_recog_without_filters_gaussian_retangular_deitada/face_recog.con | sed -r "s/const OUT_HEIGHT(.*?)= (.*?);/const OUT_HEIGHT = $NEURONS;/" | sed -r "s/const SYNAPSES(.*?)= (.*?);/const SYNAPSES = $SYNAPSES;/" > face_recog.con
		cd ..
	end
end


