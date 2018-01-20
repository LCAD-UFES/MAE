#!/bin/csh

echo fatfast-100 em funcao de r
foreach NEURON_MEMORY_INDEX_SIZE ( 1 5 10 15 20 25 30 )
	set resultado = resultado_fatfastrand_100_r_${NEURON_MEMORY_INDEX_SIZE}.txt
	if ! (-f "$resultado" ) then
		echo fatfast_RAND_${NEURON_MEMORY_INDEX_SIZE}

		rm -rf place_recognition.con

		sed -r "s/set NEURON_MEMORY_INDEX_SIZE(.*?)= (.*?);/set NEURON_MEMORY_INDEX_SIZE = $NEURON_MEMORY_INDEX_SIZE;/" place_recognition.con.source | sed -r "s/minchinton/minchinton_fat_fast_rand/" > place_recognition.con

		make clean; make -f Makefile.no_interface VERBOSE=0

		mv place_recognition.con place_recognition_r_${NEURON_MEMORY_INDEX_SIZE}.con 

		./place_recognition place_recognition_original_100.cml > resultado_fatfastrand_100_r_${NEURON_MEMORY_INDEX_SIZE}.txt
	endif
end


echo fatfast-zip-r-alto
set NEURON_MEMORY_INDEX_SIZE = 30
foreach CLUSTERING ( kmeans )
	foreach COMPRESSION ( 001 045 075 )
	set resultado = resultado_fatfastrand_${CLUSTERING}_${COMPRESSION}_r_${NEURON_MEMORY_INDEX_SIZE}.txt
	if ! ( -f "$resultado" ) then

		echo fatfast_${CLUSTERING}_${COMPRESSION}

		rm -rf place_recognition.con

		sed -r "s/set NEURON_MEMORY_INDEX_SIZE(.*?)= (.*?);/set NEURON_MEMORY_INDEX_SIZE = $NEURON_MEMORY_INDEX_SIZE;/" place_recognition.con.source | sed -r "s/minchinton/minchinton_fat_fast_rand/" > place_recognition.con

		make clean; make -f Makefile.no_interface VERBOSE=0

		mv place_recognition.con place_recognition_r_${NEURON_MEMORY_INDEX_SIZE}.con 

		./place_recognition place_recognition_${CLUSTERING}_${COMPRESSION}.cml > resultado_fatfastrand_${CLUSTERING}_${COMPRESSION}_r_${NEURON_MEMORY_INDEX_SIZE}.txt
	endif
	end
end


echo fatfast-100-r-baixo
set NEURON_MEMORY_INDEX_SIZE = 1
foreach CLUSTERING ( kmeans )
	foreach COMPRESSION ( 001 045 075 )
	set resultado = resultado_fatfastrand_${CLUSTERING}_${COMPRESSION}_r_${NEURON_MEMORY_INDEX_SIZE}.txt
	if ! ( -f "$resultado" ) then

		echo fatfast_${CLUSTERING}_${COMPRESSION}

		rm -rf place_recognition.con

		sed -r "s/set NEURON_MEMORY_INDEX_SIZE(.*?)= (.*?);/set NEURON_MEMORY_INDEX_SIZE = $NEURON_MEMORY_INDEX_SIZE;/" place_recognition.con.source | sed -r "s/minchinton/minchinton_fat_fast_rand/" > place_recognition.con

		make clean; make -f Makefile.no_interface VERBOSE=0

		mv place_recognition.con place_recognition_r_${NEURON_MEMORY_INDEX_SIZE}.con 

		./place_recognition place_recognition_${CLUSTERING}_${COMPRESSION}.cml > resultado_fatfastrand_${CLUSTERING}_${COMPRESSION}_r_${NEURON_MEMORY_INDEX_SIZE}.txt
	endif
	end
end


echo vgram-zip
foreach CLUSTERING ( kmeans )
	foreach COMPRESSION ( 001 045 075 )
	set resultado = resultado_${CLUSTERING}_${COMPRESSION}.txt
	if ! ( -f "$resultado" ) then
		 
		echo vgram_${CLUSTERING}_${COMPRESSION}

		cp place_recognition.con.source place_recognition.con

		make clean; make -f Makefile.no_interface VERBOSE=0

		./place_recognition place_recognition_${CLUSTERING}_${COMPRESSION}.cml > resultado_${CLUSTERING}_${COMPRESSION}.txt
	endif
	end
end


echo vgram_100

cp place_recognition.con.source place_recognition.con

make clean; make -f Makefile.no_interface VERBOSE=0

set resultado = resultado_original_100.txt
if ! ( -f "$resultado" ) then
	./place_recognition place_recognition_original_100.cml > resultado_original_100.txt
endif
