/*============================================================================
| (c) Copyright Arthur L. Corcoran, 1992, 1993.  All rights reserved.
|
| Pool management
|
| Functions:
|    PL_alloc()    - allocate a pool
|    PL_resize()   - resize a pool
|    PL_free()     - deallocate a pool
|    PL_valid()    - is a pool valid?
|    PL_reset()    - reset a pool
|    PL_eval()     - evaluate a pool
|    PL_get_num()  - get a number
|    PL_generate() - generate a pool
|    PL_read()     - read a pool from a file
|    PL_rand()     - generate a random pool
|    PL_stats()    - calculate pool statistics
|    PL_index()    - index a pool
|    PL_update_ptf() - update the percent of total fitness in a pool
|    PL_clean()    - empty a pool
|    PL_append()   - append a chrom to a pool
|    PL_insert()   - insert a chrom in a pool at a spec loc
|    PL_remove()   - remove a chrom from a pool
|    PL_move()     - move a chrom in a pool
|    PL_swap()     - swap two chroms in a pool
|    PL_sort()     - sort a pool
============================================================================*/
#include "ga.h"

/* Number of chromosome pointers to alloc at a time */
#define PL_ALLOC_SIZE 10 

/*----------------------------------------------------------------------------
| Allocate a pool
----------------------------------------------------------------------------*/
Pool_Ptr PL_alloc(max_size) 
   int max_size;
{
   Pool_Ptr pool;

   /*--- Error check ---*/
   if(max_size <= 0) UT_error("PL_alloc: invalid max_size");

   /*--- Allocate memory for pool ---*/
   pool = (Pool_Ptr)calloc(1, sizeof(Pool_Type));
   if(pool == NULL) UT_error("PL_alloc: pool alloc failed");
   pool->max_size = max_size;

   /*--- Allocate memory for chromosome pointers ---*/
   pool->chrom = (Chrom_Ptr *)calloc(max_size, sizeof(Chrom_Ptr));
   if(pool->chrom == NULL) UT_error("PL_alloc: chrom alloc failed");

   /*--- Put in magic cookie ---*/
   pool->magic_cookie = PL_cookie;

   /*--- Reset pool ---*/
   PL_reset(pool);

   return pool;
}

/*----------------------------------------------------------------------------
| Resize pool
|
| NOTE: chromosomes still in pool after resize are still valid
----------------------------------------------------------------------------*/
PL_resize(pool, new_size) 
   Pool_Ptr  pool;
   int       new_size;
{
   int old_size;

   /*--- Error check ---*/
   if(!PL_valid(pool)) UT_error("PL_resize: invalid pool");
   if(new_size <= 0) UT_error("PL_resize: invalid new_size");

   /*--- Free any excess chromosomes ---*/
   if(new_size < pool->max_size) PL_clean(pool, new_size, pool->max_size);

   /*--- Reallocate memory for chromosome pointers ---*/
   pool->chrom = (Chrom_Ptr *)realloc(pool->chrom, new_size*sizeof(Chrom_Ptr));
   if(pool->chrom == NULL) UT_error("PL_resize: chrom realloc failed");

   /*--- Update pool size ---*/
   old_size       = pool->max_size;
   pool->max_size = new_size;

   /*--- Make any new chromosomes NULL ---*/
   if(new_size > old_size) PL_clean(pool, old_size, new_size);
}

/*----------------------------------------------------------------------------
| De-Allocate pool
----------------------------------------------------------------------------*/
PL_free(pool) 
   Pool_Ptr pool;
{
   /*--- Error check ---*/
   if(!PL_valid(pool)) return;

   /*--- Release memory for chromosome pointers ---*/
   if(pool->chrom != NULL) {

      /*--- Release memory for chromosomes ---*/
      PL_clean(pool, 0, pool->max_size);

      /*--- Release memory for chromosome pointers ---*/
      free(pool->chrom);
      pool->chrom = NULL;
   }

   /*--- Put in a NULL magic cookie ---*/
   pool->magic_cookie = NL_cookie;

   /*--- Release memory for pool ---*/
   free(pool);
}

