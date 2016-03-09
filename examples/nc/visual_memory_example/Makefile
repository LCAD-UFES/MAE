FACE_RECOG_DIR=$(MAEHOME)/examples/face_recog_planar
VISUAL_MEMORY_DIR=$(MAEHOME)/examples/nc/visual_memory
CC=gcc
CCC=g++
CCFLAGS=-D NO_INTERFACE -Wall -g -O3
AR = ar
ARFLAGS = -rc
LIBS = -lcv -lcxcore -lhighgui -lface_recog -lnc_visual_memory -lnet_conn -lforms -lGA -lX11 -lglut -lGLU -lGL -lm -lfl
LIBPATH = -L/usr/local/lib -L/usr/X11R6/lib -L$(MAEHOME)/lib.no_interface -L$(FACE_RECOG_DIR) -L$(VISUAL_MEMORY_DIR)
INCPATH = -I/usr/local/include/opencv -I./ -I$(MAEHOME)/include -I$(FACE_RECOG_DIR)/face_recog_user_functions -I$(VISUAL_MEMORY_DIR)

##########################################################

EXECUTABLE = VisualMemoryExample
OBJECTS = VisualMemoryExample.o
	  
##########################################################

.c.o:
	$(CC) $(CCFLAGS) -c $(INCPATH) -o $@ $*.c

.cpp.o:	
	$(CCC) $(CCFLAGS) -c $(INCPATH) -o $@ $*.cpp
	
##########################################################

build: $(EXECUTABLE)
	@echo OK

$(EXECUTABLE): $(OBJECTS)
	$(CCC) $(CCFLAGS) -o $(EXECUTABLE) $(OBJECTS) $(LIBPATH) $(LIBS)

##########################################################

VisualMemoryExample.o:
	$(CCC) $(CCFLAGS) -c $(INCPATH) -o $@ $*.cpp

##########################################################

clean:
	rm -f *.o
	rm -f $(EXECUTABLE)
