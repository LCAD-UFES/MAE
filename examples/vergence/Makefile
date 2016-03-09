FILES := $(shell ls *.con)
COMPILE := $(FILES:%.con=%)

run:
	netcompiler $(COMPILE) 
	
clean:
	rm -rf *.core
	rm -rf *.o
	rm -rf *.dat
