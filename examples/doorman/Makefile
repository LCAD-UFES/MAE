CC := gcc296
FILES := $(shell ls *.con)
COMPILE := $(FILES:%.con=%)

run:
	netcompiler $(COMPILE)

clean:
	rm -f *.o */*.o doorman doorman.c doorman.h core *.out

cleanall:
	rm -f *.o */*.o doorman doorman.c doorman.h core *.out
	rm -f *.sh.* *.test


