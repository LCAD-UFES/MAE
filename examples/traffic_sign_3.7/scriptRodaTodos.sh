#/bin/bash
cd traffic_sign_3.6_RED/
./traffic_sign traffic_sign.cml 2> resultado.txt
cd ..
cd traffic_sign_3.6_GREEN/
./traffic_sign traffic_sign.cml 2> resultado.txt
cd ..
cd traffic_sign_3.6_BLUE/
./traffic_sign traffic_sign.cml 2> resultado.txt

