/*****************************************************************************
* PORTFOLIO WORKBENCH PROJECT (PWB)                                     	 *
* pwb_analyse_series.c - Analyse PWB generated time-series                   *
* Fabio D Freitas                                                            *
* Versions:                                                                  *
* v1.0, MAR-2005 Intro version.                                              *
* v2.0, MAR-2009 New errors and input column processing                      *
* v2.1, FEB-2010 Predictions columns and blank fields                        *
*****************************************************************************/
#ifndef __PWB_ANALYSE_SERIES_C__
#define __PWB_ANALYSE_SERIES_C__
#endif

#include "pwb_analyse_series.h"

//#define  DEBUG	0



int doubleeqcmp(double d1, double d2, double epsilon)
{

	/* DEPRECATED in MAY/2007 */

	if ( fabs(d1 - d2) < epsilon ) return 1;
	return 0;
}


#ifdef EXEC

/*****************************************************************************
* @@ main                                                            	     *
* Rmk...: program entry point                                                *
* Date..: MAR/2005                                                           * 
* Author.: Fabio Daros de Freitas.                                           *
*****************************************************************************/
int main(int argc, char **argv)
{
	int i;
	int n;
	int t;
	int ftype;
	
	int start_col;
	int cols;
	double *v;
	static char fname[PWB_ANALYSE_SERIES_FNAMESIZE];
	static char header[PWB_ANALYSE_SERIES_FNAMESIZE];	
	static char id[PWB_ANALYSE_SERIES_FNAMESIZE];		


	/* Set Locale default */
	setlocale(LC_ALL, "en_US");
	
	if ( argc == 1 || cmdline_parser(argc, argv, "-h ", NULL, NULL) )
	{
		HelpMsg(stdout); 
		exit(-1); 
	}

	/* ==== Read series ====*/
	
	/* inits */
	ts_out.v = NULL;
	ts_tgt.v = NULL;	
	ts_err.v = NULL;
	ftype = 0;

	/* cols */
	start_col = 1;
	
	/* --- default cols ---*/
	cols = 3;
	
	if ( cmdline_parser(argc, argv, "-t", &ftype, "(int *)") )
	{
			switch ( ftype ) 
		{
			case 0: start_col = 1; cols = 3; break;
			case 1: start_col = 1; cols = 2; break;			
			case 2: start_col = 1; cols = 1; break;
			case 3: start_col = 2; cols = 3; break;
			case 4: start_col = 2; cols = 2; break;
			case 5: start_col = 2; cols = 1; break;												
			default:
			{
				fprintf(stderr, 
				"(main) *ERROR*: Invalid -t argument. Use -h for details. Exiting..\n");
				exit (-1);
			}
		}
	}

	/* series ID */
	strcpy(id, "SERIES"); 
	cmdline_parser(argc, argv, "-id", id, "(char *)");

	/* series name */
	if ( cmdline_parser(argc, argv, "-f", ts_tgt.fname, "(char *)") )
	{
		strcpy(ts_tgt.desc, id);
		sprintf(ts_tgt.id, "%s_%s", id, "TGT"); 
		
		ts_tgt.v = ReadSeriesColumn(ts_tgt.fname, &(ts_tgt.n_v), start_col);
		if ( ts_tgt.v == NULL )
		{
			fprintf(stderr, 
			"(main) *ERROR*: Error reading TGT (col=%d) time series (-f) on file [%s]. Exiting..\n",
			start_col,
			ts_tgt.fname);
			exit (-2);
		}
		//DumpTS(&ts_out);
		

		if ( cols >= 2 )		
		{
			strcpy(ts_out.desc, id);
			sprintf(ts_out.id, "%s_%s", id, "OUT"); 
			strcpy(ts_out.fname, ts_tgt.fname);	

			ts_out.v = ReadSeriesColumn(ts_out.fname, &(ts_out.n_v), start_col+1);
			if ( ts_out.v == NULL )
			{
				fprintf(stderr, 
				"(main) *ERROR*: Error reading OUT (col=%d) time series (-f) on file [%s]. Exiting..\n",
				start_col+1,
				ts_out.fname);
				exit (-3);
			}
		}
		
		if ( cols == 3 )		
		{
			strcpy(ts_err.desc, id);
			sprintf(ts_err.id, "%s_%s", id, "ERR"); 

			strcpy(ts_err.fname, ts_tgt.fname);			

			ts_err.v = ReadSeriesColumn(ts_err.fname, &(ts_err.n_v), start_col+2);
			if ( ts_err.v == NULL )
			{
				fprintf(stderr, 
				"(main) *ERROR*: Error reading ERR (col=%d) time series (-f) on file [%s]. Exiting..\n",
				start_col+2,
				ts_err.fname);
				exit (-4);
			}
		}
	}
	else
	{
		fprintf(stderr, 
		"(main) *ERROR*: No datafile selected. Use -f option\n");
		HelpMsg(stdout);
		exit(-1);
	}
	

	/* read dates */
	if ( start_col == 1 )
	{
		dates = ReadSeriesColumnString(ts_tgt.fname, &n_dates, 0);
		
		if ( dates == NULL  )
		{
			fprintf(stderr, 
			"(main) *ERROR*: Error reading dates (col=%d) time series (-f) on file [%s]. Exiting..\n",
			start_col,
			ts_tgt.fname);
			exit (-5);
		}		
	}
	
	/* complete ERR series in case of OUT and TGT */
	if ( ftype == 1 || ftype == 4 )
	{
		ts_err.n_v = ts_out.n_v;
		ts_err.v = createvector_double(ts_out.v, ts_out.n_v);
		if ( ts_err.v == NULL )
		{
			fprintf(stderr, 
			"(main) *ERROR*: Cannot create ERR vector (n=%d)\n", ts_err.n_v);
			exit (-5);
		}
		
		for ( i = 0; i < ts_err.n_v; i++ )
		{
			ts_err.v[i] = ts_tgt.v[i] - ts_out.v[i];		
		}
	}	

	/* All Predictor STATS */
	
	
	/* HEADER */
	if ( cmdline_parser(argc, argv, "-hdr", NULL, NULL) )
		Header();


/*
printf("tgt.n=%d  out.n=%d err.n=%d tgt.sd=%lf  out.sd=%lf err.sd=%lf\n",
ts_tgt.n_v,
ts_out.n_v,
ts_err.n_v,
ts_tgt.sd,
ts_out.sd,
ts_err.sd);

exit(-1); 
*/

	
	Detail(id, &ts_tgt, &ts_out, &ts_err, dates);

	
////////////	
//for ( i = 0; i < ts.n_v; i++ ){
//fprintf(stdout, ">> ts[%d]=%lf\n", i, ts.v[i]); 
//}



	
	return 0;
}


/*****************************************************************************
* @@ ReadSeriesColumn                                             	         *
* Rmk...: read a time-series in column                                       *
*                                                                            *
*                ...            ...     ...                                  *
* Date..: OCT/2007                                                           * 
*         FEB/2010 Allow blank fields                                        * 
* Author.: Fabio Daros de Freitas.                                           *
*****************************************************************************/
double *ReadSeriesColumn(char *fname, int *size, int col)
{	 
	char *b;
	int i;
	int j;
	int k;
	int n;
	int len;
	int nlines;
	int pos;
	char line[1024];
	char bvalue[1024];
	char *ptr;
	char date[16];
	double value;
	double *v;
	int error;
	char dir[PWB_ANALYSE_SERIES_PATHSIZE];	
	
	/* Entry test */
	if ( fname == NULL || strlen(fname) < 1 )
	{
		fprintf(stderr, "(ReadSeriesColumn) *ERROR*: BAD fname\n");
		return NULL;
	}		

	getcwd(dir, PWB_ANALYSE_SERIES_PATHSIZE);		

	/* read series */
	b = readfile(fname, &n);
	if ( b == NULL )
	{
		fprintf(stdout, "(ReadSeriesColumn) *ERROR*: In dir [%s] cannot readfile(%s)\n",
		dir, fname);
		return NULL;	
	}

	nlines = tokeninstring(b, '\n');
	len = strlen(b);

//printf("[%s] n=%d\n", fname, nlines);


	/* read series */
	v = calloc(nlines, sizeof(double)); 
	if ( v == NULL )
	{
		fprintf(stdout, "(ReadSeriesColumn) **ERROR**: Cannot calloc(%d) fname=%s v - %s\n",
		nlines, fname, strerror(errno));
		free(b);
		return NULL;	
	}

	/* Loop in file...*/
	error = 0;
	pos = 0;
	j   = 0;
	for ( i = 0; i < nlines; i++ )
	{
		if ( pos < len )
		{ 
			memset(line, 0, 1024); 
			pos += 1 + sgets(line, 1023, b + pos);
		}
		else
			break;

		/* reset error */
		if ( error == 1 ) 
			error = 0;

		/* Block comments */
		if ( strstr(line, "/*") )
		{
			error = 2;	
		}
		else if ( strstr(line, "*/") )
		{
			error = 1;
		}		
		if ( error ) continue;


		/* Data error and line comments */		

		value = 0.0;

		if ( *line == '#' || *line == '@' )
		{
			error = 1;	
		}
		
		else
		{
			/* scan the column buffer */
			ptr = line;						

			for ( k = 0; k < col; k++ )
			{
				ptr = strchr(ptr, ';' );

				if ( ptr == NULL || ptr == &(b[len-1]) )
				{
					fprintf(stdout, 
					"(ReadSeriesColumn) *ERROR*: Cannot read column buffer in [%s] col=%d of %d at line=%d of [%s]. Read Aborted!\n",
					line,
					k,
					col,
					i,
					fname);
					ffree(v);
					ffree(b);
					return NULL;
				}
				ptr++;
			}

#if 0

FDF feb/2010  allowing blank fields 

			/* scan value  */			
			if ( sscanf(ptr, "%lf", &value) != 1 )	
			{
				fprintf(stderr, 
				"(ReadSeriesColumn) *ERROR*: Cannot read value in [%s] at col=%d at line=%d of [%s]. Read Aborted!\n",
				bvalue,
				k,
				i,
				fname);
				ffree(v);
				ffree(b);
				return NULL;
			}
			v[j] = value;
			j++;					
#endif


			/* scan value  */			
			if ( sscanf(ptr, "%lf", &value) == 1 )	
			{
				v[j] = value;
				j++;					
			}
			else
			{
				fprintf(stderr, 
				"(ReadSeriesColumn) *WARNING*: Cannot read a valid value in [%s] at col=%d at line=%d of [%s]. Blank field?\n",
				bvalue,
				k,
				i,
				fname);
			}
			
			ptr += pos;	
		}			
	}
	
	if ( size != NULL )
		*size = j;	

	ffree(b);

	return v;
}