/*----------------------------------------------------------------------------
| Is pool valid, i.e., has it been allocated by PL_alloc()?
----------------------------------------------------------------------------*/
PL_valid(pool) 
   Pool_Ptr pool;
{
   /*--- Check for NULL pointers ---*/
   if(pool == NULL) return FALSE;
   if(pool->chrom == NULL) return FALSE;

   /*--- Check for magic cookie ---*/
   if(pool->magic_cookie != PL_cookie) return FALSE;

   /*--- Otherwise valid ---*/
   return TRUE;
}

/*----------------------------------------------------------------------------
| Reset pool
----------------------------------------------------------------------------*/
PL_reset(pool) 
   Pool_Ptr pool;
{
   int i;

   /*--- Error check ---*/
   if(!PL_valid(pool)) UT_error("PL_reset: invalid pool");

   /*--- Reset chromosomes ---*/
   for(i=0; i<pool->max_size; i++) {
      if(CH_valid(pool->chrom[i]))
         CH_reset(pool->chrom[i]);
      else
         pool->chrom[i] = NULL;
   }

   /*--- Reset pool ---*/
   pool->size = 0;
   pool->total_fitness = 0.0;
   pool->min = 0.0;
   pool->max = 0.0;
   pool->ave = 0.0;
   pool->var = 0.0;
   pool->dev = 0.0;
   pool->min_index = -1;
   pool->max_index = -1;
   pool->minimize = TRUE;
   pool->sorted   = FALSE;
}

/*----------------------------------------------------------------------------
| Evaluate pool
----------------------------------------------------------------------------*/
PL_eval(ga_info, pool) 
   GA_Info_Ptr ga_info;
   Pool_Ptr pool;
{
   int i;

   /*--- Error check ---*/
   if(!CF_valid(ga_info)) UT_error("PL_eval: invalid ga_info");
   if(!PL_valid(pool)) UT_error("PL_eval: invalid pool");

   /*--- Evaluate each chromosome ---*/
   for(i = 0; i < pool->size; i++) {
      ga_info->EV_fun(pool->chrom[i]);
   }
}

/*============================================================================
|                            Generate pool
============================================================================*/
/*----------------------------------------------------------------------------
| Read a number for PL_generate()
----------------------------------------------------------------------------*/
char *PL_get_num(fid)
   FILE   *fid;
{
   static char str[80]; /* This MUST be static */
   int  ch, len = 0;

   /*--- Search for a digit ---*/
   while(TRUE) {

      /*--- Get a character ---*/
      ch = fgetc(fid);

      /*--- Error or Quit ---*/
      if(ch == EOF || ch == 'q' || ch == 'Q') return NULL;

      /*--- Digit ---*/
      if(isdigit(ch)) break;

      /*--- Comment: skip to end of line ---*/
      if(ch == '#') 
         while((ch = fgetc(fid)) != '\n') 
            ;
   }

   /*--- Make sure digit found above ---*/
   if(!isdigit(ch)) UT_error("PL_get_num: bad digit");

   /*--- Digit found, now put into str ---*/
   while(len < 80 && ch != EOF && !isspace(ch) && ch != '#') {
      str[len++] = ch;
      ch = fgetc(fid);
   }
   str[len] = 0;

   /*--- Return number in string ---*/
   return str;
}

