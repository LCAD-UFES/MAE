#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include "../wnn_pred.h"
#include "timeutil.h"
#ifdef	USE_IPC
#include "ipc.h"
#include "messages.h"
#endif
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
	else if (date_l == timePlusDays(g_train_data_set[LAST_SAMPLE].date_l, 5)) // se o day index procurado for na proxima semana ainda nao disponivel
	{
		return (LAST_SAMPLE + 5);
	}
#endif
	else
	{
		return (-1);
	}
}



FILE *
get_file(char *base_file_name, char *dir_name, char *file_type)
{
	FILE *data_set_file;
	char file_name[1000];
	
	strcpy (file_name, dir_name);
	strcat (file_name, base_file_name);
	strcat (file_name, file_type);
	if ((data_set_file = fopen (file_name, "r")) == NULL)
	{
		Erro ("Could not open file: ", file_name, "");
		return (NULL);
	}
	return (data_set_file);
}



FILE *
get_peixe_piloto_file (char *base_file_name, char *file_type)
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
	double price;
	long date_l_1, date_l_n;
	
	if (fgets (data_set_file_first_line, 900, data_set_file) == NULL)
	{
		Erro ("Could not read first line of the dataset (.clo file)", "", "");
		return (0);
	}
		
	while (fgets (aux, 900, data_set_file) != NULL)
		strcpy (data_set_file_last_line, aux);
	
	if (sscanf (data_set_file_first_line, "%s %lf", (char *)date, &price) != 2)
	{
		Erro ("Could not read first date of the dataset (.clo file)", "", "");
		return (0);
	}
	date_l_1 = isodate2long (date);
	
	if (sscanf (data_set_file_last_line, "%s %lf", (char *)date, &price) != 2)
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
fill_in_missing_samples (TRAIN_DATA_SET *data_set, long previous_date, long current_date, double closing_price, int current_sample)
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
			data_set[current_sample+i] = data_set[current_sample - 1];
			data_set[current_sample+i].date_l = timePlusDays (data_set[current_sample - 1].date_l, i+1);
		}
	}		

	data_set[new_current_sample].date_l = current_date;
	data_set[new_current_sample].clo = closing_price;

	return (new_current_sample);
}



void
free_data_set(TRAIN_DATA_SET *data_set)
{
	int i;
	
	for (i = 0; i < g_train_num_samples; i++)
	{
		if (data_set[i].intraday_price != NULL)
			free(data_set[i].intraday_price);
		if (data_set[i].intraday_volume != NULL)
			free(data_set[i].intraday_volume);
	}

	free(data_set);
	data_set = NULL;
}