/*****************************************************************************
* @@ ReadSeriesColumnString                                       	         *
* Rmk...: read a time-series in ColumnString                                 *
*                                                                            *
*                ...            ...     ...                                  *
* Date..:                                                                    * 
*         FEB/2010                                                           * 
* Author.: Fabio Daros de Freitas.                                           *
*****************************************************************************/
char **ReadSeriesColumnString(char *fname, int *size, int col)
{	 
	char *b;
	int i;
	int j;
	int k;
	int n;
	int len;
	int nlines;
	int pos;
	char line[1024];
	char bvalue[1024];
	char *ptr;
	char date[16];

	double value;
	char svalue[1024];
	
	
	double *v;
	char **sv;	

	
	int error;
	char dir[PWB_ANALYSE_SERIES_PATHSIZE];	
	
	/* Entry test */
	if ( fname == NULL || strlen(fname) < 1 )
	{
		fprintf(stderr, "(ReadSeriesColumnString) *ERROR*: BAD fname\n");
		return NULL;
	}		

	getcwd(dir, PWB_ANALYSE_SERIES_PATHSIZE);		

	/* read series */
	b = readfile(fname, &n);
	if ( b == NULL )
	{
		fprintf(stdout, "(ReadSeriesColumnString) *ERROR*: In dir [%s] cannot readfile(%s)\n",
		dir, fname);
		return NULL;	
	}

	nlines = tokeninstring(b, '\n');
	len = strlen(b);

//printf("[%s] n=%d\n", fname, nlines);


	/* read series */
//	v = calloc(nlines, sizeof(double)); 
	
	sv = calloc(nlines, sizeof(char *)); 	
	if ( sv == NULL )
	{
		fprintf(stdout, "(ReadSeriesColumnString) **ERROR**: Cannot calloc(%d) fname=%s sv - %s\n",
		nlines, fname, strerror(errno));
		free(b);
		return NULL;	
	}

	/* Loop in file...*/
	error = 0;
	pos = 0;
	j   = 0;
	for ( i = 0; i < nlines; i++ )
	{
		if ( pos < len )
		{ 
			memset(line, 0, 1024); 
			pos += 1 + sgets(line, 1023, b + pos);
		}
		else
			break;

		/* reset error */
		if ( error == 1 ) 
			error = 0;

		/* Block comments */
		if ( strstr(line, "/*") )
		{
			error = 2;	
		}
		else if ( strstr(line, "*/") )
		{
			error = 1;
		}		
		if ( error ) continue;


		/* Data error and line comments */		

		value = 0.0;

		if ( *line == '#' || *line == '@' )
		{
			error = 1;	
		}
		
		else
		{
			/* scan the ColumnString buffer */
			ptr = line;						

			for ( k = 0; k < col; k++ )
			{
				ptr = strchr(ptr, ';' );

				if ( ptr == NULL || ptr == &(b[len-1]) )
				{
					fprintf(stdout, 
					"(ReadSeriesColumnString) *ERROR*: Cannot read ColumnString buffer in [%s] col=%d of %d at line=%d of [%s]. Read Aborted!\n",
					line,
					k,
					col,
					i,
					fname);
					ffree(v);
					ffree(b);
					return NULL;
				}
				ptr++;
			}

			/* scan value  */			
//			if ( sscanf(ptr, "%lf", &value) == 1 )	

			if ( sscanf(ptr, "%s", svalue) == 1 )
			{

//printf("@@@ svalue[%s]\n",
//svalue); 

				sv[j] = strdup(svalue);
				j++;					
			}
			else
			{
				fprintf(stderr, 
				"(ReadSeriesColumnString) *WARNING*: Cannot read a valid value in [%s] at col=%d at line=%d of [%s]. Blank field?\n",
				bvalue,
				k,
				i,
				fname);
			}
			
			ptr += pos;	
		}			
	}
	
	if ( size != NULL )
		*size = j;	

	ffree(b);

	return sv;
}




/*****************************************************************************
* @@ Legends                                                          	     *
* Rmk...: shows variables legends                                            *
*                ...            ...     ...                                  *
* Date..: OCT/2007                                                           * 
* Author.: Fabio Daros de Freitas.                                           *
*****************************************************************************/
int Legends(void)
{	
	fprintf(stdout, "# Legends:\n");
	fprintf(stdout, "#\n");	
	fprintf(stdout, "# CODE  = Stock code;\n");
	fprintf(stdout, "# TGT   = target value;        OUT = predicted value;        ERR = error (TGT - OUT) value;\n");

	fprintf(stdout, "# TGT+  = count TGT > 0;       TGT-  = count TGT < 0;        TGTz = count TGT == 0;\n");
	fprintf(stdout, "# OUT+  = count OUT > 0;       OUT-  = count OUT < 0;        OUTz = count OUT == 0;\n");

	fprintf(stdout, "# H     = count TGT * OUT > 0; Hnz   = count TGT * OUT != 0; H+   = count TGT > 0 AND OUT > 0;  H-   = count TGT < 0 AND OUT < 0;\n");  
	fprintf(stdout, "# HR    = H / Hnz;             HR+   = H+ / OUT+;            HR-  = H- / OUT-;\n");  

	fprintf(stdout, "# ===\n"); 
	fprintf(stdout, "#\n"); 		
		
	return 0;
}


/*****************************************************************************
* @@ Header                                                          	     *
* Rmk...: shows the header                                                   *
*                ...            ...     ...                                  *
* Date..: OCT/2007                                                           * 
*         JUN/2009 --- Added UTheil()                                        * 
*         JUL/2009 --- Added R^2                                             * 
* Author.: Fabio Daros de Freitas.                                           *
*****************************************************************************/
int Header(void)
{	 
	char  hdr1[16];

	Legends();

	fprintf(stdout, "# Header:\n");
	fprintf(stdout, "#\n");	

	sprintf(hdr1, "#%s", "CODE");

	fprintf(stdout,
	"%-30s; ",
	hdr1);

	PredictionStats(NULL, NULL, NULL, NULL, "PRD", 1);
	BasicStats(NULL, "TGT", 1);
	BasicStats(NULL, "OUT", 1);
	BasicStats(NULL, "ERR", 1);

	fprintf(stdout,
	"%-15s; %-15s; %-15s; %-15s; ",
	"ERR_RMSE",
	"ERR_MAPE",
	"U_THEIL",
	"R^2");	

	PredSignalError(NULL, NULL, "PRD", 1);
	
	NormalityTest(NULL, "TGT", 1);
	NormalityTest(NULL, "OUT", 1);	
	NormalityTest(NULL, "ERR", 1);	

	fprintf(stdout, "\n"); 		
	fprintf(stdout, "# ===\n"); 
	fprintf(stdout, "#\n"); 		
		
	return 0;
}


/*****************************************************************************
* @@ Detail                                                          	     *
* Rmk...: shows the detail line                                              *
*                ...            ...     ...                                  *
* Date..: OCT/2007                                                           * 
*         JUN/2009 --- Added UTheil()                                        * 
*         JUL/2009 --- Added R^2                                              * 
* Author.: Fabio Daros de Freitas.                                           *
*****************************************************************************/
int Detail(char *id,
          PWB_ANALYSE_SERIES_TS *tgt,
		  PWB_ANALYSE_SERIES_TS *out,
		  PWB_ANALYSE_SERIES_TS *err,
          char **dates)
{
	fprintf(stdout, "%-30s; ",
	id);

	PredictionStats(out, tgt, err, dates, "PRD", 0); 
	BasicStats(tgt, "TGT", 0); 
	BasicStats(out, "OUT", 0); 
	BasicStats(err, "ERR", 0); 

	if ( tgt->v != NULL && out->v != NULL )
		fprintf(stdout, "%-15.2lf; %-15.2lf; %-15.2lf; %-15.2lf; ",
		RMSE(tgt, out),
		MAPE(tgt, out),
		UTheil(tgt, out),
		R2(tgt, out));
	else
		fprintf(stdout, "%-15s; %-15s; %-15s; %-15s; ",
		"n.a.",
		"n.a.",		
		"n.a.",			
		"n.a.");

	PredSignalError(tgt, out, "PRD", 0);
	
	NormalityTest(tgt, "TGT", 0);
	NormalityTest(out, "OUT", 0);	
	NormalityTest(err, "ERR", 0);

	fprintf(stdout, "\n");
		
	return 0;
}

#if 1

/*****************************************************************************
* @@ PredictionStats                                                       	 *
* Rmk...: shows prediction stats of series                                   *
*                ...            ...     ...                                  *
* Date..: FEB/2010                                                           * 
*                ...            ...     ...                                  *
* Author.: Fabio Daros de Freitas.                                           *
*****************************************************************************/
int PredictionStats(PWB_ANALYSE_SERIES_TS *out,
                    PWB_ANALYSE_SERIES_TS *tgt,
                    PWB_ANALYSE_SERIES_TS *err, 
					char **dates,
                    char *prefix, int header_mode)
{	 
	int j;
	double old_v;
	int i;
	int f;
	int n;
	int n_seq;

	char  hdr1[16];
	char  hdr2[16];
	char  hdr3[16];
	char  hdr4[16];	
	char  hdr5[16];
	char  hdr6[16];

	if ( header_mode )
	{
		sprintf(hdr1, "%s_%s", prefix, "DATE"); 
		sprintf(hdr2, "%s_%s", prefix, "VALUE"); 		
		sprintf(hdr3, "%s_%s", prefix, "ERR_SD"); 	
		sprintf(hdr4, "%s_%s", prefix, "QUALITY"); 
			
		fprintf(stdout,
		"%-15s; %-15s; %-15s; %-15s; ",
		hdr1,
		hdr2,		
		hdr3,		
		hdr4);
			
		return 0;
	}
		
	/* Entry test */
	if ( out == NULL || tgt == NULL || err == NULL)
	{
		fprintf(stderr, "(PredictionStats) Error:NULL ts or NULL ts->v\n");
		return -1;
	}		

	if ( (out->v != NULL && out->n_v > 1) &&  (tgt->v != NULL && tgt->n_v > 1) &&
         (err->v != NULL && err->n_v > 1) )
	{

		gsl_stats_minmax(&(err->min), &(err->max), err->v, 1, err->n_v);
		err->mean = gsl_stats_mean (err->v, 1, err->n_v);
		err->var = gsl_stats_variance_m (err->v, 1, err->n_v, err->mean); 
		err->sd = sqrt(err->var); 	



		fprintf(stdout,
		"%-15s; %-15lf; %-15lf; %-15lf; ",	
		dates[out->n_v-1],
		out->v[out->n_v-1],
		err->sd,
		out->v[out->n_v-1]/err->sd);
	}
	else
	{
		fprintf(stdout,
		"%-15s; %-15s; %-15s; %-15s; ",	
		"n.a.",
		"n.a.",
		"n.a.",
		"n.a.");		
	}
		
	return 0;
}


