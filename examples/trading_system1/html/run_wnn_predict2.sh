#!/bin/bash
#

shopt -s extglob

day_list=$1
pred_history_dir=$2
cml_file=$3

#current_dir=`pwd`
#cd $last_data_dir

FILES_LIST=`cat acoes_de_interesse.txt`
COUNT_FILES=0
#cd $current_dir

# For each file...
for DATA_SET in $FILES_LIST; do
	echo $DATA_SET >> caco
done

echo "Number processed files: " $COUNT_FILES
