CC := gcc296
FILES := $(shell ls *.con)
COMPILE := $(FILES:%.con=%)

run:
	netcompiler $(COMPILE)

clean:
	rm -f *.o trafic_sign_user_functions/*.o
	rm -f trafic_sign trafic_sign.c trafic_sign.h
	rm -f *.out