#endif

/*****************************************************************************
* @@ BasicStats                                                       	     *
* Rmk...: shows basic stats of series                                        *
*                ...            ...     ...                                  *
* Date..: MAR/2005                                                           * 
*         OCT/2007 - reformulated outputs!                                   * 
* Author.: Fabio Daros de Freitas.                                           *
*****************************************************************************/
int BasicStats(PWB_ANALYSE_SERIES_TS *ts, char *prefix, int header_mode)
{	 
	int j;
	double old_v;
	int i;
	int f;
	int n;
	int n_seq;

	char  hdr1[16];
	char  hdr2[16];
	char  hdr3[16];
	char  hdr4[16];	
	char  hdr5[16];
	char  hdr6[16];

	if ( header_mode )
	{
		sprintf(hdr1, "%s_%s", prefix, "N"); 
		sprintf(hdr2, "%s_%s", prefix, "MIN"); 		
		sprintf(hdr3, "%s_%s", prefix, "MAX"); 	
		sprintf(hdr4, "%s_%s", prefix, "MEAN"); 
		sprintf(hdr5, "%s_%s", prefix, "VAR"); 		
		sprintf(hdr6, "%s_%s", prefix, "SD"); 		
			
		fprintf(stdout,
		"%-15s; %-15s; %-15s; %-15s; %-15s; %-15s; ",
		hdr1,
		hdr2,		
		hdr3,		
		hdr4,		
		hdr5,		
		hdr6); 
			
		return 0;
	}
		
	/* Entry test */
	if ( ts == NULL )
	{
		fprintf(stderr, "(BasicStats) Error:NULL ts or NULL ts->v\n");
		return -1;
	}		

	if ( ts->v != NULL && ts->n_v > 1 )
	{
		gsl_stats_minmax(&(ts->min), &(ts->max), ts->v, 1, ts->n_v);
		ts->mean = gsl_stats_mean (ts->v, 1, ts->n_v);
		ts->var = gsl_stats_variance_m (ts->v, 1, ts->n_v, ts->mean); 
		ts->sd = sqrt(ts->var); 	
		
		fprintf(stdout,
		"%-15d; %-15lf; %-15lf; %-15lf; %-15lf; %-15lf; ",	
		ts->n_v,
		ts->min,
		ts->max,
		ts->mean,		
		ts->var,
		ts->sd);		
	}
	else
	{
		fprintf(stdout,
		"%-15s; %-15s; %-15s; %-15s; %-15s; %-15s; ",	
		"n.a.",
		"n.a.",
		"n.a.",
		"n.a.",
		"n.a.",
		"n.a.");		
	}
		
	return 0;
}


#endif


/*****************************************************************************
* @@ PredSignalError                                                  	     *
* Rmk...: calculate the error of the predicited signals as Thomas Hellstrom- *
*          "Data Snooping in the stock market "                              *
*                ...            ...     ...                                  *
* Date..: ABR/2005                                                           * 
*         OCT/2007  -reformating output                                      * 
* Author.: Fabio Daros de Freitas.                                           *
*****************************************************************************/
int PredSignalError(
PWB_ANALYSE_SERIES_TS *tgt,PWB_ANALYSE_SERIES_TS *out, char *prefix, int header_mode)
{	 
	int i;
	char *id;

	/* counters */
	int h         = 0;	/* 		r * r^  > 0					*/
	int h_nz      = 0;	/* 		r * r^  !=  0				*/
	int h_up      = 0;	/* 		r > 0 AND r^ > 0			*/
	int h_dn      = 0;	/* 		r < 0 AND r^ < 0			*/	
	
	int r_up = 0;		/* 		r > 0 						*/
	int r_dn = 0;		/* 		r < 0 						*/
	int r_eq = 0;		/* 		r == 0 						*/

	int pr_up = 0;		/* 		r^  > 0 					*/
	int pr_dn = 0;		/* 		r^  > 0 					*/
	int pr_eq = 0;		/* 		r^  > 0 					*/

	double hr     = 0.0;		/* 	h / h_nz  				*/
	double hr_up  = 0.0;		/* 	h_up / pr_up			*/	
	double hr_dn  = 0.0;		/* 	h_dn / pr_dn			*/	

	char  hdr1[16];
	char  hdr2[16];
	char  hdr3[16];
	char  hdr4[16];	
	char  hdr5[16];
	char  hdr6[16];
	char  hdr7[16];
	char  hdr8[16];
	char  hdr9[16];
	char  hdr10[16];	
	char  hdr11[16];
	char  hdr12[16];
	char  hdr13[16];
	char  hdr14[16];

	if ( header_mode )
	{
		if ( prefix != NULL )
		{
			sprintf(hdr1, "%s_%s", prefix, "N"); 
			sprintf(hdr2, "%s_%s", prefix, "HR"); 		
			sprintf(hdr3, "%s_%s", prefix, "HR+"); 	
			sprintf(hdr4, "%s_%s", prefix, "HR-"); 
			sprintf(hdr5, "%s_%s", prefix, "H"); 		
			sprintf(hdr6, "%s_%s", prefix, "Hnz"); 		
			sprintf(hdr7, "%s_%s", prefix, "H+"); 
			sprintf(hdr8, "%s_%s", prefix, "H-"); 		
			sprintf(hdr9, "%s_%s", prefix, "TGT+"); 	
			sprintf(hdr10,"%s_%s", prefix, "TGT-"); 
			sprintf(hdr11,"%s_%s", prefix, "TGTz"); 		
			sprintf(hdr12,"%s_%s", prefix, "OUT+"); 		
			sprintf(hdr13,"%s_%s", prefix, "OUT-"); 		
			sprintf(hdr14,"%s_%s", prefix, "OUTz"); 		
		}
		else
		{
			sprintf(hdr1, "%s", "N"); 
			sprintf(hdr2, "%s", "HR");  	 
			sprintf(hdr3, "%s", "HR+");  
			sprintf(hdr4, "%s", "HR-"); 
			sprintf(hdr5, "%s", "H");		 
			sprintf(hdr6, "%s", "Hnz"); 	 
			sprintf(hdr7, "%s", "H+"); 
			sprintf(hdr8, "%s", "H-");  	 
			sprintf(hdr9, "%s", "TGT+");   
			sprintf(hdr10,"%s", "TGT-"); 
			sprintf(hdr11,"%s", "TGTz");  	 
			sprintf(hdr12,"%s", "OUT+"); 	 
			sprintf(hdr13,"%s", "OUT-"); 	 
			sprintf(hdr14,"%s", "OUTz"); 	 
		}
		
	
		fprintf(stdout,
		"%-15s; %-15s; %-15s; %-15s; %-15s; %-15s; %-15s; %-15s; %-15s; %-15s; %-15s; %-15s; %-15s; %-15s; ",
		hdr1,
		hdr2,		
		hdr3,		
		hdr4,
		hdr5,		
		hdr6,		
		hdr7,		
		hdr8,		
		hdr9,		
		hdr10,		
		hdr11,				
		hdr12,				
		hdr13,				
		hdr14);

		return 0;
	}

	/* Entry test */
	if ( tgt== NULL || out==NULL )
	{
		fprintf(stderr, "(PredSignalError) Error:NULL ts\n");
		return -1;
	}		

	if ( tgt->v != NULL && out->v != NULL )
	{
		id = malloc(PWB_ANALYSE_SERIES_FNAMESIZE); 
		if ( id == NULL ) return -2;
		sprintf(id, "%s_PSE", tgt->desc); 

		for ( i = 0; i < out->n_v; i++ )
		{
			/* counters */
			if      ( tgt->v[i] > 0.0 )	r_up++;
			else if ( tgt->v[i] < 0.0 )	r_dn++;
			else                        r_eq++;		

			if      ( out->v[i] > 0.0 )	pr_up++;
			else if ( out->v[i] < 0.0 )	pr_dn++;
			else                        pr_eq++;		

			/* Hits */

			if ( tgt->v[i] * out->v[i] > 0.0 )			h++;
			if ( tgt->v[i] * out->v[i] != 0.0 )			h_nz++;

			if ( tgt->v[i] > 0.0 && out->v[i] >  0.0 )	h_up++;		
			if ( tgt->v[i] < 0.0 && out->v[i] <  0.0 )	h_dn++;						
		}

		if ( h_nz > 0 )
			hr     = (double) h    / (double) h_nz ;
		else
			hr     = 0.0 ;

		if ( pr_up > 0 )		
			hr_up  = (double) h_up / (double) pr_up;
		else
			hr_up = 0.0;		

		if ( pr_dn > 0 )
			hr_dn  = (double) h_dn / (double) pr_dn;
		else
			hr_dn = 0.0;		


		fprintf(stdout,
		"%-15d; %-15f; %-15f; %-15f; %-15d; %-15d; %-15d; %-15d; %-15d; %-15d; %-15d; %-15d; %-15d; %-15d; ",	

		out->n_v,

		hr,
		hr_up,
		hr_dn,

		h,
		h_nz,
		h_up,
		h_dn,

		r_up,
		r_dn,
		r_eq,

		pr_up,
		pr_dn,
		pr_eq
		);
	}
	else
	{
		fprintf(stdout,
		"%-15s; %-15s; %-15s; %-15s; %-15s; %-15s; %-15s; %-15s; %-15s; %-15s; %-15s; %-15s; %-15s; %-15s; ",	

		"n.a.",

		"n.a.",
		"n.a.",
		"n.a.",

		"n.a.",
		"n.a.",
		"n.a.",
		"n.a.",

		"n.a.",
		"n.a.",
		"n.a.",

		"n.a.",
		"n.a.",
		"n.a."
		);
	}
	
	return 0;
}



