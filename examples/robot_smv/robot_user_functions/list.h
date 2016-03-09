#ifndef _LIST_H
#define _LIST_H

// Includes
#include <stdio.h>
#include <stdlib.h>

// Definitions
//#define LIST_TEST

// Macros

// Structs
typedef struct _no
{
    void *info;
    struct _no *next;
} NO;

typedef struct _list   
{
    int size;
    NO *head;
} LIST;

// Prototypes
LIST *listCreate ();
void listEmpty (LIST *list);
void listDestroy (LIST *list);
void listAppend (LIST *list, void *info);

#endif
