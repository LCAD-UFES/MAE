#!/bin/csh

foreach N_SCALE ( 1 2 3 )
	foreach SYNAPSES ( 8 16 32 64 128 256 512 )
		foreach GAUSS_RAD ( 1.0 3.0 5.0 7.0 9.0 )
			foreach LOG_FACTOR ( 2.0 4.0 6.0 8.0 10.0 )
				foreach VOTE_PARAM (2.0 2.5 3.0)

rm -rf traffic_sign_test_${N_SCALE}_${SYNAPSES}_${GAUSS_RAD}_${LOG_FACTOR}_${VOTE_PARAM}

				end
			end
		end
	end
end


