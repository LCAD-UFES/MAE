#ifndef _SMV_API_H
#define _SMV_API_H

// Includes
#include "mapping.h"
#include "list.h"
#include "robot_user_functions.h"

// Definitions
#define COPLANAR				0
#define COPLANAR_WITH_FULL_OPTIMIZATION		1
#define NONCOPLANAR				2
#define NONCOPLANAR_WITH_FULL_OPTIMIZATION	3
#define SCALE_FACTOR				2

#ifdef	WINDOWS
#define DLLEXPORT __declspec (dllexport)
#else
#define DLLEXPORT
#endif

//#define DEBUG_SMV_API

#ifdef DEBUG_SMV_API
#define LOG_FILE_NAME		"LogFile.txt"
#define IMAGE_RIGHT_FILE_NAME	"ImageRightTest.pnm"
#define IMAGE_LEFT_FILE_NAME	"ImageLeftTest.pnm"
#endif

#define LEFT_CAMERA                             0
#define RIGHT_CAMERA                            1

// Macros

// Types
struct _key_desc
{
	XY_PAIR dominantEyePoint;
	XY_PAIR nonDominantEyePoint;
	WORLD_POINT world_point;
};
typedef struct _key_desc KEY;

// Prototypes
DLLEXPORT void maeInitialize (void);
DLLEXPORT void maeQuit (void);

DLLEXPORT void maeSetCameraDistance (double fltNewCameraDistance);
DLLEXPORT void maeSetCameraParameters (int nCameraSide, double* pNewCameraParameters);
DLLEXPORT void maeSetVergenceParameters (int nCorrection, double dblVergenceWindowSize);

DLLEXPORT double* maeGetCameraParameters (int nCameraSide);

DLLEXPORT void maeInitializeCameraParameters (double *pNewCameraParameters);
DLLEXPORT void maeCameraCalibration (int nCameraSide, int nCalibrationType, double *pPixelVectorX, double *pPixelVectorY, double *pPointsVectorX, double *pPointsVectorY, double *pPointsVectorZ, int nPointsNumber);

DLLEXPORT void maeUpdateImageLeft  (int *pPixelVector, int nWidth, int nHeight);
DLLEXPORT void maeUpdateImageRight (int *pPixelVector, int nWidth, int nHeight);

DLLEXPORT void maeMapWorldPoints (int *pPointsVectorX, int *pPointsVectorY, int nPointsNumber);
DLLEXPORT void maeManualMapWorldPoints (int *pPointsVectorRightX, int *pPointsVectorRightY,int *pPointsVectorLeftX, int *pPointsVectorLeftY, int nPointsNumber);
DLLEXPORT double *maeGetWorldPointCoordinate (int numPoints,double *pDoubVector);
DLLEXPORT int *maeGetImageLeftPoint (int numPoints, int *pIntVector);
DLLEXPORT void maeCleanCurrentPoints (void);
DLLEXPORT int *maeDistorted2UndistortedImage (int *pDistortedImage, int nWidth, int nHeight, int nCameraSide);

// Global Variables
LIST *g_pList;
double g_pCameraParameters[19]; // Tsai's parameters number
int *g_pUndistortedImage;

#ifdef DEBUG_SMV_API
FILE *g_pLogFile;
FILE *g_pImageFile;
#endif

#endif