/*****************************************************************************
* @@ HR                                                  	     *
* Rmk...: calculate the error of the predicited signals as Thomas Hellstrom- *
*          "Data Snooping in the stock market "                              *
*                ...            ...     ...                                  *
* Date..: ABR/2005                                                           * 
*         OCT/2007  -reformating output                                      * 
* Author.: Fabio Daros de Freitas.                                           *
*****************************************************************************/
double HR(PWB_ANALYSE_SERIES_TS *tgt,PWB_ANALYSE_SERIES_TS *out)
{	 
	int i;
	char *id;

	/* counters */
	int h         = 0;	/* 		r * r^  > 0					*/
	int h_nz      = 0;	/* 		r * r^  !=  0				*/
	int h_up      = 0;	/* 		r > 0 AND r^ > 0			*/
	int h_dn      = 0;	/* 		r < 0 AND r^ < 0			*/	
	
	int r_up = 0;		/* 		r > 0 						*/
	int r_dn = 0;		/* 		r < 0 						*/
	int r_eq = 0;		/* 		r == 0 						*/

	int pr_up = 0;		/* 		r^  > 0 					*/
	int pr_dn = 0;		/* 		r^  > 0 					*/
	int pr_eq = 0;		/* 		r^  > 0 					*/

	double hr     = 0.0;		/* 	h / h_nz  				*/
	double hr_up  = 0.0;		/* 	h_up / pr_up			*/	
	double hr_dn  = 0.0;		/* 	h_dn / pr_dn			*/	


	/* Entry test */
	if ( tgt== NULL || out==NULL )
	{
		fprintf(stderr, "(PredSignalError) Error:NULL ts\n");
		return -1;
	}		

	if ( tgt->v != NULL && out->v != NULL )
	{
		id = malloc(PWB_ANALYSE_SERIES_FNAMESIZE); 
		if ( id == NULL ) return -2;
		sprintf(id, "%s_PSE", tgt->desc); 

		for ( i = 0; i < out->n_v; i++ )
		{
			/* counters */
			if      ( tgt->v[i] > 0.0 )	r_up++;
			else if ( tgt->v[i] < 0.0 )	r_dn++;
			else                        r_eq++;		

			if      ( out->v[i] > 0.0 )	pr_up++;
			else if ( out->v[i] < 0.0 )	pr_dn++;
			else                        pr_eq++;		

			/* Hits */

			if ( tgt->v[i] * out->v[i] > 0.0 )			h++;
			if ( tgt->v[i] * out->v[i] != 0.0 )			h_nz++;

			if ( tgt->v[i] > 0.0 && out->v[i] >  0.0 )	h_up++;		
			if ( tgt->v[i] < 0.0 && out->v[i] <  0.0 )	h_dn++;						
		}

		if ( h_nz > 0 )
			hr     = (double) h    / (double) h_nz ;
		else
			hr     = 0.0 ;

		if ( pr_up > 0 )		
			hr_up  = (double) h_up / (double) pr_up;
		else
			hr_up = 0.0;		

		if ( pr_dn > 0 )
			hr_dn  = (double) h_dn / (double) pr_dn;
		else
			hr_dn = 0.0;		

	}
		
	return hr;
}



/*****************************************************************************
* @@ HR                                                  	     *
* Rmk...: calculate the error of the predicited signals as Thomas Hellstrom- *
*          "Data Snooping in the stock market "                              *
*                ...            ...     ...                                  *
* Date..: ABR/2005                                                           * 
*         OCT/2007  -reformating output                                      * 
* Author.: Fabio Daros de Freitas.                                           *
*****************************************************************************/
double HR_UP(PWB_ANALYSE_SERIES_TS *tgt,PWB_ANALYSE_SERIES_TS *out)
{	 
	int i;
	char *id;

	/* counters */
	int h         = 0;	/* 		r * r^  > 0					*/
	int h_nz      = 0;	/* 		r * r^  !=  0				*/
	int h_up      = 0;	/* 		r > 0 AND r^ > 0			*/
	int h_dn      = 0;	/* 		r < 0 AND r^ < 0			*/	
	
	int r_up = 0;		/* 		r > 0 						*/
	int r_dn = 0;		/* 		r < 0 						*/
	int r_eq = 0;		/* 		r == 0 						*/

	int pr_up = 0;		/* 		r^  > 0 					*/
	int pr_dn = 0;		/* 		r^  > 0 					*/
	int pr_eq = 0;		/* 		r^  > 0 					*/

	double hr     = 0.0;		/* 	h / h_nz  				*/
	double hr_up  = 0.0;		/* 	h_up / pr_up			*/	
	double hr_dn  = 0.0;		/* 	h_dn / pr_dn			*/	


	/* Entry test */
	if ( tgt== NULL || out==NULL )
	{
		fprintf(stderr, "(PredSignalError) Error:NULL ts\n");
		return -1;
	}		

	if ( tgt->v != NULL && out->v != NULL )
	{
		id = malloc(PWB_ANALYSE_SERIES_FNAMESIZE); 
		if ( id == NULL ) return -2;
		sprintf(id, "%s_PSE", tgt->desc); 

		for ( i = 0; i < out->n_v; i++ )
		{
			/* counters */
			if      ( tgt->v[i] > 0.0 )	r_up++;
			else if ( tgt->v[i] < 0.0 )	r_dn++;
			else                        r_eq++;		

			if      ( out->v[i] > 0.0 )	pr_up++;
			else if ( out->v[i] < 0.0 )	pr_dn++;
			else                        pr_eq++;		

			/* Hits */

			if ( tgt->v[i] * out->v[i] > 0.0 )			h++;
			if ( tgt->v[i] * out->v[i] != 0.0 )			h_nz++;

			if ( tgt->v[i] > 0.0 && out->v[i] >  0.0 )	h_up++;		
			if ( tgt->v[i] < 0.0 && out->v[i] <  0.0 )	h_dn++;						
		}

		if ( h_nz > 0 )
			hr     = (double) h    / (double) h_nz ;
		else
			hr     = 0.0 ;

		if ( pr_up > 0 )		
			hr_up  = (double) h_up / (double) pr_up;
		else
			hr_up = 0.0;		

		if ( pr_dn > 0 )
			hr_dn  = (double) h_dn / (double) pr_dn;
		else
			hr_dn = 0.0;		

	}
		
	return hr_up;
}



/*****************************************************************************
* @@ HR                                                  	     *
* Rmk...: calculate the error of the predicited signals as Thomas Hellstrom- *
*          "Data Snooping in the stock market "                              *
*                ...            ...     ...                                  *
* Date..: ABR/2005                                                           * 
*         OCT/2007  -reformating output                                      * 
* Author.: Fabio Daros de Freitas.                                           *
*****************************************************************************/
double HR_DN(PWB_ANALYSE_SERIES_TS *tgt,PWB_ANALYSE_SERIES_TS *out)
{	 
	int i;
	char *id;

	/* counters */
	int h         = 0;	/* 		r * r^  > 0					*/
	int h_nz      = 0;	/* 		r * r^  !=  0				*/
	int h_up      = 0;	/* 		r > 0 AND r^ > 0			*/
	int h_dn      = 0;	/* 		r < 0 AND r^ < 0			*/	
	
	int r_up = 0;		/* 		r > 0 						*/
	int r_dn = 0;		/* 		r < 0 						*/
	int r_eq = 0;		/* 		r == 0 						*/

	int pr_up = 0;		/* 		r^  > 0 					*/
	int pr_dn = 0;		/* 		r^  > 0 					*/
	int pr_eq = 0;		/* 		r^  > 0 					*/

	double hr     = 0.0;		/* 	h / h_nz  				*/
	double hr_up  = 0.0;		/* 	h_up / pr_up			*/	
	double hr_dn  = 0.0;		/* 	h_dn / pr_dn			*/	


	/* Entry test */
	if ( tgt== NULL || out==NULL )
	{
		fprintf(stderr, "(PredSignalError) Error:NULL ts\n");
		return -1;
	}		

	if ( tgt->v != NULL && out->v != NULL )
	{
		id = malloc(PWB_ANALYSE_SERIES_FNAMESIZE); 
		if ( id == NULL ) return -2;
		sprintf(id, "%s_PSE", tgt->desc); 

		for ( i = 0; i < out->n_v; i++ )
		{
			/* counters */
			if      ( tgt->v[i] > 0.0 )	r_up++;
			else if ( tgt->v[i] < 0.0 )	r_dn++;
			else                        r_eq++;		

			if      ( out->v[i] > 0.0 )	pr_up++;
			else if ( out->v[i] < 0.0 )	pr_dn++;
			else                        pr_eq++;		

			/* Hits */

			if ( tgt->v[i] * out->v[i] > 0.0 )			h++;
			if ( tgt->v[i] * out->v[i] != 0.0 )			h_nz++;

			if ( tgt->v[i] > 0.0 && out->v[i] >  0.0 )	h_up++;		
			if ( tgt->v[i] < 0.0 && out->v[i] <  0.0 )	h_dn++;						
		}

		if ( h_nz > 0 )
			hr     = (double) h    / (double) h_nz ;
		else
			hr     = 0.0 ;

		if ( pr_up > 0 )		
			hr_up  = (double) h_up / (double) pr_up;
		else
			hr_up = 0.0;		

		if ( pr_dn > 0 )
			hr_dn  = (double) h_dn / (double) pr_dn;
		else
			hr_dn = 0.0;		

	}
		
	return hr_dn;
}


#ifdef EXEC

