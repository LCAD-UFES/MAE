FILES := $(shell ls *.con)
COMPILE := $(FILES:%.con=%)

run:
	netcompiler $(COMPILE) 
	
clean:
	rm -f *.core
	rm -f *.o
	rm -f stereo
	rm -f stereo.c
	rm -f stereo.h
	rm -f */*.o
	rm -f *.out
