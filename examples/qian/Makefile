FILES := $(shell ls *.con)
COMPILE := $(FILES:%.con=%)

run:
	netcompiler $(COMPILE) 
clean:
	rm qian
	rm qian.c
	rm *.o
	rm qian_user_functions/*.o	

