CC=gcc
CCFLAGS= -Wall -g -O3
LIBS = -lnet_conn -lGA -lstereo -lXmu -lXext -lX11 -ltiff -lglut -lGLU -lGL -lXi -lforms -lm -lfl 
LIBPATH = -L/usr/local/lib -L/usr/X11R6/lib -L../../lib -L./
INCPATH = -I/usr/local/matlab7/extern/include -I../../include -I./ -I./robot_user_functions 

##########################################################

EXECUTABLE = robot
OBJECTS = robot.o \
	  robot_user_functions/robot_user_functions.o \
	  robot_user_functions/robot_filters.o \
	  robot_user_functions/smv_api.o \
	  robot_user_functions/viewer.o \
	  robot_user_functions/map.o \
	  robot_user_functions/convert.o \
	  robot_user_functions/view.o 
	  
##########################################################

.c.o:
	$(CC) $(CCFLAGS) -c $(INCPATH) -o $@ $*.c

.cpp.o:	
	g++ $(CCFLAGS) -c -I$(INCPATH) -o $@ $*.cpp
	
##########################################################

build: $(EXECUTABLE)
	@echo OK

$(EXECUTABLE): $(OBJECTS)
	g++ $(CCFLAGS) -o $(EXECUTABLE) $(OBJECTS) $(LIBPATH) $(LIBS)

##########################################################

robot.o: libstereo.so robot.con
	netcomp -o $* < $*.con
	$(CC) $(CCFLAGS) -c $(INCPATH) -o $@ $*.c

libstereo.so:
	$(MAKE) libstereo.so -C robot_user_functions/matlab_scripts
	
robot_user_functions/robot_user_functions.o: robot_user_functions/robot_user_functions.h robot.h
robot_user_functions/smv_api.o: robot_user_functions/smv_api.h
robot_user_functions/robot_filters.o: ../../include/filter.h
robot_user_functions/viewer.o: robot_user_functions/viewer.hpp
robot_user_functions/map.o: robot_user_functions/map.hpp
robot_user_functions/convert.o: robot_user_functions/convert.hpp
robot_user_functions/view.o: robot_user_functions/view.hpp	  

##########################################################

clean:
	rm -f *.o robot_user_functions/*.o
	rm -f robot
	rm -f robot.[c,h]
	rm -f *.out
	rm -f robot.sh.*
	rm -f robot_user_functions/matlab_scripts/libstereo.so 
	rm -f libstereo.so 
	
