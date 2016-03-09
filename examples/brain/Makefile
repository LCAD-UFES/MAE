CC := gcc296
FILES := $(shell ls *.con)
COMPILE := $(FILES:%.con=%)

run:
	netcompiler $(COMPILE) 
clean:
	rm -rf *.o 
	rm -rf brain_user_functions/*.o
	rm -rf brain
	rm -rf brain.c
	

