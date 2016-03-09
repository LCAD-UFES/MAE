/*---------------------------------------------------------------------------
SISTEMA PAINEL - TIMEUTIL.CPP
MODULO DE FUNCOES DE MANIPULACCAO E ARITMETICAC DE TEMPO
---------------------------------------------------------------------------*/

/*****************************************************************************
* * ADDED NEW FUNCTIONS FOR NEW PROJECTS...                              	 *
* Fabio D Freitas                                                            *
* Versions:                                                                  *
* v2.0, SEP-2003                                                             *
*****************************************************************************/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "timeutil.h" 

/*
 * @@ DATA_ZERO - Retorna o long (time_t) de Jan 01 00:00:00 1970
 * (c) 11/1998, Fabio Daros de Freitas
 */
time_t DATA_ZERO (void)
{
   struct tm stmZ ;
   time_t t ;
   
   memset (&stmZ, 0, sizeof (struct tm)) ;

   /* Segundos */
   stmZ.tm_sec = 0 ;
   /* Minutos */
   stmZ.tm_min = 0 ;
   /* Horas */
   stmZ.tm_hour = 0 ;
   /* Dia */
   stmZ.tm_mday = 1 ;
   /* Mes */
   stmZ.tm_mon = 0 ;
   /* Ano */
   stmZ.tm_year = 70 ;                   

   t = mktime (&stmZ)  ;

   return (t) ;
}


/*
 * @@ adiffetime - Calcula a diferencca de elasped times entre duas horas  ascii
 *                (t1 - t2) no formato HH:MM:SS. 
   Se t1 < t2 ou t1 - t2 > "23:59:59", retorna "**:**:**".
 * (c) 11/1998, Fabio Daros de Freitas
 */

char *adiffetime (char *t1, char *t2)
{
   long total, t_1, t_2 ;
   static char  r [60] ;

    t_1 = etime2long (t1) ;
    t_2 = etime2long (t2) ;
    total = t_1 - t_2  ;   
    strcpy (r, cetime ((time_t) total));         
   
    return ((char *)r) ;
}

/*
 * @@ asumetime - Calcula a soma de elapsed times entre duas horas  ascii
 *                (t1 + t2) no formato HH:MM:SS
 * (c) 11/1998, Fabio Daros de Freitas
 */

char *asumetime (char *t1, char *t2)
{
   long total, t_1, t_2 ;
   static char  r [60] ;
   
   t_1 = etime2long (t1) ;
   t_2 = etime2long (t2) ;
   total = t_1 + t_2  ;   
   strcpy (r, cetime ((time_t) total));
   return ((char *) r) ;
}


/*
 * @@ tdifftime - Calcula o elapsed time de diferencca entre dois elapsed time_t
 * (c) 11/1998, Fabio Daros de Freitas
 */

time_t tdiffetime (time_t t1, time_t t2)
{
   long total ;
   
   total = (long) t1 - (long) t2 ;
	return ((time_t) total) ;
}

/*
 * @@ tsumtime - Calcula o elapsed time de soma entre dois elapsed time_t
 * (c) 11/1998, Fabio Daros de Freitas
 */

time_t tsumetime (time_t t1, time_t t2)
{
   long total ;
   
   total = (long) t1 + (long) t2 ;
	return ((time_t) total) ;
}

/*
 * @@ time2long - Converte um Data ("DD/MM/AAAA") e Hora ("HH:MM:SS") para  
                  long.
 * (c) 01/1999, Fabio Daros de Freitas
 */
