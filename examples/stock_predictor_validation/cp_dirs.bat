#!/bin/csh

# Camada neural 1x2
foreach NL_WIDTH ( 1 )
	foreach NL_HEIGHT ( 2 )
		foreach SYNAPSES ( 32 64 128 256 512 )
			foreach IN_HEIGHT ( 22 44 66 88 176 )	
				foreach GAUSSIAN_DISTRIBUITON ( 2 3 4 )
					cp -r stock_predictor_random stock_predictor_random_${NL_WIDTH}_${NL_HEIGHT}_${SYNAPSES}_${IN_HEIGHT}_${GAUSSIAN_DISTRIBUITON}
					cd stock_predictor_random_${NL_WIDTH}_${NL_HEIGHT}_${SYNAPSES}_${IN_HEIGHT}_${GAUSSIAN_DISTRIBUITON}
					sed -r "s/const NL_WIDTH(.*?)= (.*?);/const NL_WIDTH = $NL_WIDTH;/"  ../stock_predictor_random/wnn_pred.con \
					| sed -r "s/const NL_HEIGHT(.*?)= (.*?);/const NL_HEIGHT = $NL_HEIGHT;/" \
					| sed -r "s/const SYNAPSES(.*?)= (.*?);/const SYNAPSES = $SYNAPSES;/" \
					| sed -r "s/const IN_HEIGHT(.*?)= (.*?);/const IN_HEIGHT = $IN_HEIGHT;/" \
					| sed -r "s/const GAUSSIAN_DISTRIBUITON(.*?)= (.*?);/const GAUSSIAN_DISTRIBUITON = $GAUSSIAN_DISTRIBUITON;/" > wnn_pred.con
					cd ..
					cp -r stock_predictor_gaussian stock_predictor_gaussian_${NL_WIDTH}_${NL_HEIGHT}_${SYNAPSES}_${IN_HEIGHT}_${GAUSSIAN_DISTRIBUITON}
					cd stock_predictor_gaussian_${NL_WIDTH}_${NL_HEIGHT}_${SYNAPSES}_${IN_HEIGHT}_${GAUSSIAN_DISTRIBUITON}
					sed -r "s/const NL_WIDTH(.*?)= (.*?);/const NL_WIDTH = $NL_WIDTH;/"  ../stock_predictor_gaussian/wnn_pred.con \
					| sed -r "s/const NL_HEIGHT(.*?)= (.*?);/const NL_HEIGHT = $NL_HEIGHT;/" \
					| sed -r "s/const SYNAPSES(.*?)= (.*?);/const SYNAPSES = $SYNAPSES;/" \
					| sed -r "s/const IN_HEIGHT(.*?)= (.*?);/const IN_HEIGHT = $IN_HEIGHT;/" \
					| sed -r "s/const GAUSSIAN_DISTRIBUITON(.*?)= (.*?);/const GAUSSIAN_DISTRIBUITON = $GAUSSIAN_DISTRIBUITON;/" > wnn_pred.con
					cd ..
				end
			end
		end	
	end
end

# Camada neural 2x4
foreach NL_WIDTH ( 2 )
	foreach NL_HEIGHT ( 4 )
		foreach SYNAPSES ( 32 64 128 256 512 )
			foreach IN_HEIGHT ( 22 44 66 88 176 )	
				foreach GAUSSIAN_DISTRIBUITON ( 2 3 4 )
					cp -r stock_predictor_random stock_predictor_random_${NL_WIDTH}_${NL_HEIGHT}_${SYNAPSES}_${IN_HEIGHT}_${GAUSSIAN_DISTRIBUITON}
					cd stock_predictor_random_${NL_WIDTH}_${NL_HEIGHT}_${SYNAPSES}_${IN_HEIGHT}_${GAUSSIAN_DISTRIBUITON}
					sed -r "s/const NL_WIDTH(.*?)= (.*?);/const NL_WIDTH = $NL_WIDTH;/"  ../stock_predictor_random/wnn_pred.con \
					| sed -r "s/const NL_HEIGHT(.*?)= (.*?);/const NL_HEIGHT = $NL_HEIGHT;/" \
					| sed -r "s/const SYNAPSES(.*?)= (.*?);/const SYNAPSES = $SYNAPSES;/" \
					| sed -r "s/const IN_HEIGHT(.*?)= (.*?);/const IN_HEIGHT = $IN_HEIGHT;/" \
					| sed -r "s/const GAUSSIAN_DISTRIBUITON(.*?)= (.*?);/const GAUSSIAN_DISTRIBUITON = $GAUSSIAN_DISTRIBUITON;/" > wnn_pred.con
					cd ..
					cp -r stock_predictor_gaussian stock_predictor_gaussian_${NL_WIDTH}_${NL_HEIGHT}_${SYNAPSES}_${IN_HEIGHT}_${GAUSSIAN_DISTRIBUITON}
					cd stock_predictor_gaussian_${NL_WIDTH}_${NL_HEIGHT}_${SYNAPSES}_${IN_HEIGHT}_${GAUSSIAN_DISTRIBUITON}
					sed -r "s/const NL_WIDTH(.*?)= (.*?);/const NL_WIDTH = $NL_WIDTH;/"  ../stock_predictor_gaussian/wnn_pred.con \
					| sed -r "s/const NL_HEIGHT(.*?)= (.*?);/const NL_HEIGHT = $NL_HEIGHT;/" \
					| sed -r "s/const SYNAPSES(.*?)= (.*?);/const SYNAPSES = $SYNAPSES;/" \
					| sed -r "s/const IN_HEIGHT(.*?)= (.*?);/const IN_HEIGHT = $IN_HEIGHT;/" \
					| sed -r "s/const GAUSSIAN_DISTRIBUITON(.*?)= (.*?);/const GAUSSIAN_DISTRIBUITON = $GAUSSIAN_DISTRIBUITON;/" > wnn_pred.con
					cd ..
				end
			end
		end	
	end
