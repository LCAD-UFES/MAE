/*============================================================================
| (c) Copyright Arthur L. Corcoran, 1992, 1993.  All rights reserved.
|
| Replacement operators 
|
| Operators
|    RE_append()        - simply append to pool
|    RE_by_rank()       - insert into pool by rank
|       RE_do_by_rank() - helper for RE_by_rank()
|    RE_first_weaker()  - replace first weaker member of pool
|    RE_weakest()       - replace weakest member of pool
|
| Interface
|    RE_table[]   - used in selection of replacement method
|    RE_set_fun() - set and select user defined replacement function
|    RE_select()  - select replacement method by name
|    RE_name()    - get name of current crossover function
|    RE_fun()     - setup and perform current crossover operator
|    
| Utility
|    RE_pick_best() - pick the best two out of four chromosomes
============================================================================*/
#include "ga.h"

int RE_append(), RE_by_rank(), RE_first_weaker(), RE_weakest();

/*============================================================================
|                     Replacement interface
============================================================================*/
/*----------------------------------------------------------------------------
| Replacement table
----------------------------------------------------------------------------*/
FN_Table_Type RE_table[] = {
   { NULL,           NULL            },  /* user defined function */
   { "append",       RE_append       },
   { "by_rank",      RE_by_rank      },
   { "first_weaker", RE_first_weaker },
   { "weakest",      RE_weakest      },
   { NULL,           NULL            }
};

/*----------------------------------------------------------------------------
| Select user replacement method
----------------------------------------------------------------------------*/
RE_set_fun(ga_info, fn_name, fn_ptr)
   GA_Info_Ptr  ga_info;
   char         *fn_name;
   FN_Ptr       fn_ptr;
{
   return FN_set_fun(ga_info, RE_table, fn_name, fn_ptr, &ga_info->RE_fun);
}

/*----------------------------------------------------------------------------
| Select replacement method
----------------------------------------------------------------------------*/
RE_select(ga_info, fn_name)
   GA_Info_Ptr  ga_info;
   char         *fn_name;
{
   return FN_select(ga_info, RE_table, fn_name, &ga_info->RE_fun);
}

/*----------------------------------------------------------------------------
| Replacement name
----------------------------------------------------------------------------*/
char *RE_name(ga_info)
   GA_Info_Ptr  ga_info;
{
   return FN_name(ga_info, RE_table, ga_info->RE_fun);
}

/*----------------------------------------------------------------------------
| Replacement interface
----------------------------------------------------------------------------*/
RE_fun(ga_info, pool, p1, p2, c1, c2)
   GA_Info_Ptr    ga_info;
   Pool_Ptr       pool;
   Chrom_Ptr      p1, p2, c1, c2;
{
   /*--- Error checking ---*/
   if(pool == NULL || pool->size < 0) UT_error("RE_fun: invalid pool");
   if(ga_info == NULL) UT_error("RE_fun: invalid ga_info");
   if(p1 == NULL) UT_error("RE_fun: invalid p1");
   if(p2 == NULL) UT_error("RE_fun: invalid p2");
   if(c1 == NULL) UT_error("RE_fun: invalid c1");
   if(c2 == NULL) UT_error("RE_fun: invalid c2");
   if(ga_info->RE_fun == NULL) 
      UT_error("RE_fun: null replacement function");

   if(ga_info->elitist)
      RE_pick_best(ga_info, p1, p2, c1, c2);

   ga_info->RE_fun(ga_info, pool, p1, p2, c1, c2);
}

/*============================================================================
|                             Replacement Methods
============================================================================*/
/*----------------------------------------------------------------------------
| Append children to pool
----------------------------------------------------------------------------*/
RE_append(ga_info, pool, p1, p2, c1, c2)
   GA_Info_Ptr    ga_info;
   Pool_Ptr       pool;
   Chrom_Ptr      p1, p2, c1, c2;
{
   /*--- Error check ---*/
   if(!CF_valid(ga_info)) UT_error("RE_append: invalid ga_info");
   if(!PL_valid(pool)) UT_error("RE_append: invalid pool");
   if(!CH_valid(p1)) UT_error("RE_append: invalid p1");
   if(!CH_valid(p2)) UT_error("RE_append: invalid p2");
   if(!CH_valid(c1)) UT_error("RE_append: invalid c1");
   if(!CH_valid(c2)) UT_error("RE_append: invalid c2");

   /*--- Error conditions ---*/
   if(pool == NULL) UT_error("RE_append: null pool");
   if(pool->size < 0) UT_error("RE_append: invalid pool");

   PL_append(pool, c1, TRUE);
   PL_append(pool, c2, TRUE);
}

