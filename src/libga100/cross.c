/*============================================================================
| (c) Copyright Arthur L. Corcoran, 1992, 1993.  All rights reserved.
|
| Crossover operators 
|
| Bit String Representations
|    X_simple()   - simple crossover
|    X_uniform()  - uniform crossover
|
| Order-Based Integer Representations
|    X_order1()     - order1   (Starkweather, et. al., 1991 GA Conf.)
|    X_order2()     - order2   (Starkweather, et. al., 1991 GA Conf.)
|    X_pos()        - position (Starkweather, et. al., 1991 GA Conf.)
|    X_cycle()      - cycle    (Starkweather, et. al., 1991 GA Conf.)
|    X_pmx()        - PMX      (Starkweather, et. al., 1991 GA Conf.)
|    X_uox()        - uniform order crossover
|    X_rox()        - relative order crossover
|    X_asex()       - asexual crossover
|       X_do_asex() - helper for X_asex()
|
| Interface
|    X_table[]    - used in selection of crossover method
|    X_set_fun()  - set and select user defined crossover function
|    X_select()   - select crossover method by name
|    X_name()     - get name of current crossover function
|    X_fun()      - setup and perform current crossover operator
|
| Utility
|    X_gen_xp()    - generate a random crossover point
|    X_gen_2_xp()  - generate two sorted, random crossover points
|    X_gen_4_xp()  - generate four sorted, random crossover points
|    X_init_kids() - reset children for crossover
|    X_map()       - find allele in a chromosome
|
| NOTE: Crossover points should always be thought of as "inclusive"
============================================================================*/
#include "ga.h"

int X_simple(), X_uniform(), X_order1(), X_order2(), X_pos(), X_cycle(), 
    X_pmx(), X_uox(), X_rox(), X_asex();

/*============================================================================
|                           Crossover interface
============================================================================*/
/*----------------------------------------------------------------------------
| Table for selection of crossover method
----------------------------------------------------------------------------*/
FN_Table_Type X_table[] = {
   {  NULL,       NULL      }, /* user defined function */
   { "simple",    X_simple  },
   { "uniform",   X_uniform },
   { "order1",    X_order1  },
   { "order2",    X_order2  },
   { "position",  X_pos,    },
   { "cycle",     X_cycle,  },
   { "pmx",       X_pmx,    },
   { "uox",       X_uox,    },
   { "rox",       X_rox,    },
   { "asexual",   X_asex,   },
   { NULL,        NULL,     }
};

/*----------------------------------------------------------------------------
| Select user crossover method
----------------------------------------------------------------------------*/
X_set_fun(ga_info, fn_name, fn_ptr)
   GA_Info_Ptr  ga_info;
   char         *fn_name;
   FN_Ptr       fn_ptr;
{
   return FN_set_fun(ga_info, X_table, fn_name, fn_ptr, &ga_info->X_fun);
}

/*----------------------------------------------------------------------------
| Select crossover method
----------------------------------------------------------------------------*/
X_select(ga_info, fn_name)
   GA_Info_Ptr  ga_info;
   char         *fn_name;
{
   return FN_select(ga_info, X_table, fn_name, &ga_info->X_fun);
}

/*----------------------------------------------------------------------------
| Crossover name
----------------------------------------------------------------------------*/
char *X_name(ga_info)
   GA_Info_Ptr  ga_info;
{
   return FN_name(ga_info, X_table, ga_info->X_fun);
}

/*----------------------------------------------------------------------------
| Crossover interface
----------------------------------------------------------------------------*/
X_fun(ga_info, parent_1, parent_2, child_1, child_2)
   GA_Info_Ptr ga_info;
   Chrom_Ptr   parent_1, parent_2;
   Chrom_Ptr   child_1, child_2;
{
   /*--- Init children ---*/
   X_init_kids(parent_1, parent_2, child_1, child_2);

   /*--- Clone instead of crossover ---*/
   if(ga_info->x_rate < 1.0 && RAND_FRAC() > ga_info->x_rate) {
      CH_copy(parent_1, child_1);
      CH_copy(parent_2, child_2);
      child_1->parent_1 = parent_1->index;
      child_1->parent_2 = parent_2->index;
      child_2->parent_1 = parent_1->index;
      child_2->parent_2 = parent_2->index;
      return OK;
   }

   /*--- No crossover function ---*/
   if(ga_info->X_fun == NULL) 
      return ERROR;

   /*--- Crossover ---*/
   ga_info->X_fun(ga_info, parent_1, parent_2, child_1, child_2);
}

