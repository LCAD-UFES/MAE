CC = gcc
CCC = g++
CCFLAGS = -O4 -Wall -fopenmp

LIBPATH = -L/usr/local/lib -L/usr/X11R6/lib 
LIBPATH += -L$(MAEHOME)/lib
LIBPATH += -lnet_conn -lforms -lGA -lX11 -lglut -lGLU -lGL -lm -lfl -lstdc++
LIBPATH += `pkg-config --libs opencv`

##########################################################

EXECUTABLE = traffic_sign
OBJECTS = traffic_sign.o \
	  traffic_sign_user_functions/traffic_sign_user_functions.o \
	  traffic_sign_user_functions/traffic_sign_filters.o \
	  traffic_sign_user_functions/traffic_sign_utils.o \
	  traffic_sign_clahe/clahe.o

##########################################################

build: $(EXECUTABLE)
	@echo OK

##########################################################
clean:
	rm -f *.o traffic_sign_user_functions/*.o traffic_sign_clahe/*.o
	rm -f traffic_sign traffic_sign.c traffic_sign.h

##########################################################

$(EXECUTABLE): $(OBJECTS)
	$(CCC) $(CCFLAGS) -o $(EXECUTABLE) $(OBJECTS) $(LIBPATH)

##########################################################

traffic_sign_clahe/clahe.o: traffic_sign_clahe/clahe.cpp traffic_sign_clahe/clahe.h
	$(CCC) $(CCFLAGS) -c `pkg-config --cflags opencv` -o $@ $<

.c.o:
	$(CC) $(CCFLAGS) -c `pkg-config --cflags opencv` -I$(MAEHOME)/include -o $@ $*.c

##########################################################

traffic_sign.o:
	$(MAEHOME)/bin/netcomp -o $* < $*.con	
	$(CC) $(CCFLAGS) -c -I$(MAEHOME)/include -o $@ $*.c

