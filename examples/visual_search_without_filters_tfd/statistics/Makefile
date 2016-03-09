OUTPUT_DIR	= 08_07_2013/
#OUTPUT_FILE	= data.txt
CAR_ID		= 00
CUT_OFF 	= 0.0 

all:
	g++ -g -Wall -o parser parser_checking_groundtruth.cpp ground_truth_manipulation.cpp -lm
	g++ -g -Wall -o use_statistics use_statistics.cpp -lm 
	g++ -g -Wall -o find_probability_limiar find_probability_limiar.cpp ground_truth_manipulation.cpp -lm
	g++ -g -Wall -o probability_filter probability_filter.cpp -lm
	g++ -g -Wall -o generate_ex_format_from_confidence generate_ex_format_from_confidence.cpp
	g++ -g -Wall -o generate_ex_format_from_probability generate_ex_format_from_probability.cpp 
run:
	grep "image = " $(OUTPUT_DIR)/data_full.txt > $(OUTPUT_DIR)data.txt
	./parser $(OUTPUT_DIR)/data.txt ../gt.txt > $(OUTPUT_DIR)/probabilities.txt
	./use_statistics $(OUTPUT_DIR)/probabilities.txt $(OUTPUT_DIR)/data.txt 0 > $(OUTPUT_DIR)/data_probability.txt
#	./find_probability_limiar $(OUTPUT_DIR)/data.txt $(OUTPUT_DIR)/data_probability.txt ../gt.txt  0.0 > $(OUTPUT_DIR)/output.txt 2> $(OUTPUT_DIR)/not_detected.txt
	./probability_filter $(OUTPUT_DIR)/data.txt $(OUTPUT_DIR)/data_probability.txt 0.0 > $(OUTPUT_DIR)/filtered_data.txt
train_and_test:
	grep "image = " $(OUTPUT_DIR)/data_train_full.txt > $(OUTPUT_DIR)/data_train.txt
	grep "image = " $(OUTPUT_DIR)/data_test_full.txt > $(OUTPUT_DIR)/data_test.txt
	./parser $(OUTPUT_DIR)/data_train.txt ../gt.txt > $(OUTPUT_DIR)/probabilities_bayes.txt
	./use_statistics $(OUTPUT_DIR)/probabilities_bayes.txt $(OUTPUT_DIR)/data_test.txt 0 > $(OUTPUT_DIR)/data_probability_test.txt
	./probability_filter $(OUTPUT_DIR)/data_test.txt $(OUTPUT_DIR)/data_probability_test.txt $(CUT_OFF) > $(OUTPUT_DIR)/filtered_data_test.txt
clean:
	rm -rf parser use_statistics find_probability_limiar probability_filter
generate_car_results:
	grep "image = " $(OUTPUT_DIR)/final/result_statistics_Final.txt > $(OUTPUT_DIR)/data_train_car$(CAR_ID).txt
	grep "image = " $(OUTPUT_DIR)/final/teste_teste_Final.txt > $(OUTPUT_DIR)/data_test_car$(CAR_ID).txt
	./parser $(OUTPUT_DIR)/data_train_car$(CAR_ID).txt ../gt.txt > $(OUTPUT_DIR)/probabilities_train_car$(CAR_ID).txt
	./use_statistics $(OUTPUT_DIR)/probabilities_train_car$(CAR_ID).txt $(OUTPUT_DIR)/data_train_car$(CAR_ID).txt 0 > $(OUTPUT_DIR)/data_probability_train_car$(CAR_ID).txt 
	./probability_filter $(OUTPUT_DIR)/data_train_car$(CAR_ID).txt $(OUTPUT_DIR)/data_probability_train_car$(CAR_ID).txt 0.10 > $(OUTPUT_DIR)/ONLY_C_filtered_signs_car$(CAR_ID)_cutoff_0.10.txt
	./probability_filter $(OUTPUT_DIR)/data_train_car$(CAR_ID).txt $(OUTPUT_DIR)/data_probability_train_car$(CAR_ID).txt 0.50 > $(OUTPUT_DIR)/ONLY_C_filtered_signs_car$(CAR_ID)_cutoff_0.50.txt
