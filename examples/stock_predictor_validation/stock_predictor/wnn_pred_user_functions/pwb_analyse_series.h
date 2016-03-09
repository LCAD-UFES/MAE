/*****************************************************************************
* PORTFOLIO WORKBENCH PROJECT (PWB)                                     	 *
* pwb_analyse_series.h - Analyse PWB generated time-series header file       *
* Fabio D Freitas                                                            *
* Versions:                                                                  *
* v1.0, MAR-2005 Intro versio.                                               *
*****************************************************************************/
#ifndef __PWB_ANALYSE_SERIES_H__
#define __PWB_ANALYSE_SERIES_H__

//#define EXEC

#include <stdio.h>
#include <stdlib.h>
#include <values.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h> 
#include <fcntl.h> 
#include <string.h>
#include <time.h>
#include <signal.h>
#include <libgen.h>
#include <locale.h>
#include <ctype.h>
#ifdef EXEC
#include <fmisc.h>
#include <timeutil.h>

#include <gsl/gsl_statistics.h>
#include <gsl/gsl_math.h>

#include <gsl/gsl_histogram.h>

#include <gsl/gsl_rng.h>
#include <gsl/gsl_cdf.h>
#else
#include <math.h>
#endif


#ifndef __PWB_ANALYSE_SERIES_C__
#define __PWB_ANALYSE_SERIES_H__PUBEXT extern
#else
#define __PWB_ANALYSE_SERIES_H__PUBEXT
#endif



/* Constants, filenames, etc... */

#define	PWB_ANALYSE_SERIES_VERSION	"2.1.0 <25-feb-2010>"	/* version, release, revision */ 
#define	PWB_ANALYSE_SERIES_PATHSIZE	 512		/* size of path buffers */ 
#define	PWB_ANALYSE_SERIES_FNAMESIZE 256		/* size of filename buffers */ 


#define	COLUMN_TYPE_ERROR	-1
#define	COLUMN_TYPE_NUMBER	0
#define	COLUMN_TYPE_DATE	1
#define	COLUMN_TYPE_OTHER	9


/* structures */

typedef struct
{
	char id[PWB_ANALYSE_SERIES_FNAMESIZE];
	char desc[PWB_ANALYSE_SERIES_FNAMESIZE];
	char fname[PWB_ANALYSE_SERIES_FNAMESIZE];
	double *v;
	int n_v;

	/* some stats */
	double min;
	double max;	
	double mean;
	double var;	
	double sd;

} PWB_ANALYSE_SERIES_TS; 


/* From pwb_pdf by Fabio D Freitas */

/* Chi-square  Goodness of Fit (GoF) test structure */
typedef struct
{
	double alpha;	/* significance level - probability of Type 2 error */
					/* Type 2 error is to reject H0 when it is true */

	int k; 			/* Selected Number of bins  */
	int k_eff; 		/* Number of effective bins (bins may be combined at tails) */
	double dof; 	/* degrees of freedom of the test */

	double X_2_test_statistic; /* Test statistic (calculated from data and Normal) */
	double X_2_critical_value; 	/* Critical value of Chi-square distribution table */

	int H0; 		/* (H0 == normality of data) Test results: 1== acceptd, 0== rejected */

} PWB_PDF_CHISQGOF; 


/* prototypes */


double *ReadSeries(char *fname, int *size);
int ReadSeriesTT(char *fname, 
                   PWB_ANALYSE_SERIES_TS *ts, PWB_ANALYSE_SERIES_TS *ts2,
				   PWB_ANALYSE_SERIES_TS *ts3,PWB_ANALYSE_SERIES_TS *ts4);

int ReadSeriesTT_RW(PWB_ANALYSE_SERIES_TS *rw, PWB_ANALYSE_SERIES_TS *tgt_trn,
				   PWB_ANALYSE_SERIES_TS *tgt,PWB_ANALYSE_SERIES_TS *out);

int ReadSeriesTT_MEAN(PWB_ANALYSE_SERIES_TS *mean, PWB_ANALYSE_SERIES_TS *tgt_trn,
				   PWB_ANALYSE_SERIES_TS *tgt,PWB_ANALYSE_SERIES_TS *out);


