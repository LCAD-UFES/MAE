/*============================================================================
| (c) Copyright Arthur L. Corcoran, 1992, 1993.  All rights reserved.
|
| GA reports
|
| Functions:
|    RP_report() - periodic report function
|    RP_config() - configuration report function
|    RP_final()  - final report function
|    RP_time()   - time for a report?
|    RP_short()  - short report format
|    RP_long()   - long report format
============================================================================*/
#include "ga.h"

/*----------------------------------------------------------------------------
| Print a report
----------------------------------------------------------------------------*/
RP_report(ga_info, pool)
   GA_Info_Ptr    ga_info;
   Pool_Ptr       pool;
{
   /*--- Error check ---*/
   if(!CF_valid(ga_info)) UT_error("RP_report: invalid ga_info");
   if(!PL_valid(pool)) UT_error("RP_report: invalid pool");

   /*--- Is it report time? ---*/
   if(!RP_time(ga_info, pool)) return;

   /*--- Yes, do appropriate report ---*/
   switch(ga_info->rp_type) {
      case RP_NONE:    break;
      case RP_MINIMAL: break;
      case RP_SHORT:   RP_short(ga_info, pool);
                       break;
      case RP_LONG:    RP_long(ga_info, pool);
                       break;
   }
}

/*----------------------------------------------------------------------------
| Print a configuration report
----------------------------------------------------------------------------*/
RP_config(ga_info)
   GA_Info_Ptr    ga_info;
{
   /*--- Error check ---*/
   if(!CF_valid(ga_info)) UT_error("RP_config: invalid ga_info");

   switch(ga_info->rp_type) {

      /*--- These do not have a config report ---*/
      case RP_NONE:    
                       break;

      /*--- All others have a config report ---*/
      default:
                       CF_report(ga_info);
                       break;
   }
}

/*----------------------------------------------------------------------------
| Print a final report
----------------------------------------------------------------------------*/
RP_final(ga_info)
   GA_Info_Ptr    ga_info;
{
   int i;

   /*--- Error check ---*/
   if(!CF_valid(ga_info)) UT_error("RP_final: invalid ga_info");
   if(ga_info->rp_fid == NULL) UT_error("RP_final: invalid ga_info->rp_fid");

   /*--- Skip final report if RP_NONE ---*/
   if(ga_info->rp_type == RP_NONE) return;

   /*--- Reason for stopping ---*/
   if(ga_info->use_convergence && ga_info->converged) {
      fprintf(ga_info->rp_fid,
              "\nThe GA has converged after %d iterations.\n",
              ga_info->iter);
   } else {
      fprintf(ga_info->rp_fid,
              "\nThe specified number of iterations has been reached.\n");
   }

   /*--- Print best ---*/
   fprintf(ga_info->rp_fid,"\nBest: ");
   for(i = 0; i < ga_info->best->length; i++) {
      fprintf(ga_info->rp_fid,"%G ", ga_info->best->gene[i]);
      if(i % 20 == 19 && i+1 < ga_info->best->length) 
         fprintf(ga_info->rp_fid,"\n      ");
   }
   fprintf(ga_info->rp_fid," (%g)\n\n", ga_info->best->fitness);
}

/*----------------------------------------------------------------------------
| See if time to print a report
----------------------------------------------------------------------------*/
RP_time(ga_info, pool)
   GA_Info_Ptr    ga_info;
   Pool_Ptr       pool;
{
   /*--- Error check ---*/
   if(!CF_valid(ga_info)) UT_error("RP_time: invalid ga_info");
   if(!PL_valid(pool)) UT_error("RP_time: invalid pool");

   /*--- First generation ---*/
   if(ga_info->iter == 0) return TRUE; 

   /*--- Report interval reached ---*/
   if(!((ga_info->iter+1) % ga_info->rp_interval)) return TRUE;

   /*--- Last generation reached ---*/
   if(ga_info->iter+1 == ga_info->max_iter) return TRUE;

   /*--- GA has converged ---*/
   if(ga_info->use_convergence && ga_info->converged) return TRUE;

   /*--- Otherwise, not time for report ---*/
   return FALSE;
}

