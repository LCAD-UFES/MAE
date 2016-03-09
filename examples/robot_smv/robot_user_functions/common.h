#ifndef _COMMON_H
#define _COMMON_H

// Includes
#include <stdio.h> 
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <float.h>
#include <malloc.h>
#include <string.h>
#include <GL/glut.h>
#include <stdarg.h>

// Definitions
#ifdef WINDOWS
#ifndef M_PI
#define M_PI            3.1415926535897932384626433832795f
#endif
#define COS(x)          cos (x)
#define SIN(x)          sin (x)
#define TAN(x)          tan (x)
#define ACOS(x)         acos (x)
#define ASIN(x)         asin (x)
#define ATAN(x)         atan (x)
#define SQRT(x)         sqrt (x)
#define POW(x,y)        pow (x, y)
#else
#define COS(x)          cosf (x)
#define SIN(x)          sinf (x)
#define TAN(x)          tanf (x)
#define ACOS(x)         acosf (x)
#define ASIN(x)         asinf (x)
#define ATAN(x)         atanf (x)
#define SQRT(x)         sqrtf (x)
#define POW(x,y)        powf (x, y)
#endif

// Macros

// Types

// Global Variables

// Prototypes

#endif
