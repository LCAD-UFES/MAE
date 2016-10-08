# Generated automatically from Makefile.in by configure.
BIN_DIR = ../bin
LIB_DIR = ../lib
EXAMPLES_DIR = ../examples

DEL_DIR := $(shell ls $(EXAMPLES_DIR)/*/*.c) 
FILES := $(DEL_DIR:%.c=%)
EXAMPLES_AUX := $(shell ls $(EXAMPLES_DIR)/*/*.con)
EXAMPLES_FILES := $(EXAMPLES_AUX:%.con=%)
EXAMPLES_FILES_C := $(EXAMPLES_FILES:%=%.c)

CC = gcc

#CFLAGS = -O4 -fopenmp -mpopcnt -mfpmath=sse -msse2 -msse4.2 -ffast-math -fassociative-math #-mtune=native -march=native
CFLAGS = -O4

INCPATH = ../include
LIBS = -lfl
YACC = yacc
LEX = flex
AR = ar
ARFLAGS = -rc

DIR = `ls -d ../examples/*`

#
#############################################################
EXECUTABLE = 	netcomp
EXEC_OBJS  = 	netcomp_yacc.o \
		netcomp_lex.o 

LIBRARY    =	libnet_conn.a
LIB_OBJS   = 	mae_control.o \
		mae_control_main_true.o \
		mae_control_file_cb.o \
		io_display.o \
		mae_control_actions_cb.o \
		global.o \
		syn_neu.o \
		syn_neu_io.o \
		syn_neu_correlate.o \
		syn_neu_default.o \
		syn_neu_label.o \
		syn_neu_logiredu.o \
		syn_neu_mem_overwrite.o \
		syn_neu_minchinton_center_surround.o \
		syn_neu_minchinton_fat_fast.o \
		syn_neu_minchinton.o \
		syn_neu_rnd_mem_type.o \
		syn_neu_wisard.o \
		learning.o \
		connect.o \
		io.o \
		filter.o \
		user_functions.o \
		interpreter_yacc.o \
		interpreter_lex.o 


LIBRARY2    =   libmae.a
LIB_OBJS2   =   mae_control.o \
		mae_control_file_cb.o \
		io_display.o \
		mae_control_actions_cb.o \
		global.o \
		syn_neu.o \
		syn_neu_io.o \
		syn_neu_correlate.o \
		syn_neu_default.o \
		syn_neu_label.o \
		syn_neu_logiredu.o \
		syn_neu_mem_overwrite.o \
		syn_neu_minchinton_center_surround.o \
		syn_neu_minchinton_fat_fast.o \
		syn_neu_minchinton.o \
		syn_neu_rnd_mem_type.o \
		syn_neu_wisard.o \
		learning.o \
		connect.o \
		io.o \
		filter.o \
		user_functions.o \
		interpreter_yacc.o \
		interpreter_lex.o 
#############################################################
#

.c.o:
	$(CC) $(CFLAGS) -Wall -c -I$(INCPATH) -o $@ $*.c

.l.o:
	$(LEX) $*.l
	$(CC) $(CFLAGS) -c -I$(INCPATH) -o $@ lex.yy.c

.y.o:
	$(YACC) -v -d $*.y
	$(CC) $(CFLAGS) -c -I$(INCPATH) -o $@ y.tab.c
	mv y.tab.h $*.tab.h


build: $(LIBRARY) $(LIBRARY2) $(EXECUTABLE) libGA.a libTsai.a libStereo.a libbee.a
	@echo OK
	
$(LIBRARY): $(LIB_OBJS) libGA.a
	$(AR) $(ARFLAGS) $(LIB_DIR)/$(LIBRARY) $(LIB_OBJS)
	ranlib $(LIB_DIR)/$(LIBRARY)

$(LIBRARY2): $(LIB_OBJS2) libGA.a
	$(AR) $(ARFLAGS) $(LIB_DIR)/$(LIBRARY2) $(LIB_OBJS2)
	ranlib $(LIB_DIR)/$(LIBRARY2)

$(EXECUTABLE): $(EXEC_OBJS)
	$(CC) $(CFLAGS) -o $(BIN_DIR)/$(EXECUTABLE) $(EXEC_OBJS) $(LIBS)

libGA.a:
	$(MAKE) -C libga100
	cp  libga100/libGA.a $(LIB_DIR)
	
libTsai.a:
	$(MAKE) -C libtsai
	cp  libtsai/libTsai.a $(LIB_DIR)

libbee.a:
	$(MAKE) -C libbee
	cp  libbee/lib/libbee.a $(LIB_DIR)
	cp  libbee/triclops_64/lib/*.a $(LIB_DIR)
	cp  libbee/pgr-stereo-examples-libdc-2.0.2/pgrlibdcstereo/*.a $(LIB_DIR)

libStereo.a:
	$(MAKE) -C libstereo

libStereo3.a:
	$(MAKE) -C libstereo3

##############################################################

mae_control.o 			: $(INCPATH)/mae_control.h
mae_control_main_true.o 	: $(INCPATH)/mae_control.h $(INCPATH)/mae.h
mae_control_file_cb.o 		: $(INCPATH)/mae_control.h $(INCPATH)/mae.h
mae_control_actions_cb.o 	: $(INCPATH)/mae_control.h $(INCPATH)/mae.h
global.o 			: $(INCPATH)/mae.h
syn_neu.o 			: $(INCPATH)/mae.h
learning.o 			: $(INCPATH)/mae.h
connect.o 			: $(INCPATH)/mae.h
io_display.o	 		: $(INCPATH)/mae.h
io.o 				: $(INCPATH)/mae.h
filter.o 			: $(INCPATH)/mae.h $(INCPATH)/filter.h
gabor_filters.o 		: $(INCPATH)/mae.h $(INCPATH)/filter.h
user_functions.o 		: $(INCPATH)/mae.h
netcomp_yacc.o 			: netcomp_yacc.y $(INCPATH)/mae.h
netcomp_lex.o 			: netcomp_lex.l netcomp_yacc.y $(INCPATH)/mae.h
interpreter_yacc.o 		: interpreter_yacc.y $(INCPATH)/mae.h 
interpreter_lex.o 		: interpreter_lex.l interpreter_yacc.y $(INCPATH)/mae.h

#
#############################################################
#

clean :
	rm -f *.o *yacc.c *lex.c *.tab.* lex.yy.c y.* $(BIN_DIR)/$(EXECUTABLE) $(LIB_DIR)/$(LIBRARY) $(LIB_DIR)/libGA.a $(LIB_DIR)/libTsai.a $(LIB_DIR)/libStereo.a core
	rm -f ../examples/*/*.o ../examples/*/*/*.o ../examples/*/core ../examples/*/*.out
	rm -f $(EXAMPLES_FILES_C)
	rm -f $(EXAMPLES_FILES)
	$(MAKE) clean -C libga100
	$(MAKE) clean -C libtsai
	$(MAKE) clean -C libstereo
	$(MAKE) clean -C libstereo3
	$(MAKE) clean -C libbee
	rm -f $(LIB_DIR)/*.a
	rm -f *.dot
