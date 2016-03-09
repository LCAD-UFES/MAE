#ifndef _POINTLIST_H
#define _POINTLIST_H

#include <limits.h>
#include "calibrate.h"

#define DO_NOTHING	1
#define INSERT_BEFORE	2
#define INSERT_ON_HEAD	3
#define INSERT_AFTER	4


struct _key_desc
{
	XY_PAIR dominantEyePoint;
	XY_PAIR nonDominantEyePoint;
	WORLD_POINT world_point;
};
 
typedef struct _key_desc KEY;

struct _no_point_list
{
	KEY element;
	struct _no_point_list *next; 
	struct _no_point_list *previous; 
};
 
typedef struct _no_point_list NO_POINT_LIST;

struct _point_list
{
	int totalNo;
	NO_POINT_LIST  *head ; 
};
 
typedef struct _point_list    POINT_LIST;

POINT_LIST point_list;

int emptyList (NO_POINT_LIST *L);
NO_POINT_LIST  *createNewNo (KEY element);
int insertPointAfterCurrent (POINT_LIST *L, NO_POINT_LIST *current, KEY element);
int insertPointBeforeCurrent (POINT_LIST *L, NO_POINT_LIST *current, KEY element);
int insertPointOnListHead (POINT_LIST *L, NO_POINT_LIST *current, KEY element);
int addPointToPointList (POINT_LIST *L, KEY element);
int deleteList (POINT_LIST *L, NO_POINT_LIST *N);
void disposeList (POINT_LIST *L);
NO_POINT_LIST  *listSearch (POINT_LIST *L, KEY element);
NO_POINT_LIST  *findNearestPoint (POINT_LIST *L, KEY element);
void changeNoPointListData (NO_POINT_LIST  *current, KEY newValue);
void changePointData (POINT_LIST *L, KEY elementToChange, KEY newValue);
int insertOnList(POINT_LIST *L, KEY element);
void changePointOnList (POINT_LIST *L, KEY elementToChange, KEY newValue);
void removePointFromList (POINT_LIST *L, KEY element);

#endif