/*----------------------------------------------------------------------------
| Generate pool according to ga_info->ip_flag:
|
|   IP_INTERACTIVE: interactively read pool 
|   IP_FROM_FILE:   read pool from file given in file_name argument  
|   IP_RANDOM:      random pool limited by pool_size & chrom_len
|   IP_NONE:        do nothing
----------------------------------------------------------------------------*/
PL_generate(ga_info, pool) 
   GA_Info_Ptr ga_info;
   Pool_Ptr pool;
{
   FILE *fid;
   long chrom_len;
   char *sptr;

   /*--- Error check ---*/
   if(!CF_valid(ga_info)) UT_error("PL_generate: invalid ga_info");
   if(!PL_valid(pool)) UT_error("PL_generate: invalid pool");

   /*--- Get initial pool data ---*/
   switch(ga_info->ip_flag) {

      case IP_INTERACTIVE:

         /*--- Get chrom_len ---*/
         puts("\nEnter chromosome length:");
         if((sptr = PL_get_num(stdin)) == NULL) {
            UT_error("PL_generate: No chrom_len was read");
         }
         if(sscanf(sptr,"%ld",&chrom_len) != 1) 
            UT_error("PL_generate: error reading chrom_len");
         if(chrom_len <= 0) 
            UT_error("PL_generate: invalid chrom_len");
         ga_info->chrom_len = chrom_len;

         /*--- Get the pool ---*/
         puts("\nEnter initial pool (`q' to quit):");
         PL_read(pool, chrom_len, stdin);

         break;

      case IP_FROM_FILE: 

         /*--- Open the file ---*/
         if((fid = fopen(ga_info->ip_data, "r")) == NULL)
            UT_error("PL_generate: Invalid data file");

         /*--- Get chrom_len ---*/
         if((sptr = PL_get_num(fid)) == NULL) {
            UT_error("PL_generate: No chrom_len was read");
         }
         if(sscanf(sptr,"%ld",&chrom_len) != 1) 
            UT_error("PL_generate: error reading chrom_len");
         if(chrom_len <= 0) 
            UT_error("PL_generate: invalid chrom_len");
         ga_info->chrom_len = chrom_len;

         /*--- Get the pool ---*/
         PL_read(pool, chrom_len, fid);

         /*--- Close the file ---*/
         fclose(fid);
         break;

      case IP_RANDOM:
         PL_rand(pool, ga_info->pool_size, ga_info->chrom_len, 
                    ga_info->datatype, ga_info->max_gene_val);
         break;

      case IP_NONE:
         break;

      default:
         UT_error("PL_generate: invalid ip_flag");
   }

   /*--- Evaluate the pool ---*/
   PL_eval(ga_info, pool);

   /*--- Compute pool statistics ---*/
   PL_stats(ga_info, pool);
}

/*----------------------------------------------------------------------------
| Initialize pool from file.
----------------------------------------------------------------------------*/
PL_read(pool, chrom_len, fid) 
   Pool_Ptr pool;
   long     chrom_len;
   FILE     *fid;
{
   Chrom_Ptr  chrom;
   double     gene;
   long       i;
   char       *sptr;

   /*--- Error check ---*/
   if(!PL_valid(pool)) UT_error("PL_read: invalid pool");
   if(chrom_len <= 0) UT_error("PL_read: invalid chrom_len");
   if(fid == NULL) UT_error("PL_read: invalid fid");

   /*--- While there is a chromosome to read ---*/
   while(TRUE) {

      /*--- Allocate or reuse a chromosome ---*/
      if(CH_valid(pool->chrom[pool->size])) {
         chrom = pool->chrom[pool->size];
         CH_resize(chrom, chrom_len);
         pool->chrom[pool->size] = NULL;
      } else {
         chrom = CH_alloc(chrom_len);
      }

      /*--- Read genes ---*/
      for(i=0; i < chrom_len; i++) {

         /*--- Read a gene ---*/
         sptr = PL_get_num(fid);

         /*--- Get number from sptr if valid ---*/
         if(sptr == NULL || sscanf(sptr,"%lf",&gene) != 1) {

            /*--- Free chromosome ---*/
            CH_free(chrom);

            /*--- EOF in middle of chromosome ---*/
            if(i != 0 && (sptr == NULL || *sptr != 'q')) {
               UT_warn("PL_read: premature eof reading chromosome");
            }

            /*--- EOF: no more to read ---*/
            return;
         }

         /*--- Valid gene was read ---*/
         chrom->gene[i] = (Gene_Type)gene;
      }

      /*--- Put the chromosome into the pool ---*/
      PL_append(pool, chrom, FALSE);
   }
}