end

# Camada neural 4x8
foreach NL_WIDTH ( 4 )
	foreach NL_HEIGHT ( 8 )
		foreach SYNAPSES ( 32 64 128 256 512 )
			foreach IN_HEIGHT ( 22 44 66 88 176 )	
				foreach GAUSSIAN_DISTRIBUITON ( 2 3 4 )
					cp -r stock_predictor_random stock_predictor_random_${NL_WIDTH}_${NL_HEIGHT}_${SYNAPSES}_${IN_HEIGHT}_${GAUSSIAN_DISTRIBUITON}
					cd stock_predictor_random_${NL_WIDTH}_${NL_HEIGHT}_${SYNAPSES}_${IN_HEIGHT}_${GAUSSIAN_DISTRIBUITON}
					sed -r "s/const NL_WIDTH(.*?)= (.*?);/const NL_WIDTH = $NL_WIDTH;/"  ../stock_predictor_random/wnn_pred.con \
					| sed -r "s/const NL_HEIGHT(.*?)= (.*?);/const NL_HEIGHT = $NL_HEIGHT;/" \
					| sed -r "s/const SYNAPSES(.*?)= (.*?);/const SYNAPSES = $SYNAPSES;/" \
					| sed -r "s/const IN_HEIGHT(.*?)= (.*?);/const IN_HEIGHT = $IN_HEIGHT;/" \
					| sed -r "s/const GAUSSIAN_DISTRIBUITON(.*?)= (.*?);/const GAUSSIAN_DISTRIBUITON = $GAUSSIAN_DISTRIBUITON;/" > wnn_pred.con
					cd ..
					cp -r stock_predictor_gaussian stock_predictor_gaussian_${NL_WIDTH}_${NL_HEIGHT}_${SYNAPSES}_${IN_HEIGHT}_${GAUSSIAN_DISTRIBUITON}
					cd stock_predictor_gaussian_${NL_WIDTH}_${NL_HEIGHT}_${SYNAPSES}_${IN_HEIGHT}_${GAUSSIAN_DISTRIBUITON}
					sed -r "s/const NL_WIDTH(.*?)= (.*?);/const NL_WIDTH = $NL_WIDTH;/"  ../stock_predictor_gaussian/wnn_pred.con \
					| sed -r "s/const NL_HEIGHT(.*?)= (.*?);/const NL_HEIGHT = $NL_HEIGHT;/" \
					| sed -r "s/const SYNAPSES(.*?)= (.*?);/const SYNAPSES = $SYNAPSES;/" \
					| sed -r "s/const IN_HEIGHT(.*?)= (.*?);/const IN_HEIGHT = $IN_HEIGHT;/" \
					| sed -r "s/const GAUSSIAN_DISTRIBUITON(.*?)= (.*?);/const GAUSSIAN_DISTRIBUITON = $GAUSSIAN_DISTRIBUITON;/" > wnn_pred.con
					cd ..
				end
			end
		end	
	end
end

