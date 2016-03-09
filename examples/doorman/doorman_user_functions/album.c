#include "album.h"

// Module global variables

/*
********************************************************
* Function: albumCreate		 		       *
* Description:                                         *
* Inputs:                                              *
* Output:                                              *
********************************************************
*/

ALBUM *albumCreate (char *strAlbumName)
{
	ALBUM *pAlbum = NULL;

	// Allocates memory for the album struct
	if ((pAlbum = (ALBUM *) malloc (sizeof (ALBUM))) == NULL)
	{
		printf ("Cannot allocate memory for the album struct.\n");
		return (NULL);	
	}

	// Saves the album name
	strcpy (pAlbum->name, strAlbumName);

	// Sets the album size
	pAlbum->size = 0;

	return (pAlbum);
}

  
	
/*
********************************************************
* Function: keyFunction		 	               *
* Description:  				       *
* Inputs: 	  				       *
* Output:					       *
********************************************************
*/

int albumKeyGenerator (void *pData)
{	
	int nHashEntry;
	PHOTO *pPhoto = NULL;
	
	pPhoto = (PHOTO *) pData;
	
	nHashEntry = (pPhoto->person & 0x03FF) | 
		         (pPhoto->pose       & 0x0F) << 10 | 
		         (pPhoto->expression & 0x03) << 14 | 
		         (pPhoto->date       & 0x0F) << 16;
	
	return (nHashEntry);
}



/*
********************************************************
* Function: albumLoad   		 	       *
* Description:  				       *
* Inputs: 	  				       *
* Output:					       *
********************************************************
*/

int albumLoad (ALBUM *pAlbum, char *strPhotosPath)
{
    	FILE *pAlbumFile = NULL;
    	PHOTO *pPhoto = NULL;
	int i;
	char strPhotoName[STRING_SIZE];

	// Opens the album file
	if ((pAlbumFile = fopen (pAlbum->name, "r")) == NULL)
	{
		printf ("Cannot open file '%s'.\n", pAlbum->name);
		return (-1);	
	}

	// Gets the photo number
	while (!feof (pAlbumFile))
	{
		fscanf (pAlbumFile, "%s", strPhotoName);
		pAlbum->size++;
	}
	fseek (pAlbumFile, 0x0L, SEEK_SET);

	// Creates the album hash table
	if ((pAlbum->hashTable = hashCreate (pAlbum->size, pAlbum->size, albumKeyGenerator)) == NULL)
	{
		printf ("Cannot create the hash table.\n");
        	return (-1);
	}

	// Loads the photos
	for (i = 0; i < pAlbum->size; i++)
	{
		fscanf (pAlbumFile, "%s",  strPhotoName);
 
	    	if ((pPhoto = photoCreate (strPhotoName)) == NULL)
    		{
             		printf ("Cannot create the photo.\n");
             		return (-1);	
        	}

        	photoClassify (pPhoto, strPhotosPath);
 
		//photoShow (pPhoto);
		if (hashStoreData (pAlbum->hashTable, pPhoto) != 0)
		{
			printf ("Cannot store the photo.\n");
			return (-1);	
		}
	}

	fclose (pAlbumFile);

	return (0);
}	



/*
********************************************************
* Function: albumDestroy                               *	
* Description:                                         *
* Inputs:                                              *
* Output:                                              *
********************************************************
*/

void albumDestroy (ALBUM *pAlbum)
{
	hashDestroy (pAlbum->hashTable);
	free (pAlbum);

	return;
}



/*
********************************************************
* Function: albumGetPhoto                              *
* Description:                                         *
* Inputs:                                              *
* Output:                                              *
********************************************************
*/

PHOTO *albumGetPhoto (ALBUM *pAlbum, int nPerson, int nPose, int nExpression, int nDate)
{
	PHOTO photo;
	
	photo.person = nPerson;
	photo.pose   = nPose;
	photo.expression = nExpression;
	photo.date   = nDate;
        		
	return (hashGetData (pAlbum->hashTable, pAlbum->hashTable->keyGenerator ((void *) &(photo))));
}



/*
********************************************************
* Function: albumGetNextPhoto                          *
* Description:                                         *
* Inputs:                                              *
* Output:                                              *
********************************************************
*/

