#!/bin/bash -
#

last_data_dir=$1

FILES_LIST=`cd $1; ls`

# For each file...
for DATA_SET in $FILES_LIST; do
	teste=`grep -e "2005-01-04" $1/$DATA_SET`
#	echo $DATA_SET'@'"$teste"
	if [ -n "$teste" ]
	then
		echo -n $DATA_SET " " >> _volumes.txt
		tail -n 1 $1/$DATA_SET/*.vol >> _volumes.txt
	fi
done

cat _volumes.txt | sort -r -g -k 3 > volumes.txt

rm -rf _volumes.txt
