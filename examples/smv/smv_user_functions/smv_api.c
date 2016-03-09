#include "smv_api.h"



/************************************************************************/
/* Name: maeInitialize							*/
/* Description:	initializes the mae enviroment				*/
/* Inputs: none								*/
/* Output: none								*/
/************************************************************************/

void maeInitialize ()
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
	return;
}



/************************************************************************/
/* Name: maeQuit							*/
/* Description:	quit mae enviroment					*/
/* Inputs: none								*/
/* Output: none								*/
/************************************************************************/

void maeQuit ()
{
	return;
}



/************************************************************************/
/* Name: maeSetCameraDistance						*/
/* Description:	sets the distance between left and right cameras	*/
/* Inputs: the new distance between cameras				*/
/* Output: none								*/
/************************************************************************/

void maeSetCameraDistance (float fltNewCameraDistance)
{
	CAMERA_DISTANCE = fltNewCameraDistance;
	return;
}



/************************************************************************/
/* Name: maeSetFocalDistance						*/
/* Description:	sets the focal distance of the left and right cameras	*/
/* Inputs: the new focal distance of the cameras			*/
/* Output: none								*/
/************************************************************************/

void maeSetFocalDistance (float fltNewFocalDistance)
{
	FOCAL_DISTANCE = fltNewFocalDistance;
	return;
}



/************************************************************************/
/* Name: maeSetStackWidth						*/
/* Description:	sets the stack width					*/
/* Inputs: the new stack width						*/
/* Output: none								*/
/************************************************************************/

void maeSetStackWidth (float fltNewStackWidth)
{
	stackWidth = fltNewStackWidth;
}



/************************************************************************/
/* Name: maeUpdateImageLeft						*/
/* Description:	updates the image left input				*/
/* Inputs: the image's pixel vector, image width and image height	*/
/* Output: none								*/
/************************************************************************/

void maeUpdateImageLeft (int *pPixelVector, int nWidth, int nHeight)
{
	int i, j;
	static INPUT_DESC *inputLeft = NULL;
	
	if (inputLeft == (INPUT_DESC *) NULL)
		inputLeft = get_input_by_name ("image_left");
	
	for (i= 0; i < nWidth * nHeight; i++)
		inputLeft->image[i] = pPixelVector[i];
	
	update_input_neurons (inputLeft);
	return;
}



/************************************************************************/
/* Name: maeUpdateImageRight						*/
/* Description:	updates the image right input				*/
/* Inputs: the image's pixel vector, image width and image height	*/
/* Output: none								*/
/************************************************************************/

void maeUpdateImageRight (int *pPixelVector, int nWidth, int nHeight)
{
	int i, j;
	static INPUT_DESC *inputRight = NULL;
	
	if (inputRight == (INPUT_DESC *) NULL)
		inputRight = get_input_by_name ("image_right");
	
	for (i= 0; i < nWidth * nHeight; i++)
		inputRight->image[i] = pPixelVector[i];
	
	update_input_neurons (inputRight);
	return;
}



/************************************************************************/
/* Name: maeCalculateVolume						*/
/* Description:	measures the stack volume				*/
/* Inputs: the points coordinates and the points number			*/
/* Output: the volume measure						*/
/************************************************************************/

float maeCalculateVolume (int *pPointsVectorX, int *pPointsVectorY, int nPointsNumber)
{
	int i;
	FILE *maeLogFile;
	float fltVolume;
	
	if ((maeLogFile = fopen ("maeLogFile.txt", "a")) == NULL)
		return (0.0);
	
	fprintf (maeLogFile, "\tNew Set of Points:\n\tBegin\n");
	
	for (i = 0; i < nPointsNumber; i++)
		fprintf (maeLogFile, "\t\t%d -> (%d, %d)\n", i + 1, pPointsVectorX[i], pPointsVectorY[i]);
	
	fprintf (maeLogFile, "\tEnd\n\n");
	
	fltVolume = (float) (rand() % 100);

	fprintf (maeLogFile, "\tVolume Measure = %.4f\n\n", fltVolume);
	
	fclose (maeLogFile);

	return (fltVolume);
}
