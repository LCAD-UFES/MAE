#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <dirent.h>
#include "timeutil.h"
#ifdef	USE_IPC
#include "ipc.h"
#include "messages.h"
#endif
#include "trading_system.h"


// tipos de arquivo
#define PEIXE_PILOTO 	0
#define ENFOQUE 	1



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
rewind_file_until_begin_line(FILE *file)
{
	if (ftell(file) == 0)
		return;
	while (fgetc(file) != '\n')
	{
		fseek(file, -2, SEEK_CUR);
		if (ftell(file) == 0)
			return;
	}
}



int
read_isodate_from_file(char *isodate, FILE *file)
{
	char data_set_file_line[1000];

	rewind_file_until_begin_line(file);
	if (fgets (data_set_file_line, 900, file) == NULL) // le a linha corrente
	{
		Erro ("Could read file line in read_isodate_from_file()", "", "");
		exit(1);
	}
	if (sscanf (data_set_file_line, "%s ", (char *)isodate) != 1)
	{
		Erro ("Could not read isodate from file in read_isodate_from_file().", "", "");
		exit(1);
	}
	return (strlen(data_set_file_line));
}



long
binary_search_isodate_in_file(FILE *file, char *isodate, long low, long high, long size) 
{
	long mid;
	char isodate_from_file[1000];
	
	if (high < low)
		return (-1); // not found
	mid = low + (high - low) / 2;
	if (mid >= (size - 1))
		return (-1); // not found
		
	fseek(file, mid, SEEK_SET);
	read_isodate_from_file(isodate_from_file, file);
	
	if (strcmp(isodate_from_file, isodate) > 0)
		return (binary_search_isodate_in_file(file, isodate, low, mid-1, size));
	else if (strcmp(isodate_from_file, isodate) < 0)
		return (binary_search_isodate_in_file(file, isodate, mid+1, high, size));
	else
	{
		fseek(file, mid, SEEK_SET);
		rewind_file_until_begin_line(file);
		mid = ftell(file);
		return (mid); // found
	}
}



int
find_date_in_file(FILE *file, long date_l, int file_type)
{
	long size;
	long file_index;
	char *date;
	char date_from_file[1000];
	int line_size;
	
	fseek(file, 0L, SEEK_END); // seek to end of file
	size = ftell(file); // get current pointer to the end of file
	fseek(file, 0L, SEEK_SET);
	date = long2isodate(date_l);

	file_index = binary_search_isodate_in_file(file, date, 0, size, size);
	if (file_index == -1)
		return (0);
	else
	{
		if (file_type == ENFOQUE)
		{
			fseek(file, file_index, SEEK_SET);
			return (1);
		}
		else
		{	// no arquivo peixe piloto tem mais de uma linha com a mesma isodate
			line_size = read_isodate_from_file(date_from_file, file);
			file_index -= MINUTES_IN_A_DAY * line_size * 2; // volta o indice um numero de linhas tal que garanta que vai ficar um dia antes da data desejada
			if (file_index < 0)
				file_index = 0;
			
			fseek(file, file_index, SEEK_SET); // volta o tanto necessario para garantir pegar a linha certa
			do
			{
				line_size = read_isodate_from_file(date_from_file, file);
			} while (strcmp(date, date_from_file) != 0);
			fseek(file, -line_size, SEEK_CUR); // volta para o ponto em que comeca o dado de interesse
		}
	}
	return (1);
}



void
test_find_date_in_file()
{
	FILE *file;
	char *date[4] = {"2011-07-30", "2011-08-01", "2011-08-02", "2011-08-03"};
	int i;
	
	file = fopen("caco.txt", "r");
	for (i = 0; i < 4; i++)
		find_date_in_file(file, isodate2long(date[i]), ENFOQUE);
	fclose(file);
}