/*****************************************************************************
* @@ NornmalityTest                                                  	     *
* Rmk...: calculate the normality test of the series                         *
*                ...            ...     ...                                  *
* Date..: NOV/2007                                                           * 
* Author.: Fabio Daros de Freitas.                                           *
*****************************************************************************/
int NormalityTest(PWB_ANALYSE_SERIES_TS *ts, char *prefix, int header_mode)
{	 
	char  hdr1[16];
	char  hdr2[16];
	char  hdr3[16];

	PWB_PDF_CHISQGOF gof1;
	PWB_PDF_CHISQGOF gof2;
	PWB_PDF_CHISQGOF gof3;
	double *v;
	int n;
	double mean;
	double sd;


	if ( header_mode )
	{
		if ( prefix != NULL )
		{
			sprintf(hdr1, "%s_%s", prefix, "Norm(0.01)"); 
			sprintf(hdr2, "%s_%s", prefix, "Norm(0.05)"); 		
			sprintf(hdr3, "%s_%s", prefix, "Norm(0.10)"); 	
		}
		else
		{
			sprintf(hdr1, "%s", "Norm(0.01)"); 
			sprintf(hdr2, "%s", "Norm(0.05)"); 		
			sprintf(hdr3, "%s", "Norm(0.10)"); 	
		}

	
		fprintf(stdout,
		"%-15s; %-15s; %-15s; ",
		hdr1,
		hdr2,		
		hdr3);

		return 0;
	}

	/* Entry test */
	if ( ts==NULL )
	{
		fprintf(stderr, "(NormalityTest) Error:NULL ts\n");
		return -1;
	}		

	if ( ts->v != NULL )
	{	
		v = ts->v;
		n = ts->n_v;
		mean = ts->mean;
		sd = ts->sd;

		gof1.alpha = 0.01;
		gof2.alpha = 0.05;
		gof3.alpha = 0.10;

		gof1.H0 =
		ChiSquareGOFTestNormal(v, n, mean, sd, &gof1);

		gof2.H0 =
		ChiSquareGOFTestNormal(v, n, mean, sd, &gof2);

		gof3.H0 =
		ChiSquareGOFTestNormal(v, n, mean, sd, &gof3);

		fprintf(stdout,
		"%-15d; %-15d; %-15d; ",	

		gof1.H0,
		gof2.H0,		
		gof3.H0);
	}
	else
	{
		fprintf(stdout,
		"%-15s; %-15s; %-15s; ",	

		"n.a.",
		"n.a.",
		"n.a.");
	}
	
	return 0;
}



/*****************************************************************************
* @@ ChiSquareGOFTestNormal                                    	             *
* Rmk...: Chi-square goodness-of-fit test against a Normal distribution      *
*                                                                            *
*         Implemented as in NIST ENGINEERING STATISTICS HANDBOOK             *
*                                                                            *
*         http://www.itl.nist.gov/div898/handbook/eda/section3/eda35f.htm    *
*                                                                            *
*         Aditonal references:                                               *
*         1 - Papoulis, "Probability, Random Variables,                      *
*         and Stochastic Processes", 3rd ed., 1991, pag 273                  *
*                                                                            *
*         2 - Paul L. Meyer, "Probabilidade - aplicacoes a estatistica"      *
*         2rd ed., 1984 pag 387                                              *
*                                                                            *
*                                                                            *
* Date..: NOV/2004                                                           * 
*         ABR/2005                                                           * 
* Author.: Fabio Daros de Freitas.                                           *
*****************************************************************************/
int
ChiSquareGOFTestNormal(double *y, int n, double mu, double sigma, PWB_PDF_CHISQGOF *gof)
{	 
	int i;
	gsl_histogram *h;
	int k;	
	double x_min, x_max; 
	double O, E, N;
	double O_, E_;
	
	double x_l, x_u;

	double dof;
	int k_eff;	

	double X_2_critical_value;
	double X_2_test_statistic;
	
	int res;


	/* find some stats... */
	x_min = x_max = 0.0;
	gsl_stats_minmax (&(x_min), &(x_max), y, 1, n);


	/* Find N_BIN? */

	h = CreateHistogram(y, n, mu, sigma, x_min, x_max, 0, 0.0);

	k = gsl_histogram_bins (h);		

	/* calculate X_2 */
	X_2_test_statistic = 0.0; 
	N = (double) n;
	k_eff = 0;
	
	O_ = 0.0 ;	
	E_ = 0.0 ;

	for ( i = 0; i < k; i++ )	{	

		if ( ! gsl_histogram_get_range(h, i, &x_l, &x_u) )	{

			/* Observed count for bin i in data */
			O = gsl_histogram_get(h, i); 

			/* Expected  count for bin i in Normal Distribuution */
			/* E = N Prob ( x_low <= X <= x_upper ) */
			/* 	gsl_cdf_gaussian_P() calculate low tail probability,
			     P(x) = \int_{-\infty}^{x} dx' p(x') 
			*/ 

			E = N *
				(
				gsl_cdf_gaussian_P(x_u - mu, sigma) - 
				gsl_cdf_gaussian_P(x_l - mu, sigma)	
				);

			/* acumulate frequencies (bins), if needed, untill E >= 5 */
			O_ += O; 
			E_ += E;

//fprintf(stderr, "O>>i=%d WILL E=%lf O=%lf E_=%lf O_=%lf\n", i, E, O, E_, O_);

			if ( (int) E_ >= 5 )	{

				X_2_test_statistic += pow((O_ - E_), 2.0)/E_;	
				
				/* for calculate DOF: number of non-empty cells, and E <= 5 */
				if ( (int) O_ > 0 ) k_eff++;


//fprintf(stderr, "O>>O_=%lf k_eff=%d\n", O_, k_eff);

				
				/* reset accumulated frequencies */
				E_ = 0.0;
				O_ = 0.0;								
			}			
		}			
	}

	/* NIST HANDBOOK!! 
	    Calculate Degrees of Freedom: dof = k_eff - c
		k_eff = number of non-empty cells 
		c = 1 + number of estimated parameters (including location and scale
		parameters and shape parameters) for distribution.
		Example: For a 3 parameter Weibull distribution, c == 4;
		For a 2-parameters Normal Distro, c == 3 
	
		References [1] and [2] states simply dof = k - 1 	
	 */

	/* From the paper
	   "START - The Chi-Square: a Large-Sample Goodness of Fit Test"
	   http://rac.alionscience.com/pdf/Chi_Square.pdf
	   I understood that the dof must be clipper in zero!
     */ 

	/* From the Nist.gov DATAPLOT source code (dp10.f)

...

C
C               ****************************************
C               **  STEP 4.1--                        **
C               **  COMPUTE CHI-SQUARE OBSERVED AND   **
C               **  EXPECTED                          **
C               ****************************************
C
 1100 CONTINUE
C
      DSUM1=0.0D0
      CLWIDT=X2(2)-X2(1)
      NUMPAR=0
      NEMPTY=0
C
C  NOTE: HANDLE DISCRETE DISTRIBUTIONS DIFFERENTLY.  FOR DISCRETE,
C        BIN MID-POINTS SHOULD BE AT INTEGER VALUES!
C  NOTE: A FEW DISCRETE DISTRIBUTIONS ARE COMPUTED BY BRUTE
C        FORCE.  FOR THESE (ESPECIALLY WHEN THE TAILS CAN BE
C        LONG), INSTEAD OF COMPUTING CDF FUNCTION, KEEP RUNNING
C        TALLY BASED ON CDF FUNCTION.  THIS AVOIDS EXCESIVE
C        REDUNDANT COMPUTATION.
C
      DCDFL=0.0D0
      DCDFU=0.0D0
      IXLPV=-1
      IXUPV=-1
C
      IF(IDISFL.EQ.'DISC')THEN
        IF(IDATSW.EQ.'CLAS')THEN
          IDIFF=0
        ELSE
          ADIFF=X2(2) - X2(1)
          IDIFF=INT(ADIFF+0.1)
        ENDIF
      ENDIF
C
      DO1199I=1,N2                     // MAIN LOOP 
C
        IF(IDISFL.EQ.'DISC')THEN
          IF(IDATSW.EQ.'CLAS')THEN
            IXL=CEIL(X2(I))
CCCCC       IXU=FLOOR(Z2(I))
C
            ARG1=Z2(I)
            IF(ARG1.LE.0.0)THEN
              IARG2=INT(ARG1)
              ARG3=REAL(IARG2)
              ARG4=ARG1-ARG3
              TERM=ARG3
              IF(ARG4.NE.0.0)TERM=TERM-1.0
            ELSE
              IARG2=INT(ARG1)
              TERM=REAL(IARG2)
            ENDIF
            IXU=INT(TERM)
C
          ELSE
            IXU=INT(X2(I) + 0.5)
            IXL=IXU-IDIFF
          ENDIF
        ELSE
          IF(IDATSW.EQ.'CLAS')THEN
            XL=X2(I)
            XU=Z2(I)
          ELSE
            XL=X2(I) - CLWIDT/2.0
            XU=X2(I) + CLWIDT/2.0
          ENDIF
        ENDIF
        XMIN=X2(1) - CLWIDT/2.0
C
C  FOLLOWING ARE DISCRETE DISTRIBUTIONS
C
        IF(ICASPL.EQ.'BIPP')GOTO1290

...

C
C  FOLLOWING ARE CONTINUOUS DISTRIBUTIONS
C
        IF(ICASPL.EQ.'UNPP')GOTO1110
...

C
 1120   CONTINUE
        IDIST='NORMAL'
        NUMPAR=2
        XL=(XL-CHSLOC)/CHSCAL
        XU=(XU-CHSLOC)/CHSCAL
        IF(CHSLOC.EQ.0.0)NUMPAR=NUMPAR-1
        IF(CHSCAL.EQ.1.0)NUMPAR=NUMPAR-1
        CALL NORCDF(XL,XOUT1)
        CALL NORCDF(XU,XOUT2)
        GOTO2990
C
...

 2990 CONTINUE
C
CCCCC IF(I.EQ.1)THEN
CCCCC   D2(I)=XOUT2
CCCCC ELSEIF(I.EQ.N2)THEN
CCCCC   D2(I)=1.0-XOUT1
CCCCC ELSE
        D2(I)=XOUT2-XOUT1
CCCCC ENDIF
C
      IF(D2(I).EQ.0.0)THEN
        NEMPTY=NEMPTY+1
        GOTO1199
      ENDIF
      D2(I)=D2(I)*AN
C
      DTEMP1=DBLE(Y2(I))
      DTEMP2=DBLE(D2(I))
      IF(DTEMP2.GT.0.0D0)DSUM1=DSUM1 + (DTEMP1-DTEMP2)**2/DTEMP2
C
 1199 CONTINUE                                // MAIN LOOP END 
C
      N2=N2-NEMPTY
      STAT=REAL(DSUM1)
      IDF=N2-NUMPAR-1
      IF(IDF.LT.1)THEN
        WRITE(ICOUT,999)
        CALL DPWRST('XXX','BUG ')
        WRITE(ICOUT,2011)
 2011   FORMAT('***** ERROR DPCHS2--NON-POSITIVE DEGREES OF FREEDOM')
        CALL DPWRST('XXX','BUG ')
        WRITE(ICOUT,2013)N2
 2013   FORMAT('      NUMBER OF CELLS       = ',I8)
        CALL DPWRST('XXX','BUG ')
        WRITE(ICOUT,2015)NUMPAR
 2015   FORMAT('      NUMBER OF PARAMETERS  = ',I8)
        CALL DPWRST('XXX','BUG ')
        WRITE(ICOUT,2017)IDF
 2017   FORMAT('      DEGREES OF FREEDOM    = ',I8)
        CALL DPWRST('XXX','BUG ')
        IERROR='YES'
        GOTO9000
      ENDIF
C
      CALL CHSCDF(STAT,IDF,CDF)
C
      ICONC1='REJECT'
      ICONC2='REJECT'
      ICONC3='REJECT'
C
      STATVA=STAT
      STATCD=CDF
      STATNU=IDF
      CALL CHSPPF(.90,IDF,CUTU90)
      CALL CHSPPF(.95,IDF,CUTU95)
      CALL CHSPPF(.99,IDF,CUTU99)
C
      IF(STATVA.LE.CUTU90)ICONC1='ACCEPT'
      IF(STATVA.LE.CUTU95)ICONC2='ACCEPT'
      IF(STATVA.LE.CUTU99)ICONC3='ACCEPT'
C
      IF(IWRIT2.EQ.'OFF')GOTO9000
C
C               *******************************
C               **   STEP 32--               **
C               **   WRITE OUT EVERYTHING    **
C               **   FOR A CHI-SQUARED TEST  **
C               *******************************
C


...

>> Did not found the expected value comparison with 5 in code!!!!s
>> k (N2) is k - number of empty cells 
>> Rise error if calculated dof < 1 


	*/

	dof = (double)(k_eff - 2 - 1);  /* k of non-empty cells - number of param (Normal) - 1 */

	if ( gof != NULL )	{
		gof->k     = k;
		gof->k_eff = k_eff;
		gof->dof   =  dof;		
	}
		
	if ( dof < 1.0 )	{ 

		fprintf(stderr, 
		"(ChiSquareGOFTestNormal) FATAL ERROR! dof=%lf < 1 (k=%d k_eff=%d)\n",
		dof, k, k_eff); 
		return -1;
	}	
	
	
	X_2_critical_value = gsl_cdf_chisq_Pinv(1.0-gof->alpha, dof);  	


	if ( X_2_test_statistic > X_2_critical_value )	{
	
		/* reject null hypothesis that data is Normal(mu, sigma) */
		res = 0;	
	}
	else
		/* accept null hypothesis that data is Normal(mu, sigma) */	
		res = 1;


	if ( gof != NULL )	{

		gof->X_2_test_statistic = X_2_test_statistic;
		gof->X_2_critical_value = X_2_critical_value;
		gof->H0 = res;		
	}

/*

	fprintf(stdout, 
	"Chisq test for alpha=%lf dof=%lf: test_stat=%lf critical_value=%lf H0=[%s]\n",
	alpha, dof, X_2, X_2_critical_value, (res?"ACCEPT":"REJECT"));
*/

	return res;
}

