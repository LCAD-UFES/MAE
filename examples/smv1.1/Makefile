FILES := $(shell ls *.con)
COMPILE := $(FILES:%.con=%)

run:
	netcompiler $(COMPILE) 
clean:
	rm -f smv smv.c
	rm -f *.o
	rm -f smv_user_functions/*.o
	rm -f *.out
	rm -f stockPileData.msh stockPileData.geo
