#ifndef _STEREO_API_H
#define _STEREO_API_H

// Definitions
#define RIGHT_CAMERA	0
#define LEFT_CAMERA	1

//#define _TESTING_STEREO_API

// Macros
#ifdef	WINDOWS
#define DLLEXPORT __declspec (dllexport)
#else
#define DLLEXPORT
#endif

// Types


#ifdef __cplusplus
extern "C" {
#endif 

// Exportable variables

// Prototypes

// ----------------------------------------------------------------------
// Stereo module application program interface
// ----------------------------------------------------------------------
DLLEXPORT int StereoInitialize (void);
DLLEXPORT int StereoQuit (void);

// ----------------------------------------------------------------------
// System calibration application program interface
// ----------------------------------------------------------------------

DLLEXPORT int StereoSetCameraParameters (int nCameraSide, int nXMaxResolution, int nYMaxResolution, int nCCDFormat);
DLLEXPORT int StereoLoadCameraConstants (int nCameraSide, char *strFileName);
DLLEXPORT int StereoSaveCameraConstants (int nCameraSide, char *strFileName);
DLLEXPORT int StereoShowCameraConstants (int nCameraSide);
DLLEXPORT int StereoSetCameraConstants (int nCameraSide, double *p_dblNewCameraConstants);
DLLEXPORT double *StereoGetCameraConstants (int nCameraSide);
DLLEXPORT double *StereoGetCameraOrientation (int nCameraSide);
DLLEXPORT int StereoSetCameraOrientation (int nCameraSide, double *p_dblNewCameraOrientation);
DLLEXPORT int StereoSetBaselineLength (double dblNewBaselineLength);
DLLEXPORT double StereoGetBaselineLength (void);
DLLEXPORT int StereoCameraCalibration (int nCameraSide,
				       int nCalibrationType,
				       double *p_dblImagePoints,
				       double *p_dblWorldPoints,
				       int nPointsNumber);
DLLEXPORT int *StereoDistorted2UndistortedImage (int nCameraSyde, int *p_nDistortedImage, int nImageWidth, int nImageHeight);
DLLEXPORT int *StereoUndistorted2DistortedImage (int nCameraSide, int *p_nUndistortedImage, int nImageWidth, int nImageHeight);

// ----------------------------------------------------------------------
// Epipolar geometry application program interface
// ----------------------------------------------------------------------

DLLEXPORT int StereoBuildEpipolarGeometry (void);
DLLEXPORT double *StereoGetEpipole (int nCameraSide);
DLLEXPORT double *StereoGetEpipolarLine (int nCameraSide, double *p_dblImagePoint);
DLLEXPORT double StereoGetEpipolarLineSlope (int nCameraSide, double *p_dblImagePoint);
DLLEXPORT double *StereoImage2WorldPoint (double *p_dblRightImagePoint, double *p_dblLeftImagePoint);
DLLEXPORT double *StereoGetInfinityPoint (int nCameraSide, double *p_dblImagePoint);
DLLEXPORT double *StereoGetWorldPointAtDistance (int nCameraSide, double *p_dblImagePoint, double dblDistance);
DLLEXPORT double *StereoProjectWorldPoint (int nCameraSide, double *p_dblWorldPoint);
DLLEXPORT double *StereoUndistorted2DistortedImageCoordinate (int nCameraSide, double *p_dblUndistortedImagePoint);

#ifdef __cplusplus
}
#endif

#endif
