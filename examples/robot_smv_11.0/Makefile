##########################################################
# Makefile parameters
##########################################################

CC=gcc
CCC=g++

HOST_PLATFORM=${shell $(CCC) -dumpmachine}

ifeq ($(HOST_PLATFORM),i686-pc-cygwin) # Cygwin platform configurations
  LIBS = -lnet_conn -lGA -lstereo -lXmu -lXext -ltiff -lglut -lGLU -lGL -lXi -lm -lfl -lgsl -lgslcblas -lXpm -lX11
  LIBPATH = ../../lib/libnet_conn.a /usr/X11R6/lib/libforms.a -L/usr/X11R6/lib/X11 -L/usr/X11R6/lib -L/usr/local/lib -L../../lib -L./
  INCPATH = -I/usr/X11R6/include/X11 -I/usr/X11R6/include -I/usr/local/include -I../../include \
            -I./ -I./robot_user_functions -I./robot_user_functions/matlab_scripts -I./robot_user_functions/segment \
            -I./util

  LIB_DIR = ../../lib

  #CCFLAGS= -Wall -O3
  CCFLAGS= -Wall -g
  LINKER=$(CCC) $(CCFLAGS)

  LIBSTEREO=libstereo.dll
  APPLICATION=robot.exe
else ifeq ($(HOST_PLATFORM),mingw32) # Windows platform configurations
  LIBS=-lnet_conn -lGA -llibstereo -lgsl -lgw32c -lole32 -luuid -lglut32 -lglu32 -lopengl32 -lm -lfl
  LIBPATH=-L..\..\lib -L"C:\MingW\lib" -L"C:\GnuWin32\lib" -L./
  INCPATH=-I. -I".\robot_user_functions" -I".\robot_user_functions\matlab_scripts" -I".\util" \
          -I"C:\GnuWin32\include" -I"C:\MingW\include" -I..\..\include -I".\robot_user_functions\segment"
  LIB_DIR = ..\..\lib

  #CCFLAGS= -Wall -03 -D WINDOWS -D IMPORTING
  CCFLAGS= -Wall -g -D WINDOWS -D IMPORTING
  LINKER=dllwrap.exe --driver-name=c++ --output-def $(LIB_DIR)\smv.def

  LIBSTEREO=libstereo.dll
  APPLICATION=$(LIB_DIR)\smv.dll
else # Linux platform configurations
LIBS = -lnet_conn -lGA -lstereo -lXmu -lXext -lX11 -ltiff -lglut -lGLU -lGL -lXi -lforms -lm -lfl -lgsl -lgslcblas -lXpm
  LIBPATH = ../../lib/libnet_conn.a -L/usr/local/lib -L/usr/X11R6/lib -L../../lib -L./
  INCPATH = -I/usr/local/matlab7/extern/include -I../../include \
            -I./ -I./robot_user_functions -I./robot_user_functions/matlab_scripts -I./robot_user_functions/segment \
            -I./util

  LIB_DIR = ../../lib

  #CCFLAGS= -Wall -O3
  CCFLAGS= -Wall -g
  LINKER=$(CCC) $(CCFLAGS)

  LIBSTEREO=libstereo.so
  APPLICATION=robot
endif

##########################################################
# Canned command sequences
# See http://www.gnu.org/software/make/manual/html_node/Sequences.html#Sequences
##########################################################

