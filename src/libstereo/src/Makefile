##############################################################

BIN_PATH = ../../../bin
INC_PATH = ../include
LIB_PATH = ../../../lib
DOC_PATH = ../doc

##############################################################

CPP = g++
CC = gcc
CFLAGS = -O3 -Wall
LIBS = -L../lib -lTsai -lglut -lGLU -lGL -lm
AR = ar
ARFLAGS = -rc

##############################################################

EXECUTABLE = stereo
OBJECTS = stereo_system.o \
	    stereo_api.o \
	    camera.o
		
##############################################################

.c.o:	
	$(CC) $(CFLAGS) -c -I$(INC_PATH) -o $@ $*.c
	
.cpp.o:	
	$(CPP) $(CFLAGS) -c -I$(INC_PATH) -o $@ $*.cpp

##############################################################

build: libStereo.a
	@echo OK

libStereo.a: $(OBJECTS)
	$(AR) $(ARFLAGS) $(LIB_PATH)/libStereo.a $(OBJECTS)
	ranlib $(LIB_PATH)/libStereo.a 

test: $(EXECUTABLE)
	@echo OK
	
$(EXECUTABLE): $(OBJECTS)
	$(CPP) $(CFLAGS) -o $(BIN_PATH)/$(EXECUTABLE) $(OBJECTS) $(LIBS)
	
##############################################################

stereo_system.o: $(INC_PATH)/stereo_system.hpp
stereo_api.o: $(INC_PATH)/stereo_api.hpp
camera.o: $(INC_PATH)/camera.hpp

##############################################################

clean:
	rm -f *.o
	rm -f $(BIN_PATH)/$(EXECUTABLE)

##############################################################
