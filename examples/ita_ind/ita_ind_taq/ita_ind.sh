#!/bin/sh
# Este script roda um unico job em SGE
# Ele pode ser executado pelo comando SGE abaixo:
# qsub um_job.sh

# A linha abaixo nao e' um comentario. Ela especifica que o que vem
# apos ela sera invocado com o /bin/sh shell
#$ -S /bin/sh

# Coloca os arquivos de saida e de erro do seu job no diretorio .
#$ -cwd
#--------------------------------
# exemplo: (altere a partir daqui)

#./ita ita_days_8.cml > ita_days_8.csv
#./ita ita_days_16.cml > ita_days_16.csv
#./ita ita_days_32.cml > ita_days_32.csv
#./ita ita_days_64.cml > ita_days_64.csv
#./ita ita_days_128.cml > ita_days_128.csv
./ita ita_days_256.cml > ita_days_256.csv

# Fim do script