PHOTO *albumGetNextPhoto (ALBUM *pAlbum, int nCurrentPerson, int nCurrentPose, int nCurrentExpression, int nCurrentDate)
{
	PHOTO *pPhoto = NULL;
	int nPerson, nPose, nExpression, nDate;

	// Searchs the next photo
	for (nPerson = nCurrentPerson, nPose = nCurrentPose, nExpression = nCurrentExpression, nDate = (nCurrentDate + 1); (pPhoto == NULL); nPerson = (nPerson + 1) % PERSONS_NUMBER, nPose = 0)
		for (; (pPhoto == NULL) && (nPose < POSES_NUMBER); nPose++, nExpression = 0)
			for (; (pPhoto == NULL) && (nExpression <= EXPRESSIONS_NUMBER); nExpression++, nDate = 0)
				for (; (pPhoto == NULL) && (nDate < DATES_NUMBER); nDate++)
					pPhoto = albumGetPhoto (pAlbum, nPerson, nPose, nExpression, nDate);
	return (pPhoto);
}



/*
********************************************************
* Function: albumGetLastPhoto                          *
* Description:                                         *
* Inputs:                                              *
* Output:                                              *
********************************************************
*/

PHOTO *albumGetLastPhoto (ALBUM *pAlbum, int nCurrentPerson, int nCurrentPose, int nCurrentExpression, int nCurrentDate)
{
	PHOTO *pPhoto = NULL;
	int nPerson, nPose, nExpression, nDate;

	// Searchs the last photo
	for (nPerson = nCurrentPerson, nPose = nCurrentPose, nExpression = nCurrentExpression, nDate = (nCurrentDate - 1); (pPhoto == NULL); nPerson = PERSONS_NUMBER - (PERSONS_NUMBER - nPerson + 1) % PERSONS_NUMBER, nPose = (POSES_NUMBER - 1))
		for (; (pPhoto == NULL) && (nPose >= 0); nPose--, nExpression = EXPRESSIONS_NUMBER)
			for (; (pPhoto == NULL) && (nExpression >= 0); nExpression--, nDate = (DATES_NUMBER - 1))
				for (; (pPhoto == NULL) && (nDate >= 0); nDate--)
	      	           		pPhoto = albumGetPhoto (pAlbum, nPerson, nPose, nExpression, nDate);
	return (pPhoto);
}



#ifdef _TEST_ALBUM_MODULE
/*
********************************************************
* Function: main				       *
* Description: main test function		       *
* Inputs: 	  				       *
* Output:					       *
********************************************************
*/

int main (int argc, char **argv)
{	
	int nPerson, nPose, nExpression, nDate;
	char *strAlbumFileName = NULL;
	ALBUM *pAlbum = NULL;
	PHOTO *pPhoto = NULL;

	// Tests the parameters
	if (argc != 6)
	{
		printf ("Usage:\n ./album person pose expression date album_file\n");
		return (-1);
	}

	// Gets the parameters
	nPerson = atoi (argv[1]);
	nPose = atoi (argv[2]);
	nExpression = atoi (argv[3]);
	nDate = atoi (argv[4]);
	strAlbumFileName = argv[5];

	// Creates the album
	if ((pAlbum = albumCreate (strAlbumFileName)) == NULL)
	{
        	printf ("Cannot create the album.\n");
        	return (-1);
	}

	// Loads the photos
	if (albumLoad (pAlbum, PHOTOS_PATH) != 0)
	{
        	printf ("Cannot load the album.\n");
        	return (-1);
    	}

	// Gets a photo
	/*if ((pPhoto = albumGetPhoto (pAlbum, nPerson, nPose, nExpression, nDate)) == NULL)
	{
		printf ("This photo doesn't exist in the album.\n");
		return (-1);
	}*/

	// Gets the next photo
	if ((pPhoto = albumGetNextPhoto (pAlbum, nPerson, nPose, nExpression, nDate)) == NULL)
	{
		printf ("This photo doesn't exist in the album.\n");
		return (-1);
	}
	
	// Gets the last photo
	/*if ((pPhoto = albumGetLastPhoto (pAlbum, nPerson, nPose, nExpression, nDate)) == NULL)
	{
		printf ("This photo doesn't exist in the album.\n");
		return (-1);
	}*/

	// Shows the photo
	photoShow (pPhoto);
	
	// Gets the targets coordinates
	//photoGetTargetsCoordinates (pPhoto, GROUND_TRUTHS_PATH);

	// Destroys the album
	albumDestroy (pAlbum);

	return (0);
}
#endif

