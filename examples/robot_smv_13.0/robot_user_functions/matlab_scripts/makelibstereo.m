%addpath ./TOOLBOX_calib
%addpath C:\MAE\examples\robot_smv_4.0\robot_user_functions\matlab_scripts\TOOLBOX_calib
%mcc -M '-regsvr' -v -B csharedlib:stereo LoadStereoParameters.m StereoTriangulation.m LoadRectificationIndexes.m RectifyStereoPair.m RectifyLeftImage.m RectifyRightImage.m ApplyEdgeFilter.m stereo_api.c
mcc -v -g -B csharedlib:libstereo Loadstereoparameters.m Stereotriangulation.m Loadrectificationindexes.m Rectifystereopair.m Rectifyleftimage.m Rectifyrightimage.m Cameraprojection.m Cameraprojectionright.m Getworldpointatdistance.m Getworldpointatdistanceright.m Transformpoints.m Applyedgefilter.m stereo_api.c
%mbuild stereo_api.c stereo.so
%mbuild -v teste.c stereo.lib
%quit
