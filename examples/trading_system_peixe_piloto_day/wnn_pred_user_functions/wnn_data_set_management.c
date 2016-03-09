#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>

#include "../wnn_pred.h"
#include "wnn_pred_user_functions.h"
#include "timeutil.h"


DIR *g_train_dir = NULL;
TRAIN_DATA_SET *g_train_data_set = NULL;
int g_train_num_samples = 0;
char g_current_data_set_name[1000];



FILE *
get_file(char *base_file_name, char *file_type)
{
	FILE *data_set_file;
	char file_name[1000];
	
	strcpy (file_name, DATA_PATH);
	strcat (file_name, base_file_name);
	strcat (file_name, file_type);
	if ((data_set_file = fopen (file_name, "r")) == NULL)
	{
		Erro ("Could not open file: ", file_name, "");
		exit(1);
	}
	return (data_set_file);
}



FILE *
get_buy_sell_file(char *base_file_name, char *file_type)
{
	FILE *data_set_file;
	char file_name[1000];
	char dir_name[1000];
	char *aux;
	int i;
	
	strcpy(dir_name, base_file_name);
	aux = strrchr (dir_name, '_');
	aux[0] = '\0';
	for(i = 0; dir_name[i]; i++)
		dir_name[i] = toupper(dir_name[i]);
	aux[0] = '/';
	aux[1] = '\0';

	strcpy (file_name, BUY_SELL_DATA_PATH);
	strcat (file_name, dir_name);
	strcat (file_name, base_file_name);
	strcat (file_name, file_type);
	if ((data_set_file = fopen (file_name, "r")) == NULL)
	{
		Erro ("Could not open file: ", file_name, " in get_buy_sell_file()");
		exit(1);
	}
	return (data_set_file);
}



FILE *
get_peixe_piloto_file(char *base_file_name, char *file_type)
{
	FILE *data_set_file;
	char file_name[1000];
	
	strcpy (file_name, PEIXE_PILOTO_DATA_PATH);
	strcat (file_name, base_file_name);
	strcat (file_name, file_type);
	if ((data_set_file = fopen (file_name, "r")) == NULL)
	{
		Erro ("Could not open Peixe Piloto file: ", file_name, "");
		return (NULL);
	}
	return (data_set_file);
}



int
count_samples(FILE *data_set_file)
{
	int num_samples;
	char data_set_file_first_line[1000];
	char data_set_file_last_line[1000];
	char aux[1000];
	char date[1000];
	float price;
	long date_l_1, date_l_n;
	
	if (fgets (data_set_file_first_line, 900, data_set_file) == NULL)
	{
		Erro ("Could not read first line of the dataset (.clo file)", "", "");
		return (0);
	}
		
	while (fgets (aux, 900, data_set_file) != NULL)
		strcpy (data_set_file_last_line, aux);
	
	if (sscanf (data_set_file_first_line, "%s %f", (char *)date, &price) != 2)
	{
		Erro ("Could not read first date of the dataset (.clo file)", "", "");
		return (0);
	}
	date_l_1 = isodate2long (date);
	
	if (sscanf (data_set_file_last_line, "%s %f", (char *)date, &price) != 2)
	{
		Erro ("Could not read last date of the dataset (.clo file)", "", "");
		return (0);
	}
	date_l_n = isodate2long (date);

	rewind (data_set_file);
	
	num_samples = timeDifferenceInDaysStrides (date_l_1, date_l_n, 1) + 1;

	return (num_samples);
}


int
fill_in_missing_samples (long previous_date, long current_date, float closing_price, int current_sample)
{
	int difference_in_days; /* whithout considering weekends */
	int new_current_sample, i;
	
	if (previous_date == 0)
		new_current_sample = current_sample;
	else if ((difference_in_days = timeDifferenceInDaysStrides(previous_date, current_date, 1)) == 1)
		new_current_sample = current_sample;
	else
	{
		new_current_sample = current_sample + difference_in_days - 1;
		for (i = 0; i < difference_in_days - 1; i++)
		{
			g_train_data_set[current_sample+i] = g_train_data_set[current_sample - 1];
			g_train_data_set[current_sample+i].date_l = timePlusDays (g_train_data_set[current_sample - 1].date_l, i+1);
			g_train_data_set[current_sample+i].buy = 0.0;
			g_train_data_set[current_sample+i].sell = 0.0;
		}
	}		

	g_train_data_set[new_current_sample].date_l = current_date;
	g_train_data_set[new_current_sample].clo = closing_price;

	return (new_current_sample);
}



