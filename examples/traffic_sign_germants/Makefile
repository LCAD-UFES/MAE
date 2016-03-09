CC := gcc296
FILES := $(shell ls *.con)
COMPILE := $(FILES:%.con=%)

run:
	netcompiler $(COMPILE)

clean:
	rm -f *.o traffic_sign_user_functions/*.o
	rm -f traffic_sign traffic_sign.c traffic_sign.h
	rm -f *.out
	rm -f *~ 
