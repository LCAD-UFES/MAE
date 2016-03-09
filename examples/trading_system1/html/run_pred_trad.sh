#!/bin/bash -
#

shopt -s extglob

./run_wnn_predict.sh $1 $2 $3 >/dev/null &
