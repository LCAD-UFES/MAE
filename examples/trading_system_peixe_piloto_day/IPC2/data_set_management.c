#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include "../wnn_pred.h"
#include "timeutil.h"
#include "ipc.h"
#include "messages.h"
#include "trading_system.h"



int 
compair_dates(const void *date_key, const void *current_date)
{
	TRAIN_DATA_SET *date1, *date2;
	
	date1 = (TRAIN_DATA_SET *) date_key;
	date2 = (TRAIN_DATA_SET *) current_date;
	
	if (date1->date_l < date2->date_l)
		return (-1);
	else if (date1->date_l > date2->date_l)
		return (1);
	else
		return(0);
}



int
find_day_index_by_date_l(long date_l)
{
	int day_index;
	
	TRAIN_DATA_SET *data_set_entry, date_key;
	
	date_key.date_l = date_l;
	data_set_entry = (TRAIN_DATA_SET *) bsearch((const void *) &date_key, (const void *) g_train_data_set,
                     				    (size_t) g_train_num_samples, sizeof (TRAIN_DATA_SET), compair_dates);
	if (data_set_entry != NULL)
	{
		day_index = data_set_entry - g_train_data_set;
		return (day_index);
	}
#ifdef	IPC_ON_LINE
	else if (date_l == timePlusDays(g_train_data_set[g_train_num_samples-1].date_l, 1)) // Se o day index procurado for no proximo dia ainda nao disponivel. Assume que g_train_data_set[] nao tem gaps.
	{
		return (g_train_num_samples);
	}
#endif
	else
	{
		return (-1);
	}
}



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
		Erro ("Could not open file: ", file_name, " in get_file()");
		return (NULL);
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
#ifndef	BUILD_DAILY_BUY_SELL_FILES
	FILE *buy;
	FILE *sell;
	char date_buy_sell[100];
	int state = 0;
#endif
	char data_set_file_line[1000], date[100];
	int i;
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
#ifndef	BUILD_DAILY_BUY_SELL_FILES
	buy = get_buy_sell_file (base_file_name, ".buy");
	sell = get_buy_sell_file (base_file_name, ".sell");
#endif
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
#ifndef	BUILD_DAILY_BUY_SELL_FILES
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
#endif
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
#ifndef	BUILD_DAILY_BUY_SELL_FILES
	fclose (buy);
	fclose (sell);
#endif	
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
		Erro ("Could not find daily sample index in find_daily_sample_index().", "", "");
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
			g_train_data_set[first_daily_sample+daily_sample].intraday_price = (float *) alloc_mem(7*60 * sizeof(float));
			g_train_data_set[first_daily_sample+daily_sample].intraday_volume = (float *) alloc_mem(7*60 * sizeof(float));	
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
			for ( ; current_date_intraday_sample < 7*60; current_date_intraday_sample++)
			{	// Completa as cotacoes no fim do dia
				g_train_data_set[first_daily_sample+daily_sample].intraday_price[current_date_intraday_sample] = previous_price;
				g_train_data_set[first_daily_sample+daily_sample].intraday_volume[current_date_intraday_sample] = 0.0;
			}

			// Corrige discrepancia entre dados diarios e intraday
			day_price_corrention_factor = g_train_data_set[first_daily_sample+daily_sample].clo / previous_price;
			for (current_date_intraday_sample = 0; current_date_intraday_sample < 7*60; current_date_intraday_sample++)
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

			g_train_data_set[first_daily_sample+daily_sample].intraday_price = (float *) alloc_mem(7*60 * sizeof(float));
			g_train_data_set[first_daily_sample+daily_sample].intraday_volume = (float *) alloc_mem(7*60 * sizeof(float));
			
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

	for ( ; current_date_intraday_sample < 7*60; current_date_intraday_sample++)
	{	// Completa as cotacoes no fim do ultimo dia
		previous_price = g_train_data_set[first_daily_sample+daily_sample].intraday_price[current_date_intraday_sample-1];
		g_train_data_set[first_daily_sample+daily_sample].intraday_price[current_date_intraday_sample] = previous_price;
		g_train_data_set[first_daily_sample+daily_sample].intraday_volume[current_date_intraday_sample] = 0.0;
	}
	
	fclose (peixe_piloto_file);
}



