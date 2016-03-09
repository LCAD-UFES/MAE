#!/bin/bash -
#

DIR_LIST=`ls -d TT_new-*`
COUNT_FILES=0
#cd $current_dir

# For each dir...
for DATA_SET in $DIR_LIST; do
	TRADING_SYSTEM/trading_system $DATA_SET/* | grep capital: | sed 's/\# \*\*\*\* capital: R\$//'
done