void
free_g_train_data_set()
{
	int i;
	
	for (i = 0; i < g_train_num_samples; i++)
	{
		if (g_train_data_set[i].intraday_price != NULL)
			free(g_train_data_set[i].intraday_price);
		if (g_train_data_set[i].intraday_volume != NULL)
			free(g_train_data_set[i].intraday_volume);
	}

	free(g_train_data_set);
	g_train_data_set = NULL;
}



int
read_train_data_set_data (char *base_file_name)
{
	FILE *clo;
	FILE *max;
	FILE *min;
	FILE *vol;
	FILE *buy;
	FILE *sell;

	char data_set_file_line[1000], date[100], date_buy_sell[100];
	int i, state = 0;
	long current_date, previous_date;
	float closing_price;
	
	if (g_train_data_set != NULL)
		free_g_train_data_set();

	clo = get_file (base_file_name, ".clo");
	g_train_num_samples = count_samples (clo);
	g_train_data_set = (TRAIN_DATA_SET *) alloc_mem (g_train_num_samples * sizeof (TRAIN_DATA_SET));
	
	max = get_file (base_file_name, ".max");
	min = get_file (base_file_name, ".min");
	vol = get_file (base_file_name, ".vol");

	buy = get_buy_sell_file (base_file_name, ".buy");
	sell = get_buy_sell_file (base_file_name, ".sell");

	i = 0;
	previous_date = 0;
	while (fgets (data_set_file_line, 900, clo) != NULL)
	{
		if (sscanf (data_set_file_line, "%s %f", (char *)date, &closing_price) != 2)
			break;
		
		current_date = isodate2long (date);
		i = fill_in_missing_samples(previous_date, current_date, closing_price, i);
		previous_date = current_date;

		fgets (data_set_file_line, 900, max);
		if (sscanf (data_set_file_line, "%s %f", (char *)date, &(g_train_data_set[i].max)) != 2)
		{
			Erro ("Could not read from file: ", base_file_name, ".max");
			return (0);
		}
		fgets (data_set_file_line, 900, min);
		if (sscanf (data_set_file_line, "%s %f", (char *)date, &(g_train_data_set[i].min)) != 2)
		{
			Erro ("Could not read from file: ", base_file_name, ".min");
			return (0);
		}
		fgets (data_set_file_line, 900, vol);
		if (sscanf (data_set_file_line, "%s %f", (char *)date, &(g_train_data_set[i].vol)) != 2)
		{
			Erro ("Could not read from file: ", base_file_name, ".vol");
			return (0);
		}

		fgets (data_set_file_line, 900, buy);
		if (sscanf (data_set_file_line, "%s %f", (char *)date_buy_sell, &(g_train_data_set[i].buy)) != 2)
		{
			Erro ("Could not read from file: ", base_file_name, ".buy");
			return (0);
		}
		if ((state == 0) && (strcmp(date, date_buy_sell) != 0))
		{
			g_train_data_set[i].buy = g_train_data_set[i].sell = 0.0;
			rewind(buy);
		}
		else if ((state == 0) && (strcmp(date, date_buy_sell) == 0))
		{
			state = 1;
			fgets (data_set_file_line, 900, sell);
			if (sscanf (data_set_file_line, "%s %f", (char *)date_buy_sell, &(g_train_data_set[i].sell)) != 2)
			{
				Erro ("Could not read from file: ", base_file_name, ".sell");
				return (0);
			}
		}
		else if ((state == 1) && (strcmp(date, date_buy_sell) != 0))
		{
			Erro("Buy file has a different date in read_train_data_set_data(): ", date, date_buy_sell);
			exit(1);
		}
		else
		{
			fgets (data_set_file_line, 900, sell);
			if (sscanf (data_set_file_line, "%s %f", (char *)date_buy_sell, &(g_train_data_set[i].sell)) != 2)
			{
				Erro ("Could not read from file: ", base_file_name, ".sell");
				return (0);
			}
		}

		g_train_data_set[i].opn = (g_train_data_set[i].max + g_train_data_set[i].min) / 2.0;
		g_train_data_set[i].intraday_price = NULL;
		g_train_data_set[i].intraday_volume = NULL;
		
		// printf ("%d: %s ; %s ; % .6f ; % .6f ; % .6f ; % .6f ; % .6f ; % .6f ; \n", i,
		//	long2isodate (g_train_data_set[i].date_l), long2dows (g_train_data_set[i].date_l), 
		//	g_train_data_set[i].clo, 
		//	g_train_data_set[i].max,
		//	g_train_data_set[i].min,
		//	g_train_data_set[i].buy,
		//	g_train_data_set[i].sell,
		//	g_train_data_set[i].vol);
		i++;
	}

	g_train_num_samples = i;
	
	fclose (clo);
	fclose (max);
	fclose (min);
	fclose (vol);
	fclose (buy);
	fclose (sell);

	return (g_train_num_samples);
}



