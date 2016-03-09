#!/bin/bash

./convert_image cones_disparity_map.ppm cones.pgm exp_cones.txt exp_cones_all.txt exp_dispatch_cones.txt cones
cp cones.pgm StereoEvaluation/cones/results/
cp exp_cones.txt exp_cones_all.txt exp_dispatch_cones.txt StereoEvaluation/Scripts
cd StereoEvaluation/StereoMatch/
make
./StereoMatch script ../Scripts/exp_cones_all.txt
cd -

