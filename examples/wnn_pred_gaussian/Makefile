CC := gcc296
FILES := $(shell ls *.con)
COMPILE := $(FILES:%.con=%)

build: 
	netcompiler $(COMPILE)

clean: 
	rm -f *.o wnn_pred_user_functions/*.o
	rm -f wnn_pred wnn_pred.c wnn_pred.h
