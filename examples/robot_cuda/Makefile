INCPATH = $(MAEHOME)/include
CUDALIB = -L/usr/local/cuda/include
LIBMAE = -L$(MAEHOME)/lib
LIBSHARED = -L$(SHAREDHOME)/lib
LIBPATH = -L/usr/local/lib -L/usr/X11R6/lib $(LIBMAE)
LIBRARYS = -lnet_conn -lforms -lGA  -lTsai -lXmu -lXext -lX11 -ltiff -lglut -lGLU -lGL -lXi -lm -lfl

CC = gcc
NCC = nvcc
CCFLAGS = -Wall -O3
#CFLAGS_GPU = -g -DCOD_C_CUDA
#CFLAGS_GPU = -g -deviceemu
CFLAGS_GPU = 	-O
 
CONFILE = $(shell ls *.con)
EXAMPLE = $(CONFILE:%.con=%)

##########################################################

EXECUTABLE = robot
OBJECTS = robot.o \
	  robot_user_functions/robot_user_functions.o \
	  robot_user_functions/robot_filters.o \
	  robot_user_functions/mapping.o \
	  robot_user_functions/cuda_functions.o
GPU_OBJECTS = robot.o \
	  robot_user_functions.o \
	  robot_filters.o \
	  mapping.o \
	  cuda_functions.o

##########################################################

build: $(EXECUTABLE)
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CCFLAGS) -o $(EXECUTABLE) $(OBJECTS) $(LIBPATH) $(LIBRARYS)

##########################################################

gpu:
	$(MAEHOME)/bin/netcomp -o robot < robot.con
	gcc -c $(CCFLAGS) -I$(INCPATH) robot.c
	gcc -c $(CCFLAGS) -I$(INCPATH) robot_user_functions/robot_user_functions.c
	gcc -c $(CCFLAGS) -I$(INCPATH) robot_user_functions/robot_filters.c
	gcc -c $(CCFLAGS) -I$(INCPATH) robot_user_functions/mapping.c
	nvcc -c $(CFLAGS_GPU) -I$(INCPATH) robot_user_functions/cuda_functions.cu
	nvcc $(CFLAGS_GPU) -o $(EXECUTABLE) $(GPU_OBJECTS) $(LIBPATH) $(LIBRARYS) -I$(INCPATH)

##########################################################

robot.o:
	$(MAEHOME)/bin/netcomp -o $* < $*.con	
	$(CC) $(CCFLAGS) -c -I$(INCPATH) -o $@ $*.c

##########################################################

robot_user_functions.o: $(INCPATH)/mae.h $(INCPATH)/filter.h

##########################################################

.c.o:
	@echo OK
	$(CC) $(CCFLAGS) -c -I$(INCPATH) -o $@ $*.c

##########################################################

.cu.o:
	@echo OK
	$(NCC) -c $(CFLAGS_GPU) -I$(INCPATH) -o $@ $*.cu

##########################################################

clean:
	rm -f *.o $(EXAMPLE)_user_functions/*.o
	rm -f *.linkinfo $(EXAMPLE)_user_functions/*.linkinfo
	rm -f $(EXAMPLE)
	rm -f $(EXAMPLE).[c,h]
	rm -f *.out
	rm -f $(EXAMPLE).sh.*
	rm -f core.*
