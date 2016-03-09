OUTPUT_DIR = results
CAR_ID = 

all:
	g++ -g -Wall -o parser parser_checking_groundtruth.cpp ground_truth_manipulation.cpp -lm
	g++ -g -Wall -o use_statistics use_statistics.cpp -lm 
	g++ -g -Wall -o find_probability_limiar find_probability_limiar.cpp ground_truth_manipulation.cpp -lm
	g++ -g -Wall -o probability_filter probability_filter.cpp -lm
run:
	grep "image = " /home/lcad12/MAE/examples/visual_search_without_filters_tfd/ResultadosFinais/result_statistics_Final.txt > data.txt
	./parser data.txt ../gt.txt > probabilities.txt
	./use_statistics probabilities.txt data.txt 0 > data_probability.txt
	./find_probability_limiar data.txt data_probability.txt ../gt.txt 0.4 > output.txt 2> not_detected.txt
	./probability_filter data.txt data_probability.txt 0.8 > filtered_data.txt
clean:
	rm -rf parser use_statistics find_probability_limiar probability_filter
generate_car_results:
	grep "image = " /home/lcad12/MAE/examples/visual_search_without_filters_tfd/ResultadosFinais/result_statistics_Final.txt > $(OUTPUT_DIR)/data_train_car$(CAR_ID).txt
	grep "image = " /home/lcad12/MAE/examples/visual_search_without_filters_tfd/ResultadosFinais/result_statistics_Final.txt > $(OUTPUT_DIR)/data_test_car$(CAR_ID).txt
	./parser $(OUTPUT_DIR)/data_train_car$(CAR_ID).txt ../gt.txt > $(OUTPUT_DIR)/probabilities_train_car$(CAR_ID).txt
	./use_statistics $(OUTPUT_DIR)/probabilities_train_car$(CAR_ID).txt $(OUTPUT_DIR)/data_test_car$(CAR_ID).txt 0 > $(OUTPUT_DIR)/data_probability_test_car$(CAR_ID).txt
	./probability_filter $(OUTPUT_DIR)/data_test_car$(CAR_ID).txt $(OUTPUT_DIR)/data_probability_test_car$(CAR_ID).txt 0.10 > $(OUTPUT_DIR)/NOVO_TRAIN_filtered_signs_car$(CAR_ID)_0.10.txt
	./probability_filter $(OUTPUT_DIR)/data_test_car$(CAR_ID).txt $(OUTPUT_DIR)/data_probability_test_car$(CAR_ID).txt 0.20 > $(OUTPUT_DIR)/NOVO_TRAIN_filtered_signs_car$(CAR_ID)_0.20.txt
	./probability_filter $(OUTPUT_DIR)/data_test_car$(CAR_ID).txt $(OUTPUT_DIR)/data_probability_test_car$(CAR_ID).txt 0.30 > $(OUTPUT_DIR)/NOVO_TRAIN_filtered_signs_car$(CAR_ID)_0.30.txt
	./probability_filter $(OUTPUT_DIR)/data_test_car$(CAR_ID).txt $(OUTPUT_DIR)/data_probability_test_car$(CAR_ID).txt 0.40 > $(OUTPUT_DIR)/NOVO_TRAIN_filtered_signs_car$(CAR_ID)_0.40.txt
	./probability_filter $(OUTPUT_DIR)/data_test_car$(CAR_ID).txt $(OUTPUT_DIR)/data_probability_test_car$(CAR_ID).txt 0.50 > $(OUTPUT_DIR)/NOVO_TRAIN_filtered_signs_car$(CAR_ID)_0.50.txt
	./probability_filter $(OUTPUT_DIR)/data_test_car$(CAR_ID).txt $(OUTPUT_DIR)/data_probability_test_car$(CAR_ID).txt 0.60 > $(OUTPUT_DIR)/NOVO_TRAIN_filtered_signs_car$(CAR_ID)_0.60.txt
	./probability_filter $(OUTPUT_DIR)/data_test_car$(CAR_ID).txt $(OUTPUT_DIR)/data_probability_test_car$(CAR_ID).txt 0.70 > $(OUTPUT_DIR)/NOVO_TRAIN_filtered_signs_car$(CAR_ID)_0.70.txt
	./probability_filter $(OUTPUT_DIR)/data_test_car$(CAR_ID).txt $(OUTPUT_DIR)/data_probability_test_car$(CAR_ID).txt 0.80 > $(OUTPUT_DIR)/NOVO_TRAIN_filtered_signs_car$(CAR_ID)_0.80.txt
	./probability_filter $(OUTPUT_DIR)/data_test_car$(CAR_ID).txt $(OUTPUT_DIR)/data_probability_test_car$(CAR_ID).txt 0.90 > $(OUTPUT_DIR)/NOVO_TRAIN_filtered_signs_car$(CAR_ID)_0.90.txt
	./probability_filter $(OUTPUT_DIR)/data_test_car$(CAR_ID).txt $(OUTPUT_DIR)/data_probability_test_car$(CAR_ID).txt 0.95 > $(OUTPUT_DIR)/NOVO_TRAIN_filtered_signs_car$(CAR_ID)_0.95.txt

	
# script: awk -F';' '{print $1" "$2" "$3" "$4" "$5" "$6" "$7}' data_probability.txt | awk '{if ($3 > 0.99) print($0);}'