long time2long (char *data, char *hora)
{
   struct tm stm ;
   char buffer [15] ;
   long retval ;

   memset (&stm, 0, sizeof (struct tm)) ;

   if ( strlen (hora) > 0 )
   {   
      /* Segundos */
      strcpy (buffer, hora + 6) ;
      buffer [2] = 0 ;
      stm.tm_sec = atoi (buffer) ;
      /* Minutos */
      strcpy (buffer, hora + 3) ;
      buffer [2] = 0 ;
      stm.tm_min = atoi (buffer) ;
      /* Horas */
      strcpy (buffer, hora) ;
      buffer [2] = 0 ;
      stm.tm_hour = atoi (buffer) ;
   }
   else 
   {
      /* Segundos */
      stm.tm_sec = 0 ;
      /* Minutos */
      stm.tm_min = 0 ;
      /* Horas */
      stm.tm_hour = 0 ;
   }
      
   if ( strlen (data) != 0 )
   {
      /* Dia */
      strcpy (buffer, data) ;
      buffer [2] = 0 ;
      stm.tm_mday = atoi (buffer) ;	  
	  
      /* Mes */
      strcpy (buffer, data + 3) ;
      buffer [2] = 0 ;
      stm.tm_mon = atoi (buffer) - 1 ;

      /* Ano */
      strcpy (buffer, data + 6) ;
      buffer [4] = 0 ;
      stm.tm_year = atoi (buffer) - 1900 ; /* data inicial do mktime 01/01/1970,
                                              00:00:00 h */

   }
   else
   {
      /* Dia */
      stm.tm_mday = 1 ;
      /* Mes */
      stm.tm_mon = 0 ;
      /* Ano */
      stm.tm_year = 70 ;                   /* data inicial do mktime 01/01/1970,
                                             00:00:00 h */
   }


   retval = (long)  mktime (&stm)  ;


   return (retval)  ;
}

/*
 * @@ etime2long - Converte um elapsed time no formato Hora ("HH:MM:SS"), com
                   offset 01/01/1970 00:00:00 long.
 * (c) 01/1999, Fabio Daros de Freitas
 */
long etime2long (char *hora)
{
   struct tm stm ;
   char buffer [15] ;
   long retval ;

    memset (&stm, 0, sizeof (struct tm)) ;

   /* Segundos */
   strcpy (buffer, hora + 6) ;
   buffer [2] = 0 ;
   stm.tm_sec = atoi (buffer) ;
   /* Minutos */
   strcpy (buffer, hora + 3) ;
   buffer [2] = 0 ;
   stm.tm_min = atoi (buffer) ;
   /* Horas */
   strcpy (buffer, hora) ;
   buffer [2] = 0 ;
   stm.tm_hour = atoi (buffer) ;
   /* Dia */
   stm.tm_mday = 1 ;
   /* Mes */
   stm.tm_mon = 0 ;
   /* Ano */
   stm.tm_year = 70 ;  /* data inicial do mktime 01/01/1970, 00:00:00 h */

   retval = (long) mktime (&stm) - DATA_ZERO ()  ;
   return (retval)  ;
}

/*
 * @@ cetime - Converte um elapsed time no formato time_t absoluto para um 
               string com offset 01/01/1970 00:00:00 com o formato 
               [DDD] HH:MM:SS
 * (c) 01/1999, Fabio Daros de Freitas
 */
char *cetime (time_t t)
{
   time_t t1 ;
   t1 = t + DATA_ZERO () ;

   return (time2hora (t1)) ;
}


/*
 * @@ data2ccyymmdd - Converte uma data DD/MM/CCYY para CCYYMMDD
 * (c) 01/1999, Fabio Daros de Freitas
 */
char *data2ccyymmdd (char *data)
{
	static char buffer [15] ;

   buffer [0] = data [6] ;
   buffer [1] = data [7] ;
   buffer [2] = data [8] ;
   buffer [3] = data [9] ;   
   buffer [4] = data [3] ;
   buffer [5] = data [4] ;
   buffer [6] = data [0] ;
   buffer [7] = data [1] ;   
   buffer [8] = 0 ;
	return (buffer) ;
}

/*
 * @@ ccyymmdd2data - Converte uma data CCYYMMDD para DD/MM/CCYY
 * (c) 01/1999, Fabio Daros de Freitas
 */
