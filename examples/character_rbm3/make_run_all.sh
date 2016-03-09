#!/bin/csh

rm -rf character_result.csv

foreach NL_UNIDIM_SIZE ( 2 4 8 16 32 64 )
	foreach SYNAPSES ( 8 16 32 64 128 256 512 )
		foreach GAUSS_RAD ( 1.0 2.0 4.0 6.0 8.0 10.0 )

rm -rf character.con

sed -r "s/const NL_UNIDIM_SIZE(.*?)= (.*?);/const NL_UNIDIM_SIZE = $NL_UNIDIM_SIZE;/" character.con.source | sed -r "s/const SYNAPSES(.*?)= (.*?);/const SYNAPSES = $SYNAPSES;/" | sed -r "s/const GAUSS_RAD(.*?)= (.*?);/const GAUSS_RAD = $GAUSS_RAD;/" > character.con


make -f Makefile.no_interface clean					
make -f Makefile.no_interface
./character character.cml > character_test_${NL_UNIDIM_SIZE}_${SYNAPSES}_${GAUSS_RAD}.txt
			
		end
	end
end
