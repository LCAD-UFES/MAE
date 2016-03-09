#/bin/bash
cd traffic_sign_3.6_RED/
make clean;make -f Makefile.no_interface
cd ..
cd traffic_sign_3.6_GREEN/
make clean;make -f Makefile.no_interface
cd ..
cd traffic_sign_3.6_BLUE/
make clean;make -f Makefile.no_interface
cd ..

