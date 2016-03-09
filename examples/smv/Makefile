FILES := $(shell ls *.con)
COMPILE := $(FILES:%.con=%)

run:
	netcompiler $(COMPILE) 
clean:
	rm smv smv.c
	rm *.o
	rm  smv_user_functions/*.o
	rm *.out
	rm stockPileData.msh stockPileData.geo
