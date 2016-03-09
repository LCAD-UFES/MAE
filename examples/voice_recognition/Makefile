CC = gcc
CCC = g++
#CCFLAGS = -O4 -Wall -fopenmp
CCFLAGS = -g -Wall  
#CCFLAGS = -g -Wall 

LIBPATH = -L/usr/local/lib -L/usr/X11R6/lib 
LIBPATH += -L$(MAEHOME)/lib
LIBPATH += -fopenmp -lnet_conn -lforms -lGA -lX11 -lglut -lGLU -lGL -lm -lfl -lstdc++ -lasound -lfftw3
LIBPATH += `pkg-config --libs opencv`

##########################################################

EXECUTABLE = voice_recognition
OBJECTS = voice_recognition.o \
	  voice_recognition_user_functions/voice_recognition_user_functions.o \
	  voice_recognition_user_functions/voice_recognition_filters.o \
	  voice_recognition_user_functions/voice_recognition_utils.o 
	  # voice_recognition_user_functions/ring_buffer.o 

##########################################################

build: $(EXECUTABLE)
	@echo OK

##########################################################
clean:
	rm -f *.o voice_recognition_user_functions/*.o 
	rm -f voice_recognition voice_recognition.c voice_recognition.h
	make clean -C voice_recognition_user_functions/fft

##########################################################

$(EXECUTABLE): $(OBJECTS)
	$(CCC) $(CCFLAGS) -o $(EXECUTABLE) $(OBJECTS) $(LIBPATH)

##########################################################

.c.o:
	$(CC) $(CCFLAGS) -c `pkg-config --cflags opencv` -I$(MAEHOME)/include -o $@ $*.c

##########################################################

voice_recognition.o: voice_recognition.con
	$(MAEHOME)/bin/netcomp -o $* < $*.con	
	$(CC) $(CCFLAGS) -c -I$(MAEHOME)/include -o $@ $*.c
	make -C voice_recognition_user_functions/fft