char *ccyymmdd2data (char *ccyymmdd)
{
	static char buffer [15] ;

	/* BUG coreected in 2003-09-17 by FDF. Error in copy of DD and MM!!!! */

   buffer [0] = ccyymmdd [8] ;
   buffer [1] = ccyymmdd [9] ;
   buffer [2] = '/' ;   
   buffer [3] = ccyymmdd [5] ;
   buffer [4] = ccyymmdd [6] ;   
   buffer [5] = '/' ;   
   buffer [6] = ccyymmdd [0] ;
   buffer [7] = ccyymmdd [1] ;
   buffer [8] = ccyymmdd [2] ;
   buffer [9] = ccyymmdd [3] ;
   buffer [10] = 0 ;

   return (buffer) ;
}


/*****************************************************************************
* @@																		 *
* Funcao: time2data                                                      	 *
* Rmk...: Converte o tipo time_t em data string.                             *
* Par...: time_t - t.                                                        *
*   													                     *
* Ret...: char *- data                                  		             *
*																			 *
* Data..: 03/97                                                              *
*   																		 *  
* Autor.: Fabio Daros de Freitas.                                            *
*																			 *
*****************************************************************************/
char *time2data (time_t t)
{
	static char data [16], s[64],
	dia_s [5], mes [5], ano [6], hora [12] ;
	int idia ;

	strcpy (s, ctime (&t)) ;
	//sscanf (s, "%s %s %s %s %sn", dia_s, mes, dia, hora, ano) ;
	//sprintf (data, "%s/%s/%s", dia, TraduzMes (mes), ano) ;
	sscanf (s, "%s %s %d %s %s", dia_s, mes, &idia, hora, ano) ;
	sprintf (data, "%02d/%s/%s", idia, TraduzMes(mes), ano) ;
	
	return ((char *)(data)) ;
} /* time2data */

/*****************************************************************************
* @@			                     					                     *
* Funcao: time2hora                                                      	 *
* Rmk...: Converte o tipo time_t em hora string.                             *
* Par...: time_t - t.                                                        *
*   														                 *
* Ret...: char *- hora                                  				     *
*																			 *
* Data..: 03/97                                                              *
*   																		 *
* Autor.: Fabio Daros de Freitas.                                            *
*																			 *
*****************************************************************************/
char *time2hora (time_t t)
{
   char s[30], dia_s [5], dia [5], mes [5], ano [6] ;
   static char hora [12] ;

   if ( (long) t >= (long) DATA_ZERO () )
   {      
      strcpy (s, ctime (&t)) ;
      sscanf (s, "%s %s %s %s %sn", dia_s, mes, dia, hora, ano) ; 
      hora [8] = (char) 0 ;
   }
   else
      strcpy (hora, "--:--:--") ;         
   return ((char *)(hora)) ;
} /* time2hora */

/*****************************************************************************
* @@			                     										 *
* Funcao: etime2hora                                                      	 *
* Rmk...: Converte o tipo time_t em elapsed hora string.                     *
* Par...: time_t - t.                                                        *
*   														                 *
* Ret...: char *- hora                                  				     *
*																			 *
* Data..: 03/97                                                              *
*   																		 *
* Autor.: Fabio Daros de Freitas.                                            *
*																			 *
*****************************************************************************/
char *etime2hora (time_t t)
{
   return (time2hora ((long) t + (long) DATA_ZERO ())) ; 
} /* etime2hora */


