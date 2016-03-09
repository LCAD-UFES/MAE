#!/bin/bash -
#

# $1 = diretorio com os aquivos XML
# $2 = hora de inicio do pregao. Ex.: 10 ou 11
# $3 = hora de fim do pregao. Ex.: 17 ou 18


mkdir TXT_$1

DIR_LIST=`ls -d $1/*`

# For each dir
for DIR in $DIR_LIST; do
	echo TXT_$DIR
	mkdir TXT_$DIR
	FILE_LIST=`ls $DIR/*.xml`
	for FILE in $FILE_LIST; do
		python converte_xml_txt.py $FILE $2 $3 > TXT_$FILE.txt
	done
done
