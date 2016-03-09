#!/bin/bash -
#

FILES_LIST=`cat acoes_de_interesse.txt`
COUNT_FILES=0
#cd $current_dir

# For each file...
for DATA_SET in $FILES_LIST; do
	sed s/XXX/$DATA_SET.pe/ saida.gp | gnuplot -persist

	if [ $? = "0" ]; then
		COUNT_FILES=$[COUNT_FILES + 1]
	else
		echo "File with error: " $DATA_SET
	fi
done

echo "Number processed files: " $COUNT_FILES
