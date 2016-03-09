#!/bin/bash -
#

shopt -s extglob

day=$1

FILES_LIST=`cat acoes_de_interesse.txt`
COUNT_FILES=0

# For each file...
for DATA_SET in $FILES_LIST; do
	LAST_PRICE_LINE=( `tail -1 TXT_INTRADAY_XML/$day/${DATA_SET}.SA.xml.txt` )
	echo $DATA_SET ${LAST_PRICE_LINE[1]}
done
