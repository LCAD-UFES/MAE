FILES := $(shell ls *.con)
COMPILE := $(FILES:%.con=%)

build: 
	netcompiler $(COMPILE)
	gcc -g -Wall -o TRADING_SYSTEM/trading_system TRADING_SYSTEM/trading_system.c -I${MAEHOME}/include -I./TRADING_SYSTEM TRADING_SYSTEM/timeutil.c -lm

clean: 
	rm -f *.o wnn_pred_user_functions/*.o TRADING_SYSTEM/*.o
	rm -f wnn_pred wnn_pred.c wnn_pred.h TRADING_SYSTEM/trading_system
