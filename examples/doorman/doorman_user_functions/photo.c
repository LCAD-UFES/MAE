#include "photo.h"

// Module global variables
char *g_strDateVector[] = {"930831\0", "931230\0", "940128\0", "940307\0", "940422\0", "940519\0", "940928\0", "941031\0", "941121\0", "941201\0", "941205\0", "960530\0", "960620\0", "960627\0"};
char *g_strPoseVector[] = {"fa\0", "fb\0", "pl\0", "hl\0", "ql\0", "pr\0", "hr\0", "qr\0", "ra\0", "rb\0", "rc\0", "rd\0", "re\0"};

/*
********************************************************
* Function: photoCreate	 			       *
* Description:  				       *
* Inputs: 	  				       *
* Output:					       *
********************************************************
*/

PHOTO *photoCreate (char *strPhotoName)
{
	PHOTO *pPhoto = NULL;
	
	// Allocate memory for the photo
	if ((pPhoto = (PHOTO *) malloc (sizeof (PHOTO))) == NULL)
	{
		printf ("Cannot allocate memory for the photo.\n");
		return (NULL);
	}
	
	// Saves the foto name
	sprintf (pPhoto->name, "%s", strPhotoName);

	return (pPhoto);
}



/*
********************************************************
* Function: photoDestroy	 		                   *
* Description:  				                       *
* Inputs: 	  				                           *
* Output:					                           *
********************************************************
*/

void photoDestroy (PHOTO *pPhoto)
{
	free (pPhoto);
	
	return;
}



/*
********************************************************
* Function: photoShow		 		       *
* Description:  				       *
* Inputs: 	  				       *
* Output:					       *
********************************************************
*/

void photoShow (PHOTO *pPhoto)
{
    printf ("************************************************\n");
    printf ("\tPhoto:\n");
    printf ("************************************************\n");
	printf ("name = %s\n", pPhoto->name);
	printf ("person = %d\n", pPhoto->person); 
	printf ("pose = %d\n", pPhoto->pose); 
	printf ("expression = %d\n", pPhoto->expression); 
	printf ("date = %d\n", pPhoto->date);
	printf ("leftEye = (%d, %d)\n", pPhoto->leftEye[0], pPhoto->leftEye[1]); 
	printf ("rightEye = (%d, %d)\n", pPhoto->rightEye[0], pPhoto->rightEye[1]); 
	printf ("nose = (%d, %d)\n", pPhoto->nose[0], pPhoto->nose[1]); 
	printf ("mouth = (%d, %d)\n", pPhoto->mouth[0], pPhoto->mouth[1]);
	printf ("************************************************\n");
	
	return;
}



/*
********************************************************
* Function: photoEnumDate	 		                   *
* Description:  				                       *
* Inputs: 	  				                           *
* Output:					                           *
********************************************************
*/

int photoEnumDate (char *strDate)
{
	int nDate = 0;
	
	while (strncmp (g_strDateVector[nDate], strDate, 6) && (nDate < DATES_NUMBER))
		nDate++;
	
	return (nDate);
}



/*
********************************************************
* Function: photoEnumPose	 		                   *
* Description:  				                       *
* Inputs: 	  				                           *
* Output:					                           *
********************************************************
*/

int photoEnumPose (char *strPose)
{
	int nPose = 0;
	
	while (strncmp (g_strPoseVector[nPose], strPose, 2) && (nPose < POSES_NUMBER))
		nPose++;
	
	return (nPose);
}



/*
********************************************************
* Function: photoClassify	 		                   *
* Description:  				                       *
* Inputs: 	  				                           *
* Output:					                           *
********************************************************
*/

