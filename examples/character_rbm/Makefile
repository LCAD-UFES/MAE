CC = gcc
CCC = g++

#CCFLAGS = -g -Wall -fopenmp -mpopcnt -mfpmath=sse -msse2 -msse4.2 -ffast-math -fassociative-math -mtune=native -march=native
CCFLAGS = -O4 -Wall -fopenmp -mpopcnt -mfpmath=sse -msse2 -msse4.2 -ffast-math -fassociative-math -mtune=native -march=native

LIBPATH = -L/usr/local/lib -L/usr/X11R6/lib 
LIBPATH += -L$(MAEHOME)/lib
LIBPATH += -lnet_conn -lforms -lGA -lX11 -lglut -lGLU -lGL -lm -lfl -lfann -lstdc++

##########################################################

EXECUTABLE = character
LIBRARY = libcharacter.a
OBJECTS = character.o \
	  character_user_functions/character_user_functions.o \
	  character_user_functions/rbm.o 

##########################################################

build: $(EXECUTABLE)
	@echo OK

##########################################################

clean:
	rm -f *.o character_user_functions/*.o
	rm -f character character.c character.h

##########################################################

$(EXECUTABLE): $(OBJECTS)
	$(CCC) $(CCFLAGS) -o $(EXECUTABLE) $(OBJECTS) $(LIBPATH)

$(LIBRARY): $(OBJECTS)
	$(AR) $(ARFLAGS) $(LIBRARY) $(OBJECTS)
	ranlib $(LIBRARY)

##########################################################

.c.o:
	$(CC) $(CCFLAGS) -c -I$(MAEHOME)/include -I. -o $@ $*.c

.cpp.o:
	$(CCC) $(CCFLAGS) -c -I$(MAEHOME)/include -I. -o $@ $*.cpp

character.o: character.con
	$(MAEHOME)/bin/netcomp -o $* < $*.con	
	$(CC) $(CCFLAGS) -c -I$(MAEHOME)/include -I. -o $@ $*.c