void
load_stock_intraday_data(int stock_num, long date_l, int moving_average_current_minute)
{
	FILE *peixe_piloto_file;
	long daily_sample;
	int current_date_intraday_sample;
	char data_set_file_line[1000], date[100], week_day[100], previous_date[100], aux[2000];
	int hour, minute;
	double price, num_trades, volume, previous_price, day_price_corrention_factor;
	
	daily_sample = -1;
	current_date_intraday_sample = 0;
	previous_date[0] = '\0';
	
	peixe_piloto_file = get_peixe_piloto_file(g_daily_data[stock_num]->stock_name, "_Volume.TXT");
	if (!find_date_in_file(peixe_piloto_file, date_l, PEIXE_PILOTO))
	{
		Erro ("Could not find date in file <datefile> ", long2isodate(date_l), g_daily_data[stock_num]->stock_name);
		exit(1);
	}

	while (fgets(data_set_file_line, 900, peixe_piloto_file) != NULL)
	{
		if (sscanf (data_set_file_line, "%s %s %d:%d;%lf;%lf;%lf", (char *)date, (char *)week_day, &hour, &minute, &price, &num_trades, &volume) != 7)
		{
			strcpy(aux, g_daily_data[stock_num]->stock_name);
			strcat(aux, ", line = ");
			strcat(aux, data_set_file_line);
			Erro ("Could not read line of Peixe Piloto file ", aux, " in load_stock_intraday_data().");
			exit(1);
		}
		check_data_set_file_line(date, week_day, hour, minute, price, num_trades, volume);
		
		if (daily_sample == -1)
		{	// Inicio do arquivo Peixe Piloto
			daily_sample = 0;			
			for (current_date_intraday_sample = 0; current_date_intraday_sample < minute; current_date_intraday_sample++)
			{	// Completa as cotacoes no inicio do dia
				g_daily_data[stock_num]->stock_price[current_date_intraday_sample] = price;
			}
			g_daily_data[stock_num]->stock_price[current_date_intraday_sample] = price;
			current_date_intraday_sample++;
			strcpy(previous_date, date);
		}
		else if (strcmp(date, previous_date) != 0)
		{	// Fim de um dia no arquivo Peixe Piloto
			break;
		}
		else
		{	// Dentro de um dia do arquivo Peixe Piloto. Assume que nunca pula mais de um minuto.
			if (current_date_intraday_sample >= MINUTES_IN_A_DAY)
			{
				Erro("current_date_intraday_sample >= MINUTES_IN_A_DAY in load_stock_intraday_data(); stock&date: ", g_daily_data[stock_num]->stock_name, date);
				exit(1);
			}
			g_daily_data[stock_num]->stock_price[current_date_intraday_sample] = price;
			current_date_intraday_sample++;
		}
	}

	for ( ; current_date_intraday_sample < MINUTES_IN_A_DAY; current_date_intraday_sample++)
	{	// Completa as cotacoes no fim do dia
		previous_price = g_daily_data[stock_num]->stock_price[current_date_intraday_sample-1];
		g_daily_data[stock_num]->stock_price[current_date_intraday_sample] = previous_price;
	}

	day_price_corrention_factor = g_daily_data[stock_num]->stock_close_price_for_intraday_correction[moving_average_current_minute / MINUTES_IN_A_DAY] / g_daily_data[stock_num]->stock_price[MINUTES_IN_A_DAY-1];
/*	printf("data = %s, minu. = %d, acao = %s, clo = %f\n", 
		long2isodate(g_daily_data[stock_num]->stock_close_price_date_l[NUM_SAMPLES_FOR_PREDICTION + moving_average_current_minute / MINUTES_IN_A_DAY]), 
		moving_average_current_minute,
		g_daily_data[stock_num]->stock_name, 
		g_daily_data[stock_num]->stock_close_price[NUM_SAMPLES_FOR_PREDICTION + moving_average_current_minute / MINUTES_IN_A_DAY]);
*/
	for (current_date_intraday_sample = 0; current_date_intraday_sample < MINUTES_IN_A_DAY; current_date_intraday_sample++)
	{	// Corrige discrepancia entre dados diarios e intraday
		g_daily_data[stock_num]->stock_price[current_date_intraday_sample] *= day_price_corrention_factor;
	}
	
	fclose (peixe_piloto_file);
}



