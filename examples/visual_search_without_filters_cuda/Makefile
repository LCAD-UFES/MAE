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
CFLAGS_GPU = -O -gencode arch=compute_20,code=sm_20
 
CONFILE = $(shell ls *.con)
EXAMPLE = $(CONFILE:%.con=%)

##########################################################

EXECUTABLE = visual_search
OBJECTS = visual_search.o \
	  visual_search_user_functions/visual_search_user_functions.o \
	  visual_search_user_functions/visual_search_filters.o \
	  visual_search_user_functions/cuda_functions.o
GPU_OBJECTS = visual_search.o \
	  visual_search_user_functions.o \
	  visual_search_filters.o \
	  cuda_functions.o

##########################################################

build: $(EXECUTABLE)
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CCFLAGS) -o $(EXECUTABLE) $(OBJECTS) $(LIBPATH) $(LIBRARYS)

##########################################################

gpu:
	$(MAEHOME)/bin/netcomp -o visual_search < visual_search.con
	gcc -c -Wall $(CCFLAGS) -I$(INCPATH) visual_search.c
	gcc -c -Wall $(CCFLAGS) -I$(INCPATH) visual_search_user_functions/visual_search_user_functions.c
	gcc -c -Wall $(CCFLAGS) -I$(INCPATH) visual_search_user_functions/visual_search_filters.c
	nvcc -c $(CFLAGS_GPU) -I$(INCPATH) visual_search_user_functions/cuda_functions.cu
	nvcc $(CFLAGS_GPU) -o $(EXECUTABLE) $(GPU_OBJECTS) $(LIBPATH) $(LIBRARYS) -I$(INCPATH)

##########################################################

visual_search.o:
	$(MAEHOME)/bin/netcomp -o $* < $*.con	
	$(CC) $(CCFLAGS) -c -I$(INCPATH) -o $@ $*.c

##########################################################

visual_search_user_functions.o: $(INCPATH)/mae.h $(INCPATH)/filter.h

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
