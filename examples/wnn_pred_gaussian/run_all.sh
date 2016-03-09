#!/bin/bash -
#

last_data_dir=$1
last_pred_dir=$2
pred_history_dir=$3

#current_dir=`pwd`
#cd $last_data_dir

FILES_LIST=`find $last_data_dir -name "*.txt"`
COUNT_FILES=0
#cd $current_dir

# For each file...
for i in $FILES_LIST; do

	PRED_NAME=`basename $i .txt`
#	echo $i
	rm -f DATA_TEMP/*.txt
	rm -f TT_TEMP/*.pe
	cp $last_data_dir/$PRED_NAME.txt DATA_TEMP/
	cp $last_pred_dir/$PRED_NAME.pe TT_TEMP/
	./wnn_pred wnn_pred.cml > $pred_history_dir/$PRED_NAME.pe

	if [ $? = "0" ]; then
		COUNT_FILES=$[COUNT_FILES + 1]
	else
		echo "File with error: " $PRED_NAME
	fi
done

echo "Number processed files: " $COUNT_FILES
