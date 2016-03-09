FILES := $(shell ls *.con)
COMPILE := $(FILES:%.con=%)

run:
	netcompiler $(COMPILE) 
	
clean:
	rm -f *.core
	rm -f *.o
	rm -f robot
	rm -f robot.[c,h]
	rm -f */*.o
	rm -f *.out