/*============================================================================
|                           Crossover operators
============================================================================*/
/*----------------------------------------------------------------------------
| Simple crossover
|
| A single crossover point is selected at random.  Alleles up to and including
| the crossover point are copied to the respective child.  The remaining
| alleles are copied to the alternate child.
----------------------------------------------------------------------------*/
X_simple(ga_info, parent_1, parent_2, child_1, child_2)
   GA_Info_Ptr ga_info;
   Chrom_Ptr  parent_1, parent_2;
   Chrom_Ptr  child_1, child_2;
{
   unsigned i, xp;
   Gene_Type tmp;

   /*--- Make sure datatype is compatible ---*/
   if((ga_info->datatype != DT_INT_PERM) && (ga_info->datatype != DT_INT))
      UT_error("X_simple: bad data type");

   /*--- Cannot yet deal with heterozygous parents ---*/
   if(parent_1->length != parent_2->length)
      UT_error("crossover: heterozygous parents");

   /*--- Random crossover point ---*/
   X_gen_xp(0, parent_1->length-1, &xp);
   child_1->xp1 = xp;
   child_2->xp1 = xp;

   /*--- Half is same as parent ---*/
   for(i = 0; i <= xp; i++) {
      child_1->gene[i] = parent_1->gene[i];
      child_2->gene[i] = parent_2->gene[i];
   }

   /*--- Other half is swapped ---*/
   for(i = xp+1; i < parent_1->length; i++) {
      child_1->gene[i] = parent_2->gene[i];
      child_2->gene[i] = parent_1->gene[i];
   }

   return OK;
}

/*----------------------------------------------------------------------------
| Uniform crossover
|
| Each allele is copied from a parent based on a random flip of a fair coin
----------------------------------------------------------------------------*/
X_uniform(ga_info, parent_1, parent_2, child_1, child_2)
   GA_Info_Ptr ga_info;
   Chrom_Ptr  parent_1, parent_2;
   Chrom_Ptr  child_1, child_2;
{
   unsigned i;

   /*--- Make sure datatype is compatible ---*/
   if((ga_info->datatype != DT_INT_PERM) && (ga_info->datatype != DT_INT))
      UT_error("X_uniform: bad data type");

   /*--- Cannot yet deal with heterozygous parents ---*/
   if(parent_1->length != parent_2->length)
      UT_error("crossover: heterozygous parents");

   for(i = 0; i < parent_1->length; i++) {
      if(RAND_BIT()) {
         child_1->gene[i] = parent_1->gene[i];
         child_2->gene[i] = parent_2->gene[i];
      } else {
         child_1->gene[i] = parent_2->gene[i];
         child_2->gene[i] = parent_1->gene[i];
      }
   }

   return OK;
}

