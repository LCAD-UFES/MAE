#!/bin/bash

# $1 -> Arquivo com o simbolo das acoes de referencia (ex. PETR3) de interesse. Uma por linha.
# $2 -> Arquivo com o nome dos arquivos de parametros de interesse sem a extensão .txt. Um por linha.
# $3 -> Data inicial
# $4 -> Data final
# $5 -> Se for igual a "wait", espera cada experimento terminar antes de comecar outro.

for REF_STOCK in `cat $1`;
do
	for PARAM_FILE in `cat $2`;
	do
		# remove previous experiment dirs
		rm -rf exp_${REF_STOCK}_${PARAM_FILE}

		# create experiment dirs
		mkdir exp_${REF_STOCK}_${PARAM_FILE}
		mkdir exp_${REF_STOCK}_${PARAM_FILE}/GRAFICO_ONLINE
		mkdir exp_${REF_STOCK}_${PARAM_FILE}/GRAFICOS
		mkdir exp_${REF_STOCK}_${PARAM_FILE}/DADOS_PEIXE_PILOTO
		ln -s /home/alberto/dados_peixe_piloto/quant/COM_VOLUME exp_${REF_STOCK}_${PARAM_FILE}/DADOS_PEIXE_PILOTO/
		ln -s /home/alberto/dados_peixe_piloto/quant/DATA exp_${REF_STOCK}_${PARAM_FILE}/DATA

		# copy files
		cp ../IPC2/trading_system exp_${REF_STOCK}_${PARAM_FILE}/
		cp ../IPC2/initial_capital.txt exp_${REF_STOCK}_${PARAM_FILE}/
		cp ${PARAM_FILE}.txt exp_${REF_STOCK}_${PARAM_FILE}/parameters.txt
		
		# run experiments
		cd exp_${REF_STOCK}_${PARAM_FILE}
		if [ "$5" == "wait" ]; then
			./trading_system ../acoes_de_interesse.txt ${REF_STOCK} $3 $4 > out.txt
		else
			./trading_system ../acoes_de_interesse.txt ${REF_STOCK} $3 $4 > out.txt &
		fi
		cd ..
	done
done

wait
