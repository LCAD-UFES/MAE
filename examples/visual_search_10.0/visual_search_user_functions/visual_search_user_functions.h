#ifndef _VISUAL_SEARCH_USER_FUNCTIONS_H
#define _VISUAL_SEARCH_USER_FUNCTIONS_H

// Includes
#include "filter.h"
#include "../visual_search.h"

// Definitions
#define DIRECTION_FORWARD		1
#define DIRECTION_REWIND		-1
#define IDXPHOTO_MIN			1
#define IDXPHOTO_MAX			13
#define PERSON_MIN			1
#define PERSON_MAX 			67
#define FILE_WIDTH			768
#define FILE_HEIGHT			576
//#define IMAGE_WIDTH			256
//#define IMAGE_HEIGHT			192
//#define CUT_POINT			1.0
#define INPUT_PATH			"/home/hallysson/img/dbf1_new/"


#define INPUT_NAME			"in_pattern"
#define NL_TARGET_COORDINATES_NAME	"nl_target_coordinates"
#define NL_FEATURES_NAME		"nl_features"
#define NL_TARGET_NAME			"nl_target"
#define OUT_ACTIVATION_MAP		"out_activation_map"

// Macros

// The original input size used is 384 x 288
#define X_CORRECTION_FACTOR	(double) IMAGE_WIDTH  / 384.0
#define Y_CORRECTION_FACTOR	(double) IMAGE_HEIGHT / 288.0

#define IMAGE_FACTOR		FILE_WIDTH / IMAGE_WIDTH

// Structs

// Prototypes

// Global Variables

// Mount file name: <g_nPerson><g_nIdxPhoto>.raw
int g_nIdxPhoto, g_nPerson;

// Target Coordinates
int g_nTargetX = 0, g_nTargetY = 0;

// Eye, nose and mouth image coordinates
int g_nEyeX, g_nEyeY;
int g_nNoseX, g_nNoseY;
int g_nMouthX, g_nMouthY;

// Image 
int g_pImageRAW[FILE_WIDTH * FILE_HEIGHT * 3];
int g_pImageRAWReduced[IMAGE_WIDTH * IMAGE_HEIGHT * 3];
int g_pImageBMPReduced[IMAGE_WIDTH * IMAGE_HEIGHT * 3];

float global_max_value = 0.0;

#endif