int
find_daily_sample_index(char *date)
{
	long data_l;
	int i;
	
	data_l = isodate2long(date);
	for (i = 0; i < g_train_num_samples; i++)
	{
		if (g_train_data_set[i].date_l == data_l)
			break;
	}
	if (i == g_train_num_samples)
	{
		Erro ("Could not daily sample index in find_daily_sample_index().", "", "");
		exit(1);
	}
	
	return (i);
}



void
check_data_set_file_line(char *date, char *week_day, int hour, int minute, float price, float num_trades, float volume)
{
	if (strlen(date) != 10)
		Erro("Wrong date size in check_data_set_file_line(): ", date, "");
	if (strlen(week_day) != 3)
		Erro("Wrong week day size in check_data_set_file_line(), date: ", date, "");
	if ((hour < 10) || (hour > 18))
		Erro("Hour out of range in check_data_set_file_line(), date: ", date, "");
	if ((minute < 0) || (minute > 59))
		Erro("Minute out of range in check_data_set_file_line(), date: ", date, "");
}



void
read_peixe_piloto_data_set_data (char *data_set_name)
{
	FILE *peixe_piloto_file;
	long daily_sample, first_daily_sample = 0;
	int current_date_intraday_sample;
	char data_set_file_line[1000], date[100], week_day[100], previous_date[100];
	int hour, minute;
	float price, num_trades, volume, previous_price, day_price_corrention_factor;
	
	peixe_piloto_file = get_peixe_piloto_file (data_set_name, "_Volume.TXT");
	daily_sample = -1;
	current_date_intraday_sample = 0;
	previous_date[0] = '\0';
	while (fgets (data_set_file_line, 900, peixe_piloto_file) != NULL)
	{
		if (sscanf (data_set_file_line, "%s %s %d:%d;%f;%f;%f", (char *)date, (char *)week_day, &hour, &minute, &price, &num_trades, &volume) != 7)
		{
			Erro ("Could not read line of Peixe Piloto file ", data_set_name, " in read_peixe_piloto_data_set_data().");
			exit(1);
		}
		check_data_set_file_line(date, week_day, hour, minute, price, num_trades, volume);

		if (daily_sample == -1)
		{	// Inicio do arquivo Peixe Piloto
			daily_sample = 0;
			first_daily_sample = find_daily_sample_index(date);
			g_train_data_set[first_daily_sample+daily_sample].intraday_price = (float *) alloc_mem(MINUTES_IN_A_DAY * sizeof(float));
			g_train_data_set[first_daily_sample+daily_sample].intraday_volume = (float *) alloc_mem(MINUTES_IN_A_DAY * sizeof(float));
			for (current_date_intraday_sample = 0; current_date_intraday_sample < minute; current_date_intraday_sample++)
			{	// Completa as cotacoes no inicio do dia
				g_train_data_set[first_daily_sample+daily_sample].intraday_price[current_date_intraday_sample] = price;
				g_train_data_set[first_daily_sample+daily_sample].intraday_volume[current_date_intraday_sample] = 0.0;
			}
			g_train_data_set[first_daily_sample+daily_sample].opn = price;
			g_train_data_set[first_daily_sample+daily_sample].intraday_price[current_date_intraday_sample] = price;
			g_train_data_set[first_daily_sample+daily_sample].intraday_volume[current_date_intraday_sample] = volume;
			current_date_intraday_sample++;
			strcpy(previous_date, date);
		}
		else if (strcmp(date, previous_date) != 0)
		{	// Fim de um dia no arquivo Peixe Piloto
			previous_price = g_train_data_set[first_daily_sample+daily_sample].intraday_price[current_date_intraday_sample-1];
			for ( ; current_date_intraday_sample < MINUTES_IN_A_DAY; current_date_intraday_sample++)
			{	// Completa as cotacoes no fim do dia
				g_train_data_set[first_daily_sample+daily_sample].intraday_price[current_date_intraday_sample] = previous_price;
				g_train_data_set[first_daily_sample+daily_sample].intraday_volume[current_date_intraday_sample] = 0.0;
			}

			// Corrige discrepancia entre dados diarios e intraday
			day_price_corrention_factor = g_train_data_set[first_daily_sample+daily_sample].clo / previous_price;
			for (current_date_intraday_sample = 0; current_date_intraday_sample < MINUTES_IN_A_DAY; current_date_intraday_sample++)
			{	// Completa as cotacoes no fim do dia
				g_train_data_set[first_daily_sample+daily_sample].intraday_price[current_date_intraday_sample] *= day_price_corrention_factor;
				g_train_data_set[first_daily_sample+daily_sample].intraday_volume[current_date_intraday_sample] *= day_price_corrention_factor;
			}

			daily_sample += timeDifferenceInDaysStrides (isodate2long(previous_date), isodate2long(date), 1);
			if (g_train_data_set[first_daily_sample+daily_sample].date_l != isodate2long(date))
			{
				Erro ("Date mismatch in read_peixe_piloto_data_set_data(); date = ", date, "");
				exit(1);
			}

			g_train_data_set[first_daily_sample+daily_sample].intraday_price = (float *) alloc_mem(MINUTES_IN_A_DAY * sizeof(float));
			g_train_data_set[first_daily_sample+daily_sample].intraday_volume = (float *) alloc_mem(MINUTES_IN_A_DAY * sizeof(float));

			for (current_date_intraday_sample = 0; current_date_intraday_sample < minute; current_date_intraday_sample++)
			{	// Completa as cotacoes no inicio do dia
				g_train_data_set[first_daily_sample+daily_sample].intraday_price[current_date_intraday_sample] = price;
				g_train_data_set[first_daily_sample+daily_sample].intraday_volume[current_date_intraday_sample] = 0.0;
			}
			g_train_data_set[first_daily_sample+daily_sample].opn = price;
			g_train_data_set[first_daily_sample+daily_sample].intraday_price[current_date_intraday_sample] = price;
			g_train_data_set[first_daily_sample+daily_sample].intraday_volume[current_date_intraday_sample] = volume;
			current_date_intraday_sample++;
			strcpy(previous_date, date);
		}
		else
		{
			if (current_date_intraday_sample >= MINUTES_IN_A_DAY)
			{
				Erro("current_date_intraday_sample >= MINUTES_IN_A_DAY in read_peixe_piloto_data_set_data(); date: ", date, "");
				exit(1);
			}
			// Dentro de um dia do arquivo Peixe Piloto. Assume que nunca pula mais de um minuto.
			g_train_data_set[first_daily_sample+daily_sample].intraday_price[current_date_intraday_sample] = price;
			g_train_data_set[first_daily_sample+daily_sample].intraday_volume[current_date_intraday_sample] = volume;
			current_date_intraday_sample++;
		}
	}

	for ( ; current_date_intraday_sample < MINUTES_IN_A_DAY; current_date_intraday_sample++)
	{	// Completa as cotacoes no fim do ultimo dia
		previous_price = g_train_data_set[first_daily_sample+daily_sample].intraday_price[current_date_intraday_sample-1];
		g_train_data_set[first_daily_sample+daily_sample].intraday_price[current_date_intraday_sample] = previous_price;
		g_train_data_set[first_daily_sample+daily_sample].intraday_volume[current_date_intraday_sample] = 0.0;
	}
	
	fclose (peixe_piloto_file);
}