# Wipes all built files.
# Surprinsigly enough, MingW allows rm to be used within a Windows environment. Yay MingW!
define CLEAN_PROJECT
rm -fR libstereo_mcr
rm -f \
*.o \
*.d \
robot_user_functions/*.o \
robot_user_functions/*.d \
robot_user_functions/segment/*.o \
robot_user_functions/segment/*.d \
util/*.o \
util/*.d \
robot \
robot.c robot.h \
*.out \
robot.sh.* \
robot_user_functions/matlab_scripts/$(LIBSTEREO) \
robot_user_functions/matlab_scripts/libstereo.ctf \
robot_user_functions/matlab_scripts/libstereo.h \
robot_user_functions/matlab_scripts/libstereo.c \
robot_user_functions/matlab_scripts/libstereo_mcc_component_data.c \
robot_user_functions/matlab_scripts/libstereo.exp \
robot_user_functions/matlab_scripts/libstereo.exports \
robot_user_functions/matlab_scripts/libstereo.ilk \
robot_user_functions/matlab_scripts/libstereo.lib \
robot_user_functions/matlab_scripts/libstereo.pdb \
$(LIBSTEREO) \
libstereo.ctf \
hs_err_pid*.log \
kernel_*.out \
*_disparity*.ppm *_rotate_*.ppm *_rtf_*.ppm *_segment_*.ppm
endef

# Wipes all cache files.
# Surprinsigly enough, MingW allows rm to be used within a Windows environment. Yay MingW!
define CLEAN_CACHE
rm -f *_disparity*.ppm *_image_left.ppm *_image_right.ppm *_nl_trunk*.ppm *nl_segment*.ppm
endef

##########################################################
# Project modules
##########################################################

MODULES_C = \
	robot_user_functions/circle_packing.c \
	robot_user_functions/cylinder.c \
	robot_user_functions/cylinder_list.c \
	robot_user_functions/estimate_volume.c \
	robot_user_functions/fitting.c \
	robot_user_functions/gabor_guess_cylinder.c \
	robot_user_functions/geometric_operations.c \
	robot_user_functions/line.c \
	robot_user_functions/robot_filters.c \
	robot_user_functions/robot_io.c \
	robot_user_functions/robot_nl_functions.c \
	robot_user_functions/robot_user_functions.c \
	robot_user_functions/smv_api.c \
	robot_user_functions/solver.c \
	robot_user_functions/stereo_volume.c \
	robot_user_functions/subpixel_disparity.c

MODULES_CPP = \
	robot_user_functions/convert.cpp \
	robot_user_functions/map.cpp \
	robot_user_functions/maplist.cpp \
	robot_user_functions/model_3d.cpp \
	robot_user_functions/model_3d_io.cpp \
	robot_user_functions/view.cpp \
	robot_user_functions/viewer.cpp \
	robot_user_functions/segment/segment.cpp \
	util/config.cpp \
	util/config_default.cpp \
	util/cconfig.cpp

OBJECTS = robot.o $(MODULES_C:.c=.o) $(MODULES_CPP:.cpp=.o)
	  
LIBSTEREO_FILES = \
	robot_user_functions/matlab_scripts/makelibstereo.m \
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
# Top-level rules
##########################################################

build: $(APPLICATION)
	@echo OK

$(APPLICATION): $(OBJECTS)
	$(LINKER) -o $(APPLICATION) $(OBJECTS) $(LIBPATH) $(LIBS)

clean: clean_cache
	$(CLEAN_PROJECT)
	  
clean_cache:
	$(CLEAN_CACHE)

#include $(MODULES_C:.c=.d) $(MODULES_CPP:.cpp=.d)

##########################################################
# Wildcard rules
##########################################################

.c.o:
	$(CC) $(CCFLAGS) -c $(INCPATH) -o $@ $*.c

.cpp.o:	
	$(CCC) $(CCFLAGS) -c $(INCPATH) -o $@ $*.cpp
	
%.d: %.c
	$(CC) -c $(INCPATH) -MM -MG $< -MT $(<:.c=.o) > $@

%.d: %.cpp
	$(CC) -c $(INCPATH) -MM -MG $< -MT $(<:.cpp=.o) > $@

##########################################################
# Exceptions to the wildcard rules
##########################################################

robot.o: $(LIBSTEREO) robot.con
	netcomp -o $* < $*.con
	$(CC) $(CCFLAGS) -c $(INCPATH) -o $@ $*.c

$(LIBSTEREO): $(LIBSTEREO_FILES)
	$(MAKE) -C robot_user_functions/matlab_scripts
	
robot_user_functions/segment/segment.o:
	$(MAKE) -C robot_user_functions/segment
