#ifndef _FACE_RECOG_USER_FUNCTIONS_H
#define _FACE_RECOG_USER_FUNCTIONS_H

// Includes
#include <stdlib.h>
#include <math.h>
#include "mae.h"
#include "../face_recog.h"

// Macros
#define DIRECTION_FORWARD	1
#define DIRECTION_REWIND	-1
#define POSE_MIN		1
#define POSE_MAX		26
#define MIN_PERSON_ID 		1
#define MAX_MAN_ID 		76
#define MAX_WOMAN_ID		60

#define MOVING_PHASE		0
#define TRAINING_PHASE		1
#define RECALL_PHASE		2

#define FILE_WIDTH		768
#define FILE_HEIGHT		576
#define IMAGE_WIDTH		384
#define IMAGE_HEIGHT		288

#define IMAGE_FACTOR		FILE_WIDTH / IMAGE_WIDTH

#define MALE			0
#define FEMALE			1

#define EYE			0
#define NOSE			1
#define MOUTH			2
#define VIEW_LOG_POLAR		10

#define RAMDOM_FACES_TEST	"random_faces_t.txt"
#define RAMDOM_FACES_RECALL	"random_faces_r.txt"

// Types

// Prototypes

// Exportable Variables
extern int g_nLeftEyeX, g_nLeftEyeY;
extern int g_nRightEyeX, g_nRightEyeY;
extern int g_nNoseX, g_nNoseY;

#endif
