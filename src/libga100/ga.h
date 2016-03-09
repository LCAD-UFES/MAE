/*============================================================================
| (c) Copyright Arthur L. Corcoran, 1992, 1993.  All rights reserved.
|
| Genetic Algorithm Definitions
|
| Goldberg's Terminology
| ----------------------
| Chromosome = string
| Gene       = feature, character, detector
| Allele     = feature value
| Locus      = string position
| Genotype   = structure
| Phenotype  = parameter set, alternative solution, a decoded structure
| Epistasis  = nonlinearity
============================================================================*/

/*----------------------------------------------------------------------------
| Header files
----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <string.h>

#if defined(__BORLANDC__)
#include <process.h>
#include <alloc.h>
#elif !defined(__STDC__)
#include <malloc.h>
#endif

/*----------------------------------------------------------------------------
| Constants
----------------------------------------------------------------------------*/
#define VERSION   "1.00"
#define COPYRIGHT "(c) Copyright Arthur L. Corcoran, 1992, 1993.  All rights reserved."
#define FALSE         0
#define TRUE          !(FALSE)
#define OK            0
#define ERROR         !(OK)
#define UNSPECIFIED  -1

/*--- Data type --- */
#define DT_BIT       0   /* Bit string */
#define DT_INT       1   /* Integers */
#define DT_INT_PERM  2   /* Integer Permutation */
#define DT_REAL      3   /* Reals */

/*--- Method to generate initial pool --- */
#define IP_NONE         0x00
#define IP_INTERACTIVE  0x01
#define IP_FROM_FILE    0x02
#define IP_RANDOM       0x04

/*--- Type of output report --- */
#define RP_NONE    0
#define RP_MINIMAL 1
#define RP_SHORT   2
#define RP_LONG    3

/*--- Magic cookies for validation ---*/
#define NL_cookie 0x00000000   /* NULL cookie */
#define CF_cookie 0x11111111   /* ga_info (config) cookie */
#define PL_cookie 0x22222222   /* pool cookie */
#define CH_cookie 0x33333333   /* chrom cookie */

/*----------------------------------------------------------------------------
| Type definitions 
----------------------------------------------------------------------------*/
/*--- A function pointer ---*/
typedef int (*FN_Ptr)();

/*--- A function table ---*/
typedef struct {
   char      *name;    /* Function name */
   FN_Ptr    fun;      /* Function pointer */
} FN_Table_Type, *FN_Table_Ptr;

/*--- A Gene (or allele) is a bit, int, float, etc. ---*/
typedef double Gene_Type, *Gene_Ptr;

/*--- A Chromosome ---*/
typedef struct {
   long       magic_cookie;         /* For validation */
   Gene_Ptr   gene;                 /* Encoding */
   int        length;               /* Length of gene */
   double     fitness;              /* Fitness value of chromosome */
   float      ptf;                  /* Percent of total fitness */
   int        index;                /* My index */
   int        idx_min, idx_max;     /* Reserved */
   int        parent_1, parent_2;   /* Indices of parents */
   int        xp1, xp2;             /* Crossover points */
} Chrom_Type, *Chrom_Ptr;

/*--- A Pool ---*/
typedef struct {
   long       magic_cookie;                /* For validation */
   Chrom_Ptr  *chrom;                      /* Chromosomes */
   int        size, max_size;              /* Number of chromosomes */
   double     total_fitness;               /* Total fitness of pool */
   double     min, max, ave, var, dev;     /* Current pool fitness stats */
   int        min_index, max_index;        /* Index of min/max chromosomes */
   int        best_index;                  /* Index of best chromosome */
   int        minimize;                    /* Minimize pool [y/n]? */
   int        sorted;                      /* Is pool sorted [y/n]? */
} Pool_Type, *Pool_Ptr;

/*--- GA configuration info ---*/
typedef struct {
   /*--- Basic info ---*/
   long  magic_cookie;     /* For validation */
   char  user_data[80];    /* User data file (unused) */
   int   rand_seed;        /* Seed for random number generator */
   int   datatype;         /* Data type flag */
   int   ip_flag;          /* Initial pool generation method flag */
   char  ip_data[80];      /* Data file name (IP_FROM_FILE) */
   int   pool_size;        /* Pool size (IP_RANDOM) */
   int   chrom_len;        /* Chromosome size (IP_RANDOM) */
   int   iter, max_iter;   /* Number of iterations for ga */
   int   minimize;         /* Minimize EV_fun? */
   int   elitist;          /* Use elitism? */
   int   converged;        /* Has ga converged? */
   int   use_convergence;  /* Use convergence? */
   float bias;             /* Selection bias */
   float gap;              /* Generation gap */
   float x_rate;           /* Crossover rate */
   float mu_rate;          /* Mutation rate */
   float max_gene_val;     /* Maximum value of a gene */
   float scale_factor;     /* Scale for fitness <= 0 */

   /*--- Functions ---*/
   FN_Ptr   GA_fun;   /* GA */
   FN_Ptr   SE_fun;   /* Selection */
   FN_Ptr   X_fun;    /* Crossover */
   FN_Ptr   MU_fun;   /* Mutation */
   FN_Ptr   EV_fun;   /* Evaluation */
   FN_Ptr   RE_fun;   /* Replacement */

   /*--- Reports ---*/
   int  rp_type;       /* Type of output report */
   int  rp_interval;   /* Output report interval */
   FILE *rp_fid;       /* Output report fid */
   char rp_file[80];   /* Output report file name */

   /*--- Pools ---*/
   Pool_Ptr old_pool, new_pool;

   /*--- Stats ---*/
   Chrom_Ptr  best;               /* Best chromosome */
   int        num_mut, tot_mut;   /* Mutation statistics */
} GA_Info_Type, *GA_Info_Ptr;

/*----------------------------------------------------------------------------
| Pseudo-functions
----------------------------------------------------------------------------*/
/*--- random number in [0..1] ---*/
#if defined(__BORLANDC__) || defined(WINDOWS)
#define SEED_RAND(seed) (srand((seed)))
#define RAND_FRAC() ((double)rand()/RAND_MAX)
#else
#define SEED_RAND(seed) (srandom((seed)))
#define RAND_FRAC() ((double)random()*(1.0/2147483647.0))
#endif

/*--- random number in domain [lo..hi] ---*/
#define RAND_DOM(lo,hi) ((int) floor(RAND_FRAC()*(((hi)-(lo))+0.999999))+(lo))

/*--- random bit ---*/
#define RAND_BIT()  ((RAND_FRAC()>=.5)? 1 : 0 )

/*--- min and max ---*/
#define MIN(a,b) ((a < b) ? (a) : (b))
#define MAX(a,b) ((a > b) ? (a) : (b))

#define UT_warn(message) {fprintf(stderr,"WARNING: %s\n", message);}
#define UT_error(message) {fprintf(stderr,"ERROR: %s\n", message); exit(1);}
#define UT_iswap(a, b) {int tmp; tmp = *(a); *(a) = *(b); *(b) = tmp;}

/*----------------------------------------------------------------------------
| Function prototypes
----------------------------------------------------------------------------*/
extern char *GA_name(), *SE_name(), *X_name(), *MU_name(), *RE_name();
extern char *FN_name();

extern Chrom_Ptr SE_fun(), CH_alloc();
extern Pool_Ptr PL_alloc();
extern GA_Info_Ptr GA_config(), CF_alloc();
void GA_reset(GA_Info_Ptr ga_info, char *cfg_name);
void GA_run(GA_Info_Ptr ga_info);