long
find_previous_close_price(FILE *clo, long date_l)
{
	long previous_date_l;
	
	previous_date_l = date_l;
	do
	{
		previous_date_l = timeMinusDays(previous_date_l, 1);
	} while (!find_date_in_file(clo, previous_date_l, ENFOQUE));

	return (previous_date_l);
}


void
load_stock_day_data(int stock_num, long trading_system_current_day_date_l)
{
	FILE *clo;
	char data_set_file_line[1000], date[100], base_file_name[1000], *aux;
	int i, j;
	long date_l_and_offset, sample_date_l;
	double average, day_price;
	
	strcpy(base_file_name, g_daily_data[stock_num]->stock_name);
	strcat(base_file_name, "/");
	aux = &(base_file_name[strlen(base_file_name)]);
	for(i = 0;  g_daily_data[stock_num]->stock_name[i]; i++)
		aux[i] = tolower(g_daily_data[stock_num]->stock_name[i]);
	aux[i] = '\0';
	clo = get_file(base_file_name, DATA_PATH, "_adj.clo");

	for (i = 0; i < NUM_SAMPLES_FOR_PREDICTION; i++)
	{
		date_l_and_offset = timeMinusDays(trading_system_current_day_date_l, 1 + NUM_DAYS_BEFORE_BEFORE_PREDICTION + (NUM_SAMPLES_FOR_PREDICTION - 1 - i) * DAYS_BETWEEN_SAMPLES);
		average = 0.0;
		for (j = 0; j < DAYS_BETWEEN_SAMPLES; j++)
		{
			sample_date_l = timeMinusDays(date_l_and_offset, (DAYS_BETWEEN_SAMPLES - 1) - j);
			if (!find_date_in_file(clo, (sample_date_l), ENFOQUE)) // le o preco de fechamento de cada quarta necessaria
				sample_date_l = find_previous_close_price(clo, (sample_date_l));

			if (fgets(data_set_file_line, 900, clo) == NULL)
			{
				Erro("Could not read sample from ", base_file_name, " in load_stock_day_data()");
				exit(1);
			}
			if (sscanf (data_set_file_line, "%s %lf", (char *)date, &day_price) != 2)
			{
				Erro("Could not read (sscanf) sample from ", base_file_name, " in load_stock_day_data()");
				exit(1);
			}
			average += day_price;
		}
		average /= (double) DAYS_BETWEEN_SAMPLES;
		g_daily_data[stock_num]->stock_close_price_for_prediction[i] = average;
		g_daily_data[stock_num]->stock_close_price_for_prediction_date_l[i] = sample_date_l;
	}
	
	for (i = 0; i < NUM_SAMPLES_FOR_INTRADAY_CORRECTION; i++)
	{
#ifndef	IPC_ON_LINE
		date_l_and_offset = timePlusDays(trading_system_current_day_date_l, i);
		if (!find_date_in_file(clo, (date_l_and_offset), ENFOQUE)) // le o preco de fechamento de cada dia necessario
			date_l_and_offset = find_previous_close_price(clo, (date_l_and_offset));

		if (fgets(data_set_file_line, 900, clo) == NULL)
		{
			Erro("Could not read sample from ", base_file_name, " in load_stock_day_data()");
			exit(1);
		}
		if (sscanf (data_set_file_line, "%s %lf", (char *)date, &(g_daily_data[stock_num]->stock_close_price_for_intraday_correction[i])) != 2)
		{
			Erro("Could not read (sscanf) sample from ", base_file_name, " in load_stock_day_data()");
			exit(1);
		}
		g_daily_data[stock_num]->stock_close_price_for_intraday_correction_date_l[i] = date_l_and_offset;
#else
		g_daily_data[stock_num]->stock_close_price_for_intraday_correction[i] = 1.0;
		g_daily_data[stock_num]->stock_close_price_for_intraday_correction_date_l[i] = trading_system_current_day_date_l;
#endif
	}
	
	fclose (clo);
}


