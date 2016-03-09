#!/bin/csh 

echo "NL_WIDTH; NL_HEIGHT; SYNAPSES; IN_HEIGHT; GAUSSIAN_DISTRIBUITON; num_stocks; target_return; wnn_error; standard_deviation_target_return; standard_deviation_wnn_error; wnn_same_up; wnn_same_down; wnn_same_signal"

# Camada neural 2x4
foreach NL_WIDTH ( 2 )
	foreach NL_HEIGHT ( 4 )
		foreach SYNAPSES ( 32 64 128 256 512 )
			foreach IN_HEIGHT ( 22 44 88 176 352 )	
				foreach GAUSSIAN_DISTRIBUITON ( 1 2 3 4 )
					echo -n "${NL_WIDTH}; ${NL_HEIGHT}; ${SYNAPSES}; ${IN_HEIGHT}; ${GAUSSIAN_DISTRIBUITON}; "
					tail --lines=1 stock_predictor_${NL_WIDTH}_${NL_HEIGHT}_${SYNAPSES}_${IN_HEIGHT}_${GAUSSIAN_DISTRIBUITON}/average_signal_match_result.txt
				end
			end
		end	
	end
end

# Camada neural 4x8
foreach NL_WIDTH ( 4 )
	foreach NL_HEIGHT ( 8 )
		foreach SYNAPSES ( 32 64 128 256 512 )
			foreach IN_HEIGHT ( 22 44 88 176 352 )	
				foreach GAUSSIAN_DISTRIBUITON ( 1 2 3 4 )
					echo -n "${NL_WIDTH}; ${NL_HEIGHT}; ${SYNAPSES}; ${IN_HEIGHT}; ${GAUSSIAN_DISTRIBUITON}; "
					tail --lines=1 stock_predictor_${NL_WIDTH}_${NL_HEIGHT}_${SYNAPSES}_${IN_HEIGHT}_${GAUSSIAN_DISTRIBUITON}/average_signal_match_result.txt
				end
			end
		end	
	end
end

# Camada neural 8x16
foreach NL_WIDTH ( 8 )
	foreach NL_HEIGHT ( 16 )
		foreach SYNAPSES ( 32 64 128 256 512 )
			foreach IN_HEIGHT ( 22 44 88 176 352 )	
				foreach GAUSSIAN_DISTRIBUITON ( 1 2 3 4 )
					echo -n "${NL_WIDTH}; ${NL_HEIGHT}; ${SYNAPSES}; ${IN_HEIGHT}; ${GAUSSIAN_DISTRIBUITON}; "
					tail --lines=1 stock_predictor_${NL_WIDTH}_${NL_HEIGHT}_${SYNAPSES}_${IN_HEIGHT}_${GAUSSIAN_DISTRIBUITON}/average_signal_match_result.txt
				end
			end
		end	
	end
end


# Camada neural 16x32
foreach NL_WIDTH ( 16 )
	foreach NL_HEIGHT ( 32 )
		foreach SYNAPSES ( 32 64 128 256 512 )
			foreach IN_HEIGHT ( 22 44 88 176 352 )	
				foreach GAUSSIAN_DISTRIBUITON ( 1 2 3 4 )
					echo -n "${NL_WIDTH}; ${NL_HEIGHT}; ${SYNAPSES}; ${IN_HEIGHT}; ${GAUSSIAN_DISTRIBUITON}; "
					tail --lines=1 stock_predictor_${NL_WIDTH}_${NL_HEIGHT}_${SYNAPSES}_${IN_HEIGHT}_${GAUSSIAN_DISTRIBUITON}/average_signal_match_result.txt
				end
			end
		end	
	end
end


# Camada neural 32x64
foreach NL_WIDTH ( 32 )
	foreach NL_HEIGHT ( 64 )
		foreach SYNAPSES ( 32 64 128 256 512 )
			foreach IN_HEIGHT ( 22 44 88 176 352 )	
				foreach GAUSSIAN_DISTRIBUITON ( 1 2 3 4 )
					echo -n "${NL_WIDTH}; ${NL_HEIGHT}; ${SYNAPSES}; ${IN_HEIGHT}; ${GAUSSIAN_DISTRIBUITON}; "
					tail --lines=1 stock_predictor_${NL_WIDTH}_${NL_HEIGHT}_${SYNAPSES}_${IN_HEIGHT}_${GAUSSIAN_DISTRIBUITON}/average_signal_match_result.txt
				end
			end
		end	
	end
end

# Camada neural 64x128
foreach NL_WIDTH ( 64 )
	foreach NL_HEIGHT ( 128 )
		foreach SYNAPSES ( 32 64 128 256 512 )
			foreach IN_HEIGHT ( 22 44 88 176 352 )	
				foreach GAUSSIAN_DISTRIBUITON ( 1 2 3 4 )
					echo -n "${NL_WIDTH}; ${NL_HEIGHT}; ${SYNAPSES}; ${IN_HEIGHT}; ${GAUSSIAN_DISTRIBUITON}; "
					tail --lines=1 stock_predictor_${NL_WIDTH}_${NL_HEIGHT}_${SYNAPSES}_${IN_HEIGHT}_${GAUSSIAN_DISTRIBUITON}/average_signal_match_result.txt
				end
			end
		end	
	end
end

# Camada neural 128x256
foreach NL_WIDTH ( 128 )
	foreach NL_HEIGHT ( 256 )
		foreach SYNAPSES ( 32 64 128 256 512 )
			foreach IN_HEIGHT ( 22 44 88 176 352 )	
				foreach GAUSSIAN_DISTRIBUITON ( 1 2 3 4 )
					echo -n "${NL_WIDTH}; ${NL_HEIGHT}; ${SYNAPSES}; ${IN_HEIGHT}; ${GAUSSIAN_DISTRIBUITON}; "
					tail --lines=1 stock_predictor_${NL_WIDTH}_${NL_HEIGHT}_${SYNAPSES}_${IN_HEIGHT}_${GAUSSIAN_DISTRIBUITON}/average_signal_match_result.txt
				end
			end
		end	
	end
end