/*----------------------------------------------------------------------------
| Initialize random pool.
|
| NOTE: Alleles for datatypes DT_INT and DT_REAL are generated from an
|       arbitrarily chosen domain.  There really needs to be a way for
|       the user to indicate a domain for EACH gene.
----------------------------------------------------------------------------*/
PL_rand(pool, pool_size, chrom_len, datatype, max_gene_val) 
   Pool_Ptr pool;
   int      pool_size, chrom_len, datatype; float max_gene_val;
{
   int       i, j, idx;
   Chrom_Ptr chrom;

   /*--- Error check ---*/
   if(!PL_valid(pool)) UT_error("PL_rand: invalid pool");
   if(pool_size < 0) UT_error("PL_rand: negative pool_size");
   if(chrom_len < 0) UT_error("PL_rand: negative chrom_len");
   switch(datatype) {
      case DT_BIT:
      case DT_INT:
      case DT_INT_PERM:
      case DT_REAL:
         break;
      default: UT_error("PL_rand: Invalid datatype");
   }

   /*--- Make sure there is enough space for pool ---*/
   if(pool_size > pool->max_size) PL_resize(pool, pool_size);

   /*--- For each chromosome to be generated ---*/
   for(i = 0; i < pool_size; i++) {

      /*--- Allocate or reuse a chromosome ---*/
      if(CH_valid(pool->chrom[pool->size])) {
         chrom = pool->chrom[pool->size];
         CH_resize(chrom, chrom_len);
         pool->chrom[pool->size] = NULL;
      } else {
         chrom = CH_alloc(chrom_len);
      }

      /*--- Generate random genes ---*/
      switch(datatype) {

         case DT_BIT:
            /*--- Random bit ---*/
            for(j = 0; j < chrom_len; j++)
               chrom->gene[j] = (Gene_Type)RAND_BIT();
            chrom->length = chrom_len;
            break;

         case DT_INT:
            /*--- Random integers from an arbitrary domain ---*/
            for(j = 0; j < chrom_len; j++)
               chrom->gene[j] = (Gene_Type)RAND_DOM(0,max_gene_val);
            chrom->length = chrom_len;
            break;

         case DT_INT_PERM:
            /*--- Random permutations of integers ---*/
            for(j = 0; j < chrom_len; j++) 
               chrom->gene[j] = (Gene_Type)-1;
            for(j = 0; j < chrom_len; j++) {
               idx = RAND_DOM(0,chrom_len-1);
               while(chrom->gene[idx] != -1) 
                  idx = RAND_DOM(0,chrom_len-1);
               chrom->gene[idx] = (Gene_Type)(j + 1);
            }
            chrom->length = chrom_len;
            break;

         case DT_REAL:
            /*--- Random reals from an arbitrary domain ---*/
            for(j = 0; j < chrom_len; j++)
               chrom->gene[j] = 
                  (double)RAND_DOM(0,chrom_len-1) + (double)RAND_FRAC();
            chrom->length = chrom_len;
            break;

         /*--- Should never reach here ---*/
         default: UT_error("PL_rand: Invalid datatype");
      }

      /*--- Put the chromosome into the pool ---*/
      PL_append(pool, chrom, FALSE);
   }
}

