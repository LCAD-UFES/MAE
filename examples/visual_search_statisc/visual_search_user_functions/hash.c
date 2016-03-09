#include "hash.h"

// Module global variables

/*
********************************************************
* Function: hashCreate			 	       *
* Description: Creates an empty hash table  	       *
* Inputs:                                              *
* Output:					       *
********************************************************
*/

HASH *hashCreate (int nPrimaryAreaSize, int nOverflowAreaSize, int (*keyGenerator) (void *))
{	
	int i;
	HASH *pHashTable = NULL;
	
	// Allocates memory for the hash struct
	if ((pHashTable = (HASH *) malloc (sizeof (HASH))) == NULL)
	{
		printf ("Cannot allocate memory for the hash table.\n");
		return (NULL);
	}
	
	// Sets the hash attributes
	pHashTable->primaryAreaSize = nPrimaryAreaSize;
	pHashTable->overflowAreaSize = nOverflowAreaSize;
	pHashTable->keyGenerator = keyGenerator;

	// Allocates memory for the hash entries
	if ((pHashTable->entries = (ENTRY *) malloc ((pHashTable->primaryAreaSize + pHashTable->overflowAreaSize) * sizeof (ENTRY))) == NULL)
	{
		printf ("Cannot allocate memory for the hash table entries.\n");
		return (NULL);
	}
	
	// Initializes the hash entries
	for (i = 0; i < (pHashTable->primaryAreaSize + pHashTable->overflowAreaSize); i++)
	{
		pHashTable->entries[i].data = EMPTY;
		pHashTable->entries[i].collision = FREE;
	}
	
	return (pHashTable);
}



/*
********************************************************
* Function: hashDestroy			 	       *
* Description:  				       *
* Inputs: 	  				       *
* Output:					       *
********************************************************
*/

void hashDestroy (HASH *pHashTable)
{	
	free (pHashTable->entries);
	free (pHashTable);
	
	return;
}



/*
********************************************************
* Function: hashStoreData			       *
* Description:  				       *
* Inputs: 	  				       *
* Output:					       *
********************************************************
*/

int hashStoreData (HASH *pHashTable, void *pData)
{	
	int i, nHashEntry, nKey;

	// Calculates the key
	nKey = pHashTable->keyGenerator (pData);

	// Calculates the hash entry
	nHashEntry = nKey % pHashTable->primaryAreaSize;
    
	// Checks if the entry is empty, if Ok stores the data
	if (pHashTable->entries[nHashEntry].data == EMPTY)
	{
		pHashTable->entries[nHashEntry].data = pData;
		return (0);
	}

	// Goes to the end of the collision list
	while (pHashTable->entries[nHashEntry].collision != FREE)
		nHashEntry = pHashTable->entries[nHashEntry].collision;
    
	// Finds an empty entry in the collision area
	for (i = pHashTable->primaryAreaSize; (i < (pHashTable->primaryAreaSize + pHashTable->overflowAreaSize)) && (pHashTable->entries[i].data != NULL); i++)
		;
   	
	// Checks if the hash collision area is full
	if (i == (pHashTable->primaryAreaSize + pHashTable->overflowAreaSize))
	{
		printf ("Hash table is full.\n");
		return (-1);
	}
   	
	// Stores the data
	pHashTable->entries[nHashEntry].collision = i;
	pHashTable->entries[i].data = pData;
   	
	return (0);
}



/*
********************************************************
* Function: hashStoreArrayData			       *
* Description:  				       *
* Inputs: 	  				       *
* Output:					       *
********************************************************
*/

int hashStoreArrayData (HASH *pHashTable, void *pDataArray[], int nDataArraySize)
{	
	int i;
    
	// Stores each element of the data array
	for (i = 0; i < nDataArraySize; i++)
	{
		if (hashStoreData (pHashTable, (void *) pDataArray[i]) != 0)
		{
			printf ("Cannot store data in the hash table.\n");
			return (-1);
		} 
	}
	    
	return (0);
}



