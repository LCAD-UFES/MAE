FILES := $(shell ls *.con)
COMPILE := $(FILES:%.con=%)

run:
	$(MAEHOME)/bin/netcompiler $(COMPILE) 
clean:
	rm -rf *.o character_user_functions/*.o character.c character.h gmon.out
	

