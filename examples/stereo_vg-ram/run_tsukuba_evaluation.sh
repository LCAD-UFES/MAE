#!/bin/bash

./convert_image tsukuba_disparity_map.ppm tsukuba.pgm exp_tsukuba.txt exp_tsukuba_all.txt exp_dispatch_tsukuba.txt tsukuba
cp tsukuba.pgm StereoEvaluation/tsukuba/results/
cp exp_tsukuba.txt exp_tsukuba_all.txt exp_dispatch_tsukuba.txt StereoEvaluation/Scripts
cd StereoEvaluation/StereoMatch/
make
./StereoMatch script ../Scripts/exp_tsukuba_all.txt
cd -

