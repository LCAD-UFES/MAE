/*============================================================================
| (c) Copyright Arthur L. Corcoran, 1992, 1993.  All rights reserved.
|
| Function table management
|
| Functions:
|    FN_set_fun() - Set user function
|    FN_select()  - Select function by name
|    FN_name()    - Get function name from function pointer
============================================================================*/
#include "ga.h"

/*----------------------------------------------------------------------------
| Set and select user defined function
----------------------------------------------------------------------------*/
FN_set_fun(ga_info, fn_table, fn_name, fn_ptr, rtn_fun)
   GA_Info_Ptr  ga_info;
   FN_Table_Ptr fn_table;
   char         *fn_name;
   FN_Ptr       fn_ptr, *rtn_fun;
{
   int len;

   /*--- Check for invalid ga_info ---*/
   if(!CF_valid(ga_info)) UT_error("FN_set_fun: invalid ga_info");
   if(rtn_fun == NULL) UT_error("FN_set_fun: invalid rtn_fun");

   /*--- Free current function name ---*/
   if(fn_table[0].name != NULL) {
      free(fn_table[0].name);
      fn_table[0].name = NULL;
   }

   /*--- Set function name if provided ---*/
   if(fn_name != NULL && (len = strlen(fn_name) + 1) > 1) {

      /*--- Allocate memory for function name ---*/
      fn_table[0].name = (char *)calloc(len, sizeof(char));
      if(fn_table[0].name == NULL) UT_error("FN_set_fun: alloc failed");

      /*--- Copy the function name ---*/
      strcpy(fn_table[0].name, fn_name);
   }

   /*--- Set user function ---*/
   fn_table[0].fun = fn_ptr;

   /*--- Set return function ---*/
   *rtn_fun = fn_ptr;
}

/*----------------------------------------------------------------------------
| Select function by name
----------------------------------------------------------------------------*/
FN_select(ga_info, fn_table, fn_name, rtn_fun)
   GA_Info_Ptr  ga_info;
   FN_Table_Ptr fn_table;
   char         *fn_name;
   FN_Ptr       *rtn_fun;
{
   int i;

   /*--- Check for invalid ga_info ---*/
   if(!CF_valid(ga_info)) UT_error("FN_select: invalid ga_info");
   if(rtn_fun == NULL) UT_error("FN_select: invalid rtn_fun");

   /*--- User defined crossover? ---*/
   if(fn_table[0].name != NULL &&
      !strncmp(fn_name, fn_table[0].name, strlen(fn_table[0].name))) 
   {
      /*--- Null user function ---*/
      if(fn_table[0].fun == NULL) 
         UT_warn("FN_select: User function is NULL");

      /*--- Return pointer to user function ---*/
      *rtn_fun = fn_table[0].fun; 
      return; 
   }

   /*--- Search fn_table for matching function name ---*/
   for(i = 1; fn_table[i].fun != NULL; i++) {

      /*--- Does name match? ---*/
      if(!strncmp(fn_name, 
                  fn_table[i].name, 
                  MIN(strlen(fn_name),strlen(fn_table[i].name)))) 
      {
         /*--- Return pointer to function ---*/
         *rtn_fun = fn_table[i].fun; 
         return; 
      }
   }

   /*--- Invalid selection ---*/
   UT_error("FN_select: Invalid selection");
}

/*----------------------------------------------------------------------------
| Function name
----------------------------------------------------------------------------*/
char *FN_name(ga_info, fn_table, fn_ptr)
   GA_Info_Ptr  ga_info;
   FN_Table_Ptr fn_table;
   FN_Ptr       fn_ptr;
{
   int i;

   /*--- Check for invalid ga_info ---*/
   if(!CF_valid(ga_info)) UT_error("FN_name: invalid ga_info");

   /*--- Search for current function in fn_table ---*/
   for(i = 0; i == 0 || fn_table[i].fun != NULL; i++) {

      /*--- Does this function match? ---*/
      if(fn_ptr == fn_table[i].fun) {

         /*--- Function match, but null name ---*/
         if(fn_table[i].name == NULL) 
            return "Unspecified";

         /*--- Function match, valid name ---*/
         else    
            return fn_table[i].name;
      }
   }

   /*--- Function not found ---*/
   return "Unknown";
}