/*----------------------------------------------------------------------------
| Order1 (Starkweather, et. al., 1991 GA Conf.)
|
| "The offspring inherits the elements between the two crossover points,
| inclusive, from the selected parent in the same order and position as they
| appeared in that parent.  The remaining elements are inherited from the
| alternate parent in the order in which they appear in that parent, beginning
| with the first position following the second crossover point and skipping
| over all elements already present in the offspring."
|
| L. Davis. "Applying Adaptive Algorithms to Epistatic Domains". In Proc.
|    International Joint Conference on Artificial Intelligence (1985).
----------------------------------------------------------------------------*/
X_order1(ga_info, parent_1, parent_2, child_1, child_2)
   GA_Info_Ptr ga_info;
   Chrom_Ptr  parent_1, parent_2;
   Chrom_Ptr  child_1, child_2;
{
   int xp1, xp2;
   int i, p1, p2, c;                         

   /*--- Make sure datatype is compatible ---*/
   if(ga_info->datatype != DT_INT_PERM)
      UT_error("X_order1: bad data type");

   /*--- Cannot yet deal with heterozygous parents ---*/
   if(parent_1->length != parent_2->length)
      UT_error("crossover: heterozygous parents");

   /*--- Select two sorted crossover points ---*/
   X_gen_2_xp(FALSE, 0, parent_1->length, &xp1, &xp2);
   child_1->xp1 = child_2->xp1 = xp1;
   child_1->xp2 = child_2->xp2 = xp2;

   /*--- Info between xp is same as parent ---*/
   for(i = xp1; i <= xp2; i++) {
      child_1->gene[i] = parent_1->gene[i];
      child_2->gene[i] = parent_2->gene[i];
   }

   /*--- Inherit remainder from other parent ---*/
   for (i=0, p1=p2=xp2; i < (parent_1->length - (xp2 - xp1 + 1)); i++) {

      /*--- Index to fill in children ---*/
      c = (xp2 + 1 + i) % parent_1->length;

      /*--- Child 1 gets next unused element in parent 2 ---*/
      while(TRUE) {
         p2 = (p2 + 1) % parent_1->length;
         if(X_map(&parent_2->gene[p2], parent_1, xp1, xp2) < 0) break;
      }

      /*--- Child 2 gets next unused element in parent 1 ---*/
      while(TRUE) {
         p1 = (p1 + 1) % parent_2->length;
         if(X_map(&parent_1->gene[p1], parent_2, xp1, xp2) < 0) break;
      }

      /*--- Transfer to children ---*/
      child_1->gene[c] = parent_2->gene[p2];
      child_2->gene[c] = parent_1->gene[p1];
   }

   return OK; 
}

/*----------------------------------------------------------------------------
| Order2 (Starkweather, et. al., 1991 GA Conf.)
|
| "...several key positions are chosen randomly and the order in which these
| elements appear in one parent is imposed on the other parent to produce
| two offspring..."
|
| G. Syswerda.  "Schedule Optimization Using Genetic Algorithms".  In Handbook
|    of Genetic Algorithms.  L. Davis ed.  Van Nostrand Reinhold: NY (1990).
----------------------------------------------------------------------------*/
X_order2(ga_info, parent_1, parent_2, child_1, child_2)
   GA_Info_Ptr ga_info;
   Chrom_Ptr  parent_1, parent_2;
   Chrom_Ptr  child_1, child_2;
{
   int xp1, xp2, xp3, xp4;
   int i, j1, j2;                         
   int xidx_1[4], xidx_2[4];

   /*--- Make sure datatype is compatible ---*/
   if(ga_info->datatype != DT_INT_PERM)
      UT_error("X_order2: bad data type");

   /*--- Cannot yet deal with heterozygous parents ---*/
   if(parent_1->length != parent_2->length)
      UT_error("crossover: heterozygous parents");

   /*--- Select four sorted crossover points ---*/
   X_gen_4_xp(TRUE, 0, parent_1->length, &xp1, &xp2, &xp3, &xp4);
   child_1->xp1 = xp1; child_1->xp2 = xp2;
   child_2->xp1 = xp3; child_2->xp2 = xp4;

   /*--- Children look like parents ---*/
   for (i=0; i < parent_1->length; i++) {
      child_1->gene[i] = parent_1->gene[i];
      child_2->gene[i] = parent_2->gene[i];
   }

   /*--- Map order of xp's in other parent ---*/
   for (i=0, j1=j2=0; i < parent_1->length; i++) {

      /*--- Child_1 uses order in parent_2 ---*/
      if( (int)parent_2->gene[i] == (int)parent_1->gene[xp1] || 
          (int)parent_2->gene[i] == (int)parent_1->gene[xp2] ||
          (int)parent_2->gene[i] == (int)parent_1->gene[xp3] ||
          (int)parent_2->gene[i] == (int)parent_1->gene[xp4]     ) {
         xidx_1[j1++] = i;
      }

      /*--- Child_2 uses order in parent_1 ---*/
      if( (int)parent_1->gene[i] == (int)parent_2->gene[xp1] || 
          (int)parent_1->gene[i] == (int)parent_2->gene[xp2] ||
          (int)parent_1->gene[i] == (int)parent_2->gene[xp3] ||
          (int)parent_1->gene[i] == (int)parent_2->gene[xp4]     ) {
         xidx_2[j2++] = i;
      }
   }

   /*--- Impose ordering of xp's from other parent ---*/
   child_1->gene[xp1] = parent_2->gene[xidx_1[0]];
   child_1->gene[xp2] = parent_2->gene[xidx_1[1]];
   child_1->gene[xp3] = parent_2->gene[xidx_1[2]];
   child_1->gene[xp4] = parent_2->gene[xidx_1[3]];
   child_2->gene[xp1] = parent_1->gene[xidx_2[0]];
   child_2->gene[xp2] = parent_1->gene[xidx_2[1]];
   child_2->gene[xp3] = parent_1->gene[xidx_2[2]];
   child_2->gene[xp4] = parent_1->gene[xidx_2[3]];

   return OK; 
}

