cp visual_search_10_panda.con visual_search.con
make clean
make -f Makefile.no_interface
./visual_search visual_search_tunning.cml > resultadoTLD_10_panda.txt

cp visual_search_09_carchase.con visual_search.con
make clean
make -f Makefile.no_interface
./visual_search visual_search_tunning.cml > resultadoTLD_09_carchase.txt

cp visual_search_08_volkswagen.con visual_search.con
make clean
make -f Makefile.no_interface
./visual_search visual_search_tunning.cml > resultadoTLD_08_volkswagen.txt

cp visual_search_07_motocross.con visual_search.con
make clean
make -f Makefile.no_interface
./visual_search visual_search_tunning.cml > resultadoTLD_07_motocross.txt

cp visual_search_06_car.con visual_search.con
make clean
make -f Makefile.no_interface
./visual_search visual_search_tunning.cml > resultadoTLD_06_car.txt

cp visual_search_05_pedestrian3.con visual_search.con
make clean
make -f Makefile.no_interface
./visual_search visual_search_tunning.cml > resultadoTLD_05_pedestrian3.txt

cp visual_search_04_pedestrian2.con visual_search.con
make clean
make -f Makefile.no_interface
./visual_search visual_search_tunning.cml > resultadoTLD_04_pedestrian2.txt

cp visual_search_03_pedestrian1.con visual_search.con
make clean
make -f Makefile.no_interface
./visual_search visual_search_tunning.cml > resultadoTLD_03_pedestrian1.txt

cp visual_search_02_jumping.con visual_search.con
make clean
make -f Makefile.no_interface
./visual_search visual_search_tunning.cml > resultadoTLD_02_jumping.txt

cp visual_search_01_david.con visual_search.con
make clean
make -f Makefile.no_interface
./visual_search visual_search_tunning.cml > resultadoTLD_01_david.txt



rm resultadoFinal.txt
cat resultadoTLD_01_david.txt >> resultadoFinal.txt
cat resultadoTLD_02_jumping.txt >> resultadoFinal.txt
cat resultadoTLD_03_pedestrian1.txt >> resultadoFinal.txt
cat resultadoTLD_04_pedestrian2.txt >> resultadoFinal.txt
cat resultadoTLD_05_pedestrian3.txt >> resultadoFinal.txt
cat resultadoTLD_06_car.txt >> resultadoFinal.txt
cat resultadoTLD_07_motocross.txt >> resultadoFinal.txt
cat resultadoTLD_08_volkswagen.txt >> resultadoFinal.txt
cat resultadoTLD_09_carchase.txt >> resultadoFinal.txt
cat resultadoTLD_10_panda.txt >> resultadoFinal.txt