/*----------------------------------------------------------------------------
| Print a short report
----------------------------------------------------------------------------*/
RP_short(ga_info, pool)
   GA_Info_Ptr  ga_info;
   Pool_Ptr     pool;
{
   /*--- Error check ---*/
   if(!CF_valid(ga_info)) UT_error("RP_short: invalid ga_info");
   if(!PL_valid(pool)) UT_error("RP_short: invalid pool");
   if(ga_info->rp_fid == NULL) UT_error("RP_short: invalid ga_info->rp_fid");

   /*--- Print header first time only ---*/
   if(ga_info->iter < 0) {
      fprintf(ga_info->rp_fid,"\n%s%s\n%s%s\n",
         "Gener    Min      Max      Ave    Variance  ",
         "Std Dev  Tot Fit    Best ",
         "-----  -------  -------  -------  --------  ",
         "-------  -------  -------"
      );
   }

   /*--- Print a line for current iteration ---*/
   fprintf(ga_info->rp_fid,
      "%5d  %7.6G  %7.6G  %7.3G  %8.3G  %7.3G  %7.6G  %7.6G\n", 
      ga_info->iter+1, pool->min, pool->max, pool->ave, pool->var, pool->dev,
      pool->total_fitness, ga_info->best->fitness);
   fflush(ga_info->rp_fid);
}

/*----------------------------------------------------------------------------
| print a long report
----------------------------------------------------------------------------*/
RP_long(ga_info, pool)
   GA_Info_Ptr  ga_info;
   Pool_Ptr     pool;
{
   int i, j;

   /*--- Error check ---*/
   if(!CF_valid(ga_info)) UT_error("RP_long: invalid ga_info");
   if(!PL_valid(pool)) UT_error("RP_long: invalid pool");
   if(ga_info->rp_fid == NULL) UT_error("RP_long: invalid ga_info->rp_fid");

   /*--- Report header line ---*/
   fprintf(ga_info->rp_fid,"\n");
   fprintf(ga_info->rp_fid,"=======================================");
   fprintf(ga_info->rp_fid,"=======================================");
   fprintf(ga_info->rp_fid,"\n");

   /*--- Generation and # mutations ---*/
   fprintf(ga_info->rp_fid,"Generation %d: Mutations = %d (%d total)\n\n", 
      ga_info->iter+1, ga_info->num_mut, ga_info->tot_mut);

   /*--- Pool header ---*/
   fprintf(ga_info->rp_fid," # Parents  XP   Fitness  String\n");
   fprintf(ga_info->rp_fid,"-- ------- ----- -------  ------\n");

   /*--- Print pool ---*/
   for(i = 0; i < pool->size; i++) {
     fprintf(ga_info->rp_fid,"%2d (%2d,%2d) %2d %2d %7G  ",
        i+1, pool->chrom[i]->parent_1 + 1, pool->chrom[i]->parent_2 + 1, 
        pool->chrom[i]->xp1 + 1, pool->chrom[i]->xp2 + 1, 
        pool->chrom[i]->fitness);
     for(j = 0; j < pool->chrom[i]->length; j++) {
        fprintf(ga_info->rp_fid,"%G ", pool->chrom[i]->gene[j]);
        if(j % 15 == 14 && j+1 < pool->chrom[i]->length) 
           fprintf(ga_info->rp_fid,"\n                                  ");
     }
     fprintf(ga_info->rp_fid,"\n");
   }

   /*--- Statistics ---*/
   fprintf(ga_info->rp_fid,
      "\nMin= %G   Max= %G   Ave= %.2G   Tot= %G   Var= %.2G   SD= %.2G\n", 
      pool->min, pool->max, pool->ave, pool->total_fitness, 
      pool->var, pool->dev);

   /*--- Print best ---*/
   fprintf(ga_info->rp_fid,"\nBest: ");
   for(i = 0; i < ga_info->best->length; i++) {
      fprintf(ga_info->rp_fid,"%G ", ga_info->best->gene[i]);
      if(i % 20 == 19 && i+1 < ga_info->best->length) 
         fprintf(ga_info->rp_fid,"\n      ");
   }
   fprintf(ga_info->rp_fid,"(%G)\n", ga_info->best->fitness);

   /*--- Report footer line ---*/
   fprintf(ga_info->rp_fid,"=======================================");
   fprintf(ga_info->rp_fid,"=======================================");
   fprintf(ga_info->rp_fid,"\n");
   fflush(ga_info->rp_fid);

   return(OK);
}