/*----------------------------------------------------------------------------
| Position (Starkweather, et. al., 1991 GA Conf.)
|
| "Several random locations in the sequence are selected along with one 
| parent; the elements in those positions are inherited from that parent.
| The remaining elements are inherited in the order in which they appear in
| the alternate parent, skipping over all elements which have already been
| included in the offspring."
|
| G. Syswerda.  "Schedule Optimization Using Genetic Algorithms".  In Handbook
|    of Genetic Algorithms.  L. Davis ed.  Van Nostrand Reinhold: NY (1990).
----------------------------------------------------------------------------*/
X_pos(ga_info, parent_1, parent_2, child_1, child_2)
   GA_Info_Ptr ga_info;
   Chrom_Ptr  parent_1, parent_2;
   Chrom_Ptr  child_1, child_2;
{
   int xp1, xp2, xp3, xp4;
   int i, j1, j2;                         

   /*--- Make sure datatype is compatible ---*/
   if(ga_info->datatype != DT_INT_PERM)
      UT_error("X_pos: bad data type");

   /*--- Cannot yet deal with heterozygous parents ---*/
   if(parent_1->length != parent_2->length)
      UT_error("crossover: heterozygous parents");

   /*--- Select four sorted crossover points ---*/
   X_gen_4_xp(FALSE, 0, parent_1->length, &xp1, &xp2, &xp3, &xp4);
   child_1->xp1 = xp1; child_1->xp2 = xp2;
   child_2->xp1 = xp3; child_2->xp2 = xp4;

   /*--- Children get parent's xp values ---*/
   child_1->gene[xp1] = parent_1->gene[xp1];
   child_1->gene[xp2] = parent_1->gene[xp2];
   child_1->gene[xp3] = parent_1->gene[xp3];
   child_1->gene[xp4] = parent_1->gene[xp4];
   child_2->gene[xp1] = parent_2->gene[xp1];
   child_2->gene[xp2] = parent_2->gene[xp2];
   child_2->gene[xp3] = parent_2->gene[xp3];
   child_2->gene[xp4] = parent_2->gene[xp4];

   /*--- Inherit rest using order from other parent ---*/
   for (i=0, j1=j2=0; i < parent_1->length; i++) {

      /*--- Transfer if not a crossover point (child_1) ---*/
      if( (int)parent_2->gene[i] != (int)parent_1->gene[xp1] && 
          (int)parent_2->gene[i] != (int)parent_1->gene[xp2] &&
          (int)parent_2->gene[i] != (int)parent_1->gene[xp3] && 
          (int)parent_2->gene[i] != (int)parent_1->gene[xp4]   ) {

         /*--- Make sure j1 is not a crossover point ---*/
         while(j1 == xp1 || j1 == xp2 || j1 == xp3 || j1 == xp4) j1++;

         child_1->gene[j1++] = parent_2->gene[i];
      }

      /*--- Transfer if not a crossover point (child_2) ---*/
      if( (int)parent_1->gene[i] != (int)parent_2->gene[xp1] && 
          (int)parent_1->gene[i] != (int)parent_2->gene[xp2] &&
          (int)parent_1->gene[i] != (int)parent_2->gene[xp3] && 
          (int)parent_1->gene[i] != (int)parent_2->gene[xp4]   ) {

         /*--- Make sure j2 is not a crossover point ---*/
         while(j2 == xp1 || j2 == xp2 || j2 == xp3 || j2 == xp4) j2++;

         child_2->gene[j2++] = parent_1->gene[i];
      }
   }

   return OK; 
}

