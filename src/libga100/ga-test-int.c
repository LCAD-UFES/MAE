/*============================================================================
| (c) Copyright Arthur L. Corcoran, 1992, 1993.  All rights reserved.
|
| Genetic Algorithm Test Program - integer cromossome. By Alberto F. De Souza
============================================================================*/
#include "ga.h"

int obj_fun();    /*--- Forward declaration ---*/

/*----------------------------------------------------------------------------
| main()
----------------------------------------------------------------------------*/
main(argc, argv)
   int  argc;
   char *argv[];
{
   GA_Info_Ptr ga_info;

   /*--- Initialize the genetic algorithm ---*/
   ga_info = GA_config("ga-test-int.cfg", obj_fun);

   /*--- Select crossover ---*/
   if(argc > 1) {
      X_select(ga_info, argv[1]);
   };

   /*--- Run the GA ---*/
   GA_run(ga_info);

#if 0
   /*--- Rerun the GA ---*/
   GA_run(ga_info);

   /*--- Reset and rerun the GA ---*/
   GA_reset(ga_info, "ga-test-int.cfg");
   ga_info->chrom_len = 15;
   GA_run(ga_info);
#endif
}

/*----------------------------------------------------------------------------
| obj_fun() - user specified objective function
----------------------------------------------------------------------------*/
int obj_fun(chrom) 
   Chrom_Ptr chrom;
{
   int i, how_far_off;
   double val = 0.0, penalty, fudge_factor;

   /*--- Penalty for not being in correct position ---*/
   penalty = 1.0;

   /*--- Fudge factor for variance from optimal 
         Ensure this is never more than penalty ---*/
   fudge_factor = (1.0 / (double)(chrom->length));

   /*--- Fitness is number of genes out of place for sorted order ---*/
   for(i = 0; i < chrom->length; i++) {
      if(chrom->gene[i] != 50.0) {

         how_far_off = chrom->gene[i] - 50.0;
         if(how_far_off < 0) how_far_off = -how_far_off;

         val += penalty + how_far_off * fudge_factor;
      }
   }

   chrom->fitness = val;
}
