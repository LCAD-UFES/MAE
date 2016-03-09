CC := gcc
FILES := $(shell ls *.con)
COMPILE := $(FILES:%.con=%)

all: build

build:
	netcompiler $(COMPILE)

clean:
	rm -f *.o text_detect_user_functions/*.o
	rm -f text_detect text_detect.c text_detect.h
	rm -f *.out
