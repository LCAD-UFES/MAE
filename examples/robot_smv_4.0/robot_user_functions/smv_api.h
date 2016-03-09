#ifndef _SMV_API_H
#define _SMV_API_H

// Includes
#include "robot_user_functions.h"

// Definitions
#define COPLANAR				0
#define COPLANAR_WITH_FULL_OPTIMIZATION		1
#define NONCOPLANAR				2
#define NONCOPLANAR_WITH_FULL_OPTIMIZATION	3
#define SCALE_FACTOR				2

#define LEFT_CAMERA				0
#define RIGHT_CAMERA				1

#ifdef	WINDOWS
#define DLLEXPORT __declspec (dllexport)
#else
#define DLLEXPORT
#endif


#define SMV_API_ERROR_LOG			"ErrorLogFile.txt"
#define SMV_API_EXECUTION_LOG			"ExecutionLogFile.txt"

// Macros
#ifndef ERROR
#define ERROR(str1,str2,str3)			{FILE *pLogFile = NULL; \
						pLogFile = fopen (SMV_API_ERROR_LOG, "a"); \
						fprintf (pLogFile, "Error: %s%s%s\n", str1, str2, str3); \
						fclose (pLogFile); }
#endif

#ifndef MESSAGE
#define MESSAGE(str1,str2,str3)			{printf ("%s%s%s\n", str1, str2, str3) }
#endif

#ifndef DEBUG
#define DEBUG(str1,str2,str3)			{FILE *pLogFile = NULL; \
						pLogFile = fopen (SMV_API_EXECUTION_LOG, "a"); \
						fprintf (pLogFile, "%s%s%s\n", str1, str2, str3); \
						fclose (pLogFile); }
#endif

// Types

// Prototypes

// ----------------------------------------------------------------------
// MAE API
// ----------------------------------------------------------------------
DLLEXPORT int MaeInitialize (void);
DLLEXPORT int MaeQuit (void);

// ----------------------------------------------------------------------
// Calibration system API
// ----------------------------------------------------------------------
DLLEXPORT int MaeSetCameraParameters (int nCameraSide, int nXMaxResolution, int nYMaxResolution, int nCCDFormat);
DLLEXPORT int MaeCameraCalibration (int nCameraSide,
				       int nCalibrationType,
				       double *p_dblImagePoints,
				       double *p_dblWorldPoints,
				       int nPointsNumber);
DLLEXPORT double *MaeGetCameraConstants (int nCameraSide);
DLLEXPORT int MaeSetCameraConstants (int nCameraSide, double *p_dblNewCameraConstants);
DLLEXPORT int MaeBuildStereoSystem (void);

// ----------------------------------------------------------------------
// Image manipulation API
// ----------------------------------------------------------------------
DLLEXPORT int MaeUpdateImage (int nCameraSide, int *p_nNewImage, int nImageWidth, int nImageHeight);
DLLEXPORT int *MaeDistorted2UndistortedImage (int nCameraSyde, int *p_nDistortedImage, int nImageWidth, int nImageHeight);

// ----------------------------------------------------------------------
// Mapping API
// ----------------------------------------------------------------------
DLLEXPORT int MaeSetVergenceParameters (double dblNearCutPlane, double dblFarCutPlane);
DLLEXPORT double *MaeImage2WorldPoint (int *p_nRightImagePoints);
DLLEXPORT double *MaeImage2WorldPointManual (int *p_nRightImagePoint, int *p_nLeftImagePoint);
DLLEXPORT int *MaeGetImageLeftPoint (void);
DLLEXPORT int *MaeGetLeftEpipolarLine (int *p_nRightImagePoint);

// Global Variables

#endif