void
load_all_stocks_day_data(long trading_system_current_day_date_l)
{
	int stock_num;
	
	for (stock_num = 0; stock_num < g_num_stocks_of_interest; stock_num++)
	{
		load_stock_day_data(stock_num, trading_system_current_day_date_l);
	}
}


int
check_if_date_l_is_available_for_stock(int stock_num, long date_l)
{
	FILE *peixe_piloto_file;
	int found;
	
	peixe_piloto_file = get_peixe_piloto_file(g_daily_data[stock_num]->stock_name, "_Volume.TXT");
	found = find_date_in_file(peixe_piloto_file, date_l, PEIXE_PILOTO);
	fclose(peixe_piloto_file);
	if (found)
		return (1);
	else
		return (0);
}



int
check_if_date_l_is_available_for_all_stocks(long date_l)
{
	int stock_num;
	int found_available_day = 1;
	
	for (stock_num = 0; stock_num < g_num_stocks_of_interest; stock_num++)
	{
		found_available_day = found_available_day && check_if_date_l_is_available_for_stock(stock_num, date_l);
	}
	return (found_available_day);
}



int
enough_intraday_data(long date_l)
{
#ifdef	IPC_ON_LINE
	return (1);
#else
	int i, num_days_for_first_trades = 0, num_days_for_second_trades = 0;
	
	if (long2dow(date_l) != FISRT_DAY_OF_TRADING_WEEK)
	{
		Erro("Day is not FISRT_DAY_OF_TRADING_WEEK in enough_intraday_data().", "", "");
		exit(1);
	}
	
	for (i = 0; i < 2; i++)
	{
		if (check_if_date_l_is_available_for_all_stocks(date_l + i * SECONDS_IN_A_DAY))
			num_days_for_first_trades++;
	}
	for (i = 4; i < 7; i++)
	{
		if (check_if_date_l_is_available_for_all_stocks(date_l + i * SECONDS_IN_A_DAY))
			num_days_for_second_trades++;
	}

	if ((num_days_for_first_trades >= 1) && (num_days_for_second_trades >= 1))
		return (1);
	else
		return (0);
#endif
}



long
get_week_with_enough_intraday_data(long trading_system_current_day_date_l)
{
	long date_l;
	long last_date_l;
	
	date_l = trading_system_current_day_date_l;
	last_date_l = isodate2long(g_end_date);

	for ( ; long2dow(date_l) != FISRT_DAY_OF_TRADING_WEEK; date_l = timePlusDays(date_l, 1))
		; // avanca ate o proximo FISRT_DAY_OF_TRADING_WEEK

	while (!enough_intraday_data(date_l))
	{
		date_l = timePlusDays(date_l, 5);
		if (date_l > last_date_l)
			return (-1);
	}
	return (date_l);
}



long
get_next_available_day(long trading_system_current_day_date_l)
{
	long date_l;
	
	// Avanca ate achar um dia com dados intraday. 
	// Retorna -1 se passar da data final.
	
	date_l = trading_system_current_day_date_l;
	date_l = timePlusDays(date_l, 1);
	date_l = get_week_with_enough_intraday_data(date_l);

	return (date_l);
}



long
get_first_day()
{
	long date_l;
	
	date_l = isodate2long(g_begin_date);
	date_l = get_week_with_enough_intraday_data(date_l);
	
	return (date_l);
}



