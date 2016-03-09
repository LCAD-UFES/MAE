#!/bin/csh 

rm -rf character_results.csv

foreach NL_UNIDIM_SIZE ( 2 4 8 16 32 64 )
	foreach SYNAPSES ( 8 16 32 64 128 256 512 )
		foreach GAUSS_RAD ( 1.0 2.0 4.0 6.0 8.0 10.0 )

			sed -r "s/Test time in ms:(.*?);/ ${NL_UNIDIM_SIZE} ${SYNAPSES} ${GAUSS_RAD} Test time in ms:(.*?) \n ;/" character_test_${NL_UNIDIM_SIZE}_${SYNAPSES}_${GAUSS_RAD}.txt >> character_results.csv

		end
	end
end

# Test time in ms:320.312988
#
