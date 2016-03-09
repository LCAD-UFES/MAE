#!/bin/csh

foreach EXPRESSION ( "smile" "anger" "scream" "glasses" "scarf" "left_light" "right_light" "all_side_lights" )

	cp -r face_recog_planar_gaussian face_recog_planar_gaussian_${EXPRESSION}
	cd face_recog_planar_gaussian_${EXPRESSION}
	cp faces_tv.txt random_faces_t.txt
	cp faces_te_${EXPRESSION}.txt random_faces_r.txt
	cd ..

	cp -r face_recog_planar_random face_recog_planar_random_${EXPRESSION}
	cd face_recog_planar_random_${EXPRESSION}
	cp faces_tv.txt random_faces_t.txt
	cp faces_te_${EXPRESSION}.txt random_faces_r.txt
	cd ..

end


