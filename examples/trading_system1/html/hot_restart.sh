#!/bin/bash -
#
shopt -s extglob

day=$1

#current_dir=`pwd`
#cd $last_data_dir

FILES_LIST=`cat acoes_de_interesse.txt`
COUNT_FILES=0
#cd $current_dir

# For each file...
for DATA_SET in $FILES_LIST; do
	cat TXT_INTRADAY_XML/$day/${DATA_SET}.SA.xml.txt > hot_restart.tmp.txt
	chmod 777 hot_restart.tmp.txt

    php hot_restart_complete_mod4.php hot_restart.tmp.txt
    cat hot_restart_php.tmp.txt > TXT_INTRADAY_XML/$day/${DATA_SET}.SA.xml.txt
    

	if [ $? = "0" ]; then
		COUNT_FILES=$[COUNT_FILES + 1]
	else
		echo "File with error: " $DATA_SET
	fi
done

rm -f hot_restart.tmp.txt
rm -f hot_restart_php.tmp.txt

echo "SYNCHRONIZE" > system_state.txt
chmod 777 system_state.txt

echo "Number processed files: " $COUNT_FILES
