#/bin/sh

./cp_cml.sh
for ((i = 0 ; i < 20 ; i++)) 
do 
  echo $i
  cd tfd$i
  rm -f tfd.test
  qsub run.sh
  cd ..
done


