FILES := $(shell ls *.con)
COMPILE := $(FILES:%.con=%)

run:
	netcompiler $(COMPILE) 
	
clean:
	rm -f *.core
	rm -f *.o
	rm -f robot
	rm -f robot.c
	rm -f */*.o
	rm -f *.out
