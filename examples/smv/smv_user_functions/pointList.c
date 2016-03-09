#include "pointList.h"

int
emptyList (NO_POINT_LIST *L)
{
	return (L == NULL);
}



NO_POINT_LIST  *
createNewNo (KEY element)
{
	NO_POINT_LIST  *newNo;

	newNo = (NO_POINT_LIST *) alloc_mem ((size_t) sizeof(NO_POINT_LIST));

	if (newNo == NULL)
	{
		Erro ("Doesn't have memory to store the point.", "Close the others application and", "choose the point again.");
		return NULL;
	}
	
	newNo->element.dominantEyePoint.x = element.dominantEyePoint.x;
	newNo->element.dominantEyePoint.y = element.dominantEyePoint.y;
	newNo->element.nonDominantEyePoint.x = element.nonDominantEyePoint.x;
	newNo->element.nonDominantEyePoint.y = element.nonDominantEyePoint.y;
	
	return newNo;
}



int
insertPointAfterCurrent (POINT_LIST *L, NO_POINT_LIST *current, KEY element)
{
	NO_POINT_LIST  *newNo;

	newNo = createNewNo (element);

	if (newNo == NULL)
		return 0;
		
	newNo->previous = current;
	newNo->next = current->next;
	current->next = newNo;
	L->totalNo++;
	
	return 1;
}



int
insertPointBeforeCurrent (POINT_LIST *L, NO_POINT_LIST *current, KEY element)
{
	NO_POINT_LIST  *newCurrent;
	
	newCurrent = current->previous;
	return insertPointAfterCurrent (L, newCurrent, element);
}



void
removePointFromList (POINT_LIST *L, KEY element)
{
	NO_POINT_LIST  *nearestPoint;
	
	nearestPoint = findNearestPoint (L, element);

	if (nearestPoint->previous == NULL)
	{
		nearestPoint->next->previous = nearestPoint->previous;
		L->head = nearestPoint->next;
	}
	else if (nearestPoint->next == NULL)
	{
		nearestPoint->previous->next = nearestPoint->next;
	}
	else
	{
		nearestPoint->previous->next = nearestPoint->next;
		nearestPoint->next->previous = nearestPoint->previous;
	}

	free(nearestPoint);
	L->totalNo--;
}



int
deleteList (POINT_LIST *L, NO_POINT_LIST *N)
{
	if (L->head == N)
	{
		L->head = N->next;
	}
	L->totalNo--;
	free(N);
	N->next = NULL;
	N->previous = NULL;
	N = NULL;
	return 1;
}



int
insertPointOnListHead (POINT_LIST *L, NO_POINT_LIST *current, KEY element)
{
	NO_POINT_LIST  *newNo;
	
	newNo = createNewNo (element);

	if (newNo == NULL)
		return 0;

	newNo->next = L->head;
	newNo->previous = (NO_POINT_LIST *) NULL;
	L->head->previous = newNo;
	L->head = newNo;
	L->totalNo++;
	return 1;
}



int
addPointToPointList (POINT_LIST *L, KEY element)
{
	NO_POINT_LIST  *newNo;
	POINT_LIST *point_list;
	NO_POINT_LIST  *current;
	
	
	if (L->totalNo == 0)
	{
		newNo = createNewNo (element);

		if (newNo == NULL)
			return 0;
	
		L->head = newNo;
		L->head->next = (NO_POINT_LIST *) NULL;
		L->head->previous = (NO_POINT_LIST *) NULL;
		L->totalNo++;
		return 1;
	}
	else
	{
		current = L->head;
		for (; current->next != NULL; current = current->next);
		return insertPointAfterCurrent (L, current, element);
	}
}



void
disposeList (POINT_LIST *L)
{
	while (!emptyList(L->head) && deleteList(L, L->head))
		;
	L->totalNo = 0;
}

NO_POINT_LIST  *
listSearch (POINT_LIST *L, KEY element)
{
	NO_POINT_LIST  *current; 
	
	current = L->head;
	
	if (element.dominantEyePoint.x == INT_MIN)
		for(;current != NULL && current->element.nonDominantEyePoint.x != element.nonDominantEyePoint.x;)
			current = current->next;
		
	else
	{
		for(;current != NULL && current->element.dominantEyePoint.x != element.dominantEyePoint.x;)
			current = current->next;
	}
	return current;
}



void
changeNoPointListData (NO_POINT_LIST  *current, KEY newValue)
{
	current->element.dominantEyePoint.x = newValue.dominantEyePoint.x;
	current->element.dominantEyePoint.y = newValue.dominantEyePoint.y;
	current->element.nonDominantEyePoint.x = newValue.nonDominantEyePoint.x;
	current->element.nonDominantEyePoint.y = newValue.nonDominantEyePoint.y;
	current->element.world_point.x = newValue.world_point.x;
	current->element.world_point.y = newValue.world_point.y;
	current->element.world_point.z = newValue.world_point.z;
}



