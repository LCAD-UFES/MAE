/*============================================================================
| (c) Copyright Arthur L. Corcoran, 1992, 1993.  All rights reserved.
|
| Configuration file and ga_info management
|
| Functions:
|    CF_alloc()    - allocate a ga_info
|    CF_free()     - deallocate a ga_info
|    CF_valid()    - is ga_info valid?
|    CF_reset()    - reset config
|    CF_report()   - print out current config
|    CF_read()     - read config file
|    CF_tokenize() - convert input line to tokens
|    CF_verify()   - ensure ga_info makes sense
============================================================================*/
#include "ga.h"

#define MAXTOK 10  /* Maximum number of tokens on a line */
#define STRLEN 80  /* Length of an input line */

/*----------------------------------------------------------------------------
| Allocate a GA_Info structure
----------------------------------------------------------------------------*/
GA_Info_Ptr CF_alloc() 
{
   GA_Info_Ptr ga_info;

   /*--- Allocate memory for a ga_info ---*/
   ga_info = (GA_Info_Ptr)calloc(1, sizeof(GA_Info_Type));
   if(ga_info == NULL) UT_error("CF_alloc: alloc failed");

   /*--- Ensure these are NULL ---*/
   ga_info->old_pool = NULL;
   ga_info->new_pool = NULL;
   ga_info->best     = NULL;

   /*--- Put in a magic cookie ---*/
   ga_info->magic_cookie = CF_cookie;

   /*--- Reset ga_info ---*/
   CF_reset(ga_info);

   return ga_info;
}

/*----------------------------------------------------------------------------
| De-Allocate a GA_Info structure
----------------------------------------------------------------------------*/
CF_free(ga_info) 
   GA_Info_Ptr ga_info;
{
   /*--- Error check ---*/
   if(!CF_valid(ga_info)) return;

   /*--- Free pools ---*/
   if(ga_info->old_pool != NULL) PL_free(ga_info->old_pool);
   if(ga_info->new_pool != NULL) PL_free(ga_info->new_pool);
   ga_info->old_pool = ga_info->new_pool = NULL;

   /*--- Free best chrom ---*/
   if(ga_info->best != NULL) CH_free(ga_info->best);
   ga_info->best = NULL;

   /*--- Put in a NULL cookie ---*/
   ga_info->magic_cookie = NL_cookie;

   /*--- Free ga_info ---*/
   free(ga_info);
}

/*----------------------------------------------------------------------------
| Is a ga_info valid, i.e., has it been allocated by CF_alloc()?
----------------------------------------------------------------------------*/
CF_valid(ga_info) 
   GA_Info_Ptr ga_info;
{
   /*--- Check for NULL pointers ---*/
   if(ga_info == NULL) return FALSE;

   /*--- Check for magic cookie ---*/
   if(ga_info->magic_cookie != CF_cookie) return FALSE;

   /*--- Otherwise valid ---*/
   return TRUE;
}

