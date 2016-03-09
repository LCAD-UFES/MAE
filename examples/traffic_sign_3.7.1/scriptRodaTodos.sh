#!/bin/csh

cd traffic_sign_RED

echo fatfast-100 em função de r
foreach NEURON_MEMORY_INDEX_SIZE ( 1 5 10 15 20 25 30 117627 )

	foreach CHANNEL ( RED GREEN BLUE )

		cd ../traffic_sign_${CHANNEL}

		echo fatfast_${CHANNEL}_RAND_${NEURON_MEMORY_INDEX_SIZE}

		rm -rf traffic_sign.con

		sed -r "s/set NEURON_MEMORY_INDEX_SIZE(.*?)= (.*?);/set NEURON_MEMORY_INDEX_SIZE = $NEURON_MEMORY_INDEX_SIZE;/" traffic_sign.con.source | sed -r "s/minchinton/minchinton_fat_fast_rand/" > traffic_sign.con

		make clean; make -f Makefile.no_interface VERBOSE=0

		mv traffic_sign.con traffic_sign_r_${NEURON_MEMORY_INDEX_SIZE}.${CHANNEL}.con 

		./traffic_sign traffic_sign_original_100.cml > resultado_fatfastrand_original_100_r_${NEURON_MEMORY_INDEX_SIZE}.${CHANNEL}.txt
	end
end


echo fatfast-zip-r-alto
set NEURON_MEMORY_INDEX_SIZE = 30
foreach CLUSTERING ( kmeans kmedoids )

	foreach COMPRESSION ( 0.25 0.50 001 010 )

		foreach CHANNEL ( RED GREEN BLUE )
		
			if ( ($CHANNEL == GREEN) || ( ($COMPRESSION == 001) && ($CLUSTERING == kmeans) && ($CHANNEL == BLUE || $CHANNEL == RED)) ) then
			 
				cd ../traffic_sign_${CHANNEL}

				echo fatfast_${CLUSTERING}_${COMPRESSION}.${CHANNEL}

				rm -rf traffic_sign.con

				sed -r "s/set NEURON_MEMORY_INDEX_SIZE(.*?)= (.*?);/set NEURON_MEMORY_INDEX_SIZE = $NEURON_MEMORY_INDEX_SIZE;/" traffic_sign.con.source | sed -r "s/minchinton/minchinton_fat_fast_rand/" > traffic_sign.con

				make clean; make -f Makefile.no_interface VERBOSE=0

				mv traffic_sign.con traffic_sign_r_${NEURON_MEMORY_INDEX_SIZE}.${CHANNEL}.con 

				./traffic_sign traffic_sign_${CLUSTERING}_${COMPRESSION}.cml > resultado_fatfastrand_${CLUSTERING}_${COMPRESSION}_r_${NEURON_MEMORY_INDEX_SIZE}.${CHANNEL}.txt
			endif
		end
	end
end


echo fatfast-100-r-baixo
set NEURON_MEMORY_INDEX_SIZE = 1
foreach CLUSTERING ( kmeans kmedoids )

	foreach COMPRESSION ( 0.25 0.50 001 010 )

		foreach CHANNEL ( RED GREEN BLUE )
		
			if ( ($CHANNEL == GREEN) || ( ($COMPRESSION == 001) && ($CLUSTERING == kmeans) && ($CHANNEL == BLUE || $CHANNEL == RED)) ) then
			 
				cd ../traffic_sign_${CHANNEL}

				echo fatfast_${CLUSTERING}_${COMPRESSION}.${CHANNEL}

				rm -rf traffic_sign.con

				sed -r "s/set NEURON_MEMORY_INDEX_SIZE(.*?)= (.*?);/set NEURON_MEMORY_INDEX_SIZE = $NEURON_MEMORY_INDEX_SIZE;/" traffic_sign.con.source | sed -r "s/minchinton/minchinton_fat_fast_rand/" > traffic_sign.con

				make clean; make -f Makefile.no_interface VERBOSE=0

				mv traffic_sign.con traffic_sign_r_${NEURON_MEMORY_INDEX_SIZE}.${CHANNEL}.con 

				./traffic_sign traffic_sign_${CLUSTERING}_${COMPRESSION}.cml > resultado_fatfastrand_${CLUSTERING}_${COMPRESSION}_r_${NEURON_MEMORY_INDEX_SIZE}.${CHANNEL}.txt
			endif
		end
	end
end

exit
echo vgram-zip
foreach CLUSTERING ( kmeans kmedoids )

	foreach COMPRESSION ( 0.25 0.50 001 010 )

		foreach CHANNEL ( RED GREEN BLUE )
		
			if ( ($CHANNEL == GREEN) || ( ($COMPRESSION == 001) && ($CLUSTERING == kmeans) && ($CHANNEL == BLUE || $CHANNEL == RED)) ) then
			 
				cd ../traffic_sign_${CHANNEL}

				echo vgram_${CLUSTERING}_${COMPRESSION}.${CHANNEL}

				cp traffic_sign.con.source traffic_sign.con

				make clean; make -f Makefile.no_interface VERBOSE=0

				./traffic_sign traffic_sign_${CLUSTERING}_${COMPRESSION}.cml > resultado_${CLUSTERING}_${COMPRESSION}.${CHANNEL}.txt
			endif
		end
	end
end


echo vgram-100
foreach CHANNEL ( RED GREEN BLUE )

	cd ../traffic_sign_${CHANNEL}

	echo vgram_original_100.${CHANNEL}

	cp traffic_sign.con.source traffic_sign.con

	make clean; make -f Makefile.no_interface VERBOSE=0

	./traffic_sign traffic_sign_original_100.cml > resultado_original_100.${CHANNEL}.txt
end

