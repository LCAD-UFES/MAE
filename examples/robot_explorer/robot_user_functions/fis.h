#ifndef __FIS__
#define __FIS__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/***********************************************************************
 Macros and definitions
 **********************************************************************/

/* Define portable printf */
#if defined(MATLAB_MEX_FILE)
#define PRINTF mexPrintf
#elif defined(__SIMSTRUC__)
#define PRINTF ssPrintf
#else
#define PRINTF printf
#endif

#ifndef ABS
#define ABS(x)   ( (x) > (0) ? (x): (-(x)) )
#endif
#ifndef MAX
#define MAX(x,y) ( (x) > (y) ? (x) : (y) )
#endif
#ifndef MIN
#define MIN(x,y) ( (x) < (y) ? (x) : (y) )
#endif
#define MF_PARA_N 4
#define STR_LEN 500
#define MF_POINT_N 101

/* debugging macros */
/*
#define PRINT(expr) printf(#expr " = %g\n", (double)expr)
#define PRINTMAT(mat,m,n) printf(#mat " = \n"); fisPrintMatrix(mat,m,n)
#define FREEMAT(mat,m) printf("Free " #mat " ...\n"); fisFreeMatrix(mat,m)
#define FREEARRAY(array) printf("Free " #array " ...\n"); free(array)
*/
#define FREEMAT(mat,m) fisFreeMatrix(mat,m)
#define FREEARRAY(array) free(array)



/***********************************************************************
 Data types
 **********************************************************************/

typedef struct fis_node {
	int handle;
	int load_param;
	char name[STR_LEN];
	char type[STR_LEN];
	char andMethod[STR_LEN];
	char orMethod[STR_LEN];
	char impMethod[STR_LEN];
	char aggMethod[STR_LEN];
	char defuzzMethod[STR_LEN];
	int userDefinedAnd;
	int userDefinedOr;
	int userDefinedImp;
	int userDefinedAgg;
	int userDefinedDefuzz;
	int in_n;
	int out_n;
	int rule_n;
	int **rule_list;
	double *rule_weight;
	int *and_or;	/* AND-OR indicator */
	double *firing_strength;
	double *rule_output;
	/* Sugeno: output for each rules */
	/* Mamdani: constrained output MF values of rules */
	struct io_node **input;
	struct io_node **output;
	double (*andFcn)(double, double);
	double (*orFcn)(double, double);
	double (*impFcn)(double, double);
	double (*aggFcn)(double, double);
	double (*defuzzFcn)();
	double *BigOutMfMatrix;	/* used for Mamdani system only */
	double *BigWeightMatrix;/* used for Mamdani system only */
	double *mfs_of_rule;	/* MF values in a rule */
	struct fis_node *next;
} FIS;



typedef struct io_node {
	char name[STR_LEN];
	int mf_n;
	double bound[2];
	double value;
	struct mf_node **mf;
} IO;



typedef struct mf_node {
	char label[STR_LEN];	/* MF name */
	char type[STR_LEN];		/* MF type */
	int nparams;			/* length of params field */
	double *params;			/* MF parameters */
	int userDefined;		/* 1 if the MF is user-defined */
	double (*mfFcn)(double, double *); /* pointer to a mem. fcn */ 
	double value;		    /* for Sugeno only */
	double *value_array;	/* for Mamdani only, array of MF values */
} MF;

#endif /* __FIS__ */

#ifdef __cplusplus
extern "C" {
#endif

FILE *fisOpenFile(char *file, char *mode);
double **returnDataMatrix(char *filename, int *row_n_p, int *col_n_p);
double **returnFismatrix(char *fis_file, int *row_n_p, int *col_n_p);
void fisBuildFisNode(FIS *fis, double **fismatrix, int col_n, int numofpoints);
void fisPrintData(FIS *fis);
char **fisCreateMatrix(int row_n, int col_n, int element_size);
void getFisOutput(double *input, FIS *fis, double *output);
void fisFreeFisNode(FIS *fis);
void fisFreeMatrix(void **matrix, int row_n);
void fisError(char *msg);

#ifdef __cplusplus
}
#endif
