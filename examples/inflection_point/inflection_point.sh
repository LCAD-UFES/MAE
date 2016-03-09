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

echo "$HOME       home directory on execution machine" > inflection_point.test
echo "$USER	  user ID of job owner" >> inflection_point.test
echo "$JOB_ID     current job ID" >> inflection_point.test
echo "$JOB_NAME   current job name (see -N option)" >> inflection_point.test
echo "$HOSTNAME   name of the execution host" >> inflection_point.test
killall X
export DISPLAY=:0.0
xinit -- :0&
#startx
./inflection_point inflection_point.cml > inflection_point.test
killall X

# Fim do script
