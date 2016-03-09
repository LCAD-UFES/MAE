#!/bin/bash -
#

shopt -s extglob

day_list=$1
pred_history_dir=$2

rm $pred_history_dir/*

FILES_LIST=`cat acoes_de_interesse.txt`
COUNT_FILES=0

ODD=0
# For each file...
for DATA_SET in $FILES_LIST; do
	if [ $ODD = "0" ]; then
		# echo $DATA_SET
		rm -f DATA_TEMP1/*	
		cat TXT_INTRADAY_XML/$day_list/${DATA_SET}.SA.xml.txt > DATA_TEMP1/${DATA_SET}.SA.xml.txt
		echo $DATA_SET

		out1=$pred_history_dir/$DATA_SET.pe
		./wnn_pred_proc1 wnn_pred.cml > $out1 &

		proc1_id=$!
		data_set1=$DATA_SET

		ODD=1
	else
		# echo $DATA_SET
		rm -f DATA_TEMP2/*	
		cat TXT_INTRADAY_XML/$day_list/${DATA_SET}.SA.xml.txt > DATA_TEMP2/${DATA_SET}.SA.xml.txt
		echo $DATA_SET

		out2=$pred_history_dir/$DATA_SET.pe
		./wnn_pred_proc2 wnn_pred.cml > $out2 &

		proc2_id=$!
		data_set2=$DATA_SET
		
		return_status1=0
		wait $proc1_id || return_status1=1
		return_status2=0
		wait $proc2_id || return_status2=1
		if [ "$return_status1" = "0" ]; then
			COUNT_FILES=$[COUNT_FILES + 1]
		else
			echo "File with error: " $data_set1
		fi
		if [ "$return_status2" = "0" ]; then
			COUNT_FILES=$[COUNT_FILES + 1]
		else
			echo "File with error: " $data_set2
		fi
		ODD=0
	fi
done

if [ "$ODD" = "1" ]; then
	return_status1=0
	wait $proc1_id || return_status1=1
	if [ "$return_status1" = "0" ]; then
		COUNT_FILES=$[COUNT_FILES + 1]
	else
		echo "File with error: " $data_set1
	fi
fi

echo "Number processed files: " $COUNT_FILES
