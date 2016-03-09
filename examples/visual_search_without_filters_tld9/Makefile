CC = gcc
CCC = g++
CCFLAGS = -O4 -Wall -fopenmp -mpopcnt -mfpmath=sse -msse2 -msse4.2 -ffast-math -fassociative-math -mtune=native -march=corei7
#CCFLAGS = -O4 -Wall -fopenmp
#CCFLAGS = -g -ggdb -Wall -fopenmp

LIBPATH = -L/usr/local/lib -L/usr/X11R6/lib 
LIBPATH += -L$(MAEHOME)/lib
LIBPATH += -lnet_conn -lforms -lGA -lX11 -lglut -lGLU -lGL -lm -lfl -lstdc++
#LIBPATH += `pkg-config --libs opencv`

##########################################################

EXECUTABLE = visual_search
OBJECTS = visual_search.o \
	  visual_search_user_functions/visual_search_user_functions.o \
	  visual_search_user_functions/visual_search_filters.o \
	  visual_search_user_functions/visual_search_utils.o #\
#	  visual_search_clahe/clahe.o

##########################################################

build: $(EXECUTABLE)
	@echo OK

##########################################################
clean:
	rm -f *.o visual_search_user_functions/*.o visual_search_clahe/*.o
	rm -f visual_search visual_search.c visual_search.h

##########################################################

$(EXECUTABLE): $(OBJECTS)
	$(CCC) $(CCFLAGS) -o $(EXECUTABLE) $(OBJECTS) $(LIBPATH)

##########################################################

#visual_search_clahe/clahe.o: visual_search_clahe/clahe.cpp visual_search_clahe/clahe.h
#	$(CCC) $(CCFLAGS) -c `pkg-config --cflags opencv` -o $@ $<

.c.o:
	$(CC) $(CCFLAGS) -c -I$(MAEHOME)/include -o $@ $*.c
#	$(CC) $(CCFLAGS) -c `pkg-config --cflags opencv` -I$(MAEHOME)/include -o $@ $*.c

##########################################################

visual_search.o: visual_search.con
	$(MAEHOME)/bin/netcomp -o $* < $*.con	
	$(CC) $(CCFLAGS) -c -I$(MAEHOME)/include -o $@ $*.c

