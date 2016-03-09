/*============================================================================
| (c) Copyright Arthur L. Corcoran, 1992, 1993.  All rights reserved.
|
| Genetic algorithm 
|
| Operators
|    GA_generational()  - generational GA
|       GA_gen_init()   - initialize generational GA
|       GA_init_trial() - initialize inner loop for generational GA
|    GA_steady_state()  - steady state GA
|       GA_ss_init()    - initialize steady state GA
|    
| Interface
|    GA_table[]   - used in selection of GA method
|    GA_set_fun() - set and select user defined GA function
|    GA_select()  - select GA function by name
|    GA_name()    - get name of current GA function
|    GA_config()  - configure the GA (do only once for each ga_info)
|    GA_reset()   - reset the GA
|    GA_run()     - setup and perform current GA
|                   (GA_fun would be more consistent but less intuitive)
|    
| Utility
|    GA_trial()      - a single iteration of the inner loop
|    GA_cum()        - see if children are the cumulative/historical best
|    GA_gap()        - handle generation gap
============================================================================*/
#include "ga.h"

int GA_generational(), GA_steady_state();

static Chrom_Ptr child1, child2;

/*============================================================================
|                                  Interface
============================================================================*/
/*----------------------------------------------------------------------------
| GA table
----------------------------------------------------------------------------*/
FN_Table_Type GA_table[] = {
   { NULL,           NULL            },  /* user defined function */
   { "generational", GA_generational },
   { "steady_state", GA_steady_state },
   { NULL,           NULL            }
};

/*----------------------------------------------------------------------------
| Select user GA method
----------------------------------------------------------------------------*/
GA_set_fun(ga_info, fn_name, fn_ptr)
   GA_Info_Ptr  ga_info;
   char         *fn_name;
   FN_Ptr       fn_ptr;
{
   return FN_set_fun(ga_info, GA_table, fn_name, fn_ptr, &ga_info->GA_fun);
}

/*----------------------------------------------------------------------------
| Select GA method
----------------------------------------------------------------------------*/
GA_select(ga_info, fn_name)
   GA_Info_Ptr  ga_info;
   char         *fn_name;
{
   return FN_select(ga_info, GA_table, fn_name, &ga_info->GA_fun);
}

/*----------------------------------------------------------------------------
| GA name
----------------------------------------------------------------------------*/
char *GA_name(ga_info)
   GA_Info_Ptr  ga_info;
{
   return FN_name(ga_info, GA_table, ga_info->GA_fun);
}

/*----------------------------------------------------------------------------
| Configure the genetic algorithm
----------------------------------------------------------------------------*/
GA_Info_Ptr GA_config(cfg_name, EV_fun) 
   char *cfg_name;
   int  (*EV_fun)();
{
   GA_Info_Ptr ga_info;

   /*--- Get memory for ga_info ---*/
   ga_info = CF_alloc();

   /*--- Register user's evaluation function if provided ---*/
   if(EV_fun != NULL)
      ga_info->EV_fun = EV_fun;

   /*--- Read config file if provided ---*/
   if(cfg_name != NULL && cfg_name[0] != '\0' && cfg_name[0] != '\n')
      CF_read(ga_info, cfg_name);

   return ga_info;
}

/*----------------------------------------------------------------------------
| Reset the genetic algorithm
----------------------------------------------------------------------------*/
void
GA_reset(GA_Info_Ptr ga_info, char *cfg_name)
{
   int  (*EV_fun)();

   /*--- Error check ---*/
   if(!CF_valid(ga_info)) UT_error("GA_reset: invalid ga_info");

   /*--- Save EV_fun() ---*/
   EV_fun = ga_info->EV_fun;

   /*--- Reset ga_info ---*/
   CF_reset(ga_info);

   /*--- Restore EV_fun() ---*/
   ga_info->EV_fun = EV_fun;

   /*--- Read config file if provided ---*/
   if(cfg_name != NULL && cfg_name[0] != '\0' && cfg_name[0] != '\n')
      CF_read(ga_info, cfg_name);
}

