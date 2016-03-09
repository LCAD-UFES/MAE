
cp visual_search_01_david.con ../visual_search.con
cd ..
make clean
make
./visual_search visual_search_zoom_retrain.cml
mv resultadoTLD.txt TLD/resultadoTLD_01_david.txt
cd TLD/

cp visual_search_02_jumping.con ../visual_search.con
cd ..
make clean
make
./visual_search visual_search_zoom_retrain.cml
mv resultadoTLD.txt TLD/resultadoTLD_02_jumping.txt
cd TLD/

cp visual_search_03_pedestrian1.con ../visual_search.con
cd ..
make clean
make
./visual_search visual_search_zoom_retrain.cml
mv resultadoTLD.txt TLD/resultadoTLD_03_pedestrian1.txt
cd TLD/

cp visual_search_04_pedestrian2.con ../visual_search.con
cd ..
make clean
make
./visual_search visual_search_zoom_retrain.cml
mv resultadoTLD.txt TLD/resultadoTLD_04_pedestrian2.txt
cd TLD/

cp visual_search_05_pedestrian3.con ../visual_search.con
cd ..
make clean
make
./visual_search visual_search_zoom_retrain.cml
mv resultadoTLD.txt TLD/resultadoTLD_05_pedestrian3.txt
cd TLD/

cp visual_search_06_car.con ../visual_search.con
cd ..
make clean
make
./visual_search visual_search_zoom_retrain.cml
mv resultadoTLD.txt TLD/resultadoTLD_06_car.txt
cd TLD/

cp visual_search_07_motocross.con ../visual_search.con
cd ..
make clean
make
./visual_search visual_search_zoom_retrain.cml
mv resultadoTLD.txt TLD/resultadoTLD_07_motocross.txt
cd TLD/

cp visual_search_08_volkswagen.con ../visual_search.con
cd ..
make clean
make
./visual_search visual_search_zoom_retrain.cml
mv resultadoTLD.txt TLD/resultadoTLD_08_volkswagen.txt
cd TLD/

cp visual_search_09_carchase.con ../visual_search.con
cd ..
make clean
make
./visual_search visual_search_zoom_retrain.cml
mv resultadoTLD.txt TLD/resultadoTLD_09_carchase.txt
cd TLD/

cp visual_search_10_panda.con ../visual_search.con
cd ..
make clean
make
./visual_search visual_search_zoom_retrain.cml
mv resultadoTLD.txt TLD/resultadoTLD_10_panda.txt
cd TLD/


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