char *
get_next_train_data_set (DIR *train_dir)
{
	struct dirent *train_data_set_file_entry;
	char base_file_name[1000];
	char *aux;
	int num_samples;
	int i;
	
	do
	{
		train_data_set_file_entry = readdir (train_dir);
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
	
	// printf ("# Data set: %s, num_samples: %d\n", g_current_data_set_name, num_samples);

	return (train_data_set_file_entry->d_name);
}



char *
get_next_train_and_intraday_data_sets (DIR *train_dir)
{
	struct dirent *train_data_set_file_entry;
	char base_file_name[1000];
	char *aux;
	int num_samples;
	int i;
	
	do
	{
		train_data_set_file_entry = readdir (train_dir);
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
	
	//printf ("# Data set: %s, num_samples: %d\n", g_current_data_set_name, num_samples);

	return (train_data_set_file_entry->d_name);
}
		     
		     

void
init_data_sets ()
{
	char *first_data_set_name;
	DIR *train_dir;
	
	if ((train_dir = opendir (DATA_PATH)) == NULL)
	{
		Erro ("Could not open DATA directory named: ", DATA_PATH, "");	
		return;
	}
	
	if ((first_data_set_name = get_next_train_and_intraday_data_sets (train_dir)) == NULL)
		Erro ("Could not initialize first train data set from directory:", DATA_PATH, "");

	closedir(train_dir);
}
		     
		     

void
init_daily_data_set ()
{
	char *first_data_set_name;
	DIR *train_dir;
	
	if ((train_dir = opendir (DATA_PATH)) == NULL)
	{
		Erro ("Could not open DATA directory named: ", DATA_PATH, "");	
		return;
	}
	
	if ((first_data_set_name = get_next_train_data_set (train_dir)) == NULL)
		Erro ("Could not initialize first train data set from directory:", DATA_PATH, "");

	closedir(train_dir);
}



void
get_stock_symbol_from_tt_file_name(char *stock_name, char *tt_file_name)
{
	char stock[1000];
	char *aux;

	strcpy(stock, tt_file_name);
	aux = strrchr(stock, '/') + 1;
	strcpy(stock_name, aux);
	aux = strrchr(stock_name, '.');
	aux[0] = '\0';
}



void
load_intraday_data(char *stock_name, int day)
{
	char command[1000];

	if (day >= g_tt_num_samples)
		Erro("No existing intraday data in load_intraday_data(); week in the future?", " ", "");
	
	get_stock_symbol_from_tt_file_name(stock_name, g_argv[g_tt_data_set[0][day].file_number]);

	system("rm -f DATA_TEMP/*");
	strcpy(command, "cp DATA/");
	strcat(command, stock_name);
	strcat(command, "/* DATA_TEMP/");
	system(command);
	
	init_data_sets();
}



void
load_intraday_data2(char **stocks_names, int stock_index)
{
	char stock[1000];
	char *aux;
	char command[1000];

	aux = strrchr(stocks_names[stock_index], '/') + 1;
	strcpy(stock, aux);
	aux = strrchr(stock, '.');
	aux[0] = '\0';
	
	system("rm -f DATA_TEMP/*");
	strcpy(command, "cp DATA/");
	strcat(command, stock);
	strcat(command, "/* DATA_TEMP/");
	system(command);
	
	init_data_sets();
}



void
load_daily_data(char **stocks_names, int stock_index)
{
	char stock[1000];
	char *aux;
	char command[1000];

	aux = strrchr(stocks_names[stock_index], '/') + 1;
	strcpy(stock, aux);
	aux = strrchr(stock, '.');
	aux[0] = '\0';
	
	system("rm -f DATA_TEMP/*");
	strcpy(command, "cp DATA/");
	strcat(command, stock);
	strcat(command, "/* DATA_TEMP/");
	system(command);
	
	init_daily_data_set();
}



TT_DATA_SET *
read_data_from_tt_file (FILE *data_set_file, int file_number)
{
	char data_set_file_line[1000], date[100], day_week[100];
	float target_return, wnn_predicted_return, wnn_prediction_error;
	int i;
	int num_fields_read;
	TT_DATA_SET *tt_data_set;

	
	if (g_tt_num_samples == 0)
	{
		while (fgets(data_set_file_line, 900, data_set_file) != NULL)
		{
			if (data_set_file_line[0] == '#')
				continue;
			g_tt_num_samples++;
		}
		rewind(data_set_file);
	}
	
	tt_data_set = (TT_DATA_SET *) malloc(g_tt_num_samples * sizeof(TT_DATA_SET));
	
	i = 0;
	while (fgets(data_set_file_line, 900, data_set_file) != NULL)
	{
		if (data_set_file_line[0] == '#')
			continue;

		if ((num_fields_read = sscanf(data_set_file_line, "%s %s ; %f ; %f ; %f  ;", (char *)date, (char *)day_week, 
					       &target_return, &wnn_predicted_return, &wnn_prediction_error)) != 5)
		{
			if (num_fields_read == 2)
			{
				g_no_extra_predicted_sample = 0;
				sscanf(data_set_file_line, "%s %s ;  ; %f ;  ;", (char *)date, (char *)day_week, 
					&wnn_predicted_return);
				target_return = wnn_prediction_error = 0.0;
			}
			else
				break;
		}			
		tt_data_set[i].date_l = isodate2long(date);
		tt_data_set[i].target_return = target_return;
		tt_data_set[i].wnn_predicted_return = wnn_predicted_return;
		tt_data_set[i].wnn_prediction_error = wnn_prediction_error;
		tt_data_set[i].file_number = file_number;
		i++;
	}
	if ((g_tt_num_samples != i) && (g_no_extra_predicted_sample != 0))
	{
		printf("Erro: number of samples does not match. File name = %s, g_tt_num_samples = %d, i = %d\n", 
			g_argv[file_number], g_tt_num_samples, i);
		exit(1);
	}

	return (tt_data_set);
}



void
add_data_to_g_tt_data_set(int file_number, TT_DATA_SET *tt_data_set)
{
	int i;
	
	if (g_tt_data_set == NULL)
	{
		g_tt_data_set = (TT_DATA_SET **) malloc(g_argc * sizeof(TT_DATA_SET *));
		g_tt_data_set[0] = (TT_DATA_SET *) malloc(g_tt_num_samples * sizeof(TT_DATA_SET));
		g_tt_data_set[file_number] = (TT_DATA_SET *) malloc(g_tt_num_samples * sizeof(TT_DATA_SET));
		for (i = 0; i < g_tt_num_samples; i++)
		{
			g_tt_data_set[0][i] = tt_data_set[i];
			g_tt_data_set[file_number][i] = tt_data_set[i];
		}
		return;
	}
	else
	{
		g_tt_data_set[file_number] = (TT_DATA_SET *) malloc(g_tt_num_samples * sizeof(TT_DATA_SET));
		for (i = 0; i < g_tt_num_samples; i++)
		{
			/* O >= previlegia acoes examinadas por ultimo */
			if (tt_data_set[i].wnn_predicted_return >= g_tt_data_set[0][i].wnn_predicted_return)
			{
				if (g_tt_data_set[0][i].date_l != tt_data_set[i].date_l)
				{
					printf("Erro: Date of sample %d of files %s and %s differ\n", 
						i, g_argv[g_tt_data_set[0][i].file_number], g_argv[tt_data_set[i].file_number]);
					exit(1);
				}
				g_tt_data_set[0][i] = tt_data_set[i];
			}
			g_tt_data_set[file_number][i] = tt_data_set[i];
		}
	}
}



void
free_g_tt_data_set(int argc)
{
	int i;
	
	for (i = 0; i < argc; i++)
	{
		free(g_tt_data_set[i]);
	}

	free(g_tt_data_set);
}


void
process_one_file(int file_number)
{
	FILE *file;
	TT_DATA_SET *tt_data_set;
	
	if ((file = fopen(g_argv[file_number], "r")) == NULL)
	{
		printf("Erro: Could not open file: %s\n", g_argv[file_number]);
		exit(1);
	}
	
	tt_data_set = read_data_from_tt_file(file, file_number);
	add_data_to_g_tt_data_set(file_number, tt_data_set);
	free(tt_data_set);
	
	fclose(file);
}



int
get_num_days_of_intraday_data(int stock, char **stocks_names)
{
	int i, num_days;
	
	for (i = 0; i < g_train_num_samples; i++)
	{
		if (g_train_data_set[i].intraday_price != NULL)
			break;
	}
	num_days = g_train_num_samples - i;

	return (num_days);
}



void
build_fake_tt_data_set(int stock, int num_days)
{
	int i;
	TT_DATA_SET *tt_data_set;
	
	g_tt_num_samples = num_days;

	tt_data_set = (TT_DATA_SET *) malloc(g_tt_num_samples * sizeof(TT_DATA_SET));
	for (i = 0; i < g_tt_num_samples; i++)
	{
		tt_data_set[i].date_l = g_train_data_set[g_train_num_samples - num_days + i].date_l;
		tt_data_set[i].target_return = 0.0;
		tt_data_set[i].wnn_predicted_return = 0.0;
		tt_data_set[i].wnn_prediction_error = 0.0;
		
		if (g_train_data_set[g_train_num_samples - num_days + i].intraday_price != NULL)
			tt_data_set[i].file_number = stock;
		else
			tt_data_set[i].file_number = -1;
	}
		
	g_tt_data_set = (TT_DATA_SET **) malloc(1 * sizeof(TT_DATA_SET *));
	g_tt_data_set[0] = tt_data_set;
}



BUY_SELL_FILES
open_buy_sell_files(char *base_file_name)
{
	char dir_name[1000];
	char file_name[1000];
	char buy_file_name[1000];
	char sell_file_name[1000];
	char *aux;
	int i;
	BUY_SELL_FILES buy_sell_files;
	
	aux = strrchr (base_file_name, '/');
	strcpy(dir_name, aux+1);
	aux = strrchr (dir_name, '.');
	aux[0] = '\0';
	strcpy(file_name, dir_name);
	for(i = 0; file_name[i]; i++)
		file_name[i] = tolower(file_name[i]);
	aux[0] = '/';
	aux[1] = '\0';

	strcpy (buy_file_name, BUY_SELL_DATA_PATH);
	strcat (buy_file_name, dir_name);
	strcat (buy_file_name, file_name);
	strcat (buy_file_name, "_adj.buy");
	if ((buy_sell_files.buy_file = fopen (buy_file_name, "w")) == NULL)
	{
		Erro ("Could not open file: ", file_name, " in open_buy_sell_files().");
		exit(1);
	}

	strcpy (sell_file_name, BUY_SELL_DATA_PATH);
	strcat (sell_file_name, dir_name);
	strcat (sell_file_name, file_name);
	strcat (sell_file_name, "_adj.sell");
	if ((buy_sell_files.sell_file = fopen (sell_file_name, "w")) == NULL)
	{
		Erro ("Could not open file: ", file_name, " in open_buy_sell_files().");
		exit(1);
	}
	return (buy_sell_files);
}



void
save_buy_sell_file_line(BUY_SELL_FILES buy_sell_files, int day, STATE state)
{
	fprintf(buy_sell_files.buy_file,  "%s %.6f\n", long2isodate(g_tt_data_set[0][day].date_l), state.buy_price);
	fprintf(buy_sell_files.sell_file, "%s %.6f\n", long2isodate(g_tt_data_set[0][day].date_l), state.sell_price);
}



void
close_buy_sell_files(BUY_SELL_FILES buy_sell_files)
{
	fclose(buy_sell_files.buy_file);
	fclose(buy_sell_files.sell_file);
}



void
moving_average_warm_up(int target_day_index)
{
	g_moving_average_current_minute = -1;
}