double *ReadSeriesColumn(char *fname, int *size, int col);

char **ReadSeriesColumnString(char *fname, int *size, int col);


PWB_ANALYSE_SERIES_TS *CreateAbsoluteErrorTS
(PWB_ANALYSE_SERIES_TS *tgt,PWB_ANALYSE_SERIES_TS *out, PWB_ANALYSE_SERIES_TS *ts,
char *id, char *desc);

int DumpTS(PWB_ANALYSE_SERIES_TS *ts);


/* Services */
int Legends(void);
int Header(void);

int Detail(char *id,
          PWB_ANALYSE_SERIES_TS *tgt,
		  PWB_ANALYSE_SERIES_TS *out,
		  PWB_ANALYSE_SERIES_TS *err,
          char **dates);

int PredictionStats(PWB_ANALYSE_SERIES_TS *out,
                    PWB_ANALYSE_SERIES_TS *tgt,
                    PWB_ANALYSE_SERIES_TS *err, 
					char **dates,					
                    char *prefix, int header_mode);
int BasicStats(PWB_ANALYSE_SERIES_TS *ts, char *prefix, int header_mode);
int PredSignalError(PWB_ANALYSE_SERIES_TS *tgt,PWB_ANALYSE_SERIES_TS *out, char *prefix, int header_mode);
double RMSE(PWB_ANALYSE_SERIES_TS *tgt,PWB_ANALYSE_SERIES_TS *out);
double MAPE(PWB_ANALYSE_SERIES_TS *tgt,PWB_ANALYSE_SERIES_TS *out);
double ME(PWB_ANALYSE_SERIES_TS *tgt,PWB_ANALYSE_SERIES_TS *out);
double UTheil(PWB_ANALYSE_SERIES_TS *tgt,PWB_ANALYSE_SERIES_TS *out);
double R2(PWB_ANALYSE_SERIES_TS *tgt,PWB_ANALYSE_SERIES_TS *out);
double HR(PWB_ANALYSE_SERIES_TS *tgt,PWB_ANALYSE_SERIES_TS *out);
double HR_UP(PWB_ANALYSE_SERIES_TS *tgt,PWB_ANALYSE_SERIES_TS *out);
double HR_DN(PWB_ANALYSE_SERIES_TS *tgt,PWB_ANALYSE_SERIES_TS *out);

int RepeatedValues(PWB_ANALYSE_SERIES_TS *ts);

int NormalityTest(PWB_ANALYSE_SERIES_TS *ts, char *prefix, int header_mode);


/* From pwb_pdf by Fabio D Freitas */
int
ChiSquareGOFTestNormal(double *y, int n, double mu, double sigma, PWB_PDF_CHISQGOF *gof);

#ifdef EXEC
gsl_histogram *CreateHistogram(
double *y, int n, double mu, double sigma, double x_min, double x_max,
int k, double w);
#endif


int HelpMsg(FILE *fp);



/* Globals */
PWB_ANALYSE_SERIES_TS ts_out; 
PWB_ANALYSE_SERIES_TS ts_tgt; 
PWB_ANALYSE_SERIES_TS ts_err; 

//PWB_ANALYSE_SERIES_TS ts1; 
//PWB_ANALYSE_SERIES_TS ts2; 
//PWB_ANALYSE_SERIES_TS ts3; 
//PWB_ANALYSE_SERIES_TS ts4;

PWB_ANALYSE_SERIES_TS rw;
PWB_ANALYSE_SERIES_TS mp;
PWB_ANALYSE_SERIES_TS rw_abserr;
PWB_ANALYSE_SERIES_TS mp_abserr;
PWB_ANALYSE_SERIES_TS rw_rmse;
PWB_ANALYSE_SERIES_TS mp_rmse;
PWB_ANALYSE_SERIES_TS rw_mape;
PWB_ANALYSE_SERIES_TS mp_mape;

char **dates; 
int n_dates; 

#endif /* __PWBDBH__ */

/* EOF  */
