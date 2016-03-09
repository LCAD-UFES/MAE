#ifndef _FACE_RECOG_USER_FUNCTIONS_H
#define _FACE_RECOG_USER_FUNCTIONS_H

// Macros
#define DIRECTION_FORWARD	1
#define DIRECTION_BACKWARD	-1

#define MIN_PERSON_ID 		0
#define MAX_PERSON_ID 		37
#define MIN_ILLUMINATION	0
#define MAX_ILLUMINATION	63

#define MOVING_PHASE		0
#define TRAINING_PHASE		1
#define RECALL_PHASE		2

#define FILE_WIDTH		768
#define FILE_HEIGHT		576
#define IMAGE_WIDTH		168
#define IMAGE_HEIGHT		192

#define IMAGE_FACTOR		FILE_WIDTH / IMAGE_WIDTH

#define EYE			0
#define NOSE			1
#define MOUTH			2
#define VIEW_LOG_POLAR		10

#define RAMDOM_FACES_TEST	"random_faces_t.txt"
#define RAMDOM_FACES_RECALL	"random_faces_r.txt"

// Types

// Prototypes


#endif
