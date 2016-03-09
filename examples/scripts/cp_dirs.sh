#!/bin/csh

foreach N_SCALE ( 1 2 3 )
	foreach SYNAPSES ( 8 16 32 64 128 256 512 )
		foreach GAUSS_RAD ( 1.0 3.0 5.0 7.0 9.0 )
			foreach LOG_FACTOR ( 2.0 4.0 6.0 8.0 10.0 )
				foreach VOTE_PARAM (2.0 2.5 3.0)

cp -r traffic_sign_3.7 traffic_sign_test_${N_SCALE}_${SYNAPSES}_${GAUSS_RAD}_${LOG_FACTOR}_${VOTE_PARAM}

echo traffic_sign_test_${N_SCALE}_${SYNAPSES}_${GAUSS_RAD}_${LOG_FACTOR}_${VOTE_PARAM}

cd traffic_sign_test_${N_SCALE}_${SYNAPSES}_${GAUSS_RAD}_${LOG_FACTOR}_${VOTE_PARAM}/traffic_sign_3.6_BLUE
sed -r "s/const N_SCALE(.*?)= (.*?);/const N_SCALE = $N_SCALE;/"  ../../traffic_sign_3.7/traffic_sign_3.6_BLUE/traffic_sign.con | sed -r "s/const SYNAPSES(.*?)= (.*?);/const SYNAPSES = $SYNAPSES;/" | sed -r "s/const GAUSSIAN_RADIUS_DISTRIBUTION(.*?)= (.*?);/const GAUSSIAN_RADIUS_DISTRIBUTION = $GAUSS_RAD;/" | sed -r "s/const LOG_FACTOR(.*?)= (.*?);/const LOG_FACTOR = $LOG_FACTOR;/" | sed -r "s/const VOTE_PARAM(.*?)= (.*?);/const VOTE_PARAM = $VOTE_PARAM;/" > traffic_sign.con
cd ..
cd traffic_sign_3.6_GREEN
sed -r "s/const N_SCALE(.*?)= (.*?);/const N_SCALE = $N_SCALE;/"  ../../traffic_sign_3.7/traffic_sign_3.6_GREEN/traffic_sign.con | sed -r "s/const SYNAPSES(.*?)= (.*?);/const SYNAPSES = $SYNAPSES;/" | sed -r "s/const GAUSSIAN_RADIUS_DISTRIBUTION(.*?)= (.*?);/const GAUSSIAN_RADIUS_DISTRIBUTION = $GAUSS_RAD;/" | sed -r "s/const LOG_FACTOR(.*?)= (.*?);/const LOG_FACTOR = $LOG_FACTOR;/" | sed -r "s/const VOTE_PARAM(.*?)= (.*?);/const VOTE_PARAM = $VOTE_PARAM;/" > traffic_sign.con
cd ..
cd traffic_sign_3.6_RED
sed -r "s/const N_SCALE(.*?)= (.*?);/const N_SCALE = $N_SCALE;/"  ../../traffic_sign_3.7/traffic_sign_3.6_RED/traffic_sign.con | sed -r "s/const SYNAPSES(.*?)= (.*?);/const SYNAPSES = $SYNAPSES;/" | sed -r "s/const GAUSSIAN_RADIUS_DISTRIBUTION(.*?)= (.*?);/const GAUSSIAN_RADIUS_DISTRIBUTION = $GAUSS_RAD;/" | sed -r "s/const LOG_FACTOR(.*?)= (.*?);/const LOG_FACTOR = $LOG_FACTOR;/" | sed -r "s/const VOTE_PARAM(.*?)= (.*?);/const VOTE_PARAM = $VOTE_PARAM;/" > traffic_sign.con
cd ..

cd ..

				end
			end
		end
	end
end


