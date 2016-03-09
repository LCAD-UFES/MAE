#ifndef _HASH_H
#define _HASH_H

// Includes
#include <stdio.h>
#include  <malloc.h>

// Definitions
#define FREE    	 -1
#define EMPTY		 NULL

// Macros
//#define _TEST_HASH_MODULE

// Structs
struct _entry
{
	int collision;
 	void *data;
};
typedef struct _entry ENTRY;
	
struct _hash 
{
    int primaryAreaSize;
    int overflowAreaSize;
	ENTRY *entries;
	int (*keyGenerator) (void *);
};
typedef struct _hash HASH;

// Prototypes
HASH *hashCreate (int nPrimaryAreaSize, int nOverflowAreaSize, int (*keyGenerator) (void *));
void hashDestroy (HASH *pHashTable);
int hashStoreData (HASH *pHashTable, void *pData);
int hashStoreArrayData (HASH *pHashTable, void *pDataArray[], int nDataArraySize);
void *hashGetData (HASH *pHashTable, int nKey);
void hashShowTable (HASH *pHashTable, char *(*showData) (void *));

// Global Variables

#endif

