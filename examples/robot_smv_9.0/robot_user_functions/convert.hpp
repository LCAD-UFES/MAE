#ifndef _CONVERT_H
#define _CONVERT_H

// Includes
#include "common.hpp"

// Definitions

// Macros

// Types

// Global Variables

// Prototypes
int ConvertSphe2Rect (float *p_fltX, float *p_fltY, float *p_fltZ, float fltAlfa, float fltBeta, float fltDistance);
int ConvertRect2Sphe (float *p_fltAlfa, float *p_fltBeta, float *p_fltDistance, float fltX, float fltY, float fltZ);
void CalculateWorldPoint (float *X, float *Y, float *Z, float xl, float yl, float xr, float yr, float F, float B);

#endif
