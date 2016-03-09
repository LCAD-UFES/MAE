#ifndef _NEURAL_SALIENCY_USER_FUNCTIONS_H
#define _NEURAL_SALIENCY_USER_FUNCTIONS_H

#include "filter.h"
#include "../neural_saliency.h"

#define DIRECTION_FORWARD	1
#define DIRECTION_REWIND	-1

#define MOVING_PHASE		0
#define TRAINING_PHASE		1
#define RECALL_PHASE		2

int g_frameID = 0;
int g_networkStatus = TRAINING_PHASE;

#endif