/*----------------------------------------------------------------------------
| Cycle (Starkweather, et. al., 1991 GA Conf.)
|
| "A parent sequence and a cycle starting point are randomly selected.  The 
| element at the cycle starting point of the selected parent is inherited by
| the child.  The element which is in the same position in the other parent
| cannot then be placed in this position so its position is found in the
| selected parent and is inherited from that position by the child.  This
| continues until the cycle is completed by encountering the initial item in
| the unselected parent.  Any elements which are not yet present in the
| offspring are inherited from the unselected parent."
|
| I. Oliver, D. Smith, and J. Holland.  "A Study of Permutation Crossover
|   Operators on the Traveling Salesman Problem."  In Proc. Second Interna-
|   tional Conference on Genetic Algorithms and their Applications (1987).
----------------------------------------------------------------------------*/
X_cycle(ga_info, parent_1, parent_2, child_1, child_2)
   GA_Info_Ptr ga_info;
   Chrom_Ptr  parent_1, parent_2;
   Chrom_Ptr  child_1, child_2;
{
   int xp, i;

   /*--- Make sure datatype is compatible ---*/
   if(ga_info->datatype != DT_INT_PERM)
      UT_error("X_cycle: bad data type");

   /*--- Cannot yet deal with heterozygous parents ---*/
   if(parent_1->length != parent_2->length)
      UT_error("crossover: heterozygous parents");

   /*--- Select crossover point ---*/
   X_gen_xp(0, parent_1->length, &xp);
   child_1->xp1 = xp;
   child_2->xp1 = xp;

   /*--- Transfer material to children ---*/
   for(i = 0; i < parent_1->length; i++) {
      child_1->gene[i] = parent_2->gene[i];
      child_2->gene[i] = parent_1->gene[i];
   }

   /*--- Crossover (child 1) ---*/
   for (i=xp; ; ) {
      child_1->gene[i] = parent_1->gene[i];
      i = X_map(&parent_2->gene[i], parent_1, 0, parent_1->length - 1);
      if(i == xp) break;
   }

   /*--- Crossover (child 2) ---*/
   for (i=xp; ; ) {
      child_2->gene[i] = parent_2->gene[i];
      i = X_map(&parent_1->gene[i], parent_2, 0, parent_2->length - 1);
      if(i == xp) break;
   }

   return OK; 
}

