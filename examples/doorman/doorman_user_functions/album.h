#ifndef _ALBUM_H
#define _ALBUM_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "photo.h"
#include "hash.h"

// Definitions
#define STRING_SIZE         256
#define PHOTOS_PATH		    "/home/stiven/feret/data/smaller/"
#define GROUND_TRUTHS_PATH	"/home/stiven/feret/data/ground_truths/name_value/"
#define PHOTO_WIDTH	        768
#define PHOTO_HIGHT         512	

//#define _TEST_ALBUM_MODULE

// Macros

// Structs
struct _album 
{
    char name[STRING_SIZE];
	int size;
	HASH *hashTable;
};
typedef struct _album ALBUM;

// Prototypes
ALBUM *albumCreate (char *strAlbumName);
void albumDestroy (ALBUM *pAlbum);
int albumLoad (ALBUM *pAlbum, char *strPhotosPath);
PHOTO *albumGetPhoto (ALBUM *pAlbum, int nPerson, int nPose, int nExpression, int nDate);
PHOTO *albumGetNextPhoto (ALBUM *pAlbum, int nCurrentPerson, int nCurrentPose, int nCurrentExpression, int nCurrentDate);
PHOTO *albumGetLastPhoto (ALBUM *pAlbum, int nCurrentPerson, int nCurrentPose, int nCurrentExpression, int nCurrentDate);
int albumKeyGenerator (void *pData);

// Global Variables

#endif

