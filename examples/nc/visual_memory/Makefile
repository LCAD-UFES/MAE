FACE_RECOG_DIR=$(MAEHOME)/examples/face_recog_planar
CC=gcc
CCC=g++
CCFLAGS=-D NO_INTERFACE -Wall -g #-O3
AR = ar
ARFLAGS = -rc
LIBS = -lcv -lcxcore -lhighgui -lface_recog -lnet_conn -lforms -lGA -lX11 -lglut -lGLU -lGL -lm -lfl
LIBPATH = -L/usr/local/lib -L/usr/X11R6/lib -L$(MAEHOME)/lib.no_interface -L$(FACE_RECOG_DIR)
INCPATH = -I/usr/local/include/opencv -I./ -I$(MAEHOME)/include -I$(FACE_RECOG_DIR)/face_recog_user_functions

##########################################################

EXECUTABLE = face_detect_app
LIBRARY = libnc_visual_memory.a
OBJECTS = face_detect_api.o FaceDetectApp.o
	  
##########################################################

.c.o:
	$(CC) $(CCFLAGS) -c $(INCPATH) -o $@ $*.c

.cpp.o:	
	$(CCC) $(CCFLAGS) -c $(INCPATH) -o $@ $*.cpp
	
##########################################################

build: $(EXECUTABLE) $(LIBRARY)
	@echo OK

$(EXECUTABLE): $(OBJECTS)
	$(CCC) $(CCFLAGS) -o $(EXECUTABLE) $(OBJECTS) $(LIBPATH) $(LIBS)
	
$(LIBRARY): $(OBJECTS)
	$(AR) $(ARFLAGS) $(LIBRARY) $(OBJECTS)
	ranlib $(LIBRARY) 

##########################################################

face_detect_api.o:
	$(CCC) $(CCFLAGS) -c $(INCPATH) -o $@ $*.cpp
	
FaceDetectApp.o:
	$(CCC) $(CCFLAGS) -c $(INCPATH) -o $@ $*.cpp

##########################################################

clean:
	rm -f *.o
	rm -f $(EXECUTABLE)
	rm -f $(LIBRARY)