/*----------------------------------------------------------------------------
| Set all ga_info to defaults
----------------------------------------------------------------------------*/
CF_reset(ga_info) 
   GA_Info_Ptr ga_info;
{
   /*--- Error check ---*/
   if(!CF_valid(ga_info)) UT_error("CF_reset: invalid ga_info");

   /*--- Default basic parameters ---*/
   ga_info->user_data[0]    = '\0';
   ga_info->rand_seed       = 1;
   ga_info->datatype        = DT_INT_PERM;
   ga_info->ip_flag         = IP_RANDOM;
   ga_info->ip_data[0]      = '\0';
   ga_info->chrom_len       = 10;
   ga_info->pool_size       = 100;
   ga_info->iter            = -1;
   ga_info->max_iter        = -1;
   ga_info->bias            = 1.8;
   ga_info->gap             = 0.0;
   ga_info->x_rate          = 1.0;
   ga_info->mu_rate         = 0.0;
   ga_info->max_gene_val    = 100.0;
   ga_info->scale_factor    = 0.0;
   ga_info->minimize        = TRUE;
   ga_info->elitist         = TRUE;
   ga_info->converged       = FALSE;
   ga_info->use_convergence = TRUE;

   /*--- Default operators ---*/
   SE_select(ga_info, "roulette");
    X_select(ga_info, "order1");
   MU_select(ga_info, "swap");
   RE_select(ga_info, "append");
   GA_select(ga_info, "generational");
   ga_info->EV_fun = NULL;

   /*--- Default report parameters ---*/
   ga_info->rp_type      = RP_SHORT;
   ga_info->rp_interval  = 1;
   ga_info->rp_fid       = stdout;
   ga_info->rp_file[0]   = '\0';

   /*--- Reset pools ---*/
   if(PL_valid(ga_info->old_pool)) PL_reset(ga_info->old_pool);
   if(PL_valid(ga_info->new_pool)) PL_reset(ga_info->new_pool);

   /*--- Reset best ---*/
   if(CH_valid(ga_info->best)) CH_reset(ga_info->best);
}

/*----------------------------------------------------------------------------
| Print out all of the config information
----------------------------------------------------------------------------*/
CF_report(ga_info)
   GA_Info_Ptr    ga_info;
{
   FILE *fid;
   char *sptr;

   /*--- Error check ---*/
   if(!CF_valid(ga_info)) UT_error("CF_report: invalid ga_info");
   if(ga_info->rp_fid == NULL) UT_error("CF_report: invalid ga_info->rp_fid");

   fid = ga_info->rp_fid;

   /*--- Header ---*/
   fprintf(fid,"\nLibGA Version %s\n%s\n\n", VERSION, COPYRIGHT);
   fprintf(fid,"GA Configuration Information:\n");
   fprintf(fid,"-----------------------------\n");

   /*--- Basic info --- */
   fprintf(fid,"Basic Info\n");
   if(ga_info->user_data[0] != '\0')
      fprintf(fid,"   User Data         : %s\n", ga_info->user_data);
   fprintf(fid,"   Random Seed       : %d\n", ga_info->rand_seed);
   fprintf(fid,"   Data Type         : ");
   switch(ga_info->datatype) {
      case DT_BIT:      fprintf(fid,"Bit\n"); break;
      case DT_INT:      fprintf(fid,"Integer\n"); break;
      case DT_INT_PERM: fprintf(fid,"Integer Permutation\n"); break;
      case DT_REAL:     fprintf(fid,"Real\n"); break;
      default:          fprintf(fid,"Unspecified\n"); break;
   }
   fprintf(fid,"   Init Pool Entered : ");
   switch(ga_info->ip_flag) {
      case IP_RANDOM     : fprintf(fid,"Randomly     \n"); break;
      case IP_FROM_FILE  : fprintf(fid,"From File    \n"); break;
      case IP_INTERACTIVE: fprintf(fid,"Interactively\n"); break;
      default            : fprintf(fid,"Unspecified  \n"); break;
   }
   if(ga_info->ip_flag == IP_FROM_FILE) {
      fprintf(fid,"   Initial Pool File : ");
      if(ga_info->ip_data[0] == 0)
         fprintf(fid,"None\n");
      else if(!strcmp(ga_info->ip_data, "UNSPECIFIED"))
         fprintf(fid,"Unspecified\n");
      else
         fprintf(fid,"%s\n", ga_info->ip_data);
   }
   fprintf(fid,"   Chromosome Length : %d\n", ga_info->chrom_len);
   fprintf(fid,"   Pool Size         : %d\n", ga_info->pool_size);
   fprintf(fid,"   Number of Trials  : ");
   if(ga_info->max_iter < 0)
      fprintf(fid,"Run until convergence\n");
   else
      fprintf(fid,"%d iterations, %s\n", 
              ga_info->max_iter,
              ga_info->use_convergence ? "or until convergence" 
                                       : "ignore convergence"
      );
   fprintf(fid,"   Minimize          : %s\n", 
      ga_info->minimize ? "Yes" : "No");
   fprintf(fid,"   Elitism           : %s\n", 
      ga_info->elitist ? "Yes" : "No");
   fprintf(fid,"   Scale Factor      : %G\n", ga_info->scale_factor);

   /*--- Functions ---*/
   fprintf(fid,"\n");
   fprintf(fid,"Functions\n");
   fprintf(fid,"   GA          : %s (Gap = %G)\n", 
      GA_name(ga_info), ga_info->gap);
   fprintf(fid,"   Selection   : %s ", sptr = SE_name(ga_info));
   if(!strcmp(sptr,"rank_biased")) fprintf(fid,"(Bias = %G)", ga_info->bias);
   fprintf(fid,"\n");
   fprintf(fid,"   Crossover   : %s (Rate = %G)\n", 
      X_name(ga_info), ga_info->x_rate);
   if(ga_info->mu_rate > 0.0)
      fprintf(fid,"   Mutation    : %s (Rate = %G)\n", 
         MU_name(ga_info), ga_info->mu_rate);
   fprintf(fid,"   Replacement : %s\n", RE_name(ga_info));

   /*--- Reports ---*/
   if(ga_info->rp_type != RP_NONE) {
      fprintf(fid,"\n");
      fprintf(fid,"Reports\n"); 
      fprintf(fid,"   Type     : %s\n", 
         ga_info->rp_type == RP_MINIMAL ? "Minimal" :
         ga_info->rp_type == RP_SHORT   ? "Short"   :
         ga_info->rp_type == RP_LONG    ? "Long"    :
         "Unknown");
      fprintf(fid,"   Interval : %d\n", ga_info->rp_interval);
      if(ga_info->rp_file[0] != 0) {
         fprintf(fid,"   File  : ");
         if(!strcmp(ga_info->rp_file, "UNSPECIFIED"))
            fprintf(fid,"Unspecified\n");
         else
            fprintf(fid,"%s\n", ga_info->rp_file);
      }
   }

   /*--- Footer ---*/
   fprintf(fid,"-----------------------------\n");

   /*--- Make sure it is printed immediately ---*/
   fflush(fid);
}

