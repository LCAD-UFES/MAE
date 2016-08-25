#ifndef _FACE_RECOG_USER_FUNCTIONS_H
#define _FACE_RECOG_USER_FUNCTIONS_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mae.h"
#include "../ita.h"

// Macros
#define DIRECTION_FORWARD	1
#define DIRECTION_REWIND	-1
#define POSE_MIN		0
#define POSE_MAX		3755

#define MOVING_PHASE		0
#define TRAINING_PHASE		1
#define TEST_PHASE			2
#define WARM_UP_PHASE		3

#define TRAIN_SAMPLES	"returns_20160121_0064s.txt"
#define TEST_SAMPLES	"returns_20160121_0064s.txt"

#endif
