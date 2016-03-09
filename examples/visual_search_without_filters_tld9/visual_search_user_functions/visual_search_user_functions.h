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

#define IMAGE_FACTOR		FILE_WIDTH / IMAGE_WIDTH

#define MALE			0
#define FEMALE			1

#define EYE			0
#define NOSE			1
#define MOUTH			2
#define VIEW_LOG_POLAR		10

// Variaveis globais
int g_nImagePart;
int g_nStatus;
int g_nPos;

// Variaveis utilizadas no procedimento de teste
int g_nTotalTested;
int g_nCorrect;
int g_nNoOne;

// Imagem
int g_pImageRAW[FILE_WIDTH * FILE_HEIGHT * 3];
int g_pImageRAWReduced[IMAGE_WIDTH * IMAGE_HEIGHT * 3];
int g_pImageBMPReduced[IMAGE_WIDTH * IMAGE_HEIGHT * 3];

// Coordenadas X,Y das partes da image: olho, nariz e boca
int g_nLeftEyeX, g_nLeftEyeY;
int g_nRightEyeX, g_nRightEyeY;
int g_nNoseX, g_nNoseY;
int g_nMouthX, g_nMouthY;


char g_strRandomImagesFileName[256];

double compute_weigheted_neighborhood(NEURON *n, int w, int h, int u_neuron, int v_neuron, double log_factor);
int gt_x_displecement_from_fovea(int green);
int gt_y_displecement_from_fovea(int red);

#endif

