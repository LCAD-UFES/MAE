/*============================================================================
| (c) Copyright Arthur L. Corcoran, 1992, 1993.  All rights reserved.
|
| Selection operators 
|
| Operators
|    SE_uniform_random()  - just pick one
|    SE_roulette()        - standard roulette
|       SE_max_roulette() - helper for roulette (maximizing)
|       SE_min_roulette() - helper for roulette (minimizing)
|    SE_rank_biased()     - standard linear bias 
|
| Interface
|    SE_table[]   - used in selection of selection method
|    SE_set_fun() - set and select user defined selection function
|    SE_select()  - select selection method by name
|    SE_name()    - get name of current selection method
|    SE_fun()     - setup and perform selection operator
============================================================================*/
#include "ga.h"

int SE_uniform_random(), SE_roulette(), SE_rank_biased();

/*============================================================================
|                     Selection interface
============================================================================*/
/*----------------------------------------------------------------------------
| Selection table
----------------------------------------------------------------------------*/
FN_Table_Type SE_table[] = {
   { NULL,             NULL              },  /* user defined function */
   { "uniform_random", SE_uniform_random },
   { "roulette",       SE_roulette       },
   { "rank_biased",    SE_rank_biased    },
   { NULL,             NULL              }
};

/*----------------------------------------------------------------------------
| Select user selection method
----------------------------------------------------------------------------*/
SE_set_fun(ga_info, fn_name, fn_ptr)
   GA_Info_Ptr  ga_info;
   char         *fn_name;
   FN_Ptr       fn_ptr;
{
   return FN_set_fun(ga_info, SE_table, fn_name, fn_ptr, &ga_info->SE_fun);
}

/*----------------------------------------------------------------------------
| Select selection method
----------------------------------------------------------------------------*/
SE_select(ga_info, fn_name)
   GA_Info_Ptr  ga_info;
   char         *fn_name;
{
   return FN_select(ga_info, SE_table, fn_name, &ga_info->SE_fun);
}

/*----------------------------------------------------------------------------
| Selection name
----------------------------------------------------------------------------*/
char *SE_name(ga_info)
   GA_Info_Ptr  ga_info;
{
   return FN_name(ga_info, SE_table, ga_info->SE_fun);
}

/*----------------------------------------------------------------------------
| Selection interface
----------------------------------------------------------------------------*/
Chrom_Ptr SE_fun(ga_info, pool)
   GA_Info_Ptr    ga_info;
   Pool_Ptr       pool;
{
   int       idx;

   /*--- Error check ---*/
   if(!CF_valid(ga_info)) UT_error("SE_fun: invalid ga_info");

   if(ga_info->SE_fun == NULL) UT_error("SE_fun: null SE_fun");

   /*--- Select a chromosome ---*/
   idx = ga_info->SE_fun(ga_info, pool);
   if(idx < 0 || idx >= pool->size) UT_error("SE_fun: invalid idx");
   if(pool->chrom[idx] == NULL) UT_error("SE_fun: null pool->chrom[idx]");

   return pool->chrom[idx];
}

/*============================================================================
|                             Selection Methods
============================================================================*/
/*----------------------------------------------------------------------------
| Uniform random
----------------------------------------------------------------------------*/
SE_uniform_random(ga_info, pool)
   GA_Info_Ptr ga_info;
   Pool_Ptr    pool;
{
   /*--- Error check ---*/
   if(!CF_valid(ga_info)) UT_error("SE_uniform_random: invalid ga_info");

   return RAND_DOM(0, pool->size-1);
}

/*----------------------------------------------------------------------------
| Roulette
----------------------------------------------------------------------------*/
SE_roulette(ga_info, pool)
   GA_Info_Ptr ga_info;
   Pool_Ptr    pool;
{
   /*--- Error check ---*/
   if(!CF_valid(ga_info)) UT_error("SE_roulette: invalid ga_info");

   /*--- Find PTF for each chromosome ---*/                         
   PL_update_ptf(ga_info, pool);

   if(ga_info->minimize)         
      return SE_min_roulette(ga_info, pool);
   else
      return SE_max_roulette(ga_info, pool);
}      

/*----------------------------------------------------------------------------
| Roulette helper (maximizing)
----------------------------------------------------------------------------*/
SE_max_roulette(ga_info, pool)
   GA_Info_Ptr ga_info;
   Pool_Ptr    pool;
{
   int   i = 0;
   float val = 0.0, spin_val;

   /*--- Error check ---*/
   if(!CF_valid(ga_info)) UT_error("SE_max_roulette: invalid ga_info");

   /*--- Spin the wheel ---*/
   spin_val = RAND_FRAC() * pool->total_fitness;

   /*--- Find corresponding chromosome ---*/
   while(val < spin_val && i < pool->size)
      val += pool->chrom[i++]->fitness;
   if(i > 0) i--;

   return i;
}

/*----------------------------------------------------------------------------
| Roulette helper (minimizing)
----------------------------------------------------------------------------*/
SE_min_roulette(ga_info, pool)
   GA_Info_Ptr ga_info;
   Pool_Ptr    pool;
{
   int   i = 0;
   float val = 0.0, spin_val;

   /*--- Error check ---*/
   if(!CF_valid(ga_info)) UT_error("SE_min_roulette: invalid ga_info");

   /*--- Spin the wheel (value between 0.0 and 100.0) ---*/
   spin_val = RAND_FRAC() * 100.0;

   /*--- Find corresponding chromosome ---*/
   while(val < spin_val && i < pool->size)
      val += pool->chrom[i++]->ptf;
   if(i > 0) i--;

   return i;
}

/*----------------------------------------------------------------------------
| Rank biased 
----------------------------------------------------------------------------*/
SE_rank_biased(ga_info, pool)
   GA_Info_Ptr ga_info;
   Pool_Ptr    pool;
{
   static int ranked = FALSE;

   /*--- Error check ---*/
   if(!CF_valid(ga_info)) UT_error("SE_rank_biased: invalid ga_info");

   /*--- Rank pool ---*/
   if(!ranked) {
      PL_sort(ga_info, pool);

      /*--- Only rank once if replacement is by_rank ---*/
      if(!strcmp(RE_name(ga_info), "by_rank")) ranked = TRUE;
   }

   /*--- Linear biased selection ---*/
   return pool->size * (ga_info->bias - sqrt(ga_info->bias * ga_info->bias
          - 4.0 * (ga_info->bias-1) * RAND_FRAC())) / 2.0 / (ga_info->bias-1);
}
