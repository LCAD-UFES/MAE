CC := gcc296 
FILES := $(shell ls *.con)
COMPILE := $(FILES:%.con=%)

run:
	netcompiler $(COMPILE)

clean:
	rm -f *.o snn_user_functions/*.o
	rm -f snn snn.c snn.h
	rm -f *.out
