#ifndef _TRAFFIC_SIGN_USER_FUNCTIONS_H
#define _TRAFFIC_SIGN_USER_FUNCTIONS_H

// Includes
#include <stdlib.h>
#include <math.h>
#include "mae.h"
#include "../traffic_sign.h"

// Macros
//Sequence 01
//#define MIN_FRAME_ID	32500
#define MIN_FRAME_ID	32617
#define MAX_FRAME_ID	35500

//Sequence 02
//#define MIN_FRAME_ID	800
//#define MAX_FRAME_ID	7000

//Sequence 03
//#define MIN_FRAME_ID	22000
//#define MAX_FRAME_ID	24000

//Sequence 04
//#define MIN_FRAME_ID	12600
//#define MAX_FRAME_ID	16600

#define DIRECTION_FORWARD	1
#define DIRECTION_REWIND	-1

#define MOVING_PHASE		0
#define TRAINING_PHASE		1
#define RECALL_PHASE		2

#define SIGNS_SEQ1 "/media/shared_win_dir/_mae/placas_seq01"
#define SIGNS_SEQ2 "/media/shared_win_dir/_mae/placas_seq02"
#define SIGNS_SEQ3 "/media/shared_win_dir/_mae/placas_seq03"
#define SIGNS_SEQ4 "/media/shared_win_dir/_mae/placas_seq04"

#define RANDOM_FACES_TEST "/media/shared_win_dir/_cropped_seqs/Seq03_signs_test"
#define RANDOM_FACES_TRAIN "/media/shared_win_dir/_cropped_seqs/Seq03_signs_train" //

//#define RANDOM_FACES_TEST "/media/shared_win_dir/_cropped_seqs/set1"
//#define RANDOM_FACES_TRAIN "/media/shared_win_dir/_cropped_seqs/set1"


// Types

// Prototypes

// Exportable Variables
extern int g_frameID, g_sequence, g_camera, g_number_of_boxes, g_curr_sign;
extern float g_corner0_x[10], g_corner0_y[10], g_corner1_x[10], g_corner1_y[10];

#endif /*_TRAFFIC_SIGN_USER_FUNCTIONS_H*/