char *
get_next_train_data_set ()
{
	struct dirent *train_data_set_file_entry;
	char base_file_name[1000];
	char *aux;
	int num_samples;
	int i;
	
	do
	{
		train_data_set_file_entry = readdir (g_train_dir);
		if (train_data_set_file_entry == NULL)
			return (NULL);
		aux = strrchr (train_data_set_file_entry->d_name, '.');
	} while (strcmp (aux, ".clo") != 0);
	
	strcpy (base_file_name, train_data_set_file_entry->d_name);
	
	aux = strrchr (base_file_name, '.');
	aux[0] = '\0';
	
	num_samples = read_train_data_set_data (base_file_name);
	
	strcpy (g_current_data_set_name, train_data_set_file_entry->d_name);
	aux = strchr(g_current_data_set_name, '.');
	aux[0] = '\0';
	if ((aux = strchr(g_current_data_set_name, '_')) != NULL)
		aux[0] = '\0';
	for(i = 0; g_current_data_set_name[i]; i++)
		g_current_data_set_name[i] = toupper(g_current_data_set_name[i]);
	read_peixe_piloto_data_set_data (g_current_data_set_name);
	
	printf ("# Data set: %s, num_samples: %d\n", g_current_data_set_name, num_samples);

	return (train_data_set_file_entry->d_name);
}



