#!/bin/csh

echo fatfast-100 em função de r
foreach NEURON_MEMORY_INDEX_SIZE ( 1 5 10 15 20 25 30 )

	echo fatfast_RAND_${NEURON_MEMORY_INDEX_SIZE}

	rm -rf character.con

	sed -r "s/set NEURON_MEMORY_INDEX_SIZE(.*?)= (.*?);/set NEURON_MEMORY_INDEX_SIZE = $NEURON_MEMORY_INDEX_SIZE;/" character.con.source | sed -r "s/minchinton/minchinton_fat_fast_rand/" > character.con

	make clean; make -f Makefile.no_interface VERBOSE=0

	mv character.con character_r_${NEURON_MEMORY_INDEX_SIZE}.con 

	./character character_original_100.cml > resultado_fatfastrand_100_r_${NEURON_MEMORY_INDEX_SIZE}.txt
end


echo fatfast-zip-r-alto
set NEURON_MEMORY_INDEX_SIZE = 30
foreach CLUSTERING ( kmeans kmedoids )
	foreach COMPRESSION ( 025 050 001 010 )

		echo fatfast_${CLUSTERING}_${COMPRESSION}

		rm -rf character.con

		sed -r "s/set NEURON_MEMORY_INDEX_SIZE(.*?)= (.*?);/set NEURON_MEMORY_INDEX_SIZE = $NEURON_MEMORY_INDEX_SIZE;/" character.con.source | sed -r "s/minchinton/minchinton_fat_fast_rand/" > character.con

		make clean; make -f Makefile.no_interface VERBOSE=0

		mv character.con character_r_${NEURON_MEMORY_INDEX_SIZE}.con 

		./character character_${CLUSTERING}_${COMPRESSION}.cml > resultado_fatfastrand_${CLUSTERING}_${COMPRESSION}_r_${NEURON_MEMORY_INDEX_SIZE}.txt
	end
end


echo fatfast-100-r-baixo
set NEURON_MEMORY_INDEX_SIZE = 1
foreach CLUSTERING ( kmeans kmedoids )
	foreach COMPRESSION ( 025 050 001 010 )

		echo fatfast_${CLUSTERING}_${COMPRESSION}

		rm -rf character.con

		sed -r "s/set NEURON_MEMORY_INDEX_SIZE(.*?)= (.*?);/set NEURON_MEMORY_INDEX_SIZE = $NEURON_MEMORY_INDEX_SIZE;/" character.con.source | sed -r "s/minchinton/minchinton_fat_fast_rand/" > character.con

		make clean; make -f Makefile.no_interface VERBOSE=0

		mv character.con character_r_${NEURON_MEMORY_INDEX_SIZE}.con 

		./character character_${CLUSTERING}_${COMPRESSION}.cml > resultado_fatfastrand_${CLUSTERING}_${COMPRESSION}_r_${NEURON_MEMORY_INDEX_SIZE}.txt
	end
end


echo vgram-zip
foreach CLUSTERING ( kmeans kmedoids )
	foreach COMPRESSION ( 025 050 001 010 )
		 
		echo vgram_${CLUSTERING}_${COMPRESSION}

		cp character.con.source character.con

		make clean; make -f Makefile.no_interface VERBOSE=0

		./character character_${CLUSTERING}_${COMPRESSION}.cml > resultado_${CLUSTERING}_${COMPRESSION}.txt
	end
end


echo vgram_100

cp character.con.source character.con

make clean; make -f Makefile.no_interface VERBOSE=0

./character character_original_100.cml > resultado_original_100.txt

