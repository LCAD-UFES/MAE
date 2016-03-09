
cp visual_search_vot2014_ball.con ../visual_search.con
cd ..
make clean
make
./visual_search visual_search_zoom_retrain.cml
mv resultadovot2014.txt vot2014/resultado_vot2014_ball.txt
cd vot2014/

cp visual_search_vot2014_basketball.con ../visual_search.con
cd ..
make clean
make
./visual_search visual_search_zoom_retrain.cml
mv resultadovot2014.txt vot2014/resultado_vot2014_basketball.txt
cd vot2014/

cp visual_search_vot2014_bicycle.con ../visual_search.con
cd ..
make clean
make
./visual_search visual_search_zoom_retrain.cml
mv resultadovot2014.txt vot2014/resultado_vot2014_bicycle.txt
cd vot2014/

cp visual_search_vot2014_bolt.con ../visual_search.con
cd ..
make clean
make
./visual_search visual_search_zoom_retrain.cml
mv resultadovot2014.txt vot2014/resultado_vot2014_bolt.txt
cd vot2014/

cp visual_search_vot2014_car.con ../visual_search.con
cd ..
make clean
make
./visual_search visual_search_zoom_retrain.cml
mv resultadovot2014.txt vot2014/resultado_vot2014_car.txt
cd vot2014/

cp visual_search_vot2014_david.con ../visual_search.con
cd ..
make clean
make
./visual_search visual_search_zoom_retrain.cml
mv resultadovot2014.txt vot2014/resultado_vot2014_david.txt
cd vot2014/

cp visual_search_vot2014_diving.con ../visual_search.con
cd ..
make clean
make
./visual_search visual_search_zoom_retrain.cml
mv resultadovot2014.txt vot2014/resultado_vot2014_diving.txt
cd vot2014/

cp visual_search_vot2014_drunk.con ../visual_search.con
cd ..
make clean
make
./visual_search visual_search_zoom_retrain.cml
mv resultadovot2014.txt vot2014/resultado_vot2014_drunk.txt
cd vot2014/

cp visual_search_vot2014_fernando.con ../visual_search.con
cd ..
make clean
make
./visual_search visual_search_zoom_retrain.cml
mv resultadovot2014.txt vot2014/resultado_vot2014_fernando.txt
cd vot2014/

cp visual_search_vot2014_fish1.con ../visual_search.con
cd ..
make clean
make
./visual_search visual_search_zoom_retrain.cml
mv resultadovot2014.txt vot2014/resultado_vot2014_fish1.txt
cd vot2014/

cp visual_search_vot2014_fish2.con ../visual_search.con
cd ..
make clean
make
./visual_search visual_search_zoom_retrain.cml
mv resultadovot2014.txt vot2014/resultado_vot2014_fish2.txt
cd vot2014/

cp visual_search_vot2014_gymnastics.con ../visual_search.con
cd ..
make clean
make
./visual_search visual_search_zoom_retrain.cml
mv resultadovot2014.txt vot2014/resultado_vot2014_gymnastics.txt
cd vot2014/

cp visual_search_vot2014_hand1.con ../visual_search.con
cd ..
make clean
make
./visual_search visual_search_zoom_retrain.cml
mv resultadovot2014.txt vot2014/resultado_vot2014_hand1.txt
cd vot2014/

cp visual_search_vot2014_hand2.con ../visual_search.con
cd ..
make clean
make
./visual_search visual_search_zoom_retrain.cml
mv resultadovot2014.txt vot2014/resultado_vot2014_hand2.txt
cd vot2014/

cp visual_search_vot2014_jogging.con ../visual_search.con
cd ..
make clean
make
./visual_search visual_search_zoom_retrain.cml
mv resultadovot2014.txt vot2014/resultado_vot2014_jogging.txt
cd vot2014/

cp visual_search_vot2014_motocross.con ../visual_search.con
cd ..
make clean
make
./visual_search visual_search_zoom_retrain.cml
mv resultadovot2014.txt vot2014/resultado_vot2014_motocross.txt
cd vot2014/

cp visual_search_vot2014_polarbear.con ../visual_search.con
cd ..
make clean
make
./visual_search visual_search_zoom_retrain.cml
mv resultadovot2014.txt vot2014/resultado_vot2014_polarbear.txt
cd vot2014/