/*----------------------------------------------------------------------------
| Read config file
----------------------------------------------------------------------------*/
CF_read(ga_info, cfg_name)
   GA_Info_Ptr ga_info;
   char        *cfg_name;
{
   char str[STRLEN], token[MAXTOK][STRLEN];
   int  numtok, CF_tokenize();
   FILE *fid;

   /*--- Error check ---*/
   if(!CF_valid(ga_info)) UT_error("CF_read: invalid ga_info");

   /*--- Try to open config file ---*/
   if((fid = fopen(cfg_name, "r")) == NULL) {
      UT_warn("CF_read: Error opening config file.");
      return ERROR;
   }

   /*--- Read each line from config file ---*/
   while(fgets(str, STRLEN, fid) != NULL) {

      /*--- Convert to tokens ---*/
      if((numtok=CF_tokenize(str, token)) <= 0) continue;

      /*--- Which command? ---*/
      switch(token[0][0]) {

      case 'b': 
         if(!strcmp(token[0], "bias")) {
            if(numtok >= 2 && sscanf(token[1], "%f", &ga_info->bias) == 1)
               ;
            else
               UT_warn("CF_read: Invalid bias response");
         } else
            UT_warn("CF_read: Unknown config command");
         break;

      case 'c': 
         if(!strcmp(token[0], "chrom_len")) {
            if(numtok >= 2 && sscanf(token[1], "%d", &ga_info->chrom_len) == 1)
               ;
            else
               UT_warn("CF_read: Invalid chrom_len response");
         } else if(!strcmp(token[0], "crossover")) {
            if(numtok >= 2)
               X_select(ga_info, token[1]);
            else
               UT_warn("CF_read: Invalid crossover response");
         } else
            UT_warn("CF_read: Unknown config command");
         break;

      case 'd': 
         if(!strcmp(token[0], "datatype")) {
            if(numtok >= 2 && !strcmp(token[1], "bit")) 
               ga_info->datatype = DT_BIT;
            else if(numtok >= 2 && !strcmp(token[1], "int")) 
               ga_info->datatype = DT_INT;
            else if(numtok >= 2 && !strcmp(token[1], "int_perm")) 
               ga_info->datatype = DT_INT_PERM;
            else if(numtok >= 2 && !strcmp(token[1], "real")) 
               ga_info->datatype = DT_REAL;
            else
               UT_warn("CF_read: Invalid datatype response");
         } else
            UT_warn("CF_read: Unknown config command");
         break;

      case 'e': 
         if(!strcmp(token[0], "elitism")) {
            if(numtok >= 2 && !strcmp(token[1], "true"))
               ga_info->elitist = TRUE;
            else if(numtok >= 2 && !strcmp(token[1], "false"))
               ga_info->elitist = FALSE;
            else
               UT_warn("CF_read: Invalid elitism response");
         } else
            UT_warn("CF_read: Unknown config command");
         break;

      case 'g': 
         if(!strcmp(token[0], "gap")) {
            if(numtok >= 2 && sscanf(token[1], "%f", &ga_info->gap) == 1)
               ;
            else
               UT_warn("CF_read: Invalid gap response");
         } else if(!strcmp(token[0], "ga")) {
            if(numtok >= 2) {
               GA_select(ga_info, token[1]);
               if(!strcmp(token[1], "generational")) {
                  SE_select(ga_info, "roulette");
                  RE_select(ga_info, "append");
                  ga_info->rp_interval = 1;
               } else if(!strcmp(token[1], "steady_state")) {
                  SE_select(ga_info, "rank_biased");
                  RE_select(ga_info, "by_rank");
                  ga_info->rp_interval = 100;
               }
            } else
               UT_warn("CF_read: Invalid ga response");
         } else
            UT_warn("CF_read: Unknown config command");
         break;

      case 'i': 
         if(!strcmp(token[0], "initpool")) {
            if(numtok >= 2 && !strcmp(token[1], "random")) 
               ga_info->ip_flag = IP_RANDOM;
            else if(numtok >= 2 && !strcmp(token[1], "from_file")) {
               ga_info->ip_flag = IP_FROM_FILE;
               if(numtok >= 3) strcpy(ga_info->ip_data, token[2]);
            } else if(numtok >= 2 && !strcmp(token[1], "interactive")) 
               ga_info->ip_flag = IP_INTERACTIVE;
            else
               UT_warn("CF_read: Invalid initpool response");
         } else
            UT_warn("CF_read: Unknown config command");
         break;

      case 'm': 
         if(!strcmp(token[0], "mutation")) {
            if(numtok >= 2) 
               MU_select(ga_info, token[1]);
            else
               UT_warn("CF_read: Invalid mutation response");
         } else if(!strcmp(token[0], "mu_rate")) {
            if(numtok >= 2)
               sscanf(token[1], "%f", &ga_info->mu_rate);
            else
               UT_warn("CF_read: Invalid mu_rate response");
         } else if(!strcmp(token[0], "max_gene_val")) {
            if(numtok >= 2)
               sscanf(token[1], "%f", &ga_info->max_gene_val);
            else
               UT_warn("CF_read: Invalid max_gene_val response");
         } else
            UT_warn("CF_read: Unknown config command");
         break;

      case 'o': 
         if(!strcmp(token[0], "objective")) {
            if(numtok >= 2 && !strcmp(token[1], "minimize"))
               ga_info->minimize = TRUE;
            else if(numtok >= 2 && !strcmp(token[1], "maximize"))
               ga_info->minimize = FALSE;
            else
               UT_warn("CF_read: Invalid objective response");
         } else
            UT_warn("CF_read: Unknown config command");
         break;

      case 'p': 
         if(!strcmp(token[0], "pool_size")) {
            if(numtok >= 2)
               sscanf(token[1], "%d", &ga_info->pool_size);
            else
               UT_warn("CF_read: Invalid pool_size response");
         } else
            UT_warn("CF_read: Unknown config command");
         break;

      case 'r': 
         if(!strcmp(token[0], "replacement")) {
            if(numtok >= 2)
               RE_select(ga_info, token[1]);
            else
               UT_warn("CF_read: Invalid replacement response");
         } else if(!strcmp(token[0], "rp_interval")) {
            if(numtok >= 2)
               sscanf(token[1], "%d", &ga_info->rp_interval);
            else
               UT_warn("CF_read: Invalid rp_interval response");
         } else if(!strcmp(token[0], "rp_type")) {
            if(numtok >= 2 && !strcmp(token[1], "minimal"))
               ga_info->rp_type = RP_MINIMAL;
            else if(numtok >= 2 && !strcmp(token[1], "short"))
               ga_info->rp_type = RP_SHORT;
            else if(numtok >= 2 && !strcmp(token[1], "long"))
               ga_info->rp_type = RP_LONG;
            else if(numtok >= 2 && !strcmp(token[1], "none"))
               ga_info->rp_type = RP_NONE;
            else
               UT_warn("CF_read: Invalid rp_type response");
         } else if(!strcmp(token[0], "rp_file")) {
            if(numtok >= 2) {
               char *file_mode = "a";

               /*--- Save file name ---*/
               strcpy(ga_info->rp_file, token[1]);

               /*--- Get file mode if provided ---*/
               if(numtok >= 3)
                  file_mode = token[2];

               /*--- Open report file ---*/
               ga_info->rp_fid = fopen(ga_info->rp_file, file_mode);
               if(ga_info->rp_fid == NULL)
                  UT_error("CF_read: error opening report file");
            } else
               UT_warn("CF_read: Invalid rp_file response");
         } else if(!strcmp(token[0], "rand_seed")) {
            if(numtok >= 2 && !strcmp(token[1], "my_pid"))
               ga_info->rand_seed = getpid();
            else if(numtok >= 2)
               sscanf(token[1], "%d", &ga_info->rand_seed);
            else
               UT_warn("CF_read: Invalid rand_seed response");
         } else
            UT_warn("CF_read: Unknown config command");
         break;

      case 's': 
         if(!strcmp(token[0], "selection")) {
            if(numtok >= 2)
               SE_select(ga_info, token[1]);
            else
               UT_warn("CF_read: Invalid selection response");
         } else if(!strcmp(token[0], "stop_after")) {
            if(numtok == 2 && !strcmp(token[1], "convergence")) {
               ga_info->use_convergence = TRUE;
               ga_info->max_iter = -1;
            } else if(numtok >= 2) {
               sscanf(token[1], "%d", &ga_info->max_iter);
               if(ga_info->max_iter < 1) 
                  UT_warn("CF_read: Invalid number for stop_after");
               ga_info->use_convergence = TRUE;
               if(numtok > 2 && !strcmp(token[2], "ignore_convergence"))
                  ga_info->use_convergence = FALSE;
            } else {
               UT_warn("CF_read: Invalid stop_after response");
            }
         } else
            UT_warn("CF_read: Unknown config command");
         break;

      case 'u': 
         if(!strcmp(token[0], "user_data")) {
            if(numtok >= 2)
               strcpy(ga_info->user_data, token[1]);
            else
               UT_warn("CF_read: Invalid user_data response");
         } else
            UT_warn("CF_read: Unknown config command");
         break;

      case 'x': 
         if(!strcmp(token[0], "x_rate")) {
            if(numtok >= 2 && sscanf(token[1], "%f", &ga_info->x_rate) == 1)
               ;
            else
               UT_warn("CF_read: Invalid x_rate response");
         } else
            UT_warn("CF_read: Unknown config command");
         break;

      default:
            UT_warn("CF_read: Unknown config command");
      }
   }
   /*--- PATCH 1 BEGIN ---*/
   /* Many thanks to Paul-Erik Raue (peraue@cs.vu.nl) 
    * for finding this bug. 
    * 
    * Close the configuration file 
    */
   fclose(fid);
   /*--- PATCH 1 END ---*/
}

