#!/bin/bash -
#

DIR_LIST=`ls -d TT_new-*`
COUNT_FILES=0
#cd $current_dir

shopt -s extglob

# For each dir...
for DATA_SET in $DIR_LIST; do
	TRADING_SYSTEM/trading_system $DATA_SET/$1 | grep capital: | sed 's/\# \*\*\*\* capital: R\$//'
done
