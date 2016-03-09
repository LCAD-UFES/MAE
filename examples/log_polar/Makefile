CC := gcc296
FILES := $(shell ls *.con)
COMPILE := $(FILES:%.con=%)

run:
	netcompiler $(COMPILE)

clean:
	rm -f *.o */*.o log_polar log_polar.c core *.out