/*****************************************************************************
* @@ CreateHistogram                                         	             *
* Rmk...: Create a (GSL) Histogram with adequate bin choice algorithm        *
*         Bin are choosen with central bin at mean value by:                 *
*         1: k == 0 && w == 0: calculate k and w;                            *
*         2: k == 0 && w >  0: calculate k;                                  *
*         3: k > 0  && w == 0: calculate w;                                  *
*         4: k > 0  && w >  0: NO ALLOWED - treat as case 1)                 *
*                                                                            *
* Date..: ABR/2005                                                           * 
* Author.: Fabio Daros de Freitas.                                           *
*****************************************************************************/
gsl_histogram *CreateHistogram(
double *y, int n, double mu, double sigma, double x_min, double x_max,
int k, double w)
{	 

/* -- Text from NIST Statistics Handbook, Chi-Square Goodness of Fit test --  
 (http://www.itl.nist.gov/div898/handbook/)
 
 This test is sensitive to the choice of bins.
 There is no optimal choice for the bin width (since the optimal bin width
 depends on the distribution). Most reasonable choices should produce similar,
 but not identical, results. Dataplot uses 0.3*s, where s is the sample standard
 deviation, for the class width. The lower and upper bins are at the sample mean
 plus and minus 6.0*s, respectively. For the chi-square approximation to be
 valid, the expected frequency should be at least 5. This test is not
 valid for small samples, and if some of the counts are less than five, you
 may need to combine some bins in the tails.
 ---

	I cannot understand this (yet). This leadsto (6 + 1 + 6) / 0.3 
	bins ==  43 bins!  


 */
 
 /* The other method is the Chi-Square test number of classes criteria
 (also in NISt manual *
		k = (int) ceil ( 2.0 * pow((double)n_data, 2.0/5.0) ); 

*/
 

/* Additionally, we want to use an odd number of bins with center bin 
   "centered" o sample mean
*/   


	gsl_histogram *h;
	double xmin, xmax;
	int more_k;
	int i;
	
	
	/*== Calculate mean centered xmin and xmax ==*/
	
	/* k == 0 && w == 0 : Calculate k for default w, centered on mean value */
	/* k >  0 && w >  0 : Calculate k for default w, centered on mean value */
	/* k == 0 && w >  0 : Calculate k for w,  centered on mean value */
	
	if ( (k == 0 && doubleeqcmp(w, 0.0, 0.0001)) || w > 0.0 )	{


		if ( doubleeqcmp(w, 0.0, 0.0001) )
			w = 0.3 * sigma;	

		xmin = 0.0;
		xmax = 0.0;
		k = 0;
		more_k = 1;

		while ( more_k )	{

			more_k = 0; 

			if ( k == 0 )	{
				xmin = mu - w/2.0; 
				xmax = mu + w/2.0; 
				k++;
				more_k = 1;
			}
			else	{

				if ( xmin > x_min )	{
					//xmin -= w;
					k++;
					more_k = 1;				
				}
				xmin -= w;
				
				if ( xmax < x_max )	{
					//xmax += w;
					k++;
					more_k = 1;				
				}
				xmax += w;						
			}		
		}
	}
	
	/* k > 0 && w == 0 : Calculate w for k,  centered on mean value */
	else if ( k > 0 && doubleeqcmp(w, 0.0, 0.0001) )	{

		w = (x_max - x_min) / (double) k;					
		xmin = 0.0;
		xmax = 0.0;

		for ( more_k = 0; more_k < k; more_k++ )	{

			if ( more_k == 0 )	{
				xmin = mu - w/2.0; 
				xmax = mu + w/2.0; 
			}
			else	{

				if ( xmin > x_min )	{
					xmin -= w;
				}			
				if ( xmax < x_max )	{
					xmax += w;
				}				
			}		
		}
	}

	
//fprintf(stderr, "# >> k=%d x_min=%lf xmin=%lf x_max=%lf xmax=%lf\n",
//k, x_min, xmin, x_max, xmax); 	
	
	/* make histogram for data */
    h = gsl_histogram_alloc (k);
	if ( h == NULL ) return h;

    gsl_histogram_set_ranges_uniform (h, xmin, xmax);

	for ( i = 0; i < n; i++ ) gsl_histogram_increment (h, y[i]);
	
	return h;
}




#if 0

