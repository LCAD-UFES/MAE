#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <timeutil.h> 

int
main ()
{
	char *date1 = "2001-01-04";
	char *date2 = "2001-01-05";
	char *date3 = "2001-01-08";
	char *iso_date;
	char *dows;
	long date_l, date_l2;
	int i, dow;
	
	
	// long isodate2long (char *date)
	date_l = isodate2long (date1);
	
	// char *long2isodate(long t)
	iso_date = long2isodate (date_l);
	
	printf ("data_l = %ld, iso_date = %s\n", date_l, iso_date);
	
	// long timePlusDays(long t, int i)
	date_l = timePlusDays(date_l, 1);

	iso_date = long2isodate (date_l);
	printf ("data_l = %ld, iso_date = %s\n", date_l, iso_date);

	date_l = timePlusDays(date_l, 1);

	iso_date = long2isodate (date_l);
	printf ("data_l = %ld, iso_date = %s\n", date_l, iso_date);

	// int timeDifferenceInDaysStrides(long i, long f, int stride) -> sem sinal
	date_l = isodate2long (date1);
	date_l2 = isodate2long (date2);
	i = timeDifferenceInDaysStrides(date_l, date_l2, 1);
	printf ("date1 = %s, data2 = %s, difference = %d\n", date1, date2, i);
	i = timeDifferenceInDaysStrides(date_l2, date_l, 1);
	printf ("date1 = %s, data2 = %s, difference = %d\n", date2, date1, i);
	
	// int long2dow(long t)
	dow = long2dow (date_l);
	// char *long2dows(long t)
	dows = long2dows (date_l);
	printf ("date = %s, dow inteiro = %d, dow string = %s\n", date1, dow, dows);
	
	

	return (0);
}
