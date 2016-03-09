#ifndef _VISUAL_SEARCH_USER_FUNCTIONS_H
#define _VISUAL_SEARCH_USER_FUNCTIONS_H

// Includes
#include "filter.h"
#include "../visual_search.h"

#define DIRECTION_FORWARD	1
#define DIRECTION_REWIND	-1
#define MAX_MAN_ID 			76

#define MOVING_PHASE		0
#define TRAINING_PHASE		1
#define RECALL_PHASE		2

#define FILE_WIDTH			768
#define FILE_HEIGHT			576

#define RAMDOM_IMAGES_TRAIN		"/dados/TrainIJCNN2013/gt.txt"
#define RAMDOM_IMAGES_RECALL	"/dados/TestIJCNN2013Download/image_list.txt"

// Criterions
#define	 NONE						-1
#define	 PROHIBITORY_AND_SCALED		0
#define	 PROHIBITORY				1
#define	 FOUND						2
#define MANDATORY					3

// structure for data of ground truth file
typedef struct
{
	int ImgNo, leftCol, topRow, rightCol, bottomRow, ClassID;
	int found;
	int used_for_trainning;
	double best_confidence;
	int best_x;
	int best_y;
	double best_scale_factor;
} IMAGE_INFO;

// Global variables

int g_nStatus;

char g_strRandomImagesFileName[256];

float global_max_value = 0.0;

char *g_input_path = NULL;
IMAGE_INFO *g_image_info = NULL;
int g_num_image_info = 0;

int prohibitory[] = {0, 1, 2, 3, 4, 5, 7, 8, 9, 10, 15, 16}; 				// (circular, white ground with red border)
int prohibitory_size = 12;
int mandatory[] = {33, 34, 35, 36, 37, 38, 39, 40};					// (circular, blue ground)
int mandatory_size = 8;
int danger[] = {11, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31}; 	// (triangular, white ground with red border)
int danger_size = 15;

int g_nNumImagesWithTrafficSignsAccordingToCriterion;
int g_CurrentCriterion = NONE;

int g_nImageOrder = -1;
int g_nCurrentImageInfo = -1;
int g_nNetworkStatus;

double g_scale_factor = 0.5;
double g_halph_band_width = 1.0;

double g_confidence;

char is_confidence;

#endif

