#!/bin/bash
NL_WIDTH=16
NL_HEIGHT=12
const_=4

for INPUTS_PER_NEURON in 256; do 
	for GAUSSIAN_RADIUS in 5.0; do 
		for LOG_FACTOR in 2.0; do 
			w=NL_WIDTH
			h=NL_HEIGHT
			for ((c=const_; c>1; c/=2)); do 
				cd visual_search_test_$((w*c+1))_$((h*c+1))_${INPUTS_PER_NEURON}_${GAUSSIAN_RADIUS}_${LOG_FACTOR}
				echo visual_search_test_$((w*c+1))_$((h*c+1))_${INPUTS_PER_NEURON}_${GAUSSIAN_RADIUS}_${LOG_FACTOR}
				./visual_search visual_search.cml >> result_$((w*c+1))_$((h*c+1))_${INPUTS_PER_NEURON}_${GAUSSIAN_RADIUS}_${LOG_FACTOR}.txt
				cd ..
			done

			for ((w; w>=4; w/=2)); do 
				cd visual_search_test_$((w*c+1))_$((h*c+1))_${INPUTS_PER_NEURON}_${GAUSSIAN_RADIUS}_${LOG_FACTOR}
				echo visual_search_test_$((w*c+1))_$((h*c+1))_${INPUTS_PER_NEURON}_${GAUSSIAN_RADIUS}_${LOG_FACTOR}
				./visual_search visual_search.cml >> result_$((w*c+1))_$((h*c+1))_${INPUTS_PER_NEURON}_${GAUSSIAN_RADIUS}_${LOG_FACTOR}.txt
				cd ..
				h=$((h/2))
			done
		done
	done
done
