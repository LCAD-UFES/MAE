#ifndef _VISUAL_SEARCH_USER_FUNCTIONS_H
#define _VISUAL_SEARCH_USER_FUNCTIONS_H

// Includes
#include "filter.h"
#include "../visual_search.h"

// Definitions
#define INPUT_NAME			"in_pattern"
#define NL_TARGET_COORDINATES_NAME	"nl_target_coordinates"
#define NL_FEATURES_NAME		"nl_features"
#define NL_TARGET_NAME			"nl_target"
#define OUT_ACTIVATION_MAP		"out_activation_map"
#define TESTING_PHOTOS_NUMBER           14

// Macros

// Structs

// Prototypes

// Global Variables

// Target Coordinates
int g_nTargetX = 0, g_nTargetY = 0;
int samples_memorized = 0;
float global_max_value = .0f;

#endif
