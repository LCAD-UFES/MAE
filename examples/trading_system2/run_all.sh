#!/bin/bash -
#

shopt -s extglob

day_list=$1
pred_history_dir=$2

rm $pred_history_dir/*

FILES_LIST=`cat acoes_de_interesse.txt`
COUNT_FILES=0

# For each file...
for DATA_SET in $FILES_LIST; do
	# echo $DATA_SET
	rm -f DATA_TEMP/*	
	cat TXT_INTRADAY_XML/$day_list/${DATA_SET}.SA.xml.txt > DATA_TEMP/${DATA_SET}.SA.xml.txt
	echo $DATA_SET
	./wnn_pred wnn_pred.cml > $pred_history_dir/$DATA_SET.pe
	#sleep 3
	if [ $? = "0" ]; then
		COUNT_FILES=$[COUNT_FILES + 1]
	else
		echo "File with error: " $DATA_SET
	fi
done

echo "Number processed files: " $COUNT_FILES
echo "Output dir: " $pred_history_dir
