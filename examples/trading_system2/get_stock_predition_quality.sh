#!/bin/bash -
#

day=$1

FILES_LIST=`cat acoes_de_interesse.txt`

# For each file...
for DATA_SET in $FILES_LIST; do
	echo -n $DATA_SET
	grep hr_up $day/${DATA_SET}.pe
done
