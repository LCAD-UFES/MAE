#!/bin/bash

waitps()
{
	while pgrep -u $(whoami) visual_search > /dev/null; do 
		pgrep -u $(whoami) visual_search
	done
}


for i in 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 0.91 0.92 0.93 0.94 0.95 0.96 0.97 0.98 0.99; do
	rm input_data.txt output_data.txt
	cp cayo/inputs/input_data_${i}.txt input_data.txt
	./visual_search visual_search_teste_teste.cml &
	waitps
	cp output_data.txt cayo/outputs/output_data_${i}.txt
done