/*****************************************************************************
* @@																		 * 
* Funcao: TraduzMes                                                          *
* Rmk...: Traduz o formato de mes Ingles (3 letras) para o numero do mes.    *
* Par...: int - mesa.                                                        *
*   														                 *
* Ret...: char *- mes                                  				         *
*																			 *
* Data..: 03/97                                                              *
*   																		 *
* Autor.: Fabio Daros de Freitas.                                            *
*																			 *
*****************************************************************************/
char *TraduzMes (char *mes)
{
	static char m [2+1] ;

	memset(m, 0, 3);
	
	if      ( !strcmp ("Jan", mes) ) strcpy (m, "01") ;
	else if ( !strcmp ("Feb", mes) ) strcpy (m, "02") ;
	else if ( !strcmp ("Mar", mes) ) strcpy (m, "03") ;
	else if ( !strcmp ("Apr", mes) ) strcpy (m, "04") ;
	else if ( !strcmp ("May", mes) ) strcpy (m, "05") ;
	else if ( !strcmp ("Jun", mes) ) strcpy (m, "06") ;
	else if ( !strcmp ("Jul", mes) ) strcpy (m, "07") ;
	else if ( !strcmp ("Aug", mes) ) strcpy (m, "08") ;
	else if ( !strcmp ("Sep", mes) ) strcpy (m, "09") ;
	else if ( !strcmp ("Oct", mes) ) strcpy (m, "10") ;
	else if ( !strcmp ("Nov", mes) ) strcpy (m, "11") ;
	else if ( !strcmp ("Dec", mes) ) strcpy (m, "12") ;

	return ((char *)(m)) ;

} /* TraduzMes */


/*****************************************************************************
*********************   ADDED NEW FUNCTIONS FOR NEW PROJECTS...***************
* Fabio D Freitas                                                            *
* Versions:                                                                  *
* v2.0, SEP-2003                                                             *
******************************************************************************
*****************************************************************************/

/*****************************************************************************
* @@ isodate2long                                                            *
* Rmk...: Convert an iso date (CCYY-MM-DD) to long (time_t)                  *
* Date..: SEP/2003                                                           * 
* Author.: Fabio Daros de Freitas.                                           *
*****************************************************************************/
long int isodate2long (char *date)
{
   struct tm stm ;
   char buffer [11] ;
   char *ptr;
   long ret = -1 ;

	if ( date == NULL || strlen(date) < 10 ) 
		return ret;

	memset (&stm, 0, sizeof (struct tm)) ;

	memset(buffer, 0, 11);
	strncpy(buffer, date, 10); 

	if ( date != NULL && strlen(date)>=10 && date[4]=='-' && date[7]=='-' )
	{
		/* Year */
		ptr = buffer;
		ptr[4]='\0'; 
		stm.tm_year = atoi (ptr) - 1900 ; /* Years since 1900 */

		if ( stm.tm_year < 0 ) return -1;

		/* Month */
		ptr = buffer + 5;
		ptr[2]='\0'; 
		stm.tm_mon = atoi (ptr) - 1; /* 0-11 */

		if ( stm.tm_mon < 0 || stm.tm_mon > 11 ) return -1;

		/* Day */
		ptr = buffer + 8;
		ptr[2]='\0'; 
		stm.tm_mday = atoi (ptr); /* 1-31 */

		if ( stm.tm_mday < 1 || stm.tm_mday > 31  ) return -1;

		/* time part */
		stm.tm_sec  = 0 ;
		stm.tm_min  = 0 ;
		stm.tm_hour = 0 ;

		ret = (long int)  mktime (&stm) ;
	}
	
	return ret;
}

/*****************************************************************************
* @@ isodate2ctime                                                           *
* Rmk...: Return ctime date info (withou hour)                               *
* Date..: SEP/2003                                                           * 
* Author.: Fabio Daros de Freitas.                                           *
*****************************************************************************/
char *isodate2ctime(char *date)
{
   static char sdate[64] ;
   char sday[4];
   char month[4];
   char day[4];
   char hour[16];
   char year[5];
   time_t t;
   
	t = (time_t) isodate2long(date);
	sscanf(ctime(&t), "%s %s %s %s %s", sday, month, day, hour, year);
	sprintf(sdate, "%s %s %s %s", sday, month, day, year);
	return sdate;
}

