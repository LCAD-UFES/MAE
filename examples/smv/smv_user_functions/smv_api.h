#ifndef _SMV_API_H
#define _SMV_API_H

#include "drawShape.h"

// Definitions

// Macros

// Prototypes
void maeInitialize ();
void maeQuit ();
void maeSetCameraDistance (float fltNewCameraDistance);
void maeSetFocalDistance (float fltNewFocalDistance);
void maeSetStackWidth (float fltNewStackWidth);
void maeUpdateImageLeft (int *pPixelVector, int nWidth, int nHeight);
void maeUpdateImageRight (int *pPixelVector, int nWidth, int nHeight);
float maeCalculateVolume (int *pPointsVectorX, int *pPointsVectorY, int nPointsNumber);

// Structures

#endif
