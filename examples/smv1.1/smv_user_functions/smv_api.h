#ifndef _SMV_API_H
#define _SMV_API_H

#include "drawShape.h"
#include "pointList.h"
#include "smv_user_functions.h"

// Macros
#define INPUT_LEFT_NAME		"image_left"
#define INPUT_RIGHT_NAME	"image_right"

// Prototypes
int maeInitialize ();
int maeQuit ();
int maeSetCameraDistance (float fltNewCameraDistance);
int maeSetFocalDistance (float fltNewFocalDistance);
int maeSetStackWidth (float fltNewStackWidth);
int maeUpdateImageLeft (int *pPixelVector, int nWidth, int nHeight);
int maeUpdateImageRight (int *pPixelVector, int nWidth, int nHeight);
float maeCalculateVolume (int *pPointsVectorX, int *pPointsVectorY, int nPointsNumber);
int maeGetWorldPoints (float *pWorldPointsVectorX, float *pWorldPointsVectorY, float *pWorldPointsVectorZ);

// Structures

#endif
