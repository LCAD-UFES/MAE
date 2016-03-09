cp visual_search_01_david.con visual_search.con
make clean
make
./visual_search visual_search_01_david.cml
mv resultadoTLD.txt resultadoTLD_01_david.txt


cp visual_search_02_jumping.con visual_search.con
make clean
make
./visual_search visual_search_02_jumping.cml
mv resultadoTLD.txt resultadoTLD_02_jumping.txt


cp visual_search_03_pedestrian1.con visual_search.con
make clean
make
./visual_search visual_search_03_pedestrian1.cml
mv resultadoTLD.txt resultadoTLD_03_pedestrian1.txt


cp visual_search_04_pedestrian2.con visual_search.con
make clean
make
./visual_search visual_search_04_pedestrian2.cml
mv resultadoTLD.txt resultadoTLD_04_pedestrian2.txt


cp visual_search_05_pedestrian3.con visual_search.con
make clean
make
./visual_search visual_search_05_pedestrian3.cml
mv resultadoTLD.txt resultadoTLD_05_pedestrian3.txt


cp visual_search_06_car.con visual_search.con
make clean
make
./visual_search visual_search_06_car.cml
mv resultadoTLD.txt resultadoTLD_06_car.txt


cp visual_search_07_motocross.con visual_search.con
make clean
make
./visual_search visual_search_07_motocross.cml
mv resultadoTLD.txt resultadoTLD_07_motocross.txt


cp visual_search_08_volkswagen.con visual_search.con
make clean
make
./visual_search visual_search_08_volkswagen.cml
mv resultadoTLD.txt resultadoTLD_08_volkswagen.txt


cp visual_search_09_carchase.con visual_search.con
make clean
make
./visual_search visual_search_09_carchase.cml
mv resultadoTLD.txt resultadoTLD_09_carchase.txt


cp visual_search_10_panda.con visual_search.con
make clean
make
./visual_search visual_search_10_panda.cml
mv resultadoTLD.txt resultadoTLD_10_panda.txt

rm resultadoFinal.txt
tail -n 1 resultadoTLD_01_david.txt >> resultadoFinal.txt
tail -n 1 resultadoTLD_02_jumping.txt >> resultadoFinal.txt
tail -n 1 resultadoTLD_03_pedestrian1.txt >> resultadoFinal.txt
tail -n 1 resultadoTLD_04_pedestrian2.txt >> resultadoFinal.txt
tail -n 1 resultadoTLD_05_pedestrian3.txt >> resultadoFinal.txt
tail -n 1 resultadoTLD_06_car.txt >> resultadoFinal.txt
tail -n 1 resultadoTLD_07_motocross.txt >> resultadoFinal.txt
tail -n 1 resultadoTLD_08_volkswagen.txt >> resultadoFinal.txt
tail -n 1 resultadoTLD_09_carchase.txt >> resultadoFinal.txt
tail -n 1 resultadoTLD_10_panda.txt >> resultadoFinal.txt