/*============================================================================
|                            Pool statistics
============================================================================*/
/*----------------------------------------------------------------------------
| Compute statistics for a pool
----------------------------------------------------------------------------*/
PL_stats(ga_info, pool)
   GA_Info_Ptr ga_info;
   Pool_Ptr    pool;
{
   unsigned i, min_index, max_index;
   double   min, max, total, var;
   int      no_variance, sorted;

   /*--- Error check ---*/
   if(!CF_valid(ga_info)) UT_error("PL_stats: invalid ga_info");
   if(!PL_valid(pool)) UT_error("PL_stats: invalid pool");

   /*--- Trivial cases ---*/
   if(pool->size <= 0) {
      pool->min           = 0.0;
      pool->max           = 0.0;
      pool->ave           = 0.0;
      pool->var           = 0.0;
      pool->dev           = 0.0;
      pool->total_fitness = 0.0;
      pool->min_index     = -1;
      pool->max_index     = -1;
      pool->best_index    = -1;
      return OK;
   } else if(pool->size == 1) {
      pool->min           = pool->chrom[0]->fitness;
      pool->max           = pool->chrom[0]->fitness;
      pool->ave           = pool->chrom[0]->fitness;
      pool->var           = 0.0;
      pool->dev           = 0.0;
      pool->total_fitness = pool->chrom[0]->fitness;
      pool->min_index     = 0;
      pool->max_index     = 0;
      pool->best_index    = 0;
      return OK;
   }

   /*--- Initialize stats ---*/
   min         = pool->chrom[0]->fitness;
   max         = pool->chrom[0]->fitness;
   min_index   = 0;
   max_index   = 0;
   var         = 0.0;
   total       = 0.0;
   no_variance = TRUE;
   sorted      = TRUE;

   /*--- Collect statistics and find total fitness ---*/
   for(i = 0; i < pool->size; i++) {

      /*--- Error check ---*/
      if(!CH_valid(pool->chrom[i])) UT_error("PL_stats: invalid chrom");

      /*--- Comparative statistics ---*/
      if(i != 0) {

         /*--- Check for variance in fitness ---*/
         if(no_variance && 
            pool->chrom[i]->fitness != pool->chrom[i-1]->fitness)
         {
            no_variance = FALSE;
         }

         /*--- Check for not sorted ---*/
         if(sorted && pool->minimize) {
            if(pool->chrom[i]->fitness < pool->chrom[i-1]->fitness)
               sorted = FALSE;
         } else if(sorted) {
            if(pool->chrom[i]->fitness > pool->chrom[i-1]->fitness)
               sorted = FALSE;
         }
      }

      /*--- Less than min? ---*/
      if(pool->chrom[i]->fitness < min) {
         min = pool->chrom[i]->fitness;
         min_index = i;
      }

      /*--- Greater than max? ---*/
      if(pool->chrom[i]->fitness > max) {
         max = pool->chrom[i]->fitness;
         max_index = i;
      }

      /*--- Update cum. values ---*/
      total += pool->chrom[i]->fitness;
      var   += (float)pool->chrom[i]->fitness * 
               (float)pool->chrom[i]->fitness;

      /*--- Make sure index is set ---*/
      pool->chrom[i]->index = i;
   }

   /*--- Update pool statistics ---*/
   pool->min = min;
   pool->max = max;
   pool->ave = total / pool->size;
   pool->min_index = min_index;
   pool->max_index = max_index;
   if(pool->minimize) pool->best_index = min_index;
   else               pool->best_index = max_index;
   pool->total_fitness = total;

   /*--- Variance and standard deviation ---*/
   var = (var - (pool->ave * total)) / (pool->size - 1);
   if(no_variance || var <= 0.0) {
      pool->var = 0.0;
      pool->dev = 0.0;
      ga_info->converged = TRUE;
   } else {
      pool->var = var;
      pool->dev = sqrt(var);
      ga_info->converged = FALSE;
   }
}

/*----------------------------------------------------------------------------
| Update indices in the pool
----------------------------------------------------------------------------*/
PL_index(pool)
   Pool_Ptr pool;
{
   int i;

   /*--- Error check ---*/
   if(!PL_valid(pool)) UT_error("PL_index: invalid pool");

   /*--- Set index for each chromosome ---*/
   for(i=0; i<pool->size; i++)
      pool->chrom[i]->index = i;
}

