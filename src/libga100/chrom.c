/*============================================================================
| (c) Copyright Arthur L. Corcoran, 1992, 1993.  All rights reserved.
|
| Chromosome management
|
| Functions:
|    CH_alloc()  - allocate a chrom
|    CH_resize() - resize a chrom
|    CH_free()   - deallocate a chrom
|    CH_valid()  - is a chrom valid?
|    CH_reset()  - reset a chrom
|    CH_copy()   - copy a chrom over another
|    CH_cmp()    - compare two chromosomes
|    CH_print()  - print a chrom
|    CH_verify() - ensure chrom makes sense
============================================================================*/
#include "ga.h"

/*----------------------------------------------------------------------------
| Allocate a chromosome
----------------------------------------------------------------------------*/
Chrom_Ptr CH_alloc(length) 
   int length;
{
   Chrom_Ptr chrom;

   /*--- Error check ---*/
   if(length <= 0) UT_error("CH_alloc: invalid length");

   /*--- Allocate memory for chromosome ---*/
   chrom = (Chrom_Ptr)calloc(1, sizeof(Chrom_Type));
   if(chrom == NULL) UT_error("CH_alloc: chrom alloc failed");
   chrom->length = length;

   /*--- Allocate memory for genes ---*/
   chrom->gene = (Gene_Ptr)calloc(length, sizeof(Gene_Type));
   if(chrom->gene == NULL) UT_error("CH_alloc: gene alloc failed");

   /*--- Put in magic cookie ---*/
   chrom->magic_cookie = CH_cookie;

   /*--- Reset the chromosome ---*/
   CH_reset(chrom);

   return chrom;
}

/*----------------------------------------------------------------------------
| Resize a chromosome
|
| NOTE: after the resize, the chromosome may no longer be valid, 
|       therefore it is reset
----------------------------------------------------------------------------*/
CH_resize(chrom, length)
   Chrom_Ptr chrom;
   int       length;
{
   /*--- Error check ---*/
   if(!CH_valid(chrom)) UT_error("CH_resize: invalid chrom");
   if(length <= 0) UT_error("CH_resize: invalid length");

   /*--- Reallocate memory for genes ---*/
   chrom->gene = (Gene_Ptr)realloc(chrom->gene, length * sizeof(Gene_Type));
   if(chrom->gene == NULL) UT_error("CH_resize: gene realloc failed");
   chrom->length = length;

   /*--- Reset the chromosome ---*/
   CH_reset(chrom);
}

/*----------------------------------------------------------------------------
| De-Allocate a chromosome
----------------------------------------------------------------------------*/
CH_free(chrom)
   Chrom_Ptr chrom;
{
   /*--- Error check ---*/
   if(!CH_valid(chrom)) return;

   /*--- Free memory for genes ---*/
   if(chrom->gene != NULL) {
      free(chrom->gene);
      chrom->gene = NULL;
   }

   /*--- Put in NULL magic cookie ---*/
   chrom->magic_cookie = NL_cookie;

   /*--- Free memory for chromosome ---*/
   free(chrom);
}

/*----------------------------------------------------------------------------
| Is a chromosome valid, i.e., has it been allocated by CH_alloc()?
----------------------------------------------------------------------------*/
CH_valid(chrom)
   Chrom_Ptr chrom;
{
   /*--- Check for NULL pointers ---*/
   if(chrom == NULL) return FALSE;
   if(chrom->gene == NULL) return FALSE;

   /*--- Check for magic cookie ---*/
   if(chrom->magic_cookie != CH_cookie) return FALSE;

   /*--- Otherwise valid ---*/
   return TRUE;
}

/*----------------------------------------------------------------------------
| Reset a chromosome
----------------------------------------------------------------------------*/
CH_reset(chrom)
   Chrom_Ptr chrom;
{
   int i;

   /*--- Error check ---*/
   if(!CH_valid(chrom)) UT_error("CH_reset: invalid chrom");

   /*--- Initialize genes ---*/
   for(i=0; i<chrom->length; i++)
      chrom->gene[i] = (Gene_Type)0;

   /*--- Initialize chromosome ---*/
   chrom->fitness  = 0.0;
   chrom->ptf      = 0.0;
   chrom->index    = -1;
   chrom->idx_min  = 0;
   chrom->idx_max  = chrom->length;
   chrom->parent_1 = -1;
   chrom->parent_2 = -1;
   chrom->xp1      = -1;
   chrom->xp2      = -1;
}

