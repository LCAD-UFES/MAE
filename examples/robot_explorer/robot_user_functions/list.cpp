#include "list.hpp"



// ----------------------------------------------------------------------
// TNode::TNode -
//
// Inputs:
//
// Outputs:
// ----------------------------------------------------------------------

TNode::TNode (void *pInfo)
{
        // Sets the attributes
        info = pInfo;
        next = NULL;

        return;
}



// ----------------------------------------------------------------------
// TNode::~TNode -
//
// Inputs:
//
// Outputs:
// ----------------------------------------------------------------------

TNode::~TNode (void)
{
        //delete info;

        return;
}



// ----------------------------------------------------------------------
// TList::TList -
//
// Inputs: 
//
// Outputs: 
// ----------------------------------------------------------------------

TList::TList (void)
{
        // Sets the attributes
        head = NULL;
}
        


// ----------------------------------------------------------------------
// TList::~List -
//
// Inputs:
//
// Outputs:
// ----------------------------------------------------------------------

TList::~TList (void)
{
        TNode *pAux = NULL;

	for (pAux = head; head != NULL; delete head, head = pAux)            
		pAux = pAux->next;
}



// ----------------------------------------------------------------------
// TList::Append -
//
// Inputs:
//
// Outputs:
// ----------------------------------------------------------------------

bool TList::Append (void *pInfo)
{
        TNode *aux = NULL;

        if (head == NULL)
	{
		if ((head = new TNode (pInfo)) == (TNode *) NULL)
		{
			Error("Cannot allocate list node (TList::Append).", "", "");
			return (false);
		}
	}
        else
        {
                for (aux = head; aux->next != NULL; aux = aux->next)
                        ;
                if ((aux->next = new TNode (pInfo)) == (TNode *) NULL)
		{
			Error("Cannot allocate list node (TList::Append).", "", "");
			return (false);
		}
        }

        return (true);
}



// ----------------------------------------------------------------------
// TList::Empty -
//
// Inputs:
//
// Outputs:
// ----------------------------------------------------------------------

void TList::Empty (void)
{
        TNode *pAux = NULL;
                
	for (pAux = head; head != NULL; delete head, head = pAux)            
		pAux = pAux->next;
}