# Camada neural 8x16
foreach NL_WIDTH ( 8 )
	foreach NL_HEIGHT ( 16 )
		foreach SYNAPSES ( 32 64 128 256 512 )
			foreach IN_HEIGHT ( 22 44 66 88 176 )	
				foreach GAUSSIAN_DISTRIBUITON ( 2 3 4 )
					cp -r stock_predictor_random stock_predictor_random_${NL_WIDTH}_${NL_HEIGHT}_${SYNAPSES}_${IN_HEIGHT}_${GAUSSIAN_DISTRIBUITON}
					cd stock_predictor_random_${NL_WIDTH}_${NL_HEIGHT}_${SYNAPSES}_${IN_HEIGHT}_${GAUSSIAN_DISTRIBUITON}
					sed -r "s/const NL_WIDTH(.*?)= (.*?);/const NL_WIDTH = $NL_WIDTH;/"  ../stock_predictor_random/wnn_pred.con \
					| sed -r "s/const NL_HEIGHT(.*?)= (.*?);/const NL_HEIGHT = $NL_HEIGHT;/" \
					| sed -r "s/const SYNAPSES(.*?)= (.*?);/const SYNAPSES = $SYNAPSES;/" \
					| sed -r "s/const IN_HEIGHT(.*?)= (.*?);/const IN_HEIGHT = $IN_HEIGHT;/" \
					| sed -r "s/const GAUSSIAN_DISTRIBUITON(.*?)= (.*?);/const GAUSSIAN_DISTRIBUITON = $GAUSSIAN_DISTRIBUITON;/" > wnn_pred.con
					cd ..
					cp -r stock_predictor_gaussian stock_predictor_gaussian_${NL_WIDTH}_${NL_HEIGHT}_${SYNAPSES}_${IN_HEIGHT}_${GAUSSIAN_DISTRIBUITON}
					cd stock_predictor_gaussian_${NL_WIDTH}_${NL_HEIGHT}_${SYNAPSES}_${IN_HEIGHT}_${GAUSSIAN_DISTRIBUITON}
					sed -r "s/const NL_WIDTH(.*?)= (.*?);/const NL_WIDTH = $NL_WIDTH;/"  ../stock_predictor_gaussian/wnn_pred.con \
					| sed -r "s/const NL_HEIGHT(.*?)= (.*?);/const NL_HEIGHT = $NL_HEIGHT;/" \
					| sed -r "s/const SYNAPSES(.*?)= (.*?);/const SYNAPSES = $SYNAPSES;/" \
					| sed -r "s/const IN_HEIGHT(.*?)= (.*?);/const IN_HEIGHT = $IN_HEIGHT;/" \
					| sed -r "s/const GAUSSIAN_DISTRIBUITON(.*?)= (.*?);/const GAUSSIAN_DISTRIBUITON = $GAUSSIAN_DISTRIBUITON;/" > wnn_pred.con
					cd ..
				end
			end
		end	
	end
end


# Camada neural 16x32
foreach NL_WIDTH ( 16 )
	foreach NL_HEIGHT ( 32 )
		foreach SYNAPSES ( 32 64 128 256 512 )
			foreach IN_HEIGHT ( 22 44 66 88 176 )	
				foreach GAUSSIAN_DISTRIBUITON ( 2 3 4 )
					cp -r stock_predictor_random stock_predictor_random_${NL_WIDTH}_${NL_HEIGHT}_${SYNAPSES}_${IN_HEIGHT}_${GAUSSIAN_DISTRIBUITON}
					cd stock_predictor_random_${NL_WIDTH}_${NL_HEIGHT}_${SYNAPSES}_${IN_HEIGHT}_${GAUSSIAN_DISTRIBUITON}
					sed -r "s/const NL_WIDTH(.*?)= (.*?);/const NL_WIDTH = $NL_WIDTH;/"  ../stock_predictor_random/wnn_pred.con \
					| sed -r "s/const NL_HEIGHT(.*?)= (.*?);/const NL_HEIGHT = $NL_HEIGHT;/" \
					| sed -r "s/const SYNAPSES(.*?)= (.*?);/const SYNAPSES = $SYNAPSES;/" \
					| sed -r "s/const IN_HEIGHT(.*?)= (.*?);/const IN_HEIGHT = $IN_HEIGHT;/" \
					| sed -r "s/const GAUSSIAN_DISTRIBUITON(.*?)= (.*?);/const GAUSSIAN_DISTRIBUITON = $GAUSSIAN_DISTRIBUITON;/" > wnn_pred.con
					cd ..
					cp -r stock_predictor_gaussian stock_predictor_gaussian_${NL_WIDTH}_${NL_HEIGHT}_${SYNAPSES}_${IN_HEIGHT}_${GAUSSIAN_DISTRIBUITON}
					cd stock_predictor_gaussian_${NL_WIDTH}_${NL_HEIGHT}_${SYNAPSES}_${IN_HEIGHT}_${GAUSSIAN_DISTRIBUITON}
					sed -r "s/const NL_WIDTH(.*?)= (.*?);/const NL_WIDTH = $NL_WIDTH;/"  ../stock_predictor_gaussian/wnn_pred.con \
					| sed -r "s/const NL_HEIGHT(.*?)= (.*?);/const NL_HEIGHT = $NL_HEIGHT;/" \
					| sed -r "s/const SYNAPSES(.*?)= (.*?);/const SYNAPSES = $SYNAPSES;/" \
					| sed -r "s/const IN_HEIGHT(.*?)= (.*?);/const IN_HEIGHT = $IN_HEIGHT;/" \
					| sed -r "s/const GAUSSIAN_DISTRIBUITON(.*?)= (.*?);/const GAUSSIAN_DISTRIBUITON = $GAUSSIAN_DISTRIBUITON;/" > wnn_pred.con
					cd ..
				end
			end
		end	
	end
end


