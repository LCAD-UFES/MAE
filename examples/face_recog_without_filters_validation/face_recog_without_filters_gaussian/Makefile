CC := gcc296
FILES := $(shell ls *.con)
COMPILE := $(FILES:%.con=%)

run:
	netcompiler $(COMPILE)

clean:
	rm -f *.o face_recog_user_functions/*.o
	rm -f face_recog face_recog.c face_recog.h
	rm -f *.out