/*
********************************************************
* Function: hashShowTable		 	       *
* Description:  				       *
* Inputs: 	  				       *
* Output:					       *
********************************************************
*/

void hashShowTable (HASH *pHashTable, char *(*showData) (void *))
{	
	int i;
    
	printf ("************************************************\n");
	printf ("\tHash Table: entry -> (data, collision)\n");
	printf ("************************************************\n");
    
	// Shows the hash primary area
	printf ("Primary Area (Size = %d):\n", pHashTable->primaryAreaSize);
	for (i = 0; i < pHashTable->primaryAreaSize; i++)
		printf ("%d -> (%s, %d)\n", i, showData (pHashTable->entries[i].data), pHashTable->entries[i].collision);

	// Shows the hash overflow area
	printf ("Overflow Area (Size = %d):\n", pHashTable->overflowAreaSize);
	for (i = pHashTable->primaryAreaSize; i < (pHashTable->primaryAreaSize + pHashTable->overflowAreaSize); i++)
		printf ("%d -> (%s, %d)\n", i, showData (pHashTable->entries[i].data), pHashTable->entries[i].collision);
	printf ("************************************************\n");
    
	return;
}



/*
********************************************************
* Function: hashGetData		 	       	       *
* Description:  				       *
* Inputs: 	  				       *
* Output:					       *
********************************************************
*/

void *hashGetData (HASH *pHashTable, int nKey)
{	
	int nHashEntry;
    
	// Calculates the hash entry
	nHashEntry = nKey % pHashTable->primaryAreaSize;
    
	// Checks if the entry is empty
	if (pHashTable->entries[nHashEntry].data == EMPTY)
		return (NULL);
    
	// Goes to the end of the collision list
	while (nKey != pHashTable->keyGenerator(pHashTable->entries[nHashEntry].data))
	{
		if ((nHashEntry = pHashTable->entries[nHashEntry].collision) == FREE)
			return (NULL);
	}
    
	// Returns the data
	return (pHashTable->entries[nHashEntry].data);
}


#ifdef _TEST_HASH_MODULE
/*
********************************************************
* Function: testKeyFunction		 	       *
* Description:  				       *
* Inputs: 	  				       *
* Output:					       *
********************************************************
*/

int testKeyGenerator (void *pData)
{
	char *strData = NULL;

	strData = (char *) pData;
    
	return abs ((int) strData[0] << 8 + (int) strData[1]);
}



/*
********************************************************
* Function: testShowData		 	       *
* Description:  				       *
* Inputs: 	  				       *
* Output:					       *
********************************************************
*/

char *testShowData (void *pData)
{
	return ((char *) pData);
}



/*
********************************************************
* Function: main				       *
* Description: main test function		       *
* Inputs:					       *
* Output:					       *
********************************************************
*/

int main (int argc, char **argv)
{	
	HASH *pHashTable = NULL;
	void *pDataArray[] = {"ui\0", "ai\0", "oi\0", "ei\0", "ss\0"};
	void *pData = NULL;
	char *strData = NULL;
   
	// Checks the program parameters
	if (argc != 2)
	{
		printf ("Usage:\n ./hash strData\n");
		return (-1);
	}
	
	// Gets the program parameters
	strData = argv[1];
    
	// Creates the hash table
	if ((pHashTable = hashCreate (7, 3, testKeyGenerator)) == NULL)
	{
		printf ("Cannot create hash table.\n");
		return (-1);   
	}
    
	// Stores the data in the hash table
	if (hashStoreArrayData (pHashTable, pDataArray, 5) != 0)
	{
		printf ("Cannot store data array in the hash table.\n");
		return (-1);
	}
    
	// Shows the hash table
	hashShowTable (pHashTable, testShowData);
   
	// Gets the data in the hash table
	if ((pData = hashGetData (pHashTable, testKeyGenerator (strData))) == NULL)
	{
		printf ("Cannot find data in the hash table.\n");
		return (-1);
	}
    
	// Shows the data
	printf ("Data:\n%s\n", testShowData (pData));
          
	return (0);
}
#endif

