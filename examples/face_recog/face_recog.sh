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

echo "$HOME       home directory on execution machine" > face_recog.test
echo "$USER	  user ID of job owner" >> face_recog.test
echo "$JOB_ID     current job ID" >> face_recog.test
echo "$JOB_NAME   current job name (see -N option)" >> face_recog.test
echo "$HOSTNAME   name of the execution host" >> face_recog.test
killall X
export DISPLAY=:0.0
xinit -- :0&
#startx
./face_recog face_recog_hallysson.cml >> face_recog.test
killall X

# Fim do script
