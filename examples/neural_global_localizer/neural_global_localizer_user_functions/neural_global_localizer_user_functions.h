#ifndef _VISUAL_SEARCH_USER_FUNCTIONS_H
#define _VISUAL_SEARCH_USER_FUNCTIONS_H

#include "filter.h"
#include "../neural_global_localizer.h"

#define DIRECTION_FORWARD	1
#define DIRECTION_REWIND	-1

#define MOVING_PHASE		0
#define TRAINING_PHASE		1
#define RECALL_PHASE		2

typedef struct
{
	int frame;
	float confidence;
}winner_t;

typedef struct
{
	double x, y, z;
	double roll, pitch, yaw;
}pose_6d_t;

typedef struct
{
	pose_6d_t pose;
	int correspondence;
}frame_t;

int g_frameID = 0;
int g_testedFrameID = 0;
frame_t* g_training_frame_list = NULL;
frame_t* g_test_frame_list = NULL;

int g_has_new_frame = 0;
int g_networkStatus = TRAINING_PHASE;

#endif

