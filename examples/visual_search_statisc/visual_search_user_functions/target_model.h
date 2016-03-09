#ifndef _TARGET_MODEL_H
#define _TARGET_MODEL_H

// Includes
#include "GL/glut.h"
#include "mae.h"

// Definitions

// Macros

// Structs

// Prototypes
void TargetModelInitialize (void);
void GetTargetPattern (INPUT_DESC *input);
void TargetModelDisplay (void);
void UpdateTargetModel (float fltAlpha, float fltBeta, float fltDistance);
void GetTargetWindowCoordinates (int *x, int *y);

// Global Variables


#endif
