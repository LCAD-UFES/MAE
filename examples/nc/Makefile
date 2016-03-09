FILES := $(shell ls *.con)
COMPILE := $(FILES:%.con=%)

build: 
	${MAEHOME}/bin/netcompiler $(COMPILE)

clean: 
	rm -f *.o nc_user_functions/*.o
	rm -f nc nc.c nc.h

documentation:
	$(MAKE) documentation -C docs

