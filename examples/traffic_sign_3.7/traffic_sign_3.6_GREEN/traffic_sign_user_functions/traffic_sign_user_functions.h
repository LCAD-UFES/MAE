#ifndef _TRAFFIC_SIGN_USER_FUNCTIONS_H
#define _TRAFFIC_SIGN_USER_FUNCTIONS_H

// Includes
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "mae.h"
#include "../traffic_sign.h"

// Macros
#define DIRECTION_FORWARD	1
#define DIRECTION_REWIND	-1

#define MOVING_PHASE		0
#define TRAINING_PHASE		1
#define RECALL_PHASE		2
// Types

// Prototypes

// Exportable Variables
int g_size_box;
int g_img_w, g_img_h, g_img_x1, g_img_y1;
int g_roi_x1, g_roi_y1, g_roi_x2, g_roi_y2;

#endif /*_TRAFFIC_SIGN_USER_FUNCTIONS_H*/
