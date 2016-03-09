#!/bin/bash -
#

last_data_dir=$1
pred_history_dir=$2

#current_dir=`pwd`
#cd $last_data_dir

FILES_LIST=`cat acoes_de_interesse.txt`
COUNT_FILES=0
#cd $current_dir

# For each file...
for DATA_SET in $FILES_LIST; do
	# echo $DATA_SET
	rm -f DATA_TEMP/*
	cp $last_data_dir/$DATA_SET/* DATA_TEMP/
	./wnn_pred wnn_pred.cml > $pred_history_dir/$DATA_SET.pe

	if [ $? = "0" ]; then
		COUNT_FILES=$[COUNT_FILES + 1]
	else
		echo "File with error: " $DATA_SET
	fi
done

echo "Number processed files: " $COUNT_FILES