/*----------------------------------------------------------------------------
| Insert in ranked order
----------------------------------------------------------------------------*/
RE_by_rank(ga_info, pool, p1, p2, c1, c2)
   GA_Info_Ptr    ga_info;
   Pool_Ptr       pool;
   Chrom_Ptr      p1, p2, c1, c2;
{
   /*--- Error check ---*/
   if(!CF_valid(ga_info)) UT_error("RE_by_rank: invalid ga_info");
   if(!PL_valid(pool)) UT_error("RE_by_rank: invalid pool");
   if(!CH_valid(p1)) UT_error("RE_by_rank: invalid p1");
   if(!CH_valid(p2)) UT_error("RE_by_rank: invalid p2");
   if(!CH_valid(c1)) UT_error("RE_by_rank: invalid c1");
   if(!CH_valid(c2)) UT_error("RE_by_rank: invalid c2");
   /*--- PATCH 1 BEGIN ---*/
   /* Many thanks to Paul-Erik Raue (peraue@cs.vu.nl) 
    * for finding this bug. 
    * 
    * This operation is invalid under the generational model 
    */
   if(!strcmp(GA_name(ga_info),"generational"))
      UT_error("RE_by_rank: invalid under generational model");
   /*--- PATCH 1 END ---*/

   RE_do_by_rank(ga_info, pool, c1);
   RE_do_by_rank(ga_info, pool, c2);
}

/*----------------------------------------------------------------------------
| Replace first weaker
----------------------------------------------------------------------------*/
RE_first_weaker(ga_info, pool, p1, p2, c1, c2)
   GA_Info_Ptr    ga_info;
   Pool_Ptr       pool;
   Chrom_Ptr      p1, p2, c1, c2;
{
   int       i;

   /*--- Error check ---*/
   if(!CF_valid(ga_info)) UT_error("RE_first_weaker: invalid ga_info");
   if(!PL_valid(pool)) UT_error("RE_first_weaker: invalid pool");
   if(!CH_valid(p1)) UT_error("RE_first_weaker: invalid p1");
   if(!CH_valid(p2)) UT_error("RE_first_weaker: invalid p2");
   if(!CH_valid(c1)) UT_error("RE_first_weaker: invalid c1");
   if(!CH_valid(c2)) UT_error("RE_first_weaker: invalid c2");
   /*--- PATCH 1 BEGIN ---*/
   /* Many thanks to Paul-Erik Raue (peraue@cs.vu.nl) 
    * for finding this bug. 
    * 
    * This operation is invalid under the generational model 
    */
   if(!strcmp(GA_name(ga_info),"generational"))
      UT_error("RE_first_weaker: invalid under generational model");
   /*--- PATCH 1 END ---*/

   /*--- Insert c1 ---*/
   for(i=0; i<pool->size; i++) 
      if(CH_cmp(ga_info, pool->chrom[i], c1) > 0) {
         PL_insert(pool, i, c1, TRUE);
         break;
      }

   /*--- Insert c2 ---*/
   for(i=0; i<pool->size; i++) 
      if(CH_cmp(ga_info, pool->chrom[i], c2) > 0) {
         PL_insert(pool, i, c2, TRUE);
         break;
      }
}

