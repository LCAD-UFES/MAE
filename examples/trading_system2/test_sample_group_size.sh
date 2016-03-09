#!/bin/bash -
#

shopt -s extglob

day_list=$1
return_expected=$2

FILES_LIST=`cat acoes_de_interesse.txt`
COUNT_FILES=0

# For each file...
for DATA_SET in $FILES_LIST; do
	echo -ne "$DATA_SET\t"
	php test_sample_group_size.php TXT_INTRADAY_XML/$day_list/${DATA_SET}.SA.xml.txt $return_expected
done
