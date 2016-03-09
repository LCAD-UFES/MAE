/*============================================================================
| (c) Copyright Arthur L. Corcoran, 1992, 1993.  All rights reserved.
|
| Mutation operators 
|
| Bit String Representations
|    MU_simple_invert() - random bit inversion based on mutation rate
|    MU_simple_random() - random bit value based on mutation rate
|
| Any Representation
|    MU_swap()          - random element swap based on mutation rate
|
| Interface
|    MU_table[]   - used in selection of mutation method
|    MU_set_fun() - set and select user defined mutation function
|    MU_select()  - select mutation function by name
|    MU_name()    - get name of current mutation function
|    MU_fun()     - setup and perform current mutation operator
============================================================================*/
#include "ga.h"

int MU_simple_invert(), MU_simple_random(), MU_int_random(), MU_swap();

/*============================================================================
|                     Mutation interface
============================================================================*/
/*----------------------------------------------------------------------------
| Mutation table
----------------------------------------------------------------------------*/
FN_Table_Type MU_table[] = {
   {  NULL,           NULL             }, /* user defined function */
   { "simple_invert", MU_simple_invert },
   { "simple_random", MU_simple_random },
   { "int_random",    MU_int_random    },
   { "swap",          MU_swap          },
   { NULL,            NULL             }
};

/*----------------------------------------------------------------------------
| Select user mutation method
----------------------------------------------------------------------------*/
MU_set_fun(ga_info, fn_name, fn_ptr)
   GA_Info_Ptr  ga_info;
   char         *fn_name;
   FN_Ptr       fn_ptr;
{
   return FN_set_fun(ga_info, MU_table, fn_name, fn_ptr, &ga_info->MU_fun);
}

/*----------------------------------------------------------------------------
| Select mutation method
----------------------------------------------------------------------------*/
MU_select(ga_info, fn_name)
   GA_Info_Ptr  ga_info;
   char         *fn_name;
{
   return FN_select(ga_info, MU_table, fn_name, &ga_info->MU_fun);
}

/*----------------------------------------------------------------------------
| Mutation name
----------------------------------------------------------------------------*/
char *MU_name(ga_info)
   GA_Info_Ptr  ga_info;
{
   return FN_name(ga_info, MU_table, ga_info->MU_fun);
}

/*----------------------------------------------------------------------------
| Mutation interface
----------------------------------------------------------------------------*/
MU_fun(ga_info, chrom)
   GA_Info_Ptr ga_info;
   Chrom_Ptr   chrom;
{
   /*--- Random chance to mutate ---*/
   if(RAND_FRAC() <= ga_info->mu_rate && ga_info->MU_fun != NULL) {
      ga_info->MU_fun(ga_info, chrom);
      ga_info->num_mut++;
      ga_info->tot_mut++;
   }
}

/*============================================================================
|                             Mutation operators
============================================================================*/
/*----------------------------------------------------------------------------
| Simple mutation, invert bit
----------------------------------------------------------------------------*/
MU_simple_invert(ga_info, chrom)
   GA_Info_Ptr ga_info;
   Chrom_Ptr chrom;
{
   int idx;

   /*--- Select bit at random ---*/
   idx = RAND_DOM(chrom->idx_min, chrom->length-1);

   /*--- Invert selected bit ---*/
   chrom->gene[idx] = chrom->gene[idx] ? 0 : 1;
}

/*----------------------------------------------------------------------------
| Simple mutation, random bit
----------------------------------------------------------------------------*/
MU_simple_random(ga_info, chrom)
   GA_Info_Ptr ga_info;
   Chrom_Ptr chrom;
{
   int idx;

   /*--- Select bit at random ---*/
   idx = RAND_DOM(chrom->idx_min, chrom->length-1);

   /*--- Assign random value to bit ---*/
   chrom->gene[idx] = RAND_BIT();
}

/*----------------------------------------------------------------------------
| Random int within range
----------------------------------------------------------------------------*/
MU_int_random(ga_info, chrom)
   GA_Info_Ptr ga_info;
   Chrom_Ptr chrom;
{
   int idx;

   /*--- Select bit at random ---*/
   idx = RAND_DOM(chrom->idx_min, chrom->length-1);

   /*--- Assign random value to gene ---*/
   chrom->gene[idx] = RAND_DOM(0.0,ga_info->max_gene_val);
}

/*----------------------------------------------------------------------------
| Swap random elements
----------------------------------------------------------------------------*/
MU_swap(ga_info, chrom)
   GA_Info_Ptr ga_info;
   Chrom_Ptr chrom;
{
   Gene_Type tmp;
   int       i, j;

   /*--- Select two bits at random (can be same) ---*/
   i = RAND_DOM(chrom->idx_min, chrom->length-1);
   j = RAND_DOM(chrom->idx_min, chrom->length-1);

   /*--- Swap the elements ---*/
   tmp            = chrom->gene[i];
   chrom->gene[i] = chrom->gene[j];
   chrom->gene[j] = tmp;
}
