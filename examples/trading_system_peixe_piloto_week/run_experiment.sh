#!/bin/bash -
#

first_week=$1
last_week=$2

rm experiment_log.txt
i=$last_week
while [ $i -ge $first_week ]; do
	echo $i > samples_to_discard.txt
	./run_all.sh DATA TT_new
	IPC2/trading_system TT_new/* | grep "Final Capital" >> experiment_log.txt
	tail --lines=1 experiment_log.txt | cut -f 3 > initial_capital.txt
	let i=i-1
done
