CC := gcc296
FILES := $(shell ls *.con)
COMPILE := $(FILES:%.con=%)

build: CNAE_TAB/1.pnm
	netcompiler $(COMPILE)

CNAE_TAB/1.pnm:
	tar xzvf database.tgz
	
clean: 
	rm -f *.o face_recog_user_functions/*.o
	rm -f face_recog face_recog.c face_recog.h
	rm -f *.out
	rm -rf CNAE_TAB
	rm -rf DOCS_DB
