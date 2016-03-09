#!/bin/bash -
#

last_data_dir=$1
last_pred_dir=$2
pred_history_dir=$3
concat_pred_history_dir=$4

#current_dir=`pwd`
#cd $last_data_dir

FILES_LIST=`find $last_data_dir -name "*.txt"`
COUNT_FILES=0
#cd $current_dir

# For each file...
for i in $FILES_LIST; do

	PRED_NAME=`basename $i .txt`
#	echo $i
	CMD="./concat_pred $last_data_dir/$PRED_NAME.txt $last_pred_dir/$PRED_NAME.pe $pred_history_dir/$PRED_NAME.pe $concat_pred_history_dir/$PRED_NAME.pe"
#	echo $CMD
	$CMD
	if [ $? = "0" ]; then
		COUNT_FILES=$[COUNT_FILES + 1]
	else
		echo "File with error: " $PRED_NAME
	fi
done

echo "Number of files concatenated: " $COUNT_FILES
