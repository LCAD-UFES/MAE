FILES := $(shell ls *.con)
COMPILE := $(FILES:%.con=%)

all:
	netcompiler $(COMPILE) 
clean:
	rm -rf character.c character.h character *.o character_user_functions/*.o gmon.out
