#ifndef _VISUAL_SEARCH_USER_FUNCTIONS_H
#define _VISUAL_SEARCH_USER_FUNCTIONS_H

// Includes
#include "filter.h"
#include "../visual_search.h"

// Definitions
/*#define DIRECTION_FORWARD		1
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
#define INPUT_PATH			"/dados/ARDataBase/"


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

float global_max_value = 0.0;*/

#define DIRECTION_FORWARD	1
#define DIRECTION_REWIND	-1
#define POSE_MIN		1
#define POSE_MAX		2
#define MIN_PERSON_ID 		1
#define MAX_MAN_ID 		9
#define MAX_WOMAN_ID		60

#define MOVING_PHASE		0
#define TRAINING_PHASE		1
#define RECALL_PHASE		2

#define FILE_WIDTH		768
#define FILE_HEIGHT		576
//#define IMAGE_WIDTH		384
//#define IMAGE_HEIGHT		288

#define IMAGE_FACTOR		FILE_WIDTH / IMAGE_WIDTH

#define MALE			0
#define FEMALE			1

#define EYE			0
#define NOSE			1
#define MOUTH			2
#define VIEW_LOG_POLAR		10

//#define RAMDOM_FACES_TEST	"random_faces_t.txt"
//#define RAMDOM_FACES_RECALL	"random_faces_r.txt"

// Variaveis globais
char g_cSexID = 'm';
int g_nPersonID;
int g_nPoseID;
int g_nMaxPersonID = MAX_MAN_ID;
int g_vCountPerson[MAX_MAN_ID];
int g_nFaceNum = 1;
int g_nFacePart;
int g_nStatus;
int g_nPos;
int g_nTries = 1;
int g_nTry = 1;

// Variaveis utilizadas no procedimento de teste
int g_nTotalTested;
int g_nCorrect;
int g_nNoOne;

// Imagem
int g_pImageRAW[FILE_WIDTH * FILE_HEIGHT * 3];
int g_pImageRAWReduced[IMAGE_WIDTH * IMAGE_HEIGHT * 3];
int g_pImageBMPReduced[IMAGE_WIDTH * IMAGE_HEIGHT * 3];

// Coordenadas X,Y das partes da face: olho, nariz e boca
int g_nLeftEyeX, g_nLeftEyeY;
int g_nRightEyeX, g_nRightEyeY;
int g_nNoseX, g_nNoseY;
int g_nMouthX, g_nMouthY;

// Target Coordinates
int g_nTargetX = 0, g_nTargetY = 0;


int g_nPersonUniqueID = 1;
char g_strRandomFacesFileName[256];

float global_max_value = 0.0;

#endif