int
find_date_by_date_l(long date_l)
{
	int date;
	
	for (date = 0; date < g_train_num_samples; date++)
	{
		if (date_l == g_train_data_set[date].date_l)
			return (date);
	}
	return (-1);
}



void
remove_days_without_intraday_data()
{
	int i, num_days_with_intraday_data, day;
	TRAIN_DATA_SET *new_g_train_data_set;
	
	for (i = num_days_with_intraday_data = 0; i < g_train_num_samples; i++)
	{
		if (g_train_data_set[i].intraday_price != NULL)
			num_days_with_intraday_data++;
	}

	new_g_train_data_set = (TRAIN_DATA_SET *) alloc_mem (num_days_with_intraday_data * sizeof (TRAIN_DATA_SET));
	for (i = day = 0; i < g_train_num_samples; i++)
	{
		if (g_train_data_set[i].intraday_price != NULL)
		{
			new_g_train_data_set[day] = g_train_data_set[i];
			day++;
		}
	}
	free_g_train_data_set();
	g_train_data_set = new_g_train_data_set;
	g_train_num_samples = num_days_with_intraday_data;
}



void
check_buy_and_sell_prices()
{
	int i;
	
	for (i = 0; i < g_train_num_samples; i++)
	{
		if (g_train_data_set[i].buy != 0.0)
		{
			if (((g_train_data_set[i].buy - 0.0001) > g_train_data_set[i].max) || ((g_train_data_set[i].buy + 0.0001) < g_train_data_set[i].min))
			{
				fprintf(stderr, "%s: buy = %f, max = %f, min = %f\n", long2isodate(g_train_data_set[i].date_l), g_train_data_set[i].buy, g_train_data_set[i].max, g_train_data_set[i].min);
				Erro("buy > max or buy < min in check_buy_and_sell_prices()", "", "");
			}
			if (((g_train_data_set[i].sell - 0.0001) > g_train_data_set[i].max) || ((g_train_data_set[i].sell + 0.0001) < g_train_data_set[i].min))
			{
				fprintf(stderr, "%s: sell = %f, max = %f, min = %f\n", long2isodate(g_train_data_set[i].date_l), g_train_data_set[i].sell, g_train_data_set[i].max, g_train_data_set[i].min);
				Erro("sell > max or sell < min in check_buy_and_sell_prices()", "", "");
			}
		}
	}
}




void
check_max_min_opn_clo_prices()
{
	int i, j;
	float intraday_max, intraday_min;
	
	for (i = 0; i < g_train_num_samples; i++)
	{
		if (g_train_data_set[i].buy != 0.0)
		{
/*			if (fabs(g_train_data_set[i].opn - g_train_data_set[i].intraday_price[0]) > 0.02)
			{
				fprintf(stderr, "opn = %f, intraday_opn = %f\n", g_train_data_set[i].opn, g_train_data_set[i].intraday_price[0]);
				Erro("day_opn too different from intraday_opn", "", "");
			}
*/
			g_train_data_set[i].opn = g_train_data_set[i].intraday_price[0];
			if (fabs(g_train_data_set[i].clo - g_train_data_set[i].intraday_price[MINUTES_IN_A_DAY-1]) > 0.02)
			{
				fprintf(stderr, "clo = %f, intraday_clo = %f\n", g_train_data_set[i].clo, g_train_data_set[i].intraday_price[MINUTES_IN_A_DAY-1]);
				Erro("day_clo too different from intraday_clo", "", "");
			}

			intraday_max = intraday_min = g_train_data_set[i].intraday_price[0];
			for (j = 1; j < MINUTES_IN_A_DAY; j++)
			{
				if (g_train_data_set[i].intraday_price[j] > intraday_max)
					intraday_max = g_train_data_set[i].intraday_price[j];
				if (g_train_data_set[i].intraday_price[j] < intraday_min)
					intraday_min = g_train_data_set[i].intraday_price[j];
			}
/*			if (fabs(g_train_data_set[i].max - intraday_max) > 0.02)
			{
				fprintf(stderr, "max = %f, intraday_max = %f\n", g_train_data_set[i].max, intraday_max);
				Erro("day_max too different from intraday_max", "", "");
			}
			if (fabs(g_train_data_set[i].min - intraday_min) > 0.02)
			{
				fprintf(stderr, "min = %f, intraday_min = %f\n", g_train_data_set[i].min, intraday_min);
				Erro("day_min too different from intraday_min", "", "");
			}
*/
			g_train_data_set[i].max = intraday_max;
			g_train_data_set[i].min = intraday_min;
		}
	}
}


