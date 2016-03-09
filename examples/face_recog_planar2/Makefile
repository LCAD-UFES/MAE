CC = gcc
CCC = g++
CCFLAGS = -O4 -Wall -fopenmp

LIBPATH = -L/usr/local/lib -L/usr/X11R6/lib 
LIBPATH += -L$(MAEHOME)/lib
LIBPATH += -lnet_conn -lforms -lGA -lX11 -lglut -lGLU -lGL -lm -lfl -lstdc++
LIBPATH += `pkg-config --libs opencv`

##########################################################

EXECUTABLE = face_recog
OBJECTS = face_recog.o \
	  face_recog_user_functions/face_recog_user_functions.o \
	  face_recog_user_functions/face_recog_filters.o \
	  face_recog_user_functions/face_recog_utils.o \

##########################################################

build: $(EXECUTABLE)
	@echo OK

##########################################################
clean:
	rm -f *.o face_recog_user_functions/*.o 
	rm -f face_recog face_recog.c face_recog.h

##########################################################

$(EXECUTABLE): $(OBJECTS)
	$(CCC) $(CCFLAGS) -o $(EXECUTABLE) $(OBJECTS) $(LIBPATH)

##########################################################

.c.o:
	$(CC) $(CCFLAGS) -c `pkg-config --cflags opencv` -I$(MAEHOME)/include -o $@ $*.c

##########################################################

face_recog.o:
	$(MAEHOME)/bin/netcomp -o $* < $*.con	
	$(CC) $(CCFLAGS) -c -I$(MAEHOME)/include -o $@ $*.c

