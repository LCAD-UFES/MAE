/*---------------------------------------------------------------------------
SISTEMA PAINEL - TIMEUTIL.H
MODULO DE FUNCOES DE MANIPULACCAO E ARITMETICA DE TEMPOS
ARQUIVO HEADER
01/1999
---------------------------------------------------------------------------*/

/*****************************************************************************
* * ADDED NEW FUNCTIONS FOR NEW PROJECTS...                              	 *
* Fabio D Freitas                                                            *
* Versions:                                                                  *
* v2.0, SEP-2003                                                             *
*****************************************************************************/


#ifndef TIME_UTIL_INC
#define TIME_UTIL_INC

time_t DATA_ZERO (void) ;

time_t diffetime (time_t t1, time_t t2) ;
time_t sumetime (time_t t1, time_t t2) ;

char *adiffetime (char *t1, char *t2) ;
char *asumetime (char *t1, char *t2) ;

long time2long (char *data, char *hora) ;
long etime2long (char *hora) ;
char *cetime (time_t t) ;
char *data2ccyymmdd (char *data) ;
char *ccyymmdd2data (char *ccyymmdd) ;
char *time2data (time_t t) ;
char *time2hora (time_t t) ;
char *etime2hora (time_t t) ;
char *TraduzMes (char *mes) ;

/*****************************************************************************
*********************   ADDED NEW FUNCTIONS FOR NEW PROJECTS...***************
* Fabio D Freitas                                                            *
* Versions:                                                                  *
* v2.0, SEP-2003                                                             *
******************************************************************************
*****************************************************************************/

long int isodate2long (char *date);
char *isodate2ctime(char *date);
char *long2isodate(long t);
long timePlusDays(long t, int i);
long timePlusDaysWeekends(long t, int i, int with_weekends);
long timeMinusDays(long t, int i);
long timeMinusDaysWeekends(long t, int i, int with_weekends);
int  timeInDays(long t);
int timeDifferenceInDaysStrides(long i, long f, int stride);
int timeDifferenceInDaysStridesWeekends(long i, long f, int stride, int with_weekends);
int timeStride(long i, long f);
int timeStrideWeekends(long i, long f, int with_weekends);
char *cdate(time_t *t);
int long2dow(long t);
char *long2dows(long t);
int isWeekendl(long t);
int isWeekends(char *s);
int isWeekend_isodate(char *s);

#endif
/*--------------------------------eof--------------------------------------*/