/*----------------------------------------------------------------------------
| PMX (Starkweather, et. al., 1991 GA Conf.)
|
| "A parent and two crossover sites are selected randomly and the elements
| between the two starting positions in one of the parents are directly 
| inherited by the offspring.  Each element between the two crossover points
| in the alternate parent are mapped to the position held by this element in
| the first parent.  Then the remaining elements are inherited from the
| alternate parent.
|
| D. Goldberg and R. Lingle.  "Alleles, loci, and the Traveling Salesman
| Problem".  In Proc. International Conference on Genetic Algorithms and their
| Applications (1985).
----------------------------------------------------------------------------*/
X_pmx(ga_info, parent_1, parent_2, child_1, child_2)
   GA_Info_Ptr ga_info;
   Chrom_Ptr  parent_1, parent_2;
   Chrom_Ptr  child_1, child_2;
{
   int xp1, xp2;
   int i, j, k;

   /*--- Make sure datatype is compatible ---*/
   if(ga_info->datatype != DT_INT_PERM)
      UT_error("X_pmx: bad data type");

   /*--- Cannot yet deal with heterozygous parents ---*/
   if(parent_1->length != parent_2->length)
      UT_error("crossover: heterozygous parents");

   /*--- Select two sorted crossover points ---*/
   X_gen_2_xp(FALSE, 0, parent_1->length, &xp1, &xp2);
   child_1->xp1 = child_2->xp1 = xp1; 
   child_1->xp2 = child_2->xp2 = xp2;

   /*--- Copy info to children ---*/
   for(i = 0; i < parent_1->length; i++) {
      if(i < xp1 || i > xp2) {
         child_1->gene[i] = parent_1->gene[i];
         child_2->gene[i] = parent_2->gene[i];
      } else {
         child_1->gene[i] = parent_2->gene[i];
         child_2->gene[i] = parent_1->gene[i];
      }
   }

   /*--- Fixup mapped elements ---*/
   for(i = 0; i < parent_1->length; i++) {

      /*--- Skip if between xp's ---*/
      if(i >= xp1 && i <= xp2) continue;

      /*--- A mapped element (child_1) ---*/
      if((j = X_map(&child_1->gene[i], child_1, xp1, xp2)) >= 0) {
         while(TRUE) {
            child_1->gene[i] = parent_1->gene[j];
            if((j = X_map(&child_1->gene[i], child_1, xp1, xp2)) < 0) 
               break;
         }
      }

      /*--- A mapped element (child_2) ---*/
      if((j = X_map(&child_2->gene[i], child_2, xp1, xp2)) >= 0) {
         while(TRUE) {
            child_2->gene[i] = parent_2->gene[j];
            if((j = X_map(&child_2->gene[i], child_2, xp1, xp2)) < 0) 
               break;
         }
      }
   }

   return OK; 
}

/*----------------------------------------------------------------------------
| Uniform order crossover
|
| Analogous to uniform crossover, but used for order-based representations.
----------------------------------------------------------------------------*/
X_uox(ga_info, parent_1, parent_2, child_1, child_2)
   GA_Info_Ptr ga_info;
   Chrom_Ptr  parent_1, parent_2;
   Chrom_Ptr  child_1, child_2;
{
   unsigned i, j1, j2;
   int length;
   static int  m1_length, m2_length;
   static char *m1 = NULL, *m2 = NULL;

   /*--- Make sure datatype is compatible ---*/
   if(ga_info->datatype != DT_INT_PERM)
      UT_error("X_uox: bad data type");

   /*--- Cannot yet deal with heterozygous parents ---*/
   if(parent_1->length != parent_2->length)
      UT_error("crossover: heterozygous parents");

   /*--- Make room for m1 ---*/
   if(m1 == NULL) {
      m1 = (char *)calloc(parent_1->length, sizeof(char));
      if(m1 == NULL) UT_error("X_uox: m1 alloc failed");
      m1_length = parent_1->length;
   } else {
      if(m1_length != parent_1->length) {
         m1 = (char *)realloc(m1, parent_1->length * sizeof(char));
         if(m1 == NULL) UT_error("X_uox: m1 realloc failed");
         m1_length = parent_1->length;
      }
   }

   /*--- Make room for m2 ---*/
   if(m2 == NULL) {
      m2 = (char *)calloc(parent_2->length, sizeof(char));
      if(m2 == NULL) UT_error("X_uox: m2 alloc failed");
      m2_length = parent_2->length;
   } else {
      if(m2_length != parent_2->length) {
         m2 = (char *)realloc(m2, parent_2->length * sizeof(char));
         if(m2 == NULL) UT_error("X_uox: m2 realloc failed");
         m2_length = parent_2->length;
      }
   }

   /*--- Random mask ---*/
   for(i = 0; i < parent_1->length; i++) {
      m1[i] = m2[i] = (RAND_BIT() ? 1 : 0);
   }

   /*--- Place alleles from mask ---*/
   for(i = 0; i < parent_1->length; i++) {
      if(m1[i]) child_1->gene[i] = parent_1->gene[i];
      else      child_1->gene[i] = -1;
   }
   for(i = 0; i < parent_2->length; i++) {
      if(m2[i]) child_2->gene[i] = parent_2->gene[i];
      else      child_2->gene[i] = -1;
   }

   /*--- Place remaining alleles ---*/
   j1 = 0;
   for(i = 0; i < parent_1->length; i++) {
      if((int)child_1->gene[i] == -1) {
         while(X_map(&parent_2->gene[j1], child_1, 0, child_1->length-1) != -1)
            if(j1 < parent_2->length)
               j1++;
            else
               UT_error("X_uox: invalid j1");
         child_1->gene[i] = parent_2->gene[j1];
      }
   }
   j2 = 0;
   for(i = 0; i < parent_2->length; i++) {
      if((int)child_2->gene[i] == -1) {
         while(X_map(&parent_1->gene[j2], child_2, 0, child_2->length-1) != -1)
            if(j2 < parent_1->length)
               j2++;
            else
               UT_error("X_uox: invalid j2");
         child_2->gene[i] = parent_1->gene[j2];
      }
   }
}

