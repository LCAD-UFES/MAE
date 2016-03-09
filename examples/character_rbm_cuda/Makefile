CC = gcc
CCC = g++
NCC = nvcc

#CCFLAGS = -g -Wall
#CCFLAGS_GPU = -g
CCFLAGS = -O4 -Wall -mpopcnt -mfpmath=sse -msse2 -msse4.2 -ffast-math -fassociative-math -mtune=native -march=native
#CCFLAGS = -O4 -Wall -fopenmp -mpopcnt -mfpmath=sse -msse2 -msse4.2 -ffast-math -fassociative-math -mtune=native -march=native
CCFLAGS_GPU = #-Xcompiler " -fopenmp" 

LIBPATH = -L/usr/local/lib -L/usr/X11R6/lib 
LIBPATH += -L$(MAEHOME)/lib
LIBPATH += -lnet_conn -lforms -lGA -lX11 -lglut -lGLU -lGL -lm -lfl -lstdc++
INCPATH = $(MAEHOME)/include

##########################################################

EXECUTABLE = character
LIBRARY = libcharacter.a
OBJECTS = character.o \
	  character_user_functions/character_user_functions.o \
	  character_user_functions/math_functions_wrapper.o \
	  character_user_functions/math_functions.o \
	  character_user_functions/rbm.o 
GPU_OBJECTS = character_user_functions/math_functions_cuda.o 

.SUFFIXES: .cu .cpp .c .o

##########################################################

build: $(EXECUTABLE)
	@echo OK

##########################################################

clean:
	rm -f *.o character_user_functions/*.o
	rm -f character character.c character.h

##########################################################

cuda: $(OBJECTS) $(GPU_OBJECTS)
	nvcc $(CCFLAGS_GPU) -o $(EXECUTABLE) $(OBJECTS) $(GPU_OBJECTS) $(LIBPATH) -I$(MAEHOME)/include -lcublas
	@echo OK

##########################################################

$(EXECUTABLE): $(OBJECTS)
	$(CCC) $(CCFLAGS) -o $(EXECUTABLE) $(OBJECTS) $(LIBPATH)

$(LIBRARY): $(OBJECTS)
	$(AR) $(ARFLAGS) $(LIBRARY) $(OBJECTS)
	ranlib $(LIBRARY)

##########################################################

.c.o:
	$(CC) $(CCFLAGS) -c -I$(MAEHOME)/include -o $@ $*.c

.cpp.o:
	$(CCC) $(CCFLAGS) -c -I$(MAEHOME)/include -o $@ $*.cpp

.cu.o:
	$(NCC) -c $(CCFLAGS_GPU) -I$(CUDA_ROOT)/samples/common/inc -I/home/avelino/cuda/samples/common/inc -o $@ $*.cu

character.o: character.con
	$(MAEHOME)/bin/netcomp -o $* < $*.con	
	$(CC) $(CCFLAGS) -c -I$(MAEHOME)/include -o $@ $*.c


