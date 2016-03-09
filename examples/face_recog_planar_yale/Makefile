CC := gcc296
FILES := $(shell ls *.con)
COMPILE := $(FILES:%.con=%)

run:
	netcompiler $(COMPILE)
	gcc -o generate_random_faces generate_random_faces.c

clean:
	rm -f *.o face_recog_user_functions/*.o
	rm -f face_recog face_recog.c face_recog.h
	rm -f *.out
	rm -f generate_random_faces