/*----------------------------------------------------------------------------
| Relative order crossover
|
| Not yet implemented
----------------------------------------------------------------------------*/
X_rox(ga_info, parent_1, parent_2, child_1, child_2)
   GA_Info_Ptr ga_info;
   Chrom_Ptr  parent_1, parent_2;
   Chrom_Ptr  child_1, child_2;
{
   UT_error("X_rox: not yet implemented");

   /*--- Error check ---*/
   if(!CF_valid(ga_info)) UT_error("X_rox: invalid ga_info");
   if(!CH_valid(parent_1)) UT_error("X_rox: invalid parent_1");
   if(!CH_valid(parent_2)) UT_error("X_rox: invalid parent_2");
   if(!CH_valid(child_1)) UT_error("X_rox: invalid child_1");
   if(!CH_valid(child_2)) UT_error("X_rox: invalid child_2");

   /*--- Make sure datatype is compatible ---*/
   if(ga_info->datatype != DT_INT_PERM)
      UT_error("X_rox: bad data type");

   /*--- Cannot yet deal with heterozygous parents ---*/
   if(parent_1->length != parent_2->length)
      UT_error("crossover: heterozygous parents");
}

/*----------------------------------------------------------------------------
| Asexual crossover
|
| Two crossover points are selected at random and the elements at those points
| are swapped resulting in a single child.  To remain compatible with the
| other crossover methods which generate two children from two parents, two
| independent crossovers are performed to generate the two children.
|
| This method is the same as a "two-opt" in simulated annealing.
----------------------------------------------------------------------------*/
X_asex(ga_info, parent_1, parent_2, child_1, child_2)
   GA_Info_Ptr ga_info;
   Chrom_Ptr  parent_1, parent_2;
   Chrom_Ptr  child_1, child_2;
{
   /*--- Make sure datatype is compatible ---*/
   if((ga_info->datatype != DT_INT_PERM) && (ga_info->datatype != DT_INT))
      UT_error("X_asex: bad data type");

   /*--- Perform asexual crossover ---*/
   X_do_asex(parent_1, child_1);
   X_do_asex(parent_2, child_2);

   return OK;
}

/*----------------------------------------------------------------------------
| Asexual crossover
|
| Perform asexual crossover for X_asex()
----------------------------------------------------------------------------*/
X_do_asex(parent, child)
   Chrom_Ptr  parent;
   Chrom_Ptr  child;
{
   int i, xp1, xp2;

   /*--- Copy info to child ---*/
   for(i = 0; i < parent->length; i++) {
      child->gene[i] = parent->gene[i];
   }
   child->idx_min = parent->idx_min;

   /*--- Prevent infinite loop ---*/
   if(parent->idx_min >= parent->length-1) return OK;

   /*--- Select two sorted crossover points ---*/
   X_gen_2_xp(TRUE, parent->idx_min, parent->length, &xp1, &xp2);
   child->xp1 = xp1; 
   child->xp2 = xp2;

   /*--- Crossover just swaps xp's ---*/
   child->gene[xp1] = parent->gene[xp2];
   child->gene[xp2] = parent->gene[xp1];

   return OK;
}

