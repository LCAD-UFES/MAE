#ifndef _FACE_RECOG_USER_FUNCTIONS_H
#define _FACE_RECOG_USER_FUNCTIONS_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mae.h"
#include "../face_recog.h"

// Macros
#define DIRECTION_FORWARD	1
#define DIRECTION_NONE		0
#define DIRECTION_REWIND	-1
#define POSE_MIN		1
#define POSE_MAX		2
#define MIN_PERSON_ID 		1  // Deveria ser 1. Ajustado para zero por causa de GetFace2()
#define MAX_PERSON_ID 		10 // Usado apenas em GetFace2()
#define MIN_ILLUMINATION	1  // Usado apenas em GetFace2()
#define MAX_ILLUMINATION	2  // Usado apenas em GetFace2()

#define MAX_MAN_ID 		10
#define MAX_WOMAN_ID		10

#define MOVING_PHASE		0
#define TRAINING_PHASE		1
#define RECALL_PHASE		2

#define FILE_WIDTH		768
#define FILE_HEIGHT		576


#define IMAGE_WIDTH		384      // largura da imagem da mae. Quando alterar o face recog tem que alterar aqui também
#define IMAGE_HEIGHT		288  // altura da imagem da mae. Quando alterar o face recog tem que alterar aqui também

#define IMAGE_FACTOR		FILE_WIDTH / IMAGE_WIDTH

#define MALE			0
#define FEMALE			1

#define EYE			0
#define NOSE			1
#define MOUTH			2
#define VIEW_LOG_POLAR		10

#define RAMDOM_FACES_TEST	"random_faces_t.txt"
#define RAMDOM_FACES_RECALL	"random_faces_r.txt"

// Types

// Prototypes

// Exportable Variables
extern int g_nLeftEyeX, g_nLeftEyeY;
extern int g_nRightEyeX, g_nRightEyeY;
extern int g_nNoseX, g_nNoseY;

#endif
