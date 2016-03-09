#!/bin/csh 

rm -rf face_recog_planar_gaussian_results.txt
rm -rf face_recog_planar_random_results.txt

foreach EXPRESSION ( "smile" "anger" "scream" "glasses" "scarf" "left_light" "right_light" "all_side_lights" )

	echo ${EXPRESSION} `tail -n 1 face_recog_planar_gaussian_${EXPRESSION}/face_recog.test` >>  face_recog_planar_gaussian_results.txt

	echo ${EXPRESSION} `tail -n 1 face_recog_planar_random_${EXPRESSION}/face_recog.test` >>  face_recog_planar_random_results.txt

end
