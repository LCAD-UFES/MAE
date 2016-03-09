#!/bin/bash

# $1 -> numero de semanas de tunning
# $2 -> numero de semanas de cada teste entre a data inicial e a data final
# $3 -> data inicial
# $4 -> data final

# Limpa diretorios e inicializa arquivos
rm -f full_log.txt full_out.txt
cp initial_capital.txt.default ../IPC2/initial_capital.txt

num_tunning_weeks=$1
num_testing_weeks=$2
current_week=`./get_this_thursday $3`
last_week=`./get_last_wednesday $4`

while [[ "$current_week" < "$last_week" ]]; do
	last_tuning_week=`./get_week_plus_days $current_week -1`
	num_days=$[-$num_tunning_weeks * 7]
	first_tuning_week=`./get_week_plus_days $current_week $num_days`

	#echo current_week = $current_week   last_week = $last_week   first_tuning_week = $first_tuning_week   last_tuning_week = $last_tuning_week
	echo $current_week "   "

	cp acoes_de_interesse.txt.initial acoes_de_interesse.txt
	# No periodo de tuning corrente, roda o ts com com diversos parametros, e detecta e remove as acoes que causam stop_loss
	stop_loss_happened="true"
	while [[ "$stop_loss_happened" == "true" ]]; do
		# Faz o tunning usando as acoes_de_interesse.txt correntes
		../SCALABLE_TRADING_SYSTEM/run_scalability_experiment.bat just_petr3.txt parameter_files.txt $first_tuning_week $last_tuning_week

		# Seleciona o experimento com o melhor arquivo de parametros e escreve este arquivo em parameters.txt
		experiment_with_best_parameters=`./get_experiment_best_parameters just_petr3.txt parameter_files.txt`
		cp $experiment_with_best_parameters/parameters.txt parameters.txt
		echo "Best experiment =" $experiment_with_best_parameters
		line=`sed -n "/Final Capital/p" < $experiment_with_best_parameters/out.txt`
		max_return=`echo $line | cut -s --delimiter=':' -f 4 | cut --delimiter='%' -f 1`
		echo "Return of the best experiment =" $max_return%
		
		# Remove stocks que causaram stop loss de acoes_de_interesse.txt
		stop_loss_happened=`./remove_stocks_that_caused_stop_losses $experiment_with_best_parameters/out.txt acoes_de_interesse.txt`
	done
	echo "Stocks removed: " ; diff --side-by-side --suppress-common-lines acoes_de_interesse.txt.initial acoes_de_interesse.txt

	# Roda o trading system no periodo
	cp ../IPC2/initial_capital.txt previous_initial_capital.txt
	num_days=$[$num_testing_weeks * 7 - 1]
	end_of_testing_period=`./get_week_plus_days $current_week +$num_days`
	if [[ "$end_of_testing_period" > "$last_week" ]]; then
		end_of_testing_period=$last_week
	fi
	data_available=`./check_if_there_is_data_available ../IPC2/DADOS_PEIXE_PILOTO/COM_VOLUME/PETR3_Volume.TXT $current_week`
	if [[ "$data_available" == "true" ]]; then
		../SCALABLE_TRADING_SYSTEM/run_scalability_experiment.bat just_petr3.txt just_parameters.txt $current_week $end_of_testing_period
		cat exp_PETR3_parameters/log.txt >> full_log.txt
		cat exp_PETR3_parameters/out.txt >> full_out.txt
	
		# Atualiza ../IPC2/initial_capital.txt para o proximo periodo
		grep QUIT exp_PETR3_parameters/out.txt | cut -s  -f 3 > ../IPC2/initial_capital.txt
	fi
	
	echo -n "### Current Capital = "; cat ../IPC2/initial_capital.txt
	echo -n "### Period Return   = "; ./get_accumulated_return previous_initial_capital.txt ../IPC2/initial_capital.txt
	rm previous_initial_capital.txt
	echo -n "### Current Accumulated Return = "; ./get_accumulated_return initial_capital.txt.default ../IPC2/initial_capital.txt
	echo " "
	
	num_days=$[$num_testing_weeks * 7]
	current_week=`./get_week_plus_days $current_week +$num_days`
done

echo -n "###### Final Capital = "; cat ../IPC2/initial_capital.txt
echo -n "###### Final Accumulated Return = "; ./get_accumulated_return initial_capital.txt.default ../IPC2/initial_capital.txt

cp initial_capital.txt.default ../IPC2/initial_capital.txt