/*----------------------------------------------------------------------------
| Run the GA
----------------------------------------------------------------------------*/
void
GA_run(GA_Info_Ptr ga_info)
{
   /*--- Ensure valid config information ---*/
   CF_verify(ga_info);

   /*--- Print out config information ---*/
   RP_config(ga_info);

   /*--- Seed random number generator ---*/
   SEED_RAND(ga_info->rand_seed);

   /*--- Run the GA ---*/
   ga_info->GA_fun(ga_info);
}

/*============================================================================
|                                Generational GA
============================================================================*/
/*----------------------------------------------------------------------------
| Generational GA
----------------------------------------------------------------------------*/
GA_generational(ga_info)
   GA_Info_Ptr ga_info;
{
   Pool_Ptr      tmp_pool;
 
   /*--- Initialize ---*/
   GA_gen_init(ga_info);

   /*--- Outer loop is for each generation ---*/
   for(ga_info->iter = 0; 
       ga_info->max_iter < 0 || ga_info->iter < ga_info->max_iter; 
       ga_info->iter++) {

      /*--- Check for convergence ---*/
      if(ga_info->use_convergence && ga_info->converged) break;

      /*--- Setup for new set of trials ---*/
      GA_init_trial(ga_info);

      /*--- Handle generation gap ---*/
      GA_gap(ga_info);

      /*--- Inner loop is for each reproduction ---*/
      for( ; ga_info->new_pool->size < ga_info->old_pool->size; ) {
         GA_trial(ga_info);
      }

      /*--- Print report if appropriate ---*/
      RP_report(ga_info, ga_info->new_pool);

      /*--- Swap old and new pools ---*/
      tmp_pool          = ga_info->old_pool;
      ga_info->old_pool = ga_info->new_pool;
      ga_info->new_pool = tmp_pool;
   }

   /*--- Final report ---*/
   RP_final(ga_info);

   /*--- Free genes for children ---*/
   CH_free(child1);
   CH_free(child2);

   return OK;
}
 
/*----------------------------------------------------------------------------
| Initialize the GA
----------------------------------------------------------------------------*/
GA_gen_init(ga_info)
   GA_Info_Ptr  ga_info;
{
   Pool_Ptr     old_pool, new_pool;

   /*--- Error check ---*/
   if(!CF_valid(ga_info)) UT_error("GA_generational: invalid ga_info");

   /*--- Make sure pool allocation is ok ---*/
   if(!PL_valid(ga_info->old_pool))
      ga_info->old_pool = PL_alloc(ga_info->pool_size);
   if(!PL_valid(ga_info->new_pool) || ga_info->new_pool == ga_info->old_pool)
      ga_info->new_pool = PL_alloc(ga_info->pool_size);
   old_pool = ga_info->old_pool;
   new_pool = ga_info->new_pool;

   /*--- Minimize or maximize? ---*/
   old_pool->minimize = new_pool->minimize = ga_info->minimize;
 
   /*--- Initialize or reset pools ---*/
   PL_generate(ga_info, old_pool);

   /*--- Don't initialize pool second time ---*/
   if(ga_info->ip_flag != IP_NONE) ga_info->ip_flag = IP_NONE;
 
   /*--- Pool must have even number of chromosomes ---*/
   if((old_pool->size % 2) != 0) {
      /*--- Put in a good chromosome ---*/
      if(ga_info->minimize)
         PL_append(old_pool, old_pool->chrom[old_pool->min_index], TRUE);
      else
         PL_append(old_pool, old_pool->chrom[old_pool->max_index], TRUE);
   }
   new_pool->size = 0;

   /*--- Make sure best is allocated ---*/
   if(!CH_valid(ga_info->best)) 
      ga_info->best = CH_alloc(ga_info->chrom_len);
    
   /*--- Save best member of initial pool ---*/
   if(ga_info->minimize)
      CH_copy(old_pool->chrom[old_pool->min_index], ga_info->best);
   else
      CH_copy(old_pool->chrom[old_pool->max_index], ga_info->best);
 
   /*--- No mutations yet ---*/
   ga_info->num_mut = 0;
   ga_info->tot_mut = 0;
 
   /*--- Initial pool report ---*/
   ga_info->iter = -1;
   RP_report(ga_info, old_pool);

   /*--- Allocate genes for children ---*/
   child1 = CH_alloc(ga_info->chrom_len);
   child2 = CH_alloc(ga_info->chrom_len);
}
 
