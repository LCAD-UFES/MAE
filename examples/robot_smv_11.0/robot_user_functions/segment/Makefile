INCDIR = -I.
DBG    = -g
OPT    = -O3
CPP    = g++
CFLAGS = $(DBG) $(INCDIR)

.cpp.o:
	$(CPP) $(CFLAGS) -c $< -o $@

all: segment

segment: convolve.h disjoint-set.h filter.h image.h imconv.h imutil.h misc.h pnmfile.h segment.cpp segment.hpp segment-graph.h segment-image.h 
	$(CPP) $(CFLAGS) -c segment.cpp

clean:
	/bin/rm -f segment *.o

clean-all: clean
	/bin/rm -f *~ 



