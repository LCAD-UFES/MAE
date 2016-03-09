libstereo.so: makelibstereo.m LoadStereoParameters.m StereoTriangulation.m LoadRectificationIndexes.m RectifyStereoPair.m RectifyLeftImage.m RectifyRightImage.m CameraProjection.m GetWorldPointAtDistance.m stereo_api.c
	matlab -r makelibstereo
	cp libstereo.so ../..
	cp libstereo.ctf ../..
	cp stereo_api.h ..