void
changePointData (POINT_LIST *L, KEY elementToChange, KEY newValue)
{
	NO_POINT_LIST  *current;
	
	if ((current = listSearch (L, elementToChange)) == NULL)
		Erro("Cannot find the point that you selected."," Please try again.","");
	else
	{
		changeNoPointListData (current, newValue);
	}
	
}



NO_POINT_LIST  *
findNearestPoint (POINT_LIST *L, KEY element)
{
	NO_POINT_LIST  *current;
	double minDistance;
	double currentDistance;
	NO_POINT_LIST *theNearestPoint;
	
	current = L->head;
	theNearestPoint = current;
	
	if (element.dominantEyePoint.x == INT_MIN)
	{
		minDistance = currentDistance = distanceBetweenImagePoints (element.nonDominantEyePoint, current->element.nonDominantEyePoint);
		current = current->next;
		
		for(;current != NULL;)
		{
			currentDistance = distanceBetweenImagePoints (element.nonDominantEyePoint, current->element.nonDominantEyePoint);
			if (currentDistance < minDistance)
			{
				minDistance = currentDistance;
				theNearestPoint = current;
			}
			current = current->next;
		}
	}
	else
	{
		minDistance = currentDistance = distanceBetweenImagePoints (element.dominantEyePoint, current->element.dominantEyePoint);
		current = current->next;
		
		for(;current != NULL;)
		{
			currentDistance = distanceBetweenImagePoints (element.dominantEyePoint, current->element.dominantEyePoint);
			if (currentDistance < minDistance)
			{
				minDistance = currentDistance;
				theNearestPoint = current;
			}
			current = current->next;
		}
	}
	return theNearestPoint;
	
}

int
insertOnList (POINT_LIST *L, KEY element)
{
	float nextDistance, previousDistance;
	NO_POINT_LIST  *nearestPoint;
	NO_POINT_LIST  *nextPoint;
	NO_POINT_LIST  *previousPoint;
	int whereToInsert = 0;
	

	nearestPoint = findNearestPoint (L, element);
	nextPoint = nearestPoint->next;
	previousPoint = nearestPoint->previous;
	
	if (nextPoint == NULL)
	{
		previousDistance = distanceBetweenImagePoints (element.dominantEyePoint, previousPoint->element.dominantEyePoint);
		nextDistance = distanceBetweenImagePoints (nearestPoint->element.dominantEyePoint, previousPoint->element.dominantEyePoint);
		whereToInsert = (previousDistance == nextDistance) ? DO_NOTHING : ((previousDistance < nextDistance) ? INSERT_BEFORE : INSERT_AFTER);
	} else if (previousPoint == NULL)
	{
		previousDistance = distanceBetweenImagePoints (nearestPoint->element.dominantEyePoint, nextPoint->element.dominantEyePoint);
		nextDistance = distanceBetweenImagePoints (nextPoint->element.dominantEyePoint, element.dominantEyePoint);
		whereToInsert = (previousDistance == nextDistance) ? DO_NOTHING : ((previousDistance < nextDistance) ? INSERT_ON_HEAD : INSERT_AFTER);
	}
	else
	{
		previousDistance = distanceBetweenImagePoints (element.dominantEyePoint, previousPoint->element.dominantEyePoint);
		nextDistance = distanceBetweenImagePoints (nextPoint->element.dominantEyePoint, element.dominantEyePoint);
		if (previousDistance == nextDistance)
		{
			previousDistance = distanceBetweenImagePoints (nearestPoint->element.dominantEyePoint, previousPoint->element.dominantEyePoint);
			nextDistance = distanceBetweenImagePoints (nextPoint->element.dominantEyePoint, nearestPoint->element.dominantEyePoint);
			whereToInsert = (previousDistance == nextDistance) ? DO_NOTHING : ((previousDistance < nextDistance) ? INSERT_BEFORE : INSERT_AFTER);
		}
		else
			whereToInsert = ((previousDistance < nextDistance) ? INSERT_BEFORE : INSERT_AFTER);
	}
	
	switch (whereToInsert)
	{
		case INSERT_BEFORE:
			return insertPointBeforeCurrent (L, nearestPoint, element);
		case INSERT_AFTER:
			return addPointToPointList (L, element);
		case INSERT_ON_HEAD:
			return insertPointOnListHead (L, nearestPoint, element);
		default:
			return 0;
	}
}



void
changePointOnList (POINT_LIST *L, KEY elementToChange, KEY newValue)
{
	NO_POINT_LIST  *nearestPoint;
	
	nearestPoint = findNearestPoint (L, elementToChange);
	changePointData (L, nearestPoint->element, newValue);
}
