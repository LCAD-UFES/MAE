#/bin/bash
cd traffic_sign_RED/
make clean;make -f Makefile.no_interface VERBOSE=0
cd ..
cd traffic_sign_GREEN/
make clean;make -f Makefile.no_interface VERBOSE=0
cd ..
cd traffic_sign_BLUE/
make clean;make -f Makefile.no_interface VERBOSE=0
cd ..

