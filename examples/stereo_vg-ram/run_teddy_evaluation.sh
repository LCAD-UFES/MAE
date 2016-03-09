#!/bin/bash

./convert_image teddy_disparity_map.ppm teddy.pgm exp_teddy.txt exp_teddy_all.txt exp_dispatch_teddy.txt teddy
cp teddy.pgm StereoEvaluation/teddy/results/
cp exp_teddy.txt exp_teddy_all.txt exp_dispatch_teddy.txt StereoEvaluation/Scripts
cd StereoEvaluation/StereoMatch/
make
./StereoMatch script ../Scripts/exp_teddy_all.txt
cd -