/*----------------------------------------------------------------------------
| Setup for a new set of trials (Generational GA only)
----------------------------------------------------------------------------*/
GA_init_trial(ga_info)
   GA_Info_Ptr  ga_info;
{
   /*--- Cleanup the new pool ---*/
   ga_info->new_pool->size = 0;
 
   /*--- Reset number of mutations ---*/
   ga_info->num_mut = 0;
 
   /*--- Not elitist ---*/
   if(!ga_info->elitist) return OK;
 
   /*--- Save best members if Elitist ---*/
   if(ga_info->minimize) {
      PL_append(ga_info->new_pool,
                ga_info->old_pool->chrom[ga_info->old_pool->min_index],
                TRUE);
      PL_append(ga_info->new_pool,
                ga_info->old_pool->chrom[ga_info->old_pool->min_index],
                TRUE);
   } else {
      PL_append(ga_info->new_pool,
                ga_info->old_pool->chrom[ga_info->old_pool->max_index],
                TRUE);
      PL_append(ga_info->new_pool,
                ga_info->old_pool->chrom[ga_info->old_pool->max_index],
                TRUE);
   }
   return OK;
}

/*============================================================================
|                                Steady State GA
============================================================================*/
/*----------------------------------------------------------------------------
| Steady-state GA 
----------------------------------------------------------------------------*/
GA_steady_state(ga_info)
   GA_Info_Ptr ga_info;
{
   /*--- Initialize ---*/
   GA_ss_init(ga_info);
 
   /*--- Outer loop is for each generation ---*/
   for(ga_info->iter = 0; 
       ga_info->max_iter < 0 || ga_info->iter < ga_info->max_iter; 
       ga_info->iter++) {
 
      /*--- Check convergence (only if no mutation) ---*/
      if(ga_info->use_convergence && ga_info->converged) break;
 
      /*--- "Inner loop" is a single reproduction ---*/
      GA_trial(ga_info);
 
      /*--- Print report if appropriate ---*/
      RP_report(ga_info, ga_info->new_pool);
   }
 
   /*--- Final report ---*/
   RP_final(ga_info);

   /*--- Free genes for children ---*/
   CH_free(child1);
   CH_free(child2);
 
   return OK;
}

/*----------------------------------------------------------------------------
| Initialize GA
----------------------------------------------------------------------------*/
GA_ss_init(ga_info)
   GA_Info_Ptr ga_info;
{
   Pool_Ptr pool;

   /*--- Error check ---*/
   if(!CF_valid(ga_info)) UT_error("GA_steady_state: invalid ga_info");
 
   /*--- Make sure pool allocation is ok ---*/
   if(PL_valid(ga_info->old_pool)) {
      if(ga_info->new_pool != ga_info->old_pool && PL_valid(ga_info->new_pool))
         PL_free(ga_info->new_pool);
      ga_info->new_pool = ga_info->old_pool;
   } else {
      if(PL_valid(ga_info->new_pool)) 
         ga_info->old_pool = ga_info->new_pool;
      else
         ga_info->old_pool = ga_info->new_pool = PL_alloc(ga_info->pool_size);
   }
   pool = ga_info->old_pool;

   /*--- Minimize or maximize ---*/
   pool->minimize = ga_info->minimize;

   /*--- Initialize or reset pool ---*/
   PL_generate(ga_info, pool);

   /*--- Don't initialize pool second time ---*/
   if(ga_info->ip_flag != IP_NONE) ga_info->ip_flag = IP_NONE;

   /*--- Make sure best is allocated ---*/
   if(!CH_valid(ga_info->best)) 
      ga_info->best = CH_alloc(ga_info->chrom_len);

   /*--- Save min and max members of initial pool ---*/
   if(ga_info->minimize) 
      CH_copy(pool->chrom[pool->min_index], ga_info->best);
   else
      CH_copy(pool->chrom[pool->max_index], ga_info->best);

   /*--- No mutations yet ---*/
   ga_info->num_mut = 0;
   ga_info->tot_mut = 0;

   /*--- Initial pool report ---*/
   ga_info->iter = -1;
   RP_report(ga_info, pool);

   /*--- Allocate genes for children ---*/
   child1 = CH_alloc(ga_info->chrom_len);
   child2 = CH_alloc(ga_info->chrom_len);
}