confidence_cutoff:
	./generate_ex_format_from_confidence $(OUTPUT_DIR)/final/results/data_train_car.txt 0.1 > $(OUTPUT_DIR)/final/results/confidence_cutoff_data_train_car.0.1.txt
	./generate_ex_format_from_confidence $(OUTPUT_DIR)/final/results/data_train_car.txt 0.2 > $(OUTPUT_DIR)/final/results/confidence_cutoff_data_train_car.0.2.txt
	./generate_ex_format_from_confidence $(OUTPUT_DIR)/final/results/data_train_car.txt 0.3 > $(OUTPUT_DIR)/final/results/confidence_cutoff_data_train_car.0.3.txt
	./generate_ex_format_from_confidence $(OUTPUT_DIR)/final/results/data_train_car.txt 0.4 > $(OUTPUT_DIR)/final/results/confidence_cutoff_data_train_car.0.4.txt
	./generate_ex_format_from_confidence $(OUTPUT_DIR)/final/results/data_train_car.txt 0.5 > $(OUTPUT_DIR)/final/results/confidence_cutoff_data_train_car.0.5.txt
	./generate_ex_format_from_confidence $(OUTPUT_DIR)/final/results/data_train_car.txt 0.6 > $(OUTPUT_DIR)/final/results/confidence_cutoff_data_train_car.0.6.txt
	./generate_ex_format_from_confidence $(OUTPUT_DIR)/final/results/data_train_car.txt 0.7 > $(OUTPUT_DIR)/final/results/confidence_cutoff_data_train_car.0.7.txt
	./generate_ex_format_from_confidence $(OUTPUT_DIR)/final/results/data_train_car.txt 0.8 > $(OUTPUT_DIR)/final/results/confidence_cutoff_data_train_car.0.8.txt
	./generate_ex_format_from_confidence $(OUTPUT_DIR)/final/results/data_train_car.txt 0.9 > $(OUTPUT_DIR)/final/results/confidence_cutoff_data_train_car.0.9.txt
probability_cutoff:
	./generate_ex_format_from_probability $(OUTPUT_DIR)/final/results/data_probability_train_car.txt 0.91 > $(OUTPUT_DIR)/final/results/confidence_cutoff_data_probability_train_car.0.91.txt
	./generate_ex_format_from_probability $(OUTPUT_DIR)/final/results/data_probability_train_car.txt 0.92 > $(OUTPUT_DIR)/final/results/confidence_cutoff_data_probability_train_car.0.92.txt
	./generate_ex_format_from_probability $(OUTPUT_DIR)/final/results/data_probability_train_car.txt 0.93 > $(OUTPUT_DIR)/final/results/confidence_cutoff_data_probability_train_car.0.93.txt
	./generate_ex_format_from_probability $(OUTPUT_DIR)/final/results/data_probability_train_car.txt 0.94 > $(OUTPUT_DIR)/final/results/confidence_cutoff_data_probability_train_car.0.94.txt
	./generate_ex_format_from_probability $(OUTPUT_DIR)/final/results/data_probability_train_car.txt 0.95 > $(OUTPUT_DIR)/final/results/confidence_cutoff_data_probability_train_car.0.95.txt
	./generate_ex_format_from_probability $(OUTPUT_DIR)/final/results/data_probability_train_car.txt 0.96 > $(OUTPUT_DIR)/final/results/confidence_cutoff_data_probability_train_car.0.96.txt
	./generate_ex_format_from_probability $(OUTPUT_DIR)/final/results/data_probability_train_car.txt 0.97 > $(OUTPUT_DIR)/final/results/confidence_cutoff_data_probability_train_car.0.97.txt
	./generate_ex_format_from_probability $(OUTPUT_DIR)/final/results/data_probability_train_car.txt 0.98 > $(OUTPUT_DIR)/final/results/confidence_cutoff_data_probability_train_car.0.98.txt
	./generate_ex_format_from_probability $(OUTPUT_DIR)/final/results/data_probability_train_car.txt 0.99 > $(OUTPUT_DIR)/final/results/confidence_cutoff_data_probability_train_car.0.99.txt
# script: awk -F';' '{print $1" "$2" "$3" "$4" "$5" "$6" "$7}' data_probability.txt | awk '{if ($3 > 0.99) print($0);}'
