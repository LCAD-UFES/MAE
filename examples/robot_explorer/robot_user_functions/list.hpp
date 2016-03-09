#ifndef _LIST_H
#define _LIST_H

// Includes
#include "common.hpp"

// Definitions

// Macros

// Types
class TNode
{
        public:
        void *info;
        TNode *next;
        
        public:
        TNode (void *pInfo);
        ~TNode (void);
};

class TList
{
        public:
        TNode *head;

        public:
        TList (void);
        ~TList (void);
        bool Append (void *pInfo);
	void Empty (void);
};

// Global Variables

// Prototypes

#endif
