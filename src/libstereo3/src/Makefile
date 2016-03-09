#Dirs
INCDIR=/usr/include
INCDIR_XML=/usr/include/libxml2
LIBDIR=/usr/lib
LIBDIR_XML=/usr/lib64

INCPATH=-I$(INCDIR) -I$(INCDIR_XML) -I../include
LIBPATH=-L$(LIBDIR) -L$(LIBDIR_XML)

#Compilers FLAGS
DEBUG=-g
#RELEASE=-O3 -ffast-math
CFLAGS=-D__MINMAX_DEFINED $(RELEASE) $(DEBUG) $(INCPATH) $(LIBPATH)
CPPFLAGS=$(CFLAGS)
SHAREDFLAGS=-shared -fPIC -Wl,-soname,

#Archiver
AR=ar
ARFLAGS=crs

#Built Objects
LIBOBJECTS=mono.o parser.o stereo.o stereo_api.o

#Versions of the lib.
VERSION=1.0.0
VER=1


INSTALLDIR="../../../lib"


build: libStereo3.a libStereo3.so.1 install

libStereo3.so.1:
	$(MAKE) libStereo3.so

libStereo3.so: CPPFLAGS=-fPIC $(CFLAGS)

libStereo3.so: $(LIBOBJECTS)
	g++ $(SHAREDFLAGS)$@.$(VER) -o $@.$(VERSION) $^
	test -f $@.$(VERSION) && ln -s $@.$(VERSION) $@.$(VER)
	-@rm -f $%

libStereo3.a : libStereo3.a($(LIBOBJECTS))

libStereo3.a($(LIBOBJECTS)): $(LIBOBJECTS)
	$(AR) $(ARFLAGS) $@ $%
	-@rm -f $%

%.o : %.cpp $(INCPATH)/%.h
	g++ -c $(CPPFLAGS) $@ $< `pkg-config --libs opencv`
	
clean:
	rm -rf *.o *.a *.so.* *~ *.dSYM
	

install: libStereo3.a libStereo3.so.1
	-mkdir $(INSTALLDIR)
	cp *.a *.so.$(VERSION) *.so.$(VER) $(INSTALLDIR)

