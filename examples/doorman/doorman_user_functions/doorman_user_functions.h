#ifndef _DOORMAN_USER_FUNCTIONS_H
#define _DOORMAN_USER_FUNCTIONS_H

// Includes
#include "filter.h"
#include "../doorman.h"
#include "album.h"

// Definitions
#define GET_FACE_SCRIPT		"./get_face.sh"
#define ALBUM_PATH		"/home/stiven/MAE/examples/doorman/files.txt"
#define PHOTOS_PATH		"/home/stiven/feret/data/smaller/"
#define GROUND_TRUTHS_PATH	"/home/stiven/feret/data/ground_truths/name_value/"
#define ORIGINAL_IMAGE_WIDTH    512
#define ORIGINAL_IMAGE_HEIGHT	768	
#define PERSONS_NUMBER		739
#define POSES_NUMBER		13
#define DATES_NUMBER		14
#define EXPRESSIONS_NUMBER	3

#define INPUT_NAME			"in_pattern"
#define NL_TARGET_COORDINATES_NAME	"nl_target_coordinates"
#define NL_FEATURES_NAME		"nl_features"
#define NL_TARGET_NAME			"nl_target"
#define OUT_ACTIVATION_MAP		"out_activation_map"

#define CONFIDENCE_LEVEL 0.41

// Macros
#define X_FACTOR	((float) IMAGE_WIDTH  / (float) ORIGINAL_IMAGE_WIDTH)
#define Y_FACTOR	((float) IMAGE_HEIGHT / (float) ORIGINAL_IMAGE_HEIGHT)
#define IMAGE_FACTOR	FILE_WIDTH / ORIGINAL_IMAGE_WIDTH

// Structs

// Prototypes

// Global Variables

// Target Coordinates
int g_nTargetX = 0, g_nTargetY = 0;

// Album
float global_max_value = 0.0;
ALBUM *g_pAlbum = NULL;
PHOTO *g_pCurrentFace = NULL;

#endif

