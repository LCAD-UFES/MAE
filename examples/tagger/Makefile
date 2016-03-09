CC := gcc296
FILES := $(shell ls *.con)
COMPILE := $(FILES:%.con=%)

build: 
	netcompiler $(COMPILE)
	
clean: 
	rm -f *.o class_cnae_user_functions/*.o
	rm -f class_cnae class_cnae.c class_cnae.h
	rm -f *.out