void
read_stock_list(char *stock_list_file_name)
{
	FILE *stock_list_file;
	char file_line[1000];
	char *aux;
	int i;
	
	stock_list_file = fopen(stock_list_file_name, "r");
	if (stock_list_file == NULL)
	{
		Erro("Could not open stock_list_file_name <", stock_list_file_name, "> in read_stock_list()");
		exit(1);
	}
	
	g_num_stocks_of_interest = 0;
	g_daily_data = NULL;
	while (fgets (file_line, 900, stock_list_file) != NULL)
	{
		g_daily_data = (DAILY_DATA **) realloc(g_daily_data, (g_num_stocks_of_interest + 1) * sizeof(DAILY_DATA *));
		g_daily_data[g_num_stocks_of_interest] = (DAILY_DATA *) malloc(sizeof(DAILY_DATA));
		strcpy(g_daily_data[g_num_stocks_of_interest]->stock_name, file_line);
		aux = strrchr(g_daily_data[g_num_stocks_of_interest]->stock_name, '\n');
		aux[0] = '\0';
		//printf("%s ", g_daily_data[g_num_stocks_of_interest]->stock_name);
		for (i = 0; i < MINUTES_IN_A_DAY; i++)
			g_daily_data[g_num_stocks_of_interest]->stock_price[i] = -1.0;

		g_num_stocks_of_interest++;
	}

	NUM_SAMPLES_FOR_PREDICTION = (SAMPLE_SIZE + 1);
	for (i = 0; i < g_num_stocks_of_interest; i++)
	{
		g_daily_data[i]->stock_close_price_for_prediction = (double *) malloc(NUM_SAMPLES_FOR_PREDICTION * sizeof(double));
		g_daily_data[i]->stock_close_price_for_prediction_date_l = (double *) malloc(NUM_SAMPLES_FOR_PREDICTION * sizeof(double));
	}
	//printf("\n");
	
	fclose(stock_list_file);
}


int
get_reference_stock_index(char *stock_name)
{
	int stock_index;
	
	for (stock_index = 0; stock_index < g_num_stocks_of_interest; stock_index++)
	{
		if (strcmp(stock_name, g_daily_data[stock_index]->stock_name) == 0)
			break;
	}
	if (stock_index == g_num_stocks_of_interest)
	{
		Erro("Could not find reference stock named ", stock_name, " in the list os stocks of interest");
		exit(1);
	}
	return (stock_index);
}



double *
get_stock_or_ref_current_day_intraday_prices(STATE *state, int moving_average_current_minute, int stock)
{
	int i;
	long date_l;
	double last_price_of_the_day;
	
	if ((moving_average_current_minute % MINUTES_IN_A_DAY) == 0)
	{
		if (moving_average_current_minute != 0)
			date_l = timePlusDays(g_trading_system_current_day_date_l, 1);
		else
			date_l = g_trading_system_current_day_date_l;
			
		if (check_if_date_l_is_available_for_stock(stock, date_l))
		{
			load_stock_intraday_data(stock, date_l, moving_average_current_minute);
			return (g_daily_data[stock]->stock_price);
		}
		else
		{
			while (!check_if_date_l_is_available_for_stock(stock, date_l))
				date_l = timeMinusDays(date_l, 1);
			load_stock_intraday_data(stock, date_l, moving_average_current_minute);
			last_price_of_the_day = g_daily_data[stock]->stock_price[MINUTES_IN_A_DAY-1];
			for (i = 0; i < MINUTES_IN_A_DAY; i++)
			{
				g_daily_data[stock]->stock_price[i] = last_price_of_the_day;
			}
			return (g_daily_data[stock]->stock_price);
		}
	}
	else
		return (g_daily_data[stock]->stock_price);
}



void
get_stock_or_ref_current_day_intraday_prices_on_hot_start(int stock, long date_l, int moving_average_current_minute)
{
#ifndef	IPC_ON_LINE
	int i;
	double last_price_of_the_day;
	
	if ((moving_average_current_minute % MINUTES_IN_A_DAY) != 0)
	{	// if (moving_average_current_minute % MINUTES_IN_A_DAY) == 0 get_stocks_quote() will take care of the things below
		if (check_if_date_l_is_available_for_stock(stock, date_l))
		{
			load_stock_intraday_data(stock, date_l, moving_average_current_minute);
		}
		else
		{
			while (!check_if_date_l_is_available_for_stock(stock, date_l))
				date_l = timeMinusDays(date_l, 1);
			load_stock_intraday_data(stock, date_l, moving_average_current_minute);
			last_price_of_the_day = g_daily_data[stock]->stock_price[MINUTES_IN_A_DAY-1];
			for (i = 0; i < MINUTES_IN_A_DAY; i++)
			{
				g_daily_data[stock]->stock_price[i] = last_price_of_the_day;
			}
		}
	}
#endif
}




