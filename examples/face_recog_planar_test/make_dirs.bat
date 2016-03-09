#!/bin/csh

foreach EXPRESSION ( "smile" "anger" "scream" "glasses" "scarf" "left_light" "right_light" "all_side_lights" )

	cd face_recog_planar_gaussian_${EXPRESSION}
	make clean		
	make -f Makefile.no_interface
	cd ..

	cd face_recog_planar_random_${EXPRESSION}
	make clean		
	make -f Makefile.no_interface
	cd ..

end