int
read_data_set(TRAIN_DATA_SET **in_data_set, char *base_file_name, char *dir_name)
{
	TRAIN_DATA_SET *data_set;
	FILE *clo;
	FILE *opn;
	FILE *max;
	FILE *min;
	FILE *vol;
	char data_set_file_line[1000], date[100];
	int i;
	long current_date, previous_date;
	double closing_price;
	
	if (*in_data_set != NULL)
		free_data_set(*in_data_set);

	clo = get_file (base_file_name, dir_name, ".clo");
	g_train_num_samples = count_samples (clo);
	*in_data_set = data_set = (TRAIN_DATA_SET *) alloc_mem (g_train_num_samples * sizeof (TRAIN_DATA_SET));
	
	opn = get_file (base_file_name, dir_name, ".opn");
	max = get_file (base_file_name, dir_name, ".max");
	min = get_file (base_file_name, dir_name, ".min");
	vol = get_file (base_file_name, dir_name, ".vol");
	i = 0;
	previous_date = 0;
	while (fgets (data_set_file_line, 900, clo) != NULL)
	{
		if (sscanf (data_set_file_line, "%s %lf", (char *)date, &closing_price) != 2)
			break;
		
		current_date = isodate2long (date);
		i = fill_in_missing_samples (data_set, previous_date, current_date, closing_price, i);
		previous_date = current_date;

		fgets (data_set_file_line, 900, opn);
		if (sscanf (data_set_file_line, "%s %lf", (char *)date, &(data_set[i].opn)) != 2)
		{
			Erro ("Could not read from file: ", base_file_name, ".opn");
			return (0);
		}
		fgets (data_set_file_line, 900, max);
		if (sscanf (data_set_file_line, "%s %lf", (char *)date, &(data_set[i].max)) != 2)
		{
			Erro ("Could not read from file: ", base_file_name, ".max");
			return (0);
		}
		fgets (data_set_file_line, 900, min);
		if (sscanf (data_set_file_line, "%s %lf", (char *)date, &(data_set[i].min)) != 2)
		{
			Erro ("Could not read from file: ", base_file_name, ".min");
			return (0);
		}
		fgets (data_set_file_line, 900, vol);
		if (sscanf (data_set_file_line, "%s %lf", (char *)date, &(data_set[i].vol)) != 2)
		{
			Erro ("Could not read from file: ", base_file_name, ".vol");
			return (0);
		}
		data_set[i].intraday_price = NULL;
		data_set[i].intraday_volume = NULL;
		
		// printf ("%d: %s ; %s ; % .6lf ; % .6lf ; % .6lf ; % .6lf ; % .6lf ; \n", i,
		//	long2isodate (data_set[i].date_l), long2dows (data_set[i].date_l), 
		//	data_set[i].clo, 
		//	data_set[i].opn,
		//	data_set[i].max,
		//	data_set[i].min,
		//	data_set[i].vol);
		i++;
	}

	g_train_num_samples = i;
	
	fclose (clo);
	fclose (opn);
	fclose (max);
	fclose (min);
	fclose (vol);
	
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
check_data_set_file_line(char *date, char *week_day, int hour, int minute, double price, double num_trades, double volume)
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
read_peixe_piloto_data_set_data (char *data_set_name, TRAIN_DATA_SET *data_set)
{
	FILE *peixe_piloto_file;
	long daily_sample, first_daily_sample = 0;
	int current_date_intraday_sample;
	char data_set_file_line[1000], date[100], week_day[100], previous_date[100], aux[2000];
	int hour, minute;
	double price, num_trades, volume, previous_price, day_price_corrention_factor;
	
	peixe_piloto_file = get_peixe_piloto_file (data_set_name, "_Volume.TXT");
	daily_sample = -1;
	current_date_intraday_sample = 0;
	previous_date[0] = '\0';
	while (fgets (data_set_file_line, 900, peixe_piloto_file) != NULL)
	{
		if (sscanf (data_set_file_line, "%s %s %d:%d;%lf;%lf;%lf", (char *)date, (char *)week_day, &hour, &minute, &price, &num_trades, &volume) != 7)
		{
			strcpy(aux, data_set_name);
			strcat(aux, ", line = ");
			strcat(aux, data_set_file_line);
			Erro ("Could not read line of Peixe Piloto file ", aux, " in read_peixe_piloto_data_set_data().");
			exit(1);
		}
		check_data_set_file_line(date, week_day, hour, minute, price, num_trades, volume);
		
		if (daily_sample == -1)
		{	// Inicio do arquivo Peixe Piloto
			daily_sample = 0;
			first_daily_sample = find_daily_sample_index(date);
			data_set[first_daily_sample+daily_sample].intraday_price = (double *) alloc_mem(7*60 * sizeof(double));
			data_set[first_daily_sample+daily_sample].intraday_volume = (double *) alloc_mem(7*60 * sizeof(double));
			
			for (current_date_intraday_sample = 0; current_date_intraday_sample < minute; current_date_intraday_sample++)
			{	// Completa as cotacoes no inicio do dia
				data_set[first_daily_sample+daily_sample].intraday_price[current_date_intraday_sample] = price;
				data_set[first_daily_sample+daily_sample].intraday_volume[current_date_intraday_sample] = 0.0;
			}
			data_set[first_daily_sample+daily_sample].intraday_price[current_date_intraday_sample] = price;
			data_set[first_daily_sample+daily_sample].intraday_volume[current_date_intraday_sample] = volume;
			current_date_intraday_sample++;
			strcpy(previous_date, date);
		}
		else if (strcmp(date, previous_date) != 0)
		{	// Fim de um dia no arquivo Peixe Piloto
			previous_price = data_set[first_daily_sample+daily_sample].intraday_price[current_date_intraday_sample-1];
			for ( ; current_date_intraday_sample < 7*60; current_date_intraday_sample++)
			{	// Completa as cotacoes no fim do dia
				data_set[first_daily_sample+daily_sample].intraday_price[current_date_intraday_sample] = previous_price;
				data_set[first_daily_sample+daily_sample].intraday_volume[current_date_intraday_sample] = 0.0;
			}

			// Corrige discrepancia entre dados diarios e intraday
			if (data_set[first_daily_sample+daily_sample].opn == 0.0)
			{
				Erro ("Open daily price equal zero in read_peixe_piloto_data_set_data(); date = ", date, "");
				exit(1);
			}
			day_price_corrention_factor = data_set[first_daily_sample+daily_sample].clo / data_set[first_daily_sample+daily_sample].intraday_price[7*60-1];
			for (current_date_intraday_sample = 0; current_date_intraday_sample < 7*60; current_date_intraday_sample++)
			{
				data_set[first_daily_sample+daily_sample].intraday_price[current_date_intraday_sample] *= day_price_corrention_factor;
				data_set[first_daily_sample+daily_sample].intraday_volume[current_date_intraday_sample] *= day_price_corrention_factor;
			}
			if (fabs(data_set[first_daily_sample+daily_sample].opn - data_set[first_daily_sample+daily_sample].intraday_price[0]) > 0.01)
			{
				//printf("# Difference between intraday and daily prices > R$0.01; date = %s, difference = %f\n", date, fabs(data_set[first_daily_sample+daily_sample].opn - data_set[first_daily_sample+daily_sample].intraday_price[0]));
			}

			daily_sample += timeDifferenceInDaysStrides (isodate2long(previous_date), isodate2long(date), 1);
			if (data_set[first_daily_sample+daily_sample].date_l != isodate2long(date))
			{
				Erro ("Date mismatch in read_peixe_piloto_data_set_data(); date = ", date, "");
				exit(1);
			}

			data_set[first_daily_sample+daily_sample].intraday_price = (double *) alloc_mem(7*60 * sizeof(double));
			data_set[first_daily_sample+daily_sample].intraday_volume = (double *) alloc_mem(7*60 * sizeof(double));
			
			for (current_date_intraday_sample = 0; current_date_intraday_sample < minute; current_date_intraday_sample++)
			{	// Completa as cotacoes no inicio do dia
				data_set[first_daily_sample+daily_sample].intraday_price[current_date_intraday_sample] = price;
				data_set[first_daily_sample+daily_sample].intraday_volume[current_date_intraday_sample] = 0.0;
			}
			data_set[first_daily_sample+daily_sample].intraday_price[current_date_intraday_sample] = price;
			data_set[first_daily_sample+daily_sample].intraday_volume[current_date_intraday_sample] = volume;
			current_date_intraday_sample++;
			strcpy(previous_date, date);
		}
		else
		{
			if (current_date_intraday_sample >= 7*60)
			{
				Erro("current_date_intraday_sample >= MINUTES_IN_A_DAY in read_peixe_piloto_data_set_data(); date: ", date, "");
				exit(1);
			}
			// Dentro de um dia do arquivo Peixe Piloto. Assume que nunca pula mais de um minuto.
			data_set[first_daily_sample+daily_sample].intraday_price[current_date_intraday_sample] = price;
			data_set[first_daily_sample+daily_sample].intraday_volume[current_date_intraday_sample] = volume;
			current_date_intraday_sample++;
		}
	}

	for ( ; current_date_intraday_sample < 7*60; current_date_intraday_sample++)
	{	// Completa as cotacoes no fim do ultimo dia
		previous_price = data_set[first_daily_sample+daily_sample].intraday_price[current_date_intraday_sample-1];
		data_set[first_daily_sample+daily_sample].intraday_price[current_date_intraday_sample] = previous_price;
		data_set[first_daily_sample+daily_sample].intraday_volume[current_date_intraday_sample] = 0.0;
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
	
	num_samples = read_data_set(&g_train_data_set, base_file_name, DATA_PATH);
	read_data_set(&g_ref_data_set, "_adj", REF_STOCK);
	if (g_train_num_samples != num_samples)
	{
		Erro("g_train_num_samples != num_samples in get_next_train_data_set()", "", "");
		exit(1);
	}
	
	// read stock intraday data
	strcpy (g_current_data_set_name, train_data_set_file_entry->d_name);
	aux = strchr(g_current_data_set_name, '.');
	aux[0] = '\0';
	if ((aux = strchr(g_current_data_set_name, '_')) != NULL)
		aux[0] = '\0';
	for(i = 0; g_current_data_set_name[i]; i++)
		g_current_data_set_name[i] = toupper(g_current_data_set_name[i]);
	read_peixe_piloto_data_set_data (g_current_data_set_name, g_train_data_set);
	
	// read reference intraday data
	strcpy (base_file_name, REF_STOCK);
	aux = strrchr(base_file_name, '/');
	aux[0] = '\0';
	for(i = 1; aux[i]; i++)
		aux[i] = toupper(aux[i]);
	read_peixe_piloto_data_set_data (aux+1, g_ref_data_set);
	
	// printf ("# Data set: %s, num_samples: %d\n", g_current_data_set_name, num_samples);

	return (train_data_set_file_entry->d_name);
}



void
get_first_and_last_complete_weeks ()
{
	int i, num_weeks;
	char date_first_sample[1000];
	char dow_first_sample[1000];
	char date_last_sample[1000];
	char dow_last_sample[1000];
	
	i = 0;
	num_weeks = 0;
	do
	{
		if (strcmp (long2dows(g_train_data_set[i].date_l), TARGET_DAY_WEEK) == 0)
			num_weeks++;
		if (num_weeks == SAMPLE_SIZE+1)
			break;
		i++;
	} while (i < g_train_num_samples);
	
	FIRST_SAMPLE = i;
	
	i = g_train_num_samples - 1;
	num_weeks = 0;
	do
	{
		if (strcmp (long2dows(g_train_data_set[i].date_l), TARGET_DAY_WEEK) == 0)
			break;
		i--;
	} while (i > 0);
	
	LAST_SAMPLE = i;
	
	strcpy(date_first_sample, long2isodate(g_train_data_set[FIRST_SAMPLE].date_l));
	strcpy(dow_first_sample, long2dows(g_train_data_set[FIRST_SAMPLE].date_l));
	strcpy(date_last_sample, long2isodate(g_train_data_set[LAST_SAMPLE].date_l));
	strcpy(dow_last_sample, long2dows(g_train_data_set[LAST_SAMPLE].date_l));
	// printf ("# First sample number = %d, date = %s %s; Last sample number = %d, date = %s %s; Number of weeks = %d\n", 
	//	FIRST_SAMPLE, date_first_sample, dow_first_sample, LAST_SAMPLE, date_last_sample, dow_last_sample, 
	//	1 + (LAST_SAMPLE-FIRST_SAMPLE) / 5);
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
	
	if ((first_data_set_name = get_next_train_data_set (train_dir)) == NULL)
		Erro ("Could not initialize first train data set from directory:", DATA_PATH, "");

	get_first_and_last_complete_weeks ();
	
	closedir(train_dir);
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



void
load_intraday_data(char *stock_name, int semana)
{
	char stock[1000];
	char *aux;
	char command[1000];

	if (semana >= g_tt_num_samples)
	{
		Erro("No existing intraday data in load_intraday_data(); week in the future?", " ", "");
#ifndef	IPC_ON_LINE
		exit(1);
#endif
	}
	strcpy(stock, g_argv[g_tt_data_set[0][semana].file_number]);
	aux = strrchr(stock, '/') + 1;
	strcpy(stock_name, aux);
	aux = strrchr(stock_name, '.');
	aux[0] = '\0';
	
	system("rm -f DATA_TEMP/*");
	strcpy(command, "cp DATA/");
	strcat(command, stock_name);
	strcat(command, "/* DATA_TEMP/");
	system(command);
	
	init_data_sets();
	//save_data_sets("temp/", stock_name);
}



TT_DATA_SET *
read_data_from_file (FILE *data_set_file, int file_number)
{
	char data_set_file_line[1000], date[100], day_week[100];
	double target_return_buy_sell, wnn_predicted_return_buy_sell, wnn_prediction_error_buy_sell;
	double target_return_sell_buy, wnn_predicted_return_sell_buy, wnn_prediction_error_sell_buy;
	double confidence;
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

		if ((num_fields_read = sscanf(data_set_file_line, "%s %s ; %lf ; %lf ; %lf ;  %lf ; %lf ; %lf  ; %lf  ;", (char *)date, (char *)day_week, 
					      &target_return_buy_sell, &wnn_predicted_return_buy_sell, &wnn_prediction_error_buy_sell,
					      &target_return_sell_buy, &wnn_predicted_return_sell_buy, &wnn_prediction_error_sell_buy,
					      &confidence)) != 9)
		{
			if (num_fields_read == 2)
			{
				g_no_extra_predicted_sample = 0;
				sscanf(data_set_file_line, "%s %s ;  ; %lf ;  ;  ; %lf ;  ;  ;", (char *)date, (char *)day_week, 
					&wnn_predicted_return_buy_sell, &wnn_predicted_return_sell_buy);
				target_return_buy_sell = wnn_prediction_error_buy_sell = 0.0;
				target_return_sell_buy = wnn_prediction_error_sell_buy = 0.0;
				confidence = 0.0;
			}
			else
				break;
		}			
		tt_data_set[i].date_l = isodate2long(date);
		tt_data_set[i].target_return_buy_sell = target_return_buy_sell;
		tt_data_set[i].wnn_predicted_return_buy_sell = wnn_predicted_return_buy_sell;
		tt_data_set[i].wnn_prediction_error_buy_sell = wnn_prediction_error_buy_sell;
		tt_data_set[i].target_return_sell_buy = target_return_sell_buy;
		tt_data_set[i].wnn_predicted_return_sell_buy = wnn_predicted_return_sell_buy;
		tt_data_set[i].wnn_prediction_error_sell_buy = wnn_prediction_error_sell_buy;
		tt_data_set[i].confidence = confidence;
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
	
	g_tt_data_set[file_number] = (TT_DATA_SET *) malloc(g_tt_num_samples * sizeof(TT_DATA_SET));
	for (i = 0; i < g_tt_num_samples; i++)
	{
		if (g_tt_data_set[0][i].date_l != tt_data_set[i].date_l)
		{
			printf("Erro: Date of sample %d of files %s and %s differ\n", 
				i, g_argv[g_tt_data_set[0][i].file_number], g_argv[tt_data_set[i].file_number]);
			exit(1);
		}
		g_tt_data_set[file_number][i] = tt_data_set[i];
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
	
	tt_data_set = read_data_from_file(file, file_number);
	add_data_to_g_tt_data_set(file_number, tt_data_set);
	free(tt_data_set);
	
	fclose(file);
}