double *
get_stock_current_day_intraday_prices(STATE *state, int moving_average_current_minute)
{
	return (get_stock_or_ref_current_day_intraday_prices(state, moving_average_current_minute, state->best_stock_index));
}



double *
get_ref_current_day_intraday_prices(STATE *state, int moving_average_current_minute)
{
	return (get_stock_or_ref_current_day_intraday_prices(state, moving_average_current_minute, state->ref_stock_index));
}



void
read_trading_system_parameters()
{
	FILE *parameters_file;
	char file_line[1000];
	char parameter[1000];
	
	parameters_file = fopen("parameters.txt", "r");
	if (parameters_file == NULL)
	{
		Erro("Could not open parameters.txt in read_trading_system_parameters()", "", "");
		exit(1);
	}

	while (fgets (file_line, 900, parameters_file) != NULL)
	{
		if (sscanf(file_line, "%s", parameter) != 1)
			continue;
		if ((strlen(parameter) == 0) || (parameter[0] == '#'))
			continue;
			
		if (strcmp(parameter, "LEVERAGE") == 0)
			sscanf(file_line, "%s %lf", parameter, &LEVERAGE);			
		if (strcmp(parameter, "BROKERAGE") == 0)
			sscanf(file_line, "%s %lf", parameter, &BROKERAGE);			
		if (strcmp(parameter, "MOVING_AVERAGE_NUM_PERIODS") == 0)
			sscanf(file_line, "%s %d", parameter, &MOVING_AVERAGE_NUM_PERIODS);			
		if (strcmp(parameter, "SLOW_MOVING_AVERAGE_NUM_PERIODS") == 0)
			sscanf(file_line, "%s %d", parameter, &SLOW_MOVING_AVERAGE_NUM_PERIODS);			
		if (strcmp(parameter, "TRADING_SYSTEM_STOP_GAIN") == 0)
			sscanf(file_line, "%s %lf", parameter, &TRADING_SYSTEM_STOP_GAIN);			
		if (strcmp(parameter, "TRADING_SYSTEM_STOP_LOSS") == 0)
			sscanf(file_line, "%s %lf", parameter, &TRADING_SYSTEM_STOP_LOSS);			
		if (strcmp(parameter, "TRADING_SYSTEM_BUY_SELL_DELTA_RETURN") == 0)
			sscanf(file_line, "%s %lf", parameter, &TRADING_SYSTEM_BUY_SELL_DELTA_RETURN);			
		if (strcmp(parameter, "FISRT_DAY_OF_TRADING_WEEK") == 0)
			sscanf(file_line, "%s %d", parameter, &FISRT_DAY_OF_TRADING_WEEK);			
		if (strcmp(parameter, "TRADING_OPENING_HOUR") == 0)
			sscanf(file_line, "%s %d", parameter, &TRADING_OPENING_HOUR);			
		if (strcmp(parameter, "SAMPLE_SIZE") == 0)
			sscanf(file_line, "%s %d", parameter, &SAMPLE_SIZE);			
		if (strcmp(parameter, "DAYS_BETWEEN_SAMPLES") == 0)
			sscanf(file_line, "%s %d", parameter, &DAYS_BETWEEN_SAMPLES);			
		if (strcmp(parameter, "NUM_DAYS_BEFORE_BEFORE_PREDICTION") == 0)
			sscanf(file_line, "%s %d", parameter, &NUM_DAYS_BEFORE_BEFORE_PREDICTION);			
	}
}