void
init_data_sets ()
{
	char *first_data_set_name;
	
	if ((g_train_dir = opendir (DATA_PATH)) == NULL)
	{
		Erro ("Could not open DATA directory named: ", DATA_PATH, "");	
		return;
	}
	
	if ((first_data_set_name = get_next_train_data_set ()) == NULL)
		Erro ("Could not initialize first train data set from directory:", DATA_PATH, "");

	remove_days_without_intraday_data();
	check_max_min_opn_clo_prices();
	check_buy_and_sell_prices();
	compute_target_return_and_avg();
}



void
save_data_sets(char *dir_name, char *stock_name)
{
	FILE *file;
	FILE *file2;
	char file_name[1000];
	int i, j, hora, minuto;	
	
	strcpy(file_name, dir_name);
	strcat(file_name, stock_name);
	strcat(file_name, ".clo");
	file = fopen(file_name, "w");
	if (file == NULL)
	{
		Erro("Could not open ", file_name, " in save_data_sets().");
		exit(1);
	}
	for (i = 0; i < g_train_num_samples; i++)
	{
		fprintf(file, "%s %.6f\n", long2isodate(g_train_data_set[i].date_l), g_train_data_set[i].clo);
		if (g_train_data_set[i].intraday_price != NULL)
		{
			strcpy(file_name, dir_name);
			strcat(file_name, stock_name);
			strcat(file_name, long2isodate(g_train_data_set[i].date_l));
			strcat(file_name, ".txt");
			file2 = fopen(file_name, "w");
			if (file2 == NULL)
			{
				Erro("Could not open ", file_name, " in save_data_sets().");
				exit(1);
			}
			j = 0;
			for (hora = 11; hora < 7+11; hora++) // Isso ee uma aproximaccao da hora certa. 
							     // Tem dias no ano (horario de verao ou nao) 
							     // que o pregao comecca aas 11:00 e tem dia que comecca aas 10:00.
				for (minuto = 0; minuto < 60; minuto++)
					fprintf(file2, "%s %s %02d:%02d;%.2f\n", 
					  long2isodate(g_train_data_set[i].date_l), 
					  long2dows(g_train_data_set[i].date_l), 
					  hora, minuto,
					  g_train_data_set[i].intraday_price[j++]);
			fclose(file2);
		}
	}
	fclose(file);


	strcpy(file_name, dir_name);
	strcat(file_name, stock_name);
	strcat(file_name, ".max");
	file = fopen(file_name, "w");
	if (file == NULL)
	{
		Erro("Could not open ", file_name, " in save_data_sets().");
		exit(1);
	}
	for (i = 0; i < g_train_num_samples; i++)
		fprintf(file, "%s %.6f\n", long2isodate(g_train_data_set[i].date_l), g_train_data_set[i].max);
	fclose(file);


	strcpy(file_name, dir_name);
	strcat(file_name, stock_name);
	strcat(file_name, ".min");
	file = fopen(file_name, "w");
	if (file == NULL)
	{
		Erro("Could not open ", file_name, " in save_data_sets().");
		exit(1);
	}
	for (i = 0; i < g_train_num_samples; i++)
		fprintf(file, "%s %.6f\n", long2isodate(g_train_data_set[i].date_l), g_train_data_set[i].min);
	fclose(file);
}
