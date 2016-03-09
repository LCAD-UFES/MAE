##########################################

CC = gcc
RELEASE = -O3 -ffast-math -w
#DEBUG = -g
CFLAGS = $(DEBUG) $(RELEASE) -Wall
LIBRARY = libTsai.a
OBJECTS = cal_main.o \
	  ecalmain.o \
	  cal_tran.o \
	  cal_eval.o \
	  cal_util.o \
	  matrix/matrix.o \
	  minpack/dpmpar.o \
	  minpack/enorm.o \
	  minpack/fdjac2.o \
	  minpack/lmdif.o \
	  minpack/lmpar.o \
	  minpack/qrfac.o \
	  minpack/qrsolv.o

#########################################

.c.o: cal_main.h
	$(CC) $(CFLAGS) -c -o $@ $*.c

#########################################

all: $(OBJECTS)
	ar cr $(LIBRARY) $(OBJECTS)
	ranlib $(LIBRARY)

#########################################

clean:
	rm -f $(LIBRARY)
	rm -f $(OBJECTS)