/*****************************************************************************
* @@ ReadSeriesTT                                                   	     *
* Rmk...: read a combo pred_tt generated series                              *
*                                                                            *
*        Format: TGT, OUT, ERROR                                             *
*                ...            ...     ...                                  *
*               ts = TGT series until the start of OUT and ERROR series      *
*                    i.e., this is the trainning series used for the first   *
*                    predictions of sequence in pred_tt. In data file,       *
*                    the lines with this series contains only the first col. *
*                                                                            *
*               ts2= TGT series after the first ts (while TGT and ERR series)*
*                                                                            *
*               ts3= OUT series (prediction series) at column 2              *
*                                                                            *
*               ts4= ERROR series (TGT - OUT series) at column 3             *
*                ...            ...     ...                                  *
*                ...            ...     ...                                  *
* Date..: MAR/2005 (adapted from PWB_PRED code)                              * 
* Author.: Fabio Daros de Freitas.                                           *
*****************************************************************************/
int ReadSeriesTT(char *fname,
                   PWB_ANALYSE_SERIES_TS *ts1, PWB_ANALYSE_SERIES_TS *ts2,
				   PWB_ANALYSE_SERIES_TS *ts3,PWB_ANALYSE_SERIES_TS *ts4)
{	 
	char *b;
	int i, j, n, nlines, pos;
	char line[1024];
	char *ptr;
	char date[16];
	double v1, v2, v3, v4;
	int error;
	char id[PWB_ANALYSE_SERIES_FNAMESIZE];


	
	/* Entry test */
	if ( fname==NULL || !strlen(fname)  )
	{
		fprintf(stderr, "(ReadSeriesTT) Error:BAD fname\n");
		return -1;
	}		

	/* read series */
	b = readfile(fname, &n);
	if ( b == NULL )	{
		fprintf(stderr, "(ReadSeriesTT) Error: readfile(%s)\n",
		fname);
		return -1;	
	}

	nlines = tokeninstring(b, '\n');
		
	strcpy(id, basename(fname)); 
	ptr = strchr(id, '.'); 
	if ( ptr != NULL ) *ptr = '\x0'; 
	

	/* read series */
	ts1->v = calloc(nlines, sizeof(double)); 
	if ( ts1->v == NULL )	{
		fprintf(stderr, "(ReadSeriesTT) Error: (ts1) Cannot calloc(%d) fname=%s v - %s\n",
		nlines, fname, strerror(errno));
		free(b);
		return -1;	
	}
	sprintf(ts1->id, "%s-%s", id, "TGT_TRN") ;	
	
	ts2->v = calloc(nlines, sizeof(double)); 
	if ( ts2->v == NULL )	{
		fprintf(stderr, "(ReadSeriesTT) Error: (ts2) Cannot calloc(%d) fname=%s v - %s\n",
		nlines, fname, strerror(errno));
		free(b);
		return -1;	
	}
	sprintf(ts2->id, "%s-%s", id, "TGT_PRD") ;	
		
	ts3->v = calloc(nlines, sizeof(double)); 
	if ( ts3->v == NULL )	{
		fprintf(stderr, "(ReadSeriesTT) Error: (ts3) Cannot calloc(%d) fname=%s v - %s\n",
		nlines, fname, strerror(errno));
		free(b);
		return -1;	
	}
	sprintf(ts3->id, "%s-%s", id, "OUT_PRD") ;		
	
	ts4->v = calloc(nlines, sizeof(double)); 
	if ( ts4->v == NULL )	{
		fprintf(stderr, "(ReadSeriesTT) Error: (ts4) Cannot calloc(%d) fname=%s v - %s\n",
		nlines, fname, strerror(errno));
		free(b);
		return -1;	
	}
	sprintf(ts4->id, "%s-%s", id, "ERR_PRD") ;		

	/* Loop in file...*/
	ts1->n_v = 0;
	ts2->n_v = 0;	
	ts3->n_v = 0;	
	ts4->n_v = 0;
	
	/* remove field separators ', ' */
	strmodifychars(b, ',', ' ');	
	
	error = 0;
	pos = 0;
	j   = 0;
	for ( i = 0; i < nlines; i++ )	{

		if ( pos < strlen(b) )	{ 
			memset(line, 0, 1024); 
			pos += 1 + sgets(line, 1023, b + pos);
		}
		else
			break;

		/* reset error */
		if ( error == 1 ) 
			error = 0;

		/* Block comments */
		if ( strstr(line, "/*") )	{
			error = 2;	
		}
		else if ( strstr(line, "*/") )	{
			error = 1;
		}		
		if ( error ) continue;


		/* Data error and line comments */		

		if ( isalpha(*line) || *line == '#' || *line == '@' )	{
			error = 1;	
		}	
		
		/* Try 'date' 'day of week' 'value' (PWB PM export format) */
		else
		{

			switch ( sscanf(line, "%lf %lf %lf", &v1, &v2, &v3) )
			{			
				case 1:

					//fprintf(stdout, "%lf\n", v1); 
					
					ts1->v[ts1->n_v++] = v1;
					break;
				case 3:
					//fprintf(stdout, "%lf %lf %lf\n", v1, v2, v3); 				

					ts2->v[ts2->n_v++] = v1;
					ts3->v[ts3->n_v++] = v2;					
					ts4->v[ts4->n_v++] = v3;					
					
					break;
		
			}				
		
#if 0 		
		
			/* Try 'date' 'value' (typical format) */
			if ( sscanf(line, "%s %lf", date, &value) != 2 )	{

				/* Try 'value' (raw data format) */
				if ( sscanf(line, "%lf", &value) != 1 )
				{
					error = 1;
					// A blank line or in file generate this error!!!
					//fprintf(stderr, 
					//"(ReadSeriesTT) Error: Cannot sscanf() line %d fname[%s]\n",
					//i, fname);
				}		
			}
#endif			

		}

		ptr += pos;	
	}

	return 0;
}

#endif

#if 0

/*****************************************************************************
* @@ ReadSeriesTT_RW                                                   	     *
* Rmk...: create a random walk predictor series with the TT ones.            *
*         v(t+1) = v(t)                                                      *
*                ...            ...     ...                                  *
* Date..: MAR/2005                                                           * 
* Author.: Fabio Daros de Freitas.                                           *
*****************************************************************************/
int ReadSeriesTT_RW(PWB_ANALYSE_SERIES_TS *rw, PWB_ANALYSE_SERIES_TS *tgt_trn,
				   PWB_ANALYSE_SERIES_TS *tgt,PWB_ANALYSE_SERIES_TS *out)
{	 
	char *b;
	int i, j, n, nlines, pos;
	char line[1024];
	char *ptr;
	char date[16];
	double v1, v2, v3, v4;
	int error;

	/* create rw series */
	rw->n_v = out->n_v;
	
	rw->v = calloc(rw->n_v, sizeof(double)); 
	if ( rw->v == NULL )	{
		fprintf(stderr, "(ReadSeriesTTRW) Error: (rw) Cannot calloc(%d) - %s\n",
		rw->n_v, strerror(errno));
		return -1;	
	}
	sprintf(rw->id, "%s_%s", out->id, "RWK") ;	
	sprintf(rw->desc, "%s", "v(t) = v(t-1)") ;	

	rw->v[0] = tgt_trn->v[tgt_trn->n_v -1]; 

	for ( i = 1; i < rw->n_v; i++ )
		rw->v[i] = tgt->v[i-1]; 
	
	return 0;
}


/*****************************************************************************
* @@ ReadSeriesTT_MEAN                                                	     *
* Rmk...: create a MEAN predictor series with the TT ones.                   *
*         v(t+1) = 1/(N) SUM_i=1, i=N [ v(i) ]                               *
*                ...            ...     ...                                  *
* Date..: MAR/2005                                                           * 
* Author.: Fabio Daros de Freitas.                                           *
*****************************************************************************/
int ReadSeriesTT_MEAN(PWB_ANALYSE_SERIES_TS *mean, PWB_ANALYSE_SERIES_TS *tgt_trn,
				   PWB_ANALYSE_SERIES_TS *tgt,PWB_ANALYSE_SERIES_TS *out)
{	 
	char *b;
	int i, j, n, nlines, pos;
	int start;
	char line[1024];
	char *ptr;
	char date[16];
	double v1, v2, v3, v4;
	
	double mu;
	double *tgt_all;
	int n_tgt_all;
	
	int error;

	/* create mean series */
	mean->n_v = out->n_v;	
	mean->v = calloc(mean->n_v, sizeof(double)); 
	if ( mean->v == NULL )	{
		fprintf(stderr, "(ReadSeriesTTMEAN) Error: (mean) Cannot calloc(%d) - %s\n",
		mean->n_v, strerror(errno));
		return -1;	
	}
	sprintf(mean->id, "%s_%s", out->id, "MEAN") ;	
	sprintf(mean->desc, "%s", "v(t+1) = 1/(N) SUM_i=1, i=N [ v(i) ]");


	/* aux tgt_trn + tgt series for calculate the mean */
	n_tgt_all = tgt_trn->n_v + tgt->n_v;	
	
	tgt_all = calloc(n_tgt_all, sizeof(double)); 
	if ( mean->v == NULL )	{
		fprintf(stderr, "(ReadSeriesTTMEAN) Error: (tgt_all) Cannot calloc(%d) - %s\n",
		n_tgt_all, strerror(errno));
		return -1;	
	}

	for ( i = 0; i < n_tgt_all; i++ )	{

		if ( i < tgt_trn->n_v )
			tgt_all[i] = tgt_trn->v[i];
		else
			tgt_all[i] = tgt->v[i - tgt_trn->n_v];					
	}	


	/* predictor output */
	
	
	/* ** THIS IS TEMPORARY TO MAKE EXPERIMENTS FOR CBRN2005, WICH
	      I COMPRARE WITH pred_tt 200:4:3:1:1:1:60 ****
	*/
	
	start = mean->n_v - 200;
	if ( start < 0 ) start = 0;
	
	
	for ( i = start;  i < mean->n_v; i++ )	{

		mu = gsl_stats_mean (tgt_all, 1, n_tgt_all + i);
		mean->v[i] = mu;			
	}	
	

	if ( tgt_all != NULL )	free(tgt_all);
	
	return 0;
}

#endif

#endif


/*****************************************************************************
* @@ RepeatedValues                                               	         *
* Rmk...: verify sequences of repeated values                                *
*                                                                            *
*        value:N:i:f -> N contiguos instances value, from index i to f       *
*                ...            ...     ...                                  *
* Date..: MAR/2005                                                           * 
* Author.: Fabio Daros de Freitas.                                           *
*****************************************************************************/
int RepeatedValues(PWB_ANALYSE_SERIES_TS *ts)
{	 
	int j;
	double old_v;
	int i;
	int f;
	int n;
	int n_seq;
	
		
	/* Entry test */
	if ( ts == NULL )	{
		fprintf(stderr, "(RepeatedValues) Error:NULL ts\n");
		return -1;
	}		

	if ( ts->n_v < 2 )	{
		//fprintf(stderr, "(RepeatedValues) Error: n_values(%d) < 2\n",
		//ts->n_v);
		return 0;
	}		


	n = 0; 
	i = -1;
	f = -1;
	n_seq = 0;

	old_v = ts->v[0];

	for ( j = 1; j <= ts->n_v; j++ )	{

		if ( j != ts->n_v && doubleeqcmp(ts->v[j], old_v, 0.0000001) )  {  /* equals == 1 */
			
			if ( n == 0 )	{
				/* start...*/
				i = j-1;
				n_seq++;
			}
			/* update couters...*/			
			f = j;			
			n++; 
			
		}			
		else	{
									
			/* reset counters */
			if ( n > 0 )	{
			
				/* report sequence */
				if ( n_seq == 1 )	fprintf(stdout, "%s", ts->fname);
				fprintf(stdout, ",%lf:%d:%d:%d", old_v, i, f, n);
						
				n = 0; 
				i = -1;
				f = -1;			
			}
			if ( j < ts->n_v ) 	old_v = ts->v[j];		
		}
	}
	
	/* sequence with last value */
	
	
	if ( n_seq > 0 )	fprintf(stdout, ",%d\n", n_seq);

	return n;
}



#if 0
/*****************************************************************************
* @@ CreateAbsoluteErrorTS                                                	 *
* Rmk...: Create a series of absolute error of predictions (TGT - OUT)       *
*                ...            ...     ...                                  *
* Date..: ABR/2005                                                           * 
* Author.: Fabio Daros de Freitas.                                           *
*****************************************************************************/
PWB_ANALYSE_SERIES_TS *CreateAbsoluteErrorTS
(PWB_ANALYSE_SERIES_TS *tgt,PWB_ANALYSE_SERIES_TS *out, PWB_ANALYSE_SERIES_TS *ts,
char *id, char *desc)
{	 
	int i;

	/* Entry test */
	if ( tgt== NULL || out==NULL )	{
		fprintf(stderr, "(CreateAbsoluteErrorTS) Error:NULL ts\n");
		return NULL;
	}		

	/* create Ids */
	sprintf(ts->id, "%s_%s", out->id, id) ;	
	sprintf(ts->desc, desc) ;	


	/* create mean series */
	ts->n_v = out->n_v;	
	ts->v = calloc(ts->n_v, sizeof(double)); 
	if ( ts->v == NULL )	{
		fprintf(stderr, "(CreateAbsoluteErrorTS) Error: (ts) Cannot calloc(%d) - %s\n",
		ts->n_v, strerror(errno));
		return NULL;	
	}
	
	for ( i = 0; i < ts->n_v; i++ )	{
	
		ts->v[i] = tgt->v[i] - out->v[i];
	}

	return ts;
}

