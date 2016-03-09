#!/bin/bash
NL_WIDTH=32
NL_HEIGHT=24
const_=4

for INPUTS_PER_NEURON in 256 128 64 32 16; do 
	for GAUSSIAN_RADIUS in 1.25 2.5 5.0 7.5 10.0; do 
		for LOG_FACTOR in 2.0 4.0 6.0 8.0 10.0; do 
			w=NL_WIDTH
			h=NL_HEIGHT
			for ((c=const_; c>4; c/=2)); do 
				cd visual_search_test_$((w*c+1))_$((h*c+1))_${INPUTS_PER_NEURON}_${GAUSSIAN_RADIUS}_${LOG_FACTOR}
				echo visual_search_test_$((w*c+1))_$((h*c+1))_${INPUTS_PER_NEURON}_${GAUSSIAN_RADIUS}_${LOG_FACTOR}
				./visual_search visual_search.cml >> result_$((w*c+1))_$((h*c+1))_${INPUTS_PER_NEURON}_${GAUSSIAN_RADIUS}_${LOG_FACTOR}.txt
				cd ..
			done

			for ((w; w>=32; w/=2)); do 
				cd visual_search_test_$((w*c+1))_$((h*c+1))_${INPUTS_PER_NEURON}_${GAUSSIAN_RADIUS}_${LOG_FACTOR}
				echo visual_search_test_$((w*c+1))_$((h*c+1))_${INPUTS_PER_NEURON}_${GAUSSIAN_RADIUS}_${LOG_FACTOR}
				./visual_search visual_search.cml >> result_$((w*c+1))_$((h*c+1))_${INPUTS_PER_NEURON}_${GAUSSIAN_RADIUS}_${LOG_FACTOR}.txt
				cd ..
				h=$((h/2))
			done
		done
	done
done