/*----------------------------------------------------------------------------
| Update ptf (percent of total fitness) for each chromosome
----------------------------------------------------------------------------*/
PL_update_ptf(ga_info, pool) 
   GA_Info_Ptr ga_info;
   Pool_Ptr    pool;
{
   int i, sf_changed, all_positive;

   /*--- Error check ---*/
   if(!CF_valid(ga_info)) UT_error("PL_update_ptf: invalid ga_info");
   if(!PL_valid(pool)) UT_error("PL_update_ptf: invalid pool");

   /*--- Compute scale factor (to ensure positive fitness) ---*/
   sf_changed = FALSE;
   all_positive = TRUE;
   for(i = 0; i < pool->size; i++) {

      /*--- If scaled fitness not positive ---*/
      if((pool->chrom[i]->fitness + ga_info->scale_factor) <= 0) {

         /*--- Adjust scale_factor so scaled fitness is 1.0 ---*/
         ga_info->scale_factor += 
            1.0 - (pool->chrom[i]->fitness + ga_info->scale_factor);

         /*--- The scale_factor has been changed ---*/
         sf_changed = TRUE;
      } 

      /*--- Make sure scale factor is still needed ---*/
      if(pool->chrom[i]->fitness <= 0) all_positive = FALSE;
   }

   /*--- Scale factor no longer needed ---*/
   if(all_positive) {
      if(ga_info->scale_factor > 0.0) sf_changed = TRUE;
      ga_info->scale_factor = 0.0;
   }

   /*--- Report change in scale factor ---*/
   if(sf_changed && 
      ga_info->rp_type != RP_NONE && 
      ga_info->rp_type != RP_MINIMAL) 
   {
      fprintf(ga_info->rp_fid,
              "New scale factor = %G\n", 
               ga_info->scale_factor);
   }

   /*--- Find total fitness ---*/
   pool->total_fitness = 0;
   for(i = 0; i < pool->size; i++) {
      pool->total_fitness += pool->chrom[i]->fitness + ga_info->scale_factor;
   }

   /*--- Update ptf for each chromosome (minimize) ---*/
   if(ga_info->minimize) {
      double new_total_fitness;

      /*--- Compute new fitness and total_fitness ---*/
      new_total_fitness = 0.0;
      for(i = 0; i < pool->size; i++) {

         /*--- Failed scaling leads to divide by zero ---*/
         if((pool->chrom[i]->fitness + ga_info->scale_factor) <= 0.0)
            UT_error("PL_update_ptf: fitness + scale <= 0.0");

         /*--- Save new fitness in ptf ---*/
         pool->chrom[i]->ptf = 
            pool->total_fitness / 
            (pool->chrom[i]->fitness + ga_info->scale_factor);

         /*--- New total fitness based on new ptf ---*/
         new_total_fitness += pool->chrom[i]->ptf;
      }

      /*--- Compute new ptf ---*/
      for(i = 0; i < pool->size; i++) {

         /*--- Failed scaling leads to divide by zero ---*/
         if(new_total_fitness <= 0.0) 
            UT_error("PL_update_ptf: new_total_fitness <= 0.0");

         /*--- New ptf ---*/
         pool->chrom[i]->ptf *= 100.0 / new_total_fitness; 
      }

   /*--- Update ptf for each chromosome (maximize) ---*/
   } else {

      /*--- Compute new ptf ---*/
      for(i = 0; i < pool->size; i++) {

         /*--- Failed scaling leads to divide by zero ---*/
         if(pool->total_fitness <= 0.0)
            UT_error("PL_update_ptf: pool->total_fitness <= 0.0");

         /*--- New ptf ---*/
         pool->chrom[i]->ptf = 
            ((pool->chrom[i]->fitness + ga_info->scale_factor) / 
            pool->total_fitness) * 100.0;
      }
   }

   return OK;
}

/*============================================================================
|                            Pool manipulation
============================================================================*/
/*----------------------------------------------------------------------------
| Clean out the pool
----------------------------------------------------------------------------*/
PL_clean(pool, idx_min, idx_max)
   Pool_Ptr pool;
   int      idx_min, idx_max;
{
   int i;

   /*--- Error check ---*/
   if(!PL_valid(pool)) UT_error("PL_clean: invalid pool");
   if(idx_min < 0 || idx_min > pool->max_size) 
      UT_error("PL_clean: invalid idx_min");
   if(idx_max < 0 || idx_max > pool->max_size) 
      UT_error("PL_clean: invalid idx_max");
   if(idx_min > idx_max) UT_error("PL_clean: idx_min > idx_max");

   for(i=idx_min; i<idx_max; i++)
      PL_remove(pool, i);
}
 
/*----------------------------------------------------------------------------
| Append a chromosome to the pool
----------------------------------------------------------------------------*/
PL_append(pool, chrom, make_copy)
   Pool_Ptr  pool;    
   Chrom_Ptr chrom;
   int       make_copy;
{
   /*--- Error check ---*/
   if(!PL_valid(pool)) UT_error("PL_append: invalid pool");
   if(!CH_valid(chrom)) UT_error("PL_append: invalid chrom");

   /*--- Append = insert at end of pool ---*/
   PL_insert(pool, (int)pool->size, chrom, make_copy);
   ++(pool->size); 
}

/*----------------------------------------------------------------------------
| Replace a chromosome in the pool
----------------------------------------------------------------------------*/
PL_insert(pool, index, chrom, make_copy)
   Pool_Ptr  pool;
   int       index;
   Chrom_Ptr chrom;
   int       make_copy;
{
   /*--- Error check ---*/
   if(!PL_valid(pool)) UT_error("PL_insert: invalid pool");
   if(!CH_valid(chrom)) UT_error("PL_insert: invalid chrom");
   if(index < 0 || index > pool->max_size) 
      UT_error("PL_insert: invalid index");

   /*--- Realloc for more space ---*/
   if(index == pool->max_size) 
      PL_resize(pool, pool->max_size + PL_ALLOC_SIZE);
 
   /*--- Insert the chromosome ---*/
   if(make_copy) {
      if(!CH_valid(pool->chrom[index])) 
         pool->chrom[index] = CH_alloc(chrom->length);
      CH_copy(chrom, pool->chrom[index]);
   } else {
      if(CH_valid(pool->chrom[index])) 
         PL_remove(pool, index);
      pool->chrom[index] = chrom;
   }
}
 