#endif

/*****************************************************************************
* @@ RMSE                                                                	 *
* Rmk...: Calculate RMSE of (TGT - OUT)                                      *
*                ...            ...     ...                                  *
* Date..: ABR/2005                                                           * 
* Author.: Fabio Daros de Freitas.                                           *
*****************************************************************************/
double RMSE(PWB_ANALYSE_SERIES_TS *tgt,PWB_ANALYSE_SERIES_TS *out)
{	 
	int i;
	double e;

	/* Entry test */
	if ( tgt== NULL || out==NULL )	{
		fprintf(stderr, "(RMSE) Error:NULL ts\n");
		return 0.0;
	}		
	
	e = 0.0;
	for ( i = 0; i < tgt->n_v; i++ )	{
		e += pow(tgt->v[i] - out->v[i], 2.0);
	}
	e = e / (double) tgt->n_v;
	e = sqrt(e);
	
	return e;
}


/*****************************************************************************
* @@ MAPE                                                                	 *
* Rmk...: Calculate MAPE of (TGT - OUT)                                      *
*                ...            ...     ...                                  *
* Date..: ABR/2005                                                           * 
* Author.: Fabio Daros de Freitas.                                           *
*****************************************************************************/
double MAPE(PWB_ANALYSE_SERIES_TS *tgt,PWB_ANALYSE_SERIES_TS *out)
{	 
	int i;
	double e;

	/* Entry test */
	if ( tgt== NULL || out==NULL )
	{
		fprintf(stderr, "(MAPE) Error:NULL ts\n");
		return 0.0;
	}		
	
	e = 0.0;
	for ( i = 0; i < tgt->n_v; i++ )
	{
	
		if ( tgt->v[i] != 0.0 )
			e += fabs( (tgt->v[i] - out->v[i]) / tgt->v[i] );
	}
	e = /* 100.0 * */  e / (double) tgt->n_v;
	
	return e;
}


/*****************************************************************************
* @@ MAPE                                                                	 *
* Rmk...: Calculate MAPE of (TGT - OUT)                                      *
*                ...            ...     ...                                  *
* Date..: ABR/2005                                                           * 
* Author.: Fabio Daros de Freitas.                                           *
*****************************************************************************/
double ME(PWB_ANALYSE_SERIES_TS *tgt,PWB_ANALYSE_SERIES_TS *out)
{	 
	int i;
	double e;

	/* Entry test */
	if ( tgt== NULL || out==NULL )
	{
		fprintf(stderr, "(MAPE) Error:NULL ts\n");
		return 0.0;
	}		
	
	e = 0.0;
	for ( i = 0; i < tgt->n_v; i++ )
	{	
		e += tgt->v[i] - out->v[i];
	}
	e = e / (double) tgt->n_v;
	
	return e;
}


/*****************************************************************************
* @@ UTheil                                                                	 *
* Rmk...: Calculate the Theil's U Statistic (Makridakis, 1997)               *
*                ...            ...     ...                                  *
* Date..: JUN/2009                                                           * 
* Author.: Fabio Daros de Freitas.                                           *
*****************************************************************************/
double UTheil(PWB_ANALYSE_SERIES_TS *tgt,PWB_ANALYSE_SERIES_TS *out)
{	 
	int i;
	double e;
	double e1;	
	double e2;

	/* Entry test */
	if ( tgt== NULL || out==NULL )
	{
		fprintf(stderr, "(UTheil) Error:NULL ts\n");
		return 0.0;
	}		
	
	e  = 0.0;
	e1 = 0.0;
	e2 = 0.0;	
	
	for ( i = 0; i < tgt->n_v-1; i++ )
	{
	
		if ( tgt->v[i] > 0.0 || tgt->v[i] < 0.0 )
		{
			e1 += pow( (out->v[i+1] - tgt->v[i+1])/tgt->v[i], 2.0); 
		
			e2 += pow( (tgt->v[i+1] - tgt->v[i]  )/tgt->v[i], 2.0); 
		}			
	}

	e = sqrt(e1/e2);
	
	return e;

}


/*****************************************************************************
* @@ R2                                                                	     *
* Rmk...: Calculate Coefficient of Determination (R^2) (Makridakis, 1997)    *
*                ...            ...     ...                                  *
* Date..: JUN/2009                                                           * 
* Author.: Fabio Daros de Freitas.                                           *
*****************************************************************************/
double R2(PWB_ANALYSE_SERIES_TS *tgt,PWB_ANALYSE_SERIES_TS *out)
{	 
	int i;
	double e;
	double e1;	
	double e2;
	double m;	
	int n;

	/* Entry test */
	if ( tgt== NULL || out==NULL )
	{
		fprintf(stderr, "(R2) Error:NULL ts\n");
		return 0.0;
	}		
	
	/*--- Coefficient of Determination R2 ---*/
	
	n = tgt->n_v;
	
	/* calculate  Mean of targets */
	m = 0.0;	
	for ( i = 0; i < n; i++ )
	{
		m += tgt->v[i];
	}
	
	m = m / (double) n;	

	e1 = 0.0;
	e2 = 0.0;
	for ( i = 0; i < n; i++ )
	{	
		e1 += pow(out->v[i] - m, 2.0); 

		e2 += pow(tgt->v[i] - m, 2.0); 
	}

	e = e1/e2;
	
	return e;
}


#ifdef EXEC

/*****************************************************************************
* @@ DumpTS                                                	                 *
* Rmk...: Dum a TS series to file                                            *
*                ...            ...     ...                                  *
* Date..: ABR/2005                                                           * 
* Author.: Fabio Daros de Freitas.                                           *
*****************************************************************************/
int DumpTS(PWB_ANALYSE_SERIES_TS *ts)
{	 
	int i;
	FILE *fp;
	char *fname;

	/* Entry test */
	if ( ts == NULL || ts->v == NULL )
	{
		fprintf(stderr, "(DumpTS) Error:NULL ts or NULL ts->\n");
		return -1;
	}		

	fname = malloc(PWB_ANALYSE_SERIES_FNAMESIZE); 
	if ( fname == NULL ) return -1;
	sprintf(fname, "%s.analyse_ts.txt", ts->id); 

	fp = fopen(fname, "w"); 
	if ( fp == NULL )
	{
		fprintf(stderr, "(DumpTS) Error: Cannot fopen(%s, w)\n", ts->fname);
		return -2;	
	}

	fprintf(fp, 
	"# Data generated by pwb_analyse_series version %s <freitas@computer.org>\n",
	PWB_ANALYSE_SERIES_VERSION);
	fprintf(fp, 
	"# Generated at %s \n#\n", datetime_now());
	fprintf(fp, 
	"# Series ID.....................: %s\n", ts->id);
	fprintf(fp, 
	"# Series Description............: %s\n", ts->desc);	
	fprintf(fp, 
	"# Number of points..............: %d\n", ts->n_v);		
	fprintf(fp, 
	"# min value.....................: %lf\n", ts->min);		
	fprintf(fp, 
	"# max value.....................: %lf\n", ts->max);		
	fprintf(fp, 
	"# mean value....................: %lf\n", ts->mean);		
	fprintf(fp, 
	"# var (variance)................: %lf\n", ts->var);			
	fprintf(fp, 
	"# sd  (standard deviation)......: %lf\n", ts->sd);			
	

	for ( i = 0; i < ts->n_v; i++ )
	{
		fprintf(fp, "%lf\n", ts->v[i]); 		
	}

	fprintf(fp, "# EOF\n"); 
	
	ffree(fname);
	
	fclose(fp); 

	return 0;
}



/*****************************************************************************
* @@ HelpMsg                                                          	     *
* Rmk...: help message                                                       *
* Date..: JUN/2004                                                           * 
* Author.: Fabio Daros de Freitas.                                           *
*****************************************************************************/
int HelpMsg(FILE *fp)
{	 
	fprintf(fp,  
	"pred: Portfolio Workbench (PWB) Time Series Analysis Tool\n");

	fprintf(fp, "-- PWB_ANALYSE_SERIES version %s\n", PWB_ANALYSE_SERIES_VERSION); 

	fprintf(fp,  
	"by Fabio D. Freitas <freitas@computer.org>\n");

	fprintf(fp,  
	"Use analyse_series <-f filename> <options>\n");

	fprintf(fp,  
	"(separator char = ;)\n");

	fprintf(fp,  
	"\n\tNotes: <> == required; [] == optional; do not use \'-\' char in filename\n\n");

	fprintf(fp,  
	"\t> application options :\n");

	fprintf(fp,  
	"\t-f : data file\n");


	fprintf(fp,  
	"\t-t : data file type:\n");


	fprintf(fp,  
	"\t    : 0 - <date; tgt; out; err> (default)\n");
	fprintf(fp,  
	"\t    : 1 - <date; tgt; out>\n");
	fprintf(fp,  
	"\t    : 2 - <date; tgt> \n");

	fprintf(fp,  
	"\t    : 3 - <tgt; out; err>\n");
	fprintf(fp,  
	"\t    : 4 - <tgt; out>\n");
	fprintf(fp,  
	"\t    : 5 - <tgt> \n");






	fprintf(fp,  
	"\t-r : shows sequences of repeated values: fname,value,i,f,n:, ...,n_seq\n");
	fprintf(fp,  
	"\t     value == repeated value;\n");
	fprintf(fp,  
	"\t     i     == initial 0-based index of sequence;\n");
	fprintf(fp,  
	"\t     f     == final 0-based index of sequence;\n");
	fprintf(fp,  
	"\t     n     == length of sequence (number of repetitions);\n");
	fprintf(fp,  
	"\t n_seq     == number of sequences found in file.\n");

	fprintf(fp,  
	"\t-s : shows basic statistics of data\n");

	fprintf(fp,  
	"\t-hdr : printf columns header and legends.\n");


	fprintf(fp,  
	"\t> FDF\n");

	return 0; 
	
}	

#endif

/* EOF */
