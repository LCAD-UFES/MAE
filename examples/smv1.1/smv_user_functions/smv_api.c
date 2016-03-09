#include "smv_api.h"



/************************************************************************/
/* Name: maeInitialize							*/
/* Description:	initializes the mae enviroment				*/
/* Inputs: none								*/
/* Output: none								*/
/************************************************************************/

int maeInitialize ()
{
	init_pattern_xor_table ();
	init_measuraments ();
	build_network ();
	running = 1;
   	all_inputs_update ();
   	all_filters_update ();
	all_dendrites_update (); 
	all_neurons_update ();
	all_outputs_update (); 
	running = 0;
	init_user_functions ();
	
	return (0);
}



/************************************************************************/
/* Name: maeQuit							*/
/* Description:	quit mae enviroment					*/
/* Inputs: none								*/
/* Output: none								*/
/************************************************************************/

int maeQuit ()
{
	return (0);
}



/************************************************************************/
/* Name: maeSetCameraDistance						*/
/* Description:	sets the distance between left and right cameras	*/
/* Inputs: the new distance between cameras				*/
/* Output: none								*/
/************************************************************************/

int maeSetCameraDistance (float fltNewCameraDistance)
{
	CAMERA_DISTANCE = fltNewCameraDistance;
	
	return (0);
}



/************************************************************************/
/* Name: maeSetFocalDistance						*/
/* Description:	sets the focal distance of the left and right cameras	*/
/* Inputs: the new focal distance of the cameras			*/
/* Output: none								*/
/************************************************************************/

int maeSetFocalDistance (float fltNewFocalDistance)
{
	FOCAL_DISTANCE = fltNewFocalDistance;
	
	return (0);
}



/************************************************************************/
/* Name: maeSetStackWidth						*/
/* Description:	sets the stack width					*/
/* Inputs: the new stack width						*/
/* Output: none								*/
/************************************************************************/

int maeSetStackWidth (float fltNewStackWidth)
{
	stackWidth = fltNewStackWidth;

	return (0);
}



/************************************************************************/
/* Name: maeUpdateImageLeft						*/
/* Description:	updates the image left input				*/
/* Inputs: the image's pixel vector, image width and image height	*/
/* Output: none								*/
/************************************************************************/

int maeUpdateImageLeft (int *pPixelVector, int nWidth, int nHeight)
{
	int i, j;
	int pixel, red, green, blue;
	INPUT_DESC *inputLeft = NULL;
	
	if ((inputLeft = get_input_by_name (INPUT_LEFT_NAME)) == NULL)
	{
		Erro ("Error: cannot get input ", INPUT_LEFT_NAME," by name.");

		return (-1);
	}
	
	for (i= 0; i < nWidth * nHeight; i++)
	{
		pixel = pPixelVector[i];
		red   = RED(pixel);
		green = GREEN(pixel);
		blue  = BLUE(pixel);
		inputLeft->image[3 * i + 0] = red;
		inputLeft->image[3 * i + 1] = green;
		inputLeft->image[3 * i + 2] = blue;
	}
	
	update_input_neurons (inputLeft);

	return (0);
}



/************************************************************************/
/* Name: maeUpdateImageRight						*/
/* Description:	updates the image right input				*/
/* Inputs: the image's pixel vector, image width and image height	*/
/* Output: none								*/
/************************************************************************/

int maeUpdateImageRight (int *pPixelVector, int nWidth, int nHeight)
{
	int i, j;
	int pixel, red, green, blue;
	INPUT_DESC *inputRight = NULL;
	
	if ((inputRight = get_input_by_name (INPUT_RIGHT_NAME)) == NULL)
	{
		Erro ("Error: cannot get input ", INPUT_RIGHT_NAME," by name.");

		return (-1);
	}
	
	for (i= 0; i < nWidth * nHeight; i++)
	{
		pixel = pPixelVector[i];
		red   = RED(pixel);
		green = GREEN(pixel);
		blue  = BLUE(pixel);
		inputRight->image[3 * i + 0] = red;
		inputRight->image[3 * i + 1] = green;
		inputRight->image[3 * i + 2] = blue;
	}
	
	update_input_neurons (inputRight);

	return (0);
}



/************************************************************************/
/* Name: maeCalculateVolume						*/
/* Description:	measures the stack volume				*/
/* Inputs: the points coordinates and the points number			*/
/* Output: the volume measure						*/
/************************************************************************/

float maeCalculateVolume (int *pPointsVectorX, int *pPointsVectorY, int nPointsNumber)
{
	float fltVolume;
	KEY element;
	NO_POINT_LIST  *current;
	int i;
	
	ShapeMapDispose (&shape_map);
	ShapeMapInitialize (&shape_map, nPointsNumber);
	
	for (i = 0; i < nPointsNumber; i++)
	{
		element.dominantEyePoint.x = pPointsVectorX[i];
		element.dominantEyePoint.y = pPointsVectorY[i];
		element.nonDominantEyePoint.x = INT_MIN;
		element.nonDominantEyePoint.y = INT_MIN;
		element.world_point.x = DBL_MAX;
		element.world_point.y = DBL_MAX;
		element.world_point.z = DBL_MAX;
		addPointToPointList (&point_list, element);
		current = (i == 0) ? point_list.head : current->next;
		current->element.nonDominantEyePoint = calculateVergencePoint (FAST_VERGENCE, current->element.dominantEyePoint);
		current->element.world_point = calculateWorldPoint ();
		memorizeWorldPoint (current->element.world_point, &shape_map);
	}

	generateShape ();
	fltVolume = stockPileVolume;
	
	return (fltVolume);
}



/************************************************************************/
/* Name: maeGetWorldPoints						*/
/* Description:	get the mapped world points coordinates			*/
/* Inputs: none								*/
/* Output: the world point coordinates passed by reference		*/
/************************************************************************/

int maeGetWorldPoints (float *pWorldPointsVectorX, float *pWorldPointsVectorY, float *pWorldPointsVectorZ)
{
	int i;
		
	for (i = 0; i < shape_map.totalPoints; i++)
	{
		if (shape_map.map_point[i].status == TURN_ON)
		{
			pWorldPointsVectorX[i] = shape_map.map_point[i].point.x;
			pWorldPointsVectorY[i] = shape_map.map_point[i].point.y;
			pWorldPointsVectorZ[i] = shape_map.map_point[i].point.z;	
		}
	}
	
	return (0);
}
