#!/bin/bash -
#

rm -f DATA_BUY_SELL/*/*

FILES_LIST=`ls TT_new`
COUNT_FILES=0

# For each file...
for DATA_SET in $FILES_LIST; do
	echo $COUNT_FILES $DATA_SET
	rm -f state.bin GRAFICOS/* log.txt
	./IPC2/trading_system TT_new/$DATA_SET > /dev/null
	if [ $? = "0" ]; then
		COUNT_FILES=$[COUNT_FILES + 1]
	else
		echo "File with error: " $DATA_SET
	fi

	rm -rf GRAFICOS_BUY_SELL/$DATA_SET
	mkdir GRAFICOS_BUY_SELL/$DATA_SET
	mkdir GRAFICOS_BUY_SELL/$DATA_SET/GRAFICOS
	mv GRAFICOS/* GRAFICOS_BUY_SELL/$DATA_SET/GRAFICOS
	mv log.txt GRAFICOS_BUY_SELL/$DATA_SET
done

echo "Number processed files: " $COUNT_FILES
