#!/bin/bash
./face_recog face_recog_new.cml > face_recog.test
RESULT=`tail -n 1 face_recog.test`
echo "Hit-rate = $RESULT %" | mail -s "face_recog_results" "stivendias@gmail.com"