/*----------------------------------------------------------------------------
| Replace weakest
----------------------------------------------------------------------------*/
RE_weakest(ga_info, pool, p1, p2, c1, c2)
   GA_Info_Ptr    ga_info;
   Pool_Ptr       pool;
   Chrom_Ptr      p1, p2, c1, c2;
{
   Chrom_Ptr weakest;
   int       i, index;

   /*--- Error check ---*/
   if(!CF_valid(ga_info)) UT_error("RE_weakest: invalid ga_info");
   if(!PL_valid(pool)) UT_error("RE_weakest: invalid pool");
   if(!CH_valid(p1)) UT_error("RE_weakest: invalid p1");
   if(!CH_valid(p2)) UT_error("RE_weakest: invalid p2");
   if(!CH_valid(c1)) UT_error("RE_weakest: invalid c1");
   if(!CH_valid(c2)) UT_error("RE_weakest: invalid c2");
   /*--- PATCH 1 BEGIN ---*/
   /* Many thanks to Paul-Erik Raue (peraue@cs.vu.nl) 
    * for finding this bug. 
    * 
    * This operation is invalid under the generational model 
    */
   if(!strcmp(GA_name(ga_info),"generational"))
      UT_error("RE_weakest: invalid under generational model");
   /*--- PATCH 1 END ---*/

   /*--- Insert c1 ---*/
   weakest = pool->chrom[0];
   index = 0;
   for(i=0; i<pool->size; i++) {
      if(CH_cmp(ga_info, pool->chrom[i], weakest) >= 0) {
         weakest = pool->chrom[i];
         index = i;
      }
   }
   if(CH_cmp(ga_info, weakest, c1) >= 0)
      PL_insert(pool, index, c1, TRUE);

   /*--- Insert c2 ---*/
   weakest = pool->chrom[0];
   index = 0;
   for(i=0; i<pool->size; i++) {
      if(CH_cmp(ga_info, pool->chrom[i], weakest) >= 0) {
         weakest = pool->chrom[i];
         index = i;
      }
   }
   if(CH_cmp(ga_info, weakest, c2) >= 0)
      PL_insert(pool, index, c2, TRUE);
}

/*============================================================================
|                             Utility functions
============================================================================*/
/*----------------------------------------------------------------------------
| Insert a chromosome by rank
----------------------------------------------------------------------------*/
RE_do_by_rank(ga_info, pool, chrom)
   GA_Info_Ptr    ga_info;
   Pool_Ptr       pool;
   Chrom_Ptr      chrom;
{
   int i, cmp;
   int min, max, med;

   /*--- Failure ---*/
   if(CH_cmp(ga_info, pool->chrom[pool->size-1], chrom) <= 0) return OK;

   /*--- Take place of last chrom ---*/
   PL_insert(pool, pool->size-1, chrom, TRUE);

   /*--- Bubble new chrom into position ---*/
   for(i=pool->size-1; i > 0; i--) {
      cmp = CH_cmp(ga_info, pool->chrom[i-1], pool->chrom[i]);
      if(cmp <= 0) break;
      PL_swap(pool, i-1, i);
   }

   return OK;
}

/*----------------------------------------------------------------------------
| Pick best 2 of 4 chromosomes
----------------------------------------------------------------------------*/
RE_pick_best(ga_info, p1, p2, c1, c2)
   GA_Info_Ptr ga_info;
   Chrom_Ptr   p1, p2, c1, c2;
{
   int xp1, xp2;

   /*--- Replace worst child with p1 if p1 better ---*/
   if(CH_cmp(ga_info, c1, c2) > 0) {
      if(CH_cmp(ga_info, c1, p1) > 0) {
         xp1 = c1->xp1; xp2 = c1->xp2;
         CH_copy(p1, c1);
         c1->xp1 = xp1; c1->xp2 = xp2;
      }
   } else {
      if(CH_cmp(ga_info, c2, p1) > 0) {
         xp1 = c2->xp1; xp2 = c2->xp2;
         CH_copy(p1, c2);
         c2->xp1 = xp1; c2->xp2 = xp2;
      }
   }

   /*--- Replace worst child with p2 if p2 better ---*/
   if(CH_cmp(ga_info, c1, c2) > 0) {
      if(CH_cmp(ga_info, c1, p2) > 0) {
         xp1 = c1->xp1; xp2 = c1->xp2;
         CH_copy(p2, c1);
         c1->xp1 = xp1; c1->xp2 = xp2;
      }
   } else {
      if(CH_cmp(ga_info, c2, p2) > 0) {
         xp1 = c2->xp1; xp2 = c2->xp2;
         CH_copy(p2, c2);
         c2->xp1 = xp1; c2->xp2 = xp2;
      }
   }

   /*--- Update indices ---*/
   c1->parent_1 = p1->index;
   c1->parent_2 = p2->index;
   c2->parent_1 = p1->index;
   c2->parent_2 = p2->index;
}
