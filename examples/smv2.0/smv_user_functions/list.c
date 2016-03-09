#include "list.h"



/************************************************************************/
/* Name: listCreate						                                */
/* Description:	creates the list			                            */
/* Inputs: none                    	                                    */
/* Output: a pointer to the header of the list created		            */
/************************************************************************/

LIST *listCreate ()
{
     LIST *list = NULL;
     
     list = (LIST *) malloc (sizeof (LIST));
     
     list->size = 0;
     list->head = NULL;     
     
     return (list);
}



/************************************************************************/
/* Name: listEmpty						                                */
/* Description:	emptys the list			                                */
/* Inputs: a pointer to the list                                      	*/
/* Output: none                                                         */
/************************************************************************/

void listEmpty (LIST *list)
{
     NO *aux1, *aux2;

     for (aux1 = list->head; aux1 != NULL; aux1 = aux2)
     {
         aux2 = aux1->next;
         free (aux1->info);
         free (aux1);    
     }
     
     list->size = 0;
     list->head = NULL;
     
     return;
}



/************************************************************************/
/* Name: listDestroy					                                */
/* Description:	destroys the whole list				                    */
/* Inputs: a pointer to the list                                       	*/
/* Output: none                                                         */
/************************************************************************/

void listDestroy (LIST *list)
{
     listEmpty (list);
     free (list);
     
     return;
}



/************************************************************************/
/* Name: listAppend						                                */
/* Description:	appends a node to the end of the list			        */
/* Inputs: a pointer to the list, a pointer to the info                	*/
/* Output: none                                                         */
/************************************************************************/

void listAppend (LIST *list, void *info)
{
     NO *aux;
     NO *no = NULL;
     
     no = (NO *) malloc (sizeof (NO));
     no->info = info;
     no->next = NULL;
     
     if (list->size == 0)
     {
        list->head = no;
        list->size = 1;
        
        return;
     }
     
     for (aux = list->head; aux->next != NULL; aux = aux->next)
        ;
     
     aux->next = no;
     list->size++;    
     
     return;   
}



/************************************************************************/
/* Name: main function				                                    */
/* Description:	test function				                            */
/* Inputs: default parameters                                           */
/* Output: 0 if successful                                              */
/************************************************************************/

#ifdef LIST_TEST

int main (int argc, char **argv)
{
   LIST *list = NULL;
   char info[3]= "Oi\0";
   NO *aux;
 
   list = listCreate();
   listAppend (list, (void *) info);
   listAppend (list, (void *) info);
   listAppend (list, (void *) info);
   listAppend (list, (void *) info);
 
    for (aux = list->head; aux != NULL; aux = aux->next)
        printf("%s\n", aux->info);    
 
    listEmpty (list);
    
    for (aux = list->head; aux != NULL; aux = aux->next)
        printf("%s\n", aux->info);    
  
  return (0);  
}

#endif
