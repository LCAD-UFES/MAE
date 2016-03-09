#!/bin/bash -
#

echo "SYNCHRONIZE" > system_state.txt
chmod 777 system_state.txt

rm -f TXT_INTRADAY_XML/2010-11-04/*
rm -f MEMORIES/*
rm -f TT_new/*
rm -f ERROR_DIR/*

rm -rf system_time.txt 
rm -rf system_test_time.txt

rm -rf buy_order.txt*
rm -rf sell_order.txt*

rm -rf buy_order_log.txt
rm -rf sell_order_log.txt

rm -rf stock_suggested.txt
rm -rf predicting.txt
rm -rf log.txt

touch buy_order_log.txt
touch sell_order_log.txt
touch log.txt

chmod 777 buy_order_log.txt
chmod 777 sell_order_log.txt
chmod 777 log.txt

rm -rf home_broker_data_log/*

shopt -s extglob
./run_wnn_train.sh 2010-?(10-29|11-01|11-03) TT_new wnn_pred_train.cml