/*============================================================================
|                             Utility functions
============================================================================*/
/*----------------------------------------------------------------------------
| Generate crossover point in [idx_min..idx_max-1]
----------------------------------------------------------------------------*/
X_gen_xp(idx_min, idx_max, xp)
   int idx_min, idx_max, *xp;
{
   *xp = RAND_DOM(idx_min, idx_max - 1);
}

/*----------------------------------------------------------------------------
| Generate two sorted crossover points
----------------------------------------------------------------------------*/
X_gen_2_xp(unique, idx_min, idx_max, xp1, xp2)
   int unique, idx_min, idx_max, *xp1, *xp2;
{
   /*--- Generate two points ---*/
   X_gen_xp(idx_min, idx_max, xp1);
   X_gen_xp(idx_min, idx_max, xp2);

   /*--- Make sure unique if specified ---*/
   if(unique) {
      while(*xp2 == *xp1) 
         X_gen_xp(idx_min, idx_max, xp2);
   }

   /*--- Make sure they are sorted ---*/
   if(*xp1 > *xp2) 
      UT_iswap(xp1, xp2);
}

/*----------------------------------------------------------------------------
| Generate four sorted crossover points
----------------------------------------------------------------------------*/
X_gen_4_xp(unique, idx_min, idx_max, xp1, xp2, xp3, xp4)
   int unique, idx_min, idx_max, *xp1, *xp2, *xp3, *xp4;
{
   /*--- Generate four points ---*/
   X_gen_xp(idx_min, idx_max, xp1);
   X_gen_xp(idx_min, idx_max, xp2);
   X_gen_xp(idx_min, idx_max, xp3);
   X_gen_xp(idx_min, idx_max, xp4);

   /*--- Make sure unique if specified ---*/
   if(unique) {
      while(*xp2 == *xp1) 
         X_gen_xp(idx_min, idx_max, xp2);

      while(*xp3 == *xp1 || *xp3 == *xp2) 
         X_gen_xp(idx_min, idx_max, xp3);

      while(*xp4 == *xp1 || *xp4 == *xp2 || *xp4 == *xp3) 
         X_gen_xp(idx_min, idx_max, xp4);
   }

   /*--- Make sure they are sorted (use "sorting network") ---*/
   if (*xp1 > *xp2) UT_iswap(xp1, xp2);
   if (*xp3 > *xp4) UT_iswap(xp3, xp4);
   if (*xp2 > *xp3) UT_iswap(xp2, xp3);
   if (*xp1 > *xp2) UT_iswap(xp1, xp2);
   if (*xp3 > *xp4) UT_iswap(xp3, xp4);
}

/*----------------------------------------------------------------------------
| Initialize child chromosomes for crossover
----------------------------------------------------------------------------*/
X_init_kids(parent_1, parent_2, child_1, child_2)
   Chrom_Ptr  parent_1, parent_2;
   Chrom_Ptr  child_1, child_2;
{
   /*--- Assume for now that parents are homozygous ---*/
   if(parent_1->length <= 0) UT_error("crossover: parent_1->length");
   if(parent_2->length <= 0) UT_error("crossover: parent_2->length");
   if(child_1 == NULL) UT_error("X_init_kids: null child_1");
   if(child_2 == NULL) UT_error("X_init_kids: null child_2");

   /*--- Initialize the children ---*/
   CH_reset(child_1);
   CH_reset(child_2);
   child_1->parent_1 = parent_1->index;
   child_1->parent_2 = parent_2->index;
   child_2->parent_1 = parent_1->index;
   child_2->parent_2 = parent_2->index;
}

/*----------------------------------------------------------------------------
| Allele map in Chrom->gene[lo..hi]
----------------------------------------------------------------------------*/
X_map(allele, chrom, lo, hi)
   Gene_Type  *allele;
   Chrom_Ptr  chrom;
   int lo, hi;
{
   int i;

   /*--- Error check ---*/
   if(lo < 0 || lo > hi || hi >= chrom->length) 
      UT_error("X_map: bad range");

   /*--- Find allele in range of genes ---*/
   for(i = lo; i <= hi; i++) 
      if((int)*allele == (int)chrom->gene[i]) 
         return i;

   /*--- Not found ---*/
   return -1;
}
