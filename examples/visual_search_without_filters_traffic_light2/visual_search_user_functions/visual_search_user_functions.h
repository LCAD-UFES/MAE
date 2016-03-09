#ifndef _VISUAL_SEARCH_USER_FUNCTIONS_H
#define _VISUAL_SEARCH_USER_FUNCTIONS_H

// Includes
#include "filter.h"
#include "../visual_search.h"

#define DIRECTION_FORWARD               1
#define DIRECTION_REWIND                -1
#define MAX_MAN_ID 			76

#define MOVING_PHASE                    0
#define TRAINING_PHASE                  1
#define RECALL_PHASE                    2

#define RANDOM_IMAGES_TRAIN		"/dados/tld/train/gt.txt"
#define RANDOM_IMAGES_RECALL            "/dados/tld/test/image_list.txt"

// Criterions
#define	NONE				-1
#define RED_STOP                        0
#define	YELLOW_CAUTION			1
#define GREEN_GO         		2
#define FOUND                           3

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

int red_stop[] = {0}; 				// red signal
int red_stop_size = 1;
int yellow_caution[] = {1};				// yellow signal
int yellow_caution_size = 1;
int green_go[] = {2};                             // green signal
int green_go_size = 1;

int g_nNumImagesWithTrafficLightsAccordingToCriterion;
int g_CurrentCriterion = NONE;

int g_nImageOrder = -1;
int g_nCurrentImageInfo = -1;
int g_nNetworkStatus;

double g_scale_factor = 0.5;
double g_halph_band_width = 1.0;

double g_confidence;

char is_confidence;

int g_NumTrafficSignsSearched = 0;
int g_NumTrafficSignsFound = 0;

#endif