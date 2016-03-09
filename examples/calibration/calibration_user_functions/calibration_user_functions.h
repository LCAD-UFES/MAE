#ifndef _CALIBRATION_USER_FUNCTIONS_H
#define _CALIBRATION_USER_FUNCTIONS_H

// Includes
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <float.h>
#include "mae.h"
#include "../calibration.h"
#include "../../../src/libstereo/include/stereo_api.hpp"

// Definitions
#define VERGENCE_WINDOW_SIZE		1.0/6.0

// Macros
#define DRAWSQUARE(x, y, w, h) \
{   glBegin(GL_LINES); glVertex2i ((x), (y)); glVertex2i ((x)+(w), (y)); glEnd (); \
    glBegin(GL_LINES); glVertex2i ((x)+(w), (y)); glVertex2i ((x)+(w), (y)+(h)); glEnd (); \
    glBegin(GL_LINES); glVertex2i ((x)+(w), (y)+(h)); glVertex2i ((x), (y)+(h)); glEnd (); \
    glBegin(GL_LINES); glVertex2i ((x), (y)+(h)); glVertex2i ((x), (y)); glEnd (); \
}

#define DRAWCROSS(x, y, w, h) \
{   glBegin(GL_LINES); glVertex2i ((x-w/50), (y)); glVertex2i ((x+w/50), (y)); glEnd (); \
    glBegin(GL_LINES); glVertex2i ((x), (y-h/50)); glVertex2i ((x), (y+h/50)); glEnd (); \
}

#define DRAW_XY_MARK(x, y, w, h) \
{   glBegin(GL_LINES); glVertex2i ((x), (h)); glVertex2i ((x), (h+h/20)); glEnd (); \
    glBegin(GL_LINES); glVertex2i ((x), (0)); glVertex2i ((x), (-h/20)); glEnd (); \
    glBegin(GL_LINES); glVertex2i ((w), (y)); glVertex2i ((w+h/20), (y)); glEnd (); \
    glBegin(GL_LINES); glVertex2i ((0), (y)); glVertex2i ((-h/20), (y)); glEnd (); \
}

#define DRAWLINE(x0, y0, x1, y1) \
{   glBegin(GL_LINES); glVertex2f (x0, y0); glVertex2f (x1, y1); glEnd (); \
}

// Structs

// Exportable Variables
extern double g_dblNearVergenceCutPlane;
extern double g_dblFarVergenceCutPlane;

// Prototypes
void SetVergenceByMinusFilter (void);

#endif
