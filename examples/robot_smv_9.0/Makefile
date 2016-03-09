CC=gcc
CCC=g++
CCFLAGS= -Wall -g
LIBS = -lnet_conn -lGA -lstereo -lXmu -lXext -lX11 -ltiff -lglut -lGLU -lGL -lXi -lforms -lm -lfl -lgsl -lgslcblas
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
	  robot_user_functions/maplist.o \
	  robot_user_functions/convert.o \
	  robot_user_functions/view.o \
	  robot_user_functions/cylinder.o \
	  robot_user_functions/estimate_volume.o \
	  robot_user_functions/fitting.o \
	  robot_user_functions/subpixel_disparity.o \
	  robot_user_functions/gabor_guess_cylinder.o \
	  robot_user_functions/geometric_operations.o \
	  robot_user_functions/solver.o
LIBSTEREO = robot_user_functions/matlab_scripts/makelibstereo.m \
	    robot_user_functions/matlab_scripts/LoadStereoParameters.m \
	    robot_user_functions/matlab_scripts/StereoTriangulation.m \
	    robot_user_functions/matlab_scripts/LoadRectificationIndexes.m \
	    robot_user_functions/matlab_scripts/RectifyStereoPair.m \
	    robot_user_functions/matlab_scripts/RectifyLeftImage.m \
	    robot_user_functions/matlab_scripts/RectifyRightImage.m \
	    robot_user_functions/matlab_scripts/CameraProjection.m \
	    robot_user_functions/matlab_scripts/GetWorldPointAtDistance.m \
	    robot_user_functions/matlab_scripts/stereo_api.c \
	    robot_user_functions/matlab_scripts/stereo_api.h 
	  
##########################################################

.c.o:
	$(CC) $(CCFLAGS) -c $(INCPATH) -o $@ $*.c

.cpp.o:	
	$(CCC) $(CCFLAGS) -c -I$(INCPATH) -o $@ $*.cpp
	
##########################################################

build: $(EXECUTABLE)
	@echo OK

$(EXECUTABLE): $(OBJECTS) ../../lib/libnet_conn.a
	$(CCC) $(CCFLAGS) -o $(EXECUTABLE) $(OBJECTS) $(LIBPATH) $(LIBS)

##########################################################

robot.o: libstereo.so robot.con
	netcomp -o $* < $*.con
	$(CC) $(CCFLAGS) -c $(INCPATH) -o $@ $*.c

libstereo.so: $(LIBSTEREO)
	$(MAKE) -C robot_user_functions/matlab_scripts
	
robot_user_functions/robot_user_functions.o: robot.h robot_user_functions/robot_user_functions.h robot_user_functions/viewer.hpp robot_user_functions/matlab_scripts/stereo_api.h
robot_user_functions/smv_api.o: robot_user_functions/smv_api.h robot_user_functions/matlab_scripts/stereo_api.h
robot_user_functions/robot_filters.o: ../../include/filter.h
robot_user_functions/viewer.o: robot_user_functions/viewer.hpp
robot_user_functions/map.o: robot_user_functions/map.hpp
robot_user_functions/convert.o: robot_user_functions/convert.hpp
robot_user_functions/view.o: robot_user_functions/view.hpp	  
robot_user_functions/cylinder.o: robot.h robot_user_functions/robot_user_functions.h robot_user_functions/cylinder.h robot.h robot_user_functions/view.hpp
robot_user_functions/estimate_volume.o: robot_user_functions/estimate_volume.h robot_user_functions/matlab_scripts/stereo_api.h robot_user_functions/robot_user_functions.h
robot_user_functions/fitting.o: robot_user_functions/fitting.h robot_user_functions/cylinder.h 
robot_user_functions/solver.o: robot_user_functions/solver.h
robot_user_functions/subpixel_disparity.o: robot_user_functions/subpixel_disparity.h
robot_user_functions/gabor_guess_cylinder.o: robot_user_functions/gabor_guess_cylinder.h
robot_user_functions/geometric_operations.o: robot_user_functions/geometric_operations.h


##########################################################

clean:
	rm -f *.o robot_user_functions/*.o
	rm -f robot
	rm -f robot.[c,h]
	rm -f *.out
	rm -f robot.sh.*
	rm -f robot_user_functions/matlab_scripts/libstereo.so 
	rm -f libstereo.so 
	
