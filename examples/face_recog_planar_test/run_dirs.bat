#!/bin/csh

foreach EXPRESSION ( "smile" "anger" "scream" "glasses" "scarf" "left_light" "right_light" "all_side_lights" )

	cd face_recog_planar_gaussian_${EXPRESSION}
	echo face_recog_planar_gaussian_${EXPRESSION}
	#qsub face_recog.sh
	./face_recog.sh
	cd ..

	cd face_recog_planar_random_${EXPRESSION}
	echo face_recog_planar_random_${EXPRESSION}
	#qsub face_recog.sh
	./face_recog.sh
	cd ..

end