/*============================================================================
|                               GA Inner Loop
============================================================================*/
/*----------------------------------------------------------------------------
| A single trial
----------------------------------------------------------------------------*/
GA_trial(ga_info)
   GA_Info_Ptr ga_info;
{
   Chrom_Ptr parent1, parent2;

   /*--- Selection ---*/
   parent1 = SE_fun(ga_info, ga_info->old_pool);
   parent2 = SE_fun(ga_info, ga_info->old_pool);

   /*--- Validate parents ---*/
   CH_verify(ga_info, parent1);
   CH_verify(ga_info, parent2);
   
   /*--- Crossover ---*/
   X_fun(ga_info, parent1, parent2, child1, child2);

   /*--- Mutation ---*/
   MU_fun(ga_info, child1);
   MU_fun(ga_info, child2);
   
   /*--- Evaluate children ---*/
   ga_info->EV_fun(child1);
   ga_info->EV_fun(child2);

   /*--- Validate children ---*/
   CH_verify(ga_info, child1);
   CH_verify(ga_info, child2);

   /*--- Replacement ---*/
   RE_fun(ga_info, ga_info->new_pool, parent1, parent2, child1, child2);
   
   /*--- Best So Far? ---*/
   GA_cum(ga_info, child1, child2);
   
   /*--- Update GA system statistics ---*/
   PL_stats(ga_info, ga_info->new_pool);
}

/*============================================================================
|                               Utility
============================================================================*/
/*----------------------------------------------------------------------------
| See if children are best so far
----------------------------------------------------------------------------*/
GA_cum(ga_info, c1, c2)
   GA_Info_Ptr ga_info;
   Chrom_Ptr   c1, c2;
{
   if(ga_info->minimize) {
      if(c1->fitness < ga_info->best->fitness)
         CH_copy(c1, ga_info->best);
      if(c2->fitness < ga_info->best->fitness)
         CH_copy(c2, ga_info->best);
   } else {
      if(c1->fitness > ga_info->best->fitness)
         CH_copy(c1, ga_info->best);
      if(c2->fitness > ga_info->best->fitness)
         CH_copy(c2, ga_info->best);
   }
}

/*----------------------------------------------------------------------------
| Handle generation gap
----------------------------------------------------------------------------*/
GA_gap(ga_info)
   GA_Info_Ptr ga_info;
{
   int i, num_clones;
   Chrom_Ptr parent;

   /*--- Disabled ---*/
   if(ga_info->gap <= 0.0) return OK;

   /*--- How many to copy over ---*/
   num_clones = (int)(ga_info->pool_size * ga_info->gap);

   /*--- Append to new pool ---*/
   for(i=0; 
       i < num_clones && ga_info->new_pool->size < ga_info->old_pool->size;
       i++) 
   {
      /*--- Select a chromosome ---*/
      parent = SE_fun(ga_info, ga_info->old_pool);

      /*--- Put it in the new pool ---*/
      PL_append(ga_info->new_pool, parent, TRUE);

   }

   /*--- Make sure stats are updated ---*/
   if(ga_info->new_pool->size == ga_info->old_pool->size)
      PL_stats(ga_info, ga_info->new_pool);

   return OK;
}
