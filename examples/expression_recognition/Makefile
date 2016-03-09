CC = gcc
CCC = g++
#CCFLAGS = -O4 -Wall -fopenmp
CCFLAGS = -g -Wall  

LIBPATH = -L/usr/local/lib -L/usr/X11R6/lib 
LIBPATH += -L$(MAEHOME)/lib
LIBPATH += -fopenmp -lnet_conn -lforms -lGA -lX11 -lglut -lGLU -lGL -lm -lfl -lstdc++ 
LIBPATH += `pkg-config --libs opencv`

##########################################################

EXECUTABLE = expression_recognition 
OBJECTS = expression_recognition.o \
	  expression_recognition_user_functions/expression_recognition_user_functions.o \
	  expression_recognition_user_functions/expression_recognition_filters.o \
	  expression_recognition_user_functions/expression_recognition_utils.o

##########################################################

build: $(EXECUTABLE)

##########################################################
clean:
	rm -f *.o expression_recognition_user_functions/*.o 
	rm -f expression_recognition expression_recognition.c expression_recognition.h

##########################################################

$(EXECUTABLE): $(OBJECTS)
	$(CCC) $(CCFLAGS) -o $(EXECUTABLE) $(OBJECTS) $(LIBPATH)

##########################################################

.c.o:
	$(CC) $(CCFLAGS) -c `pkg-config --cflags opencv` -I$(MAEHOME)/include -o $@ $*.c

##########################################################

expression_recognition.o: expression_recognition.con
	$(MAEHOME)/bin/netcomp -o $* < $*.con	
	$(CC) $(CCFLAGS) -c -I$(MAEHOME)/include -o $@ $*.c