/*----------------------------------------------------------------------------
| Convert input line to tokens
----------------------------------------------------------------------------*/
CF_tokenize(line, token)
   char *line, token[MAXTOK][STRLEN];
{
   int numtok, i, j, len;

   numtok = 0;
   len = strlen(line);
   for(i = 0; i < len; ) {

      /*--- Find token ---*/
      while(isspace(line[i]) && i < len) i++;

      /*--- Skip comments & blank lines ---*/
      if(i >= len || line[i] == '#' || line[i] == '\n') break;

      /*--- Save token ---*/
      for(j = 0; !isspace(line[i]) && i < len; i++, j++) 
         token[numtok][j] = line[i];
      token[numtok++][j] = 0;
   }

   return numtok;
}

/*----------------------------------------------------------------------------
| Verify configuration
----------------------------------------------------------------------------*/
CF_verify(ga_info)
   GA_Info_Ptr ga_info;
{
   /*--- Error check ---*/
   if(!CF_valid(ga_info)) UT_error("CF_verify: invalid ga_info");

   switch(ga_info->datatype) {
      case DT_BIT:
      case DT_INT:
      case DT_INT_PERM:
      case DT_REAL:
         break;
      default: UT_error("CF_verify: Invalid datatype");
   }

   switch(ga_info->ip_flag) {
      case IP_FROM_FILE:
         if(ga_info->ip_data[0] == '\0')
            UT_error("CF_verify: no file specified for initpool");
         break;
      case IP_INTERACTIVE:
      case IP_RANDOM:
      case IP_NONE:
         break;
      default: UT_error("CF_verify: Invalid ip_flag");
   }

   if(ga_info->chrom_len <= 0)
      UT_error("CF_verify: invalid chromosome length");

   if(ga_info->pool_size <= 0)
      UT_error("CF_verify: invalid pool size");

   if(ga_info->SE_fun == NULL)
      UT_error("CF_verify: no selection function specified");

   if(ga_info->X_fun == NULL)
      UT_error("CF_verify: no crossover function specified");

   if(ga_info->x_rate < 0.0 || ga_info->x_rate > 1.0)
      UT_error("CF_verify: invalid crossover rate");

   if(ga_info->mu_rate < 0.0)
      UT_error("CF_verify: invalid mutation rate");

   if(ga_info->mu_rate > 0.0 && ga_info->MU_fun == NULL)
      UT_error("CF_verify: no mutation function specified");

   if(ga_info->RE_fun == NULL)
      UT_error("CF_verify: no replacement function specified");

   if(ga_info->EV_fun == NULL)
      UT_error("CF_verify: no evaluation function specified");

   if(ga_info->GA_fun == NULL)
      UT_error("CF_verify: no ga function specified");

   if(ga_info->gap < 0.0 || ga_info->gap > 1.0)
      UT_error("CF_verify: invalid generation gap");

   if(ga_info->minimize != TRUE && ga_info->minimize != FALSE)
      UT_error("CF_verify: illegal value for minimize");

   if(ga_info->elitist != TRUE && ga_info->elitist != FALSE)
      UT_error("CF_verify: illegal value for elitism");

   switch(ga_info->rp_type) {
      case RP_NONE:
      case RP_MINIMAL:
      case RP_SHORT:
      case RP_LONG:
         break;
      default: UT_error("CF_verify: Invalid report type");
   }

   if(ga_info->rp_interval <= 0)
      UT_error("CF_verify: invalid report interval");
}
