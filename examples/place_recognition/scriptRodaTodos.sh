#!/bin/csh

echo fatfast-100 em função de r
foreach NEURON_MEMORY_INDEX_SIZE ( 1 5 10 15 )

	echo fatfast_RAND_${NEURON_MEMORY_INDEX_SIZE}

	rm -rf place_recognition.con

	sed -r "s/set NEURON_MEMORY_INDEX_SIZE(.*?)= (.*?);/set NEURON_MEMORY_INDEX_SIZE = $NEURON_MEMORY_INDEX_SIZE;/" place_recognition.con.source | sed -r "s/minchinton/minchinton_fat_fast_rand/" > place_recognition.con

	make clean; make -f Makefile.no_interface VERBOSE=0

	mv place_recognition.con place_recognition_r_${NEURON_MEMORY_INDEX_SIZE}.con 

	./place_recognition place_recognition_original_100.cml > resultado_fatfastrand_100_r_${NEURON_MEMORY_INDEX_SIZE}.txt
end


echo fatfast-zip-r-alto
set NEURON_MEMORY_INDEX_SIZE = 15
foreach CLUSTERING ( kmeans )
	foreach COMPRESSION ( 001 045 075 )

		echo fatfast_${CLUSTERING}_${COMPRESSION}

		rm -rf place_recognition.con

		sed -r "s/set NEURON_MEMORY_INDEX_SIZE(.*?)= (.*?);/set NEURON_MEMORY_INDEX_SIZE = $NEURON_MEMORY_INDEX_SIZE;/" place_recognition.con.source | sed -r "s/minchinton/minchinton_fat_fast_rand/" > place_recognition.con

		make clean; make -f Makefile.no_interface VERBOSE=0

		mv place_recognition.con place_recognition_r_${NEURON_MEMORY_INDEX_SIZE}.con 

		./place_recognition place_recognition_${CLUSTERING}_${COMPRESSION}.cml > resultado_fatfastrand_${CLUSTERING}_${COMPRESSION}_r_${NEURON_MEMORY_INDEX_SIZE}.txt
	end
end


echo fatfast-100-r-baixo
set NEURON_MEMORY_INDEX_SIZE = 1
foreach CLUSTERING ( kmeans )
	foreach COMPRESSION ( 001 045 075 )

		echo fatfast_${CLUSTERING}_${COMPRESSION}

		rm -rf place_recognition.con

		sed -r "s/set NEURON_MEMORY_INDEX_SIZE(.*?)= (.*?);/set NEURON_MEMORY_INDEX_SIZE = $NEURON_MEMORY_INDEX_SIZE;/" place_recognition.con.source | sed -r "s/minchinton/minchinton_fat_fast_rand/" > place_recognition.con

		make clean; make -f Makefile.no_interface VERBOSE=0

		mv place_recognition.con place_recognition_r_${NEURON_MEMORY_INDEX_SIZE}.con 

		./place_recognition place_recognition_${CLUSTERING}_${COMPRESSION}.cml > resultado_fatfastrand_${CLUSTERING}_${COMPRESSION}_r_${NEURON_MEMORY_INDEX_SIZE}.txt
	end
end


echo vgram-zip
foreach CLUSTERING ( kmeans )
	foreach COMPRESSION ( 001 045 075 )
		 
		echo vgram_${CLUSTERING}_${COMPRESSION}

		cp place_recognition.con.source place_recognition.con

		make clean; make -f Makefile.no_interface VERBOSE=0

		./place_recognition place_recognition_${CLUSTERING}_${COMPRESSION}.cml > resultado_${CLUSTERING}_${COMPRESSION}.txt
	end
end


echo vgram_100

cp place_recognition.con.source place_recognition.con

make clean; make -f Makefile.no_interface VERBOSE=0

./place_recognition place_recognition_original_100.cml > resultado_original_100.txt