/*----------------------------------------------------------------------------
| Remove a chromosome from the pool
----------------------------------------------------------------------------*/
PL_remove(pool, index)
   Pool_Ptr pool;
   int      index;
{
   /*--- Error check ---*/
   if(!PL_valid(pool)) UT_error("PL_remove: invalid pool");
   if(index < 0 || index >= pool->max_size) 
      UT_error("PL_remove: invalid index");

   if(CH_valid(pool->chrom[index])) CH_free(pool->chrom[index]);
   pool->chrom[index] = NULL;
}

/*----------------------------------------------------------------------------
| Move a chromosome in the pool
----------------------------------------------------------------------------*/
PL_move(pool, idx_src, idx_dst)
   Pool_Ptr pool;    
   int      idx_src, idx_dst;
{
   /*--- Error check ---*/
   if(!PL_valid(pool)) UT_error("PL_move: invalid pool");
   if(idx_src < 0 || idx_src >= pool->max_size) 
      UT_error("PL_move: invalid idx_src");
   if(idx_dst < 0 || idx_dst >= pool->max_size) 
      UT_error("PL_move: invalid idx_dst");
 
   if(CH_valid(pool->chrom[idx_dst])) PL_remove(pool, idx_dst);
   pool->chrom[idx_dst] = pool->chrom[idx_src];
   pool->chrom[idx_src] = NULL;
}

/*----------------------------------------------------------------------------
| Swap chromosomes in the pool
----------------------------------------------------------------------------*/
PL_swap(pool, idx1, idx2)
   Pool_Ptr pool;    
   int      idx1, idx2;
{
   Chrom_Ptr tmp;

   /*--- Error check ---*/
   if(!PL_valid(pool)) UT_error("PL_swap: invalid pool");
   if(idx1 < 0 || idx1 >= pool->max_size) 
      UT_error("PL_swap: invalid idx1");
   if(idx2 < 0 || idx2 >= pool->max_size) 
      UT_error("PL_swap: invalid idx2");
 
   tmp               = pool->chrom[idx1];
   pool->chrom[idx1] = pool->chrom[idx2];
   pool->chrom[idx2] = tmp;
}

/*----------------------------------------------------------------------------
| Sort comparison function for minimizing GA (ascending fitness)
----------------------------------------------------------------------------*/
static PL_cmp_min(a, b) 
   Chrom_Ptr *a, *b;
{
   if((*a)->fitness < (*b)->fitness)
      return -1;
   else if((*a)->fitness > (*b)->fitness)
      return 1;
   else
      return 0;
}

/*----------------------------------------------------------------------------
| Sort comparison function for maximizing GA (descending fitness)
----------------------------------------------------------------------------*/
static PL_cmp_max(a, b) 
   Chrom_Ptr *a, *b;
{
   if((*a)->fitness < (*b)->fitness)
      return 1;
   else if((*a)->fitness > (*b)->fitness)
      return -1;
   else
      return 0;
}

/*----------------------------------------------------------------------------
| Sort the pool
----------------------------------------------------------------------------*/
PL_sort(ga_info, pool) 
   GA_Info_Ptr ga_info;
   Pool_Ptr    pool;
{
   /*--- Error check ---*/
   if(!CF_valid(ga_info)) UT_error("PL_sort: invalid ga_info");
   if(!PL_valid(pool)) UT_error("PL_sort: invalid pool");

   /*--- Sort based on objective ---*/
   if(ga_info->minimize)
      qsort(pool->chrom, pool->size, sizeof(Chrom_Ptr), PL_cmp_min);
   else
      qsort(pool->chrom, pool->size, sizeof(Chrom_Ptr), PL_cmp_max);

   /*--- Reindex ---*/
   PL_index(pool);

   /*--- Pool is now sorted ---*/
   pool->sorted = TRUE;
}