void photoClassify (PHOTO *pPhoto, char *strPhotoPath)
{
	char *aux = NULL;
	char strField[STRING_SIZE];
	
	// Gets the person number
	aux = pPhoto->name + strlen (strPhotoPath);
	strncpy (strField, aux, 5);
	pPhoto->person = atoi (strField); 
	
	// Gets the date
	aux += 12;
	pPhoto->date = photoEnumDate (aux);
	
	// Gets the pose
	aux += 7;
	pPhoto->pose = photoEnumPose (aux);
	
	// Gets the expression
	aux += 2;
	pPhoto->expression = (*aux == '_') ? (*(++aux) - 'a') : NOEXPRESSION;

	//photoShow (pPhoto);
	return;
}



/*
********************************************************
* Function: photoGetAttributes 		                   *
* Description:  				                       *
* Inputs: 	  				                           *
* Output:					                           *
********************************************************
*/

int photoGetTargetsCoordinates (PHOTO *pPhoto, char *strGroundTruthsPath)
{
	FILE *pGroundTruthsFile = NULL;
	char strGroundTruthsFileName[STRING_SIZE];
	char strGarbage[STRING_SIZE];
	int i, nLines;
		
	// Mounts the ground truths file name
	if (pPhoto->expression != NOEXPRESSION)
		sprintf (strGroundTruthsFileName, "%s%05d/%05d_%s_%s_%c.txt",
			 strGroundTruthsPath, pPhoto->person, pPhoto->person, 
			 g_strDateVector [pPhoto->date], g_strPoseVector [pPhoto->pose], 
			 (pPhoto->expression + 'a'));
	else
		sprintf (strGroundTruthsFileName, "%s%05d/%05d_%s_%s.txt",
			 strGroundTruthsPath, pPhoto->person, pPhoto->person, 
			 g_strDateVector [pPhoto->date], g_strPoseVector [pPhoto->pose]);

	//printf ("%s\n",strGroundTruthsFileName);
	
	// Opens the ground truths file
	if ((pGroundTruthsFile = fopen (strGroundTruthsFileName, "r")) == NULL)
	{
		printf ("Cannot open file '%s'.\n", strGroundTruthsFileName);
		return (-1);
	}
	
	nLines = (pPhoto->expression != NOEXPRESSION) ? 15 : 16;
	for (i = 0; i < nLines; i++)
		fscanf (pGroundTruthsFile, "%s\n", strGarbage);
	
	fscanf (pGroundTruthsFile, "left_eye_coordinates=%d %d\n", &(pPhoto->leftEye[0]), &(pPhoto->leftEye[1]));
	fscanf (pGroundTruthsFile, "right_eye_coordinates=%d %d\n", &(pPhoto->rightEye[0]), &(pPhoto->rightEye[1]));
	fscanf (pGroundTruthsFile, "nose_coordinates=%d %d\n", &(pPhoto->nose[0]), &(pPhoto->nose[1]));
	fscanf (pGroundTruthsFile, "mouth_coordinates=%d %d\n", &(pPhoto->mouth[0]), &(pPhoto->mouth[1]));
	fclose (pGroundTruthsFile);

	return (0);
}



#ifdef _TEST_PHOTO_MODULE
/*
********************************************************
* Function: main                    	 		       *
* Description: main test function 				       *
* Inputs: 	  				                           *
* Output:					                           *
********************************************************
*/

int main (int argc, char **argv)
{	
	char *strPhotoName = NULL;
	PHOTO *pPhoto = NULL;
	
	// Tests the parameters
	if (argc != 2)
	{
		printf ("Usage:\n ./photo strPhotoName\n");
		return (-1);
	}
	
	// Gets the parameters
	strPhotoName = argv[1];
	
	// Create a photo
	if ((pPhoto = photoCreate (strPhotoName)) == NULL)
    {
        printf ("Cannot create the photo '%s'.\n", strPhotoName);
        return (-1);
    }
	
	// Classifies the photo
	photoClassify (pPhoto);
	
	// Shows the photo
	photoShow (pPhoto);
	
	// Destroys the photo
	photoDestroy (pPhoto);
	
	return (0);
}
#endif
