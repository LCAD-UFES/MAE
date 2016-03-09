FILES := $(shell ls *.con)
COMPILE := $(FILES:%.con=%)

run:
	netcompiler $(COMPILE) 

all:
	netcompiler $(COMPILE) 
	gcc -o capture `libwww-config --cflags` capture.c `libwww-config --libs` -I/usr/include/apache/xml

clean:
	rm -f smv smv.c smv.h
	rm -f *.o
	rm -f smv_user_functions/*.o
	rm -f *.out

cleanall:
	rm -f smv smv.c smv.h
	rm -f *.o
	rm -f smv_user_functions/*.o
	rm -f *.out
	rm -rf LogFile.txt
	rm -f capture
