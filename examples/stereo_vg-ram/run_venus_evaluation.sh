#!/bin/bash

./convert_image venus_disparity_map.ppm venus.pgm exp_venus.txt exp_venus_all.txt exp_dispatch_venus.txt venus
cp venus.pgm StereoEvaluation/venus/results/
cp exp_venus.txt exp_venus_all.txt exp_dispatch_venus.txt StereoEvaluation/Scripts
cd StereoEvaluation/StereoMatch/
make
./StereoMatch script ../Scripts/exp_venus_all.txt
cd -

