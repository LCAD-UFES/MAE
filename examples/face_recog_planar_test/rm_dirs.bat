#!/bin/csh

foreach EXPRESSION ( "smile" "anger" "scream" "glasses" "scarf" "left_light" "right_light" "all_side_lights" )

	rm -rf face_recog_planar_gaussian_${EXPRESSION}

	rm -rf face_recog_planar_random_${EXPRESSION}

end