/*----------------------------------------------------------------------------
| Copy a chromosome
----------------------------------------------------------------------------*/
CH_copy(src, dst)
   Chrom_Ptr src, dst;
{
   Gene_Ptr gene;

   /*--- Error check ---*/
   if(!CH_valid(src)) UT_error("CH_copy: invalid src");
   if(!CH_valid(dst)) UT_error("CH_copy: invalid dst");

   /*--- Resize if necessary ---*/
   if(dst->length != src->length) CH_resize(dst, src->length);

   /*--- Save memory pointed to by gene ---*/
   gene = dst->gene;

   /*--- Copy chrom ---*/
   memcpy(dst, src, sizeof(Chrom_Type));

   /*--- Restore memory pointed to by gene ---*/
   dst->gene = gene;

   /*--- Copy gene ---*/
   memcpy(dst->gene, src->gene, src->length * sizeof(Gene_Type));
}

/*----------------------------------------------------------------------------
| Compare chromosomes A and B:
|    -1  = A is better
|     1  = B is better
|     0  = Same
----------------------------------------------------------------------------*/
CH_cmp(ga_info, a, b)
   GA_Info_Ptr ga_info;
   Chrom_Ptr   a, b;
{
   if(ga_info->minimize)
      if(a->fitness < b->fitness)
         return -1;
      else if(a->fitness > b->fitness)
         return 1;
      else
         return 0;
   else
      if(a->fitness > b->fitness)
         return -1;
      else if(a->fitness < b->fitness)
         return 1;
      else
         return 0;
}

/*----------------------------------------------------------------------------
| Print a chromosome
----------------------------------------------------------------------------*/
CH_print(chrom)
   Chrom_Ptr chrom;
{
   int i;

   /*--- Error check ---*/
   if(!CH_valid(chrom)) UT_error("CH_print: invalid chrom");

   printf("==============================================================\n");
   printf("\nChrom: \n");
   for(i=0; i<chrom->length; i++)
      printf("%G ", chrom->gene[i]);
   printf("\n\n");
   printf("fitness = %G, ptf = %G, index = %d, idx_min = %d, idx_max = %d\n", 
      chrom->fitness, chrom->ptf, chrom->index, chrom->idx_min, chrom->idx_max);
   printf("parent_1 = %d, parent_2 = %d, xp1 = %d, xp2 = %d\n", 
      chrom->parent_1, chrom->parent_2, chrom->xp1, chrom->xp2);
   printf("==============================================================\n");
}

/*----------------------------------------------------------------------------
| Verify a chromosome
----------------------------------------------------------------------------*/
CH_verify(ga_info, chrom)
   GA_Info_Ptr ga_info;
   Chrom_Ptr chrom;
{
   int i;
   char *allele_count, err_str[80];

   /*--- Error check ---*/
   if(!CF_valid(ga_info)) UT_error("CH_verify: invalid ga_info");
   if(!CH_valid(chrom)) UT_error("CH_verify: invalid chrom");

   /*--- Check for invalid length ---*/
   if(chrom->length <= 0) {
      CH_print(chrom);
      UT_error("CH_verify: bad length");
   }

   /*--- Check for invalid idx_min ---*/
   if(chrom->idx_min < 0 || chrom->idx_min > chrom->length) {
      CH_print(chrom);
      UT_error("CH_verify: idx_min out of bounds");
   }

   /*--- Check for invalid idx_max ---*/
   if(chrom->idx_max < 0 || chrom->idx_max > chrom->length) {
      CH_print(chrom);
      UT_error("CH_verify: idx_max out of bounds");
   }

   /*--- Check for invalid permutation ---*/
   if(ga_info->datatype == DT_INT_PERM) {

      /*--- Allocate allele_count vector ---*/
      allele_count = calloc(chrom->length, sizeof(char));
      if(allele_count == NULL) 
         UT_error("CH_verify: cannot alloc allele_count");
   
      /*--- Check each gene in the chromosome ---*/
      for(i=0; i<chrom->length; i++) {

         /*--- Check for allele out of bounds ---*/
         if(chrom->gene[i] < 1 || (int)chrom->gene[i] > chrom->length) {
            CH_print(chrom);
            sprintf(err_str,"CH_verify: gene[%d] = %G is out of bounds", 
                    i, chrom->gene[i]);
            UT_error(err_str);

         /*--- Check for duplicate alleles ---*/
         } else if(++(allele_count[((int)chrom->gene[i])-1]) > 1) {
            CH_print(chrom);
            sprintf(err_str,"CH_verify: gene[%d] = %G is a duplicate", 
                    i, chrom->gene[i]);
            UT_error(err_str);
         }
      }
      free(allele_count);
   }
}