/*****************************************************************************
* @@ long2isodate                                                            *
* Rmk...: Return an isodate for a long int                                   *
* Date..: MAY/2004                                                           * 
* Author.: Fabio Daros de Freitas.                                           *
*****************************************************************************/
char *long2isodate(long t)
{
   static char sdate[11] ;
   char sday[4];
   char month[4];
   char day[4];
   char hour[16];
   char year[5+2];
   
   int iday;
      
//	sscanf(ctime(&((time_t)t)), "%s %s %s %s %s", sday, month, day, hour, year);

	sscanf(ctime((time_t *) &t), "%s %s %s %s %s", sday, month, day, hour, year);


	sscanf(day, "%d", &iday); 
	memset(sdate, 0, 11);
	sprintf(sdate, "%s-%s-%02d", year, TraduzMes(month), iday);

	return sdate;
}

/*****************************************************************************
* @@ timePlusDays                                                            *
* Rmk...: Return the time + i days                                           *
* Date..: MAY/2004                                                           * 
* Author.: Fabio Daros de Freitas.                                           *
*****************************************************************************/
long timePlusDays(long t, int i)
{
	return  timePlusDaysWeekends(t, i, 0);
}

long timePlusDaysWeekends(long t, int i, int with_weekends)
{
	int j;
#if 0	
	t_next =  t + (long)i *(3600 * 24);
	
	if ( !with_weekends )	{
	
		/* THE RULE IS: if date falls within weekend,
		                get the next valid day (stride 1)
		*/						
		
		while ( isWeekendl(t_next) ) 
			t_next = timePlusDays(t_next, 1);				
	}
#endif

	j = 0;
	while ( j < i )
	{
		t += (3600 * 24);

		if ( !isWeekendl(t) || with_weekends ) j++;
	}
	
	return t;
}

/*****************************************************************************
* @@ timeMinusDays                                                           *
* Rmk...: Return the time i i days                                           *
* Date..: MAR/2007 (only now!?!?!)                                           * 
* Author.: Fabio Daros de Freitas.                                           *
*****************************************************************************/
long timeMinusDays(long t, int i)
{
	return  timeMinusDaysWeekends(t, i, 0);	
}

long timeMinusDaysWeekends(long t, int i, int with_weekends)
{
#if 0
	long t_prev;
	
	t_prev =  t - (long)i *(3600 * 24);
	
	if ( !with_weekends )	{
	
		/* THE RULE IS: if date falls within weekend,
		                get the previous valid day (stride 1)
		*/						
		
		while ( isWeekendl(t_prev) ) 
			t_prev = timeMinusDays(t_prev, 1);				
	}
	return t_prev;
#endif

	int j;
	
	j = 0;
	while ( j < i )
	{
		t -= (3600 * 24);

		if ( !isWeekendl(t) || with_weekends ) j++;
	}

	return t;
}


/*****************************************************************************
* @@ timeInDays                                                              *
* Rmk...: Return how many days are in a long time                            *
* Date..: MAR/2007 (only now!?!?!)                                           * 
* Author.: Fabio Daros de Freitas.                                           *
*****************************************************************************/
int timeInDays(long t)
{
	return t/(3600 * 24);
}

/*****************************************************************************
* @@ timeDifferenceInDaysStrides                                             *
* Rmk...: Return how many stride days are between two long times             *
* Date..: MAR/2007 (only now!?!?!)                                           * 
* Author.: Fabio Daros de Freitas.                                           *
*****************************************************************************/
int timeDifferenceInDaysStrides(long i, long f, int stride)
{
	return timeDifferenceInDaysStridesWeekends(i, f, stride, 0);
}

int timeDifferenceInDaysStridesWeekends(long i, long f, int stride, int with_weekends)
{
	int n = 0;
	
//char is[15];	
//char fs[15];		
	
	
	if ( f == i ) return 0;	
	if ( i < f )
	{
		while ( i < f )
		{
			i = timePlusDaysWeekends(i, stride, with_weekends);
			n++;
		
//strcpy(is, long2isodate(i));		
//strcpy(fs, long2isodate(f));					
//printf("(td) i=%ld(%s)  f=%ld(%s)  n=%d\n", 
//i, is, f, fs, n); 			
			
			
		}			
	}	
	else if ( i > f )
	{
		while ( i > f )
		{
			i = timeMinusDaysWeekends(i, stride, with_weekends);
			n++;
		}
	}
	
//strcpy(is, long2isodate(i));		
//strcpy(fs, long2isodate(f));					
//printf("SAIDA(td) i=%ld(%s)  f=%ld(%s)  n=%d\n", 
//i, is, f, fs, n); 			
	
	return n;
}


