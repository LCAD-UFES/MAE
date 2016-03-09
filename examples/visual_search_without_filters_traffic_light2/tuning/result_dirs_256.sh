#!/bin/bash
NL_WIDTH=64
NL_HEIGHT=48
const_=4

for INPUTS_PER_NEURON in 256 128 64 32 16; do 
	for GAUSSIAN_RADIUS in 1.25 2.5 5.0 7.5 10.0; do 
		for LOG_FACTOR in 2.0 4.0 6.0 8.0 10.0; do 
			w=NL_WIDTH
			h=NL_HEIGHT
			for ((c=const_; c>4; c/=2)); do 
				cd visual_search_test_$((w*c+1))_$((h*c+1))_${INPUTS_PER_NEURON}_${GAUSSIAN_RADIUS}_${LOG_FACTOR}
				echo visual_search_test_$((w*c+1))_$((h*c+1))_${INPUTS_PER_NEURON}_${GAUSSIAN_RADIUS}_${LOG_FACTOR}
				cat result_$((w*c+1))_$((h*c+1))_${INPUTS_PER_NEURON}_${GAUSSIAN_RADIUS}_${LOG_FACTOR}.txt >> ../result_over_all.csv
				cd ..
			done

			for ((w; w>=64; w/=2)); do 
				cd visual_search_test_$((w*c+1))_$((h*c+1))_${INPUTS_PER_NEURON}_${GAUSSIAN_RADIUS}_${LOG_FACTOR}
				echo visual_search_test_$((w*c+1))_$((h*c+1))_${INPUTS_PER_NEURON}_${GAUSSIAN_RADIUS}_${LOG_FACTOR}
				cat result_$((w*c+1))_$((h*c+1))_${INPUTS_PER_NEURON}_${GAUSSIAN_RADIUS}_${LOG_FACTOR}.txt >> ../result_over_all.csv
				cd ..
				h=$((h/2))
			done
		done
	done
done

# awk -F';' '/@/ {print $3";"$4";"$5";"$6";"$7}' result_over_all.csv >> result_for_plot_charts.csv
# awk -F';' '/@;C/ {print $3";"$4";"$5";"$6";"$7}' result_over_all.csv  >> result_for_plot_charts_only_c.csv

# awk '/@/ {print $0}' result_over_all.csv | cat >> result_over_all_only_sacades.csv
