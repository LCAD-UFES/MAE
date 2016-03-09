#!/bin/bash
# enter a directory name
DIR="08_07_2013"

awk -F';' '{print $0}' $DIR/filtered_data_test.txt | awk -F' = ' '{if ($3 >= 0.05) print $0}' > $DIR/input_data_0.05.txt;
awk -F';' '{print $0}' $DIR/filtered_data_test.txt | awk -F' = ' '{if ($3 >= 0.1) print $0}' > $DIR/input_data_0.1.txt;
awk -F';' '{print $0}' $DIR/filtered_data_test.txt | awk -F' = ' '{if ($3 >= 0.2) print $0}' > $DIR/input_data_0.2.txt;
awk -F';' '{print $0}' $DIR/filtered_data_test.txt | awk -F' = ' '{if ($3 >= 0.3) print $0}' > $DIR/input_data_0.3.txt;
awk -F';' '{print $0}' $DIR/filtered_data_test.txt | awk -F' = ' '{if ($3 >= 0.4) print $0}' > $DIR/input_data_0.4.txt;
awk -F';' '{print $0}' $DIR/filtered_data_test.txt | awk -F' = ' '{if ($3 >= 0.5) print $0}' > $DIR/input_data_0.5.txt;
awk -F';' '{print $0}' $DIR/filtered_data_test.txt | awk -F' = ' '{if ($3 >= 0.6) print $0}' > $DIR/input_data_0.6.txt;
awk -F';' '{print $0}' $DIR/filtered_data_test.txt | awk -F' = ' '{if ($3 >= 0.7) print $0}' > $DIR/input_data_0.7.txt;
awk -F';' '{print $0}' $DIR/filtered_data_test.txt | awk -F' = ' '{if ($3 >= 0.8) print $0}' > $DIR/input_data_0.8.txt;
awk -F';' '{print $0}' $DIR/filtered_data_test.txt | awk -F' = ' '{if ($3 >= 0.9) print $0}' > $DIR/input_data_0.9.txt;
awk -F';' '{print $0}' $DIR/filtered_data_test.txt | awk -F' = ' '{if ($3 >= 0.91) print $0}' > $DIR/input_data_0.91.txt;
awk -F';' '{print $0}' $DIR/filtered_data_test.txt | awk -F' = ' '{if ($3 >= 0.92) print $0}' > $DIR/input_data_0.92.txt;
awk -F';' '{print $0}' $DIR/filtered_data_test.txt | awk -F' = ' '{if ($3 >= 0.93) print $0}' > $DIR/input_data_0.93.txt;
awk -F';' '{print $0}' $DIR/filtered_data_test.txt | awk -F' = ' '{if ($3 >= 0.94) print $0}' > $DIR/input_data_0.94.txt;
awk -F';' '{print $0}' $DIR/filtered_data_test.txt | awk -F' = ' '{if ($3 >= 0.95) print $0}' > $DIR/input_data_0.95.txt;
awk -F';' '{print $0}' $DIR/filtered_data_test.txt | awk -F' = ' '{if ($3 >= 0.96) print $0}' > $DIR/input_data_0.96.txt;
awk -F';' '{print $0}' $DIR/filtered_data_test.txt | awk -F' = ' '{if ($3 >= 0.97) print $0}' > $DIR/input_data_0.97.txt;
awk -F';' '{print $0}' $DIR/filtered_data_test.txt | awk -F' = ' '{if ($3 >= 0.98) print $0}' > $DIR/input_data_0.98.txt;
awk -F';' '{print $0}' $DIR/filtered_data_test.txt | awk -F' = ' '{if ($3 >= 0.99) print $0}' > $DIR/input_data_0.99.txt;
awk -F';' '{print $0}' $DIR/filtered_data_test.txt | awk -F' = ' '{if ($3 >= 0.999) print $0}' > $DIR/input_data_0.999.txt;
awk -F';' '{print $0}' $DIR/filtered_data_test.txt | awk -F' = ' '{if ($3 >= 0.9999) print $0}' > $DIR/input_data_0.9999.txt;