/*****************************************************************************
* @@ timeStride                                                              *
* Rmk...: Return the stride between 2 times                                  *
* Date..: OCT/2007 (only now!?!?!)                                           * 
* Author.: Fabio Daros de Freitas.                                           *
*****************************************************************************/
int timeStride(long i, long f)
{
	return timeStrideWeekends(i, f, 0);	
}

int timeStrideWeekends(long i, long f, int with_weekends)
{

	return timeDifferenceInDaysStridesWeekends(i, f, 1, with_weekends);	
}


/*****************************************************************************
* @@ cdate                                                                   *
* Rmk...: A 'ctime' witch returns only date information                      *
* Date..: MAY/2004                                                           * 
* Author.: Fabio Daros de Freitas.                                           *
*****************************************************************************/
char *cdate(time_t *t)
{
   static char sdate[64] ;
   char sday[4];
   char month[4];
   char day[4];
   char hour[16];
   char year[5];
 
	sscanf(ctime(t), "%s %s %s %s %s", sday, month, day, hour, year);
	sprintf(sdate, "%s %s %s %s", sday, month, day, year);

	return sdate;
}

/*****************************************************************************
* @@ long2dow                                                                *
* Rmk...: Return the Day of the Week (0-6) of a time                         *
* Date..: MAY/2004                                                           * 
* Author.: Fabio Daros de Freitas.                                           *
*****************************************************************************/
int long2dow(long t)
{
	struct tm tm ;
	time_t time ;

	time = (time_t) t;
	gmtime_r(&time, &tm);

	return tm.tm_wday;
}

/*****************************************************************************
* @@ long2dows                                                               *
* Rmk...: Return the Day of the Week in string (Sun-Sat) of a time           *
* Date..: MAY/2004                                                           * 
* Author.: Fabio Daros de Freitas.                                           *
*****************************************************************************/
char *long2dows(long t)
{
	static char dows[4];

	memset(dows, 0, 4);
	switch (long2dow(t))	{
		case 0:strcpy(dows, "Sun");break;
		case 1:strcpy(dows, "Mon");break;
		case 2:strcpy(dows, "Tue");break;
		case 3:strcpy(dows, "Wed");break;
		case 4:strcpy(dows, "Thu");break;
		case 5:strcpy(dows, "Fri");break;
		case 6:strcpy(dows, "Sat");break;			
	}

	return dows;
}

/*****************************************************************************
* @@ isWeekendl                                                              *
* Rmk...: is weekend day?                                                    *
* Date..: NOV/2006                                                           * 
* Author.: Fabio Daros de Freitas.                                           *
*****************************************************************************/
int isWeekendl(long t)
{

	if ( long2dow(t) == 0 ) return 1;
	if ( long2dow(t) == 6 ) return 1;	
	
	return 0;
}

/*****************************************************************************
* @@ isWeekends                                                              *
* Rmk...: is weekend day?                                                    *
* Date..: NOV/2006                                                           * 
* Author.: Fabio Daros de Freitas.                                           *
*****************************************************************************/
int isWeekends(char *s)
{
	if ( !strcmp("Sat", s)  ) return 1;
	if ( !strcmp("Sun", s)  ) return 1;	
	
	return 0;
}

/*****************************************************************************
* @@ isWeekends                                                              *
* Rmk...: is weekend day?                                                    *
* Date..: MAR/2007 - Previus string version uses "Sat" and "Sund"            * 
* Author.: Fabio Daros de Freitas.                                           *
*****************************************************************************/
int isWeekend_isodate(char *s)
{
	return isWeekendl(isodate2long(s));
}


/*--------------------------------eof----------------------------------------*/
