CC := gcc296
FILES := $(shell ls *.con)
COMPILE := $(FILES:%.con=%)

run:
	netcompiler $(COMPILE) -pg

clean:
	rm -f *.o road_finding_user_functions/*.o
	rm -f road_finding road_finding.c road_finding.h
	rm -f *.out
