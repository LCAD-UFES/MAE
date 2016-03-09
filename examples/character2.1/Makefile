FILES := $(shell ls *.con)
COMPILE := $(FILES:%.con=%)

run:
	netcompiler $(COMPILE) 
clean:
	rm -f *.o character_user_functions/*.o
	rm -f character character.c character.h
	rm -rf character	
	#rm -rf *.txt
	#rm -rf character_result.csv