cp visual_search_vot2014_skating.con ../visual_search.con
cd ..
make clean
make
./visual_search visual_search_zoom_retrain.cml
mv resultadovot2014.txt vot2014/resultado_vot2014_skating.txt
cd vot2014/

cp visual_search_vot2014_sphere.con ../visual_search.con
cd ..
make clean
make
./visual_search visual_search_zoom_retrain.cml
mv resultadovot2014.txt vot2014/resultado_vot2014_sphere.txt
cd vot2014/

cp visual_search_vot2014_sunshade.con ../visual_search.con
cd ..
make clean
make
./visual_search visual_search_zoom_retrain.cml
mv resultadovot2014.txt vot2014/resultado_vot2014_sunshade.txt
cd vot2014/

cp visual_search_vot2014_surfing.con ../visual_search.con
cd ..
make clean
make
./visual_search visual_search_zoom_retrain.cml
mv resultadovot2014.txt vot2014/resultado_vot2014_surfing.txt
cd vot2014/

cp visual_search_vot2014_torus.con ../visual_search.con
cd ..
make clean
make
./visual_search visual_search_zoom_retrain.cml
mv resultadovot2014.txt vot2014/resultado_vot2014_torus.txt
cd vot2014/

cp visual_search_vot2014_trellis.con ../visual_search.con
cd ..
make clean
make
./visual_search visual_search_zoom_retrain.cml
mv resultadovot2014.txt vot2014/resultado_vot2014_trellis.txt
cd vot2014/

cp visual_search_vot2014_tunnel.con ../visual_search.con
cd ..
make clean
make
./visual_search visual_search_zoom_retrain.cml
mv resultadovot2014.txt vot2014/resultado_vot2014_tunnel.txt
cd vot2014/

cp visual_search_vot2014_woman.con ../visual_search.con
cd ..
make clean
make
./visual_search visual_search_zoom_retrain.cml
mv resultadovot2014.txt vot2014/resultado_vot2014_woman.txt
cd vot2014/


rm resultadoFinal.txt
tail -n 1 resultado_vot2014_ball.txt >> resultadoFinal.txt
tail -n 1 resultado_vot2014_basketball.txt >> resultadoFinal.txt
tail -n 1 resultado_vot2014_bicycle.txt >> resultadoFinal.txt
tail -n 1 resultado_vot2014_bolt.txt >> resultadoFinal.txt
tail -n 1 resultado_vot2014_car.txt >> resultadoFinal.txt
tail -n 1 resultado_vot2014_david.txt >> resultadoFinal.txt
tail -n 1 resultado_vot2014_diving.txt >> resultadoFinal.txt
tail -n 1 resultado_vot2014_drunk.txt >> resultadoFinal.txt
tail -n 1 resultado_vot2014_fernando.txt >> resultadoFinal.txt
tail -n 1 resultado_vot2014_fish1.txt >> resultadoFinal.txt
tail -n 1 resultado_vot2014_fish2.txt >> resultadoFinal.txt
tail -n 1 resultado_vot2014_gymnastics.txt >> resultadoFinal.txt
tail -n 1 resultado_vot2014_hand1.txt >> resultadoFinal.txt
tail -n 1 resultado_vot2014_hand2.txt >> resultadoFinal.txt
tail -n 1 resultado_vot2014_jogging.txt >> resultadoFinal.txt
tail -n 1 resultado_vot2014_motocross.txt >> resultadoFinal.txt
tail -n 1 resultado_vot2014_polarbear.txt >> resultadoFinal.txt
tail -n 1 resultado_vot2014_skating.txt >> resultadoFinal.txt
tail -n 1 resultado_vot2014_sphere.txt >> resultadoFinal.txt
tail -n 1 resultado_vot2014_sunshade.txt >> resultadoFinal.txt
tail -n 1 resultado_vot2014_surfing.txt >> resultadoFinal.txt
tail -n 1 resultado_vot2014_torus.txt >> resultadoFinal.txt
tail -n 1 resultado_vot2014_trellis.txt >> resultadoFinal.txt
tail -n 1 resultado_vot2014_tunnel.txt >> resultadoFinal.txt
tail -n 1 resultado_vot2014_woman.txt >> resultadoFinal.txt

