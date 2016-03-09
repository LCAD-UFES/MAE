#!/bin/bash
NL_WIDTH=16
NL_HEIGHT=12
const_=4

for INPUTS_PER_NEURON in 256 128 64 32 16; do 
	for GAUSSIAN_RADIUS in 1.25 2.5 5.0 7.5 10.0; do 
		for LOG_FACTOR in 2.0 4.0 6.0 8.0 10.0; do 
			w=NL_WIDTH
			h=NL_HEIGHT
			for ((c=const_; c>1; c/=2)); do 
				#printf "visual_search_test_%dx%d_%d_%.2f_%d\n" $((w*c+1)) $((h*c+1)) $INPUTS_PER_NEURON $GAUSSIAN_RADIUS $LOG_FACTOR
				echo visual_search_test_$((w*c+1))_$((h*c+1))_${INPUTS_PER_NEURON}_${GAUSSIAN_RADIUS}_${LOG_FACTOR}
				cp -r visual_search_without_filters_tsd visual_search_test_$((w*c+1))_$((h*c+1))_${INPUTS_PER_NEURON}_${GAUSSIAN_RADIUS}_${LOG_FACTOR}
				cd visual_search_test_$((w*c+1))_$((h*c+1))_${INPUTS_PER_NEURON}_${GAUSSIAN_RADIUS}_${LOG_FACTOR}
				sed -r "s/const NL_WIDTH(.*?)= (.*?);/const NL_WIDTH = $((w*c+1));/" ../visual_search_without_filters_tsd/visual_search.con | sed -r "s/const NL_HEIGHT(.*?)= (.*?);/const NL_HEIGHT = $((h*c+1));/" | sed -r "s/const INPUTS_PER_NEURON(.*?)= (.*?);/const INPUTS_PER_NEURON = $INPUTS_PER_NEURON;/" | sed -r "s/const GAUSSIAN_RADIUS(.*?)= (.*?);/const GAUSSIAN_RADIUS = $GAUSSIAN_RADIUS;/" | sed -r "s/const LOG_FACTOR(.*?)= (.*?);/const LOG_FACTOR = $LOG_FACTOR;/" > visual_search.con
				cd ..
			done

			for ((w; w>=4; w/=2)); do 
				#printf "visual_search_test_%dx%d_%d_%.2f_%d\n" $((w*c+1)) $((h*c+1)) $INPUTS_PER_NEURON $GAUSSIAN_RADIUS $LOG_FACTOR
				echo visual_search_test_$((w*c+1))_$((h*c+1))_${INPUTS_PER_NEURON}_${GAUSSIAN_RADIUS}_${LOG_FACTOR}
				cp -r visual_search_without_filters_tsd visual_search_test_$((w*c+1))_$((h*c+1))_${INPUTS_PER_NEURON}_${GAUSSIAN_RADIUS}_${LOG_FACTOR}
				cd visual_search_test_$((w*c+1))_$((h*c+1))_${INPUTS_PER_NEURON}_${GAUSSIAN_RADIUS}_${LOG_FACTOR}
				sed -r "s/const NL_WIDTH(.*?)= (.*?);/const NL_WIDTH = $((w*c+1));/" ../visual_search_without_filters_tsd/visual_search.con | sed -r "s/const NL_HEIGHT(.*?)= (.*?);/const NL_HEIGHT = $((h*c+1));/" | sed -r "s/const INPUTS_PER_NEURON(.*?)= (.*?);/const INPUTS_PER_NEURON = $INPUTS_PER_NEURON;/" | sed -r "s/const GAUSSIAN_RADIUS(.*?)= (.*?);/const GAUSSIAN_RADIUS = $GAUSSIAN_RADIUS;/" | sed -r "s/const LOG_FACTOR(.*?)= (.*?);/const LOG_FACTOR = $LOG_FACTOR;/" > visual_search.con
				cd ..
				h=$((h/2))
			done
		done
	done
done
