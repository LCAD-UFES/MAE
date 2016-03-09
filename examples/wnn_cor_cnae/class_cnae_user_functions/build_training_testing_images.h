#ifndef _BUILD_TT_IMAGES_H
#define _BUILD_TT_IMAGES_H
/********************
    HEADER FILE
********************/


/* defines */

//#define DEBUG
#define BLACK		0.0
#define WHITE		255.0
#define NOTHING		0
#define ONE		1
#define NAME_SIZE	256
#define PATH_REPOSITORY	"REPOSITORY/"

/* structs */

/* global variables */

/* prototypes */
void build_training_testing_images (float *vts, int vts_length, int *cnae_codes, int cnae_codes_length);


#endif
