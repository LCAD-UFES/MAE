LIBMAE =-L$(MAEHOME)/lib
LIBSHARED =-L$(SHAREDHOME)/lib
LIBPATH =-L/usr/local/lib -L/usr/X11R6/lib $(LIBMAE) $(LIBSHARED)
INCPATH =$(SHAREDHOME)/include
CC=gcc
CCFLAGS=-D NO_INTERFACE -g -Wall
LIBRARYS=-lnet_conn -lforms -lGA -lTsai -lX11 -lglut -lGLU -lGL -lm -lfl -lcorerpcs
LOCAL_CFLAGS =`libwww-config --cflags` 
CLIENT_SERVER_LIBS =`xmlrpc-c-config --libs` -lxmlrpc_server -lxmlrpc_abyss -lxmlrpc_server_abyss -lxmlrpc_client -lpthread
 
##########################################################

EXECUTABLE = wnn
OBJECTS = class_cnae.o \
	  class_cnae_user_functions/class_cnae_user_functions.o \
	  class_cnae_user_functions/build_training_testing_images.o \
	  class_cnae_user_functions/specific_core_rpcs_handling_functions.o \
	  class_cnae_user_functions/io_memory.o 

##########################################################

.c.o:
	@echo OK
	$(CC) $(CCFLAGS) -c -I$(MAEHOME)/include -I$(INCPATH) -o $@ $*.c

##########################################################

build: $(EXECUTABLE)
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CCFLAGS) -o $(EXECUTABLE) $(OBJECTS) $(LIBPATH) $(LIBRARYS) $(LIBWWW_LIBS) $(CLIENT_SERVER_LIBS)

##########################################################

class_cnae_user_functions.o : $(INCPATH)/specific_core_rpcs_handling_functions.h ../class_cnae.h
specific_core_rpcs_handling_functions.o	: $(INCPATH)/specific_core_rpcs_handling_functions.h build_training_testing_images.h

##########################################################

class_cnae.o:
	$(MAEHOME)/bin/netcomp -o $* < $*.con	
	$(CC) $(CCFLAGS) -c -I$(MAEHOME)/include -o $@ $*.c

##########################################################
clean:
	rm -f *.o class_cnae_user_functions/*.o
	rm -f wnn class_cnae.c class_cnae.h
	rm -f *.out
