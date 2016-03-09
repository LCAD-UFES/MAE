##########################################################
# Makefile parameters
##########################################################

HOST_PLATFORM=${shell g++ -dumpmachine}

ifeq ($(HOST_PLATFORM),i686-pc-cygwin) # Windows platform configurations
  define COPY_LIBRARY
  cp libstereo.dll ../..
  cp libstereo.ctf ../..
  endef
else ifeq ($(HOST_PLATFORM),mingw32) # Windows platform configurations
  define COPY_LIBRARY
  copy libstereo.dll ..\..
  copy libstereo.ctf ..\..
  endef
else # Linux platform configurations
  define COPY_LIBRARY
  cp libstereo.so ../..
  cp libstereo.ctf ../..
  endef
endif

##########################################################
# Top-level rules
##########################################################

build:
	rm -fR "$${HOME}/.matlab/R14/mcc.cache/"
	mcc -g -v -B csharedlib:libstereo \
		LoadStereoParameters.m \
		StereoTriangulation.m \
		LoadRectificationIndexes.m \
		RectifyStereoPair.m \
		RectifyLeftImage.m \
		RectifyRightImage.m \
		CameraProjection.m \
		CameraProjectionRight.m \
		GetWorldPointAtDistance.m \
		GetWorldPointAtDistanceRight.m \
		TransformPoints.m \
		ApplyEdgeFilter.m \
		stereo_api.c
	$(COPY_LIBRARY)
