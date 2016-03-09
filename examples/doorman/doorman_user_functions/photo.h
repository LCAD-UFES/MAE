#ifndef _PHOTO_H
#define _PHOTO_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

// Definitions
#define PERSONS_NUMBER      739
#define POSES_NUMBER        13
#define DATES_NUMBER        14
#define EXPRESSIONS_NUMBER   3
#define STRING_SIZE         256

//#define _TEST_PHOTO_MODULE

// Macros

// Structs
enum _pose {FA, FB, PL, HL, QL, PR, HR, QR, RA, RB, RC, RD, RE, NOPOSE};
typedef enum _pose POSE;

enum _expression {EA, EB, EC, NOEXPRESSION};
typedef enum _expression EXPRESSION;

enum _date {DATA1, DATA2, DATA3, DATA4, DATA5, DATA6, DATA7, DATA8, DATA9, DATA10, DATA11, DATA12, DATA13, DATA14, NODATE};
typedef enum _date DATE;

struct _photo 
{
	int person;
	POSE pose;
	EXPRESSION expression;
	DATE date;
	int leftEye[2];
	int rightEye[2];
	int nose[2];
	int mouth[2];
	char name[STRING_SIZE];
};
typedef struct _photo PHOTO;

// Prototypes
PHOTO *photoCreate (char *strPhotoName);
void photoDestroy (PHOTO *pPhoto);
void photoClassify (PHOTO *pPhoto, char *strPhotoPath);
int photoGetTargetsCoordinates (PHOTO *pPhoto, char *strGroundTruthsPath);
void photoShow (PHOTO *pPhoto);

// Global Variables

#endif

