#ifndef _VISUAL_SEARCH_USER_FUNCTIONS_H
#define _VISUAL_SEARCH_USER_FUNCTIONS_H

// Includes
#include "filter.h"
#include "../visual_search.h"

#define IDXPHOTO_MAX		13

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

