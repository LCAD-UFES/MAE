#ifndef _SMV_USER_FUNCTIONS_H
#define _SMV_USER_FUNCTIONS_H

// Includes
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <float.h>
#include "mae.h"
#include "mapping.h"
#include "../../../src/libtsai/cal_main.h"
#include "smv_api.h"
#include "../smv.h"

// Definitions
#define READ_FROM_IMAGE_FILE		0
#define PASS_BY_PARAMETER		1
#define CAPTURED_FROM_CAMERA		2
#define CAPTURED_FROM_IP_ADDRESS	3

#define VERGENCE_WINDOW_SIZE		1.0/6.0

#define CAMERA_LEFT_PARAMETERS_FILE_NAME	"CameraLeftParameters.dat"
#define CAMERA_RIGHT_PARAMETERS_FILE_NAME	"CameraRightParameters.dat"
#define TARGET_DATA_FILE_NAME			"Target.dat"
#define CAPTURE_SCRIPT				"./capture.sh"

#define LEFT_CAMERA			0
#define	RIGHT_CAMERA			1

// Macros
#define DRAWSQUARE(x, y, w, h) \
    glBegin(GL_LINES); glVertex2i ((x), (y)); glVertex2i ((x)+(w), (y)); glEnd (); \
    glBegin(GL_LINES); glVertex2i ((x)+(w), (y)); glVertex2i ((x)+(w), (y)+(h)); glEnd (); \
    glBegin(GL_LINES); glVertex2i ((x)+(w), (y)+(h)); glVertex2i ((x), (y)+(h)); glEnd (); \
    glBegin(GL_LINES); glVertex2i ((x), (y)+(h)); glVertex2i ((x), (y)); glEnd ()

#define DRAWCROSS(x, y, w, h) \
{   glBegin(GL_LINES); glVertex2i ((x-w/50), (y)); \
    		       glVertex2i ((x+w/50), (y)); \
    glEnd (); \
    glBegin(GL_LINES); glVertex2i ((x), (y-h/50)); \
    		       glVertex2i ((x), (y+h/50)); \
    glEnd (); \
}
	
#define DRAW_XY_MARK(x, y, w, h) \
{   glBegin(GL_LINES); glVertex2i ((x), (h)); \
    		       glVertex2i ((x), (h+h/20)); \
    glEnd (); \
    glBegin(GL_LINES); glVertex2i ((x), (0)); \
    		       glVertex2i ((x), (-h/20)); \
    glEnd (); \
    glBegin(GL_LINES); glVertex2i ((w), (y)); \
    		       glVertex2i ((w+h/20), (y)); \
    glEnd (); \
    glBegin(GL_LINES); glVertex2i ((0), (y)); \
    		       glVertex2i ((-h/20), (y)); \
    glEnd (); \
}

// Structs

// Global Variables
int g_nCorrection;
float g_fltVergenceWindowSize;

int g_nInputType;
int g_nImageType;

int g_nCameraSide, g_nCameraWindowSide;
int g_nCalibrationPointsCounter, g_nCalibrationPointsNumber;
BOOLEAN g_bRunningCalibrationProcess;
struct camera_parameters cameraLeftCalibrationParameters;
struct camera_parameters cameraRightCalibrationParameters;
struct calibration_constants cameraLeftCalibrationConstants;
struct calibration_constants cameraRightCalibrationConstants;

double g_dblCalibrationPointX, g_dblCalibrationPointY;
double *g_pPixelVectorX, *g_pPixelVectorY;
double *g_pPointsVectorX, *g_pPointsVectorY, *g_pPointsVectorZ;

// Prototypes
WORLD_POINT calculateWorldPoint (IMAGE_COORDINATE distortedLeftPoint, IMAGE_COORDINATE distortedRightPoint);
XY_PAIR calculateVergencePoint (XY_PAIR inputRightPoint);

#endif
