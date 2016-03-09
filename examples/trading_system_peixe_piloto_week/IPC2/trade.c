#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <sys/types.h>
#include <dirent.h>
#include "mae.h"
#include "../wnn_pred.h"
#include "timeutil.h"
#ifdef	USE_IPC
#include "ipc.h"
#include "messages.h"
#endif
#include "trading_system.h"



#if EXPONENTIAL_MOVING_AVERAGE == 1

float
moving_average(float *intraday_price, int current_minute)
{
	int i;
	int num_periods = MOVING_AVERAGE_NUM_PERIODS;
	float em_average;
	float alpha;
	
	if ((current_minute - MOVING_AVERAGE_NUM_PERIODS + 1) < 0)
		num_periods = current_minute + 1;
	else
		num_periods = MOVING_AVERAGE_NUM_PERIODS;

	alpha = 2.0 / (num_periods + 1.0);
	em_average = intraday_price[current_minute - num_periods + 1];
	for (i = current_minute - num_periods + 2; i <= current_minute; i++)
		em_average = em_average + alpha * (intraday_price[i] - em_average);

	return (em_average);
}


#else

float
moving_average_old(float *intraday_price, int current_minute)
{
	int i;
	int num_periods = MOVING_AVERAGE_NUM_PERIODS;
	float m_average = 0.0;
	
	if ((current_minute - MOVING_AVERAGE_NUM_PERIODS + 1) < 0)
		num_periods = current_minute + 1;
	else
		num_periods = MOVING_AVERAGE_NUM_PERIODS;

	for (i = current_minute - num_periods + 1; i <= current_minute; i++)
		m_average += intraday_price[i];

	return (m_average / num_periods);
}



void
get_moving_average_price_sample(float *intraday_price, int day, int minute)
{
	int current_minute;
	int minutes_in_a_day = MINUTES_IN_A_DAY;
	int intraday_minute;
	int i;
	float last_price;
	
	current_minute = day * minutes_in_a_day + minute;
	while ((current_minute - g_moving_average_current_minute) >= minutes_in_a_day)
	{	// Tem um dia ou mais de dados faltando -> repete a última cotacao
		if (g_moving_average_current_minute == -1) // Nao tem ultima cotacao (feriado quinta) -> pega a corrente
			last_price = intraday_price[minute];
		else
			last_price = g_price_samples[g_moving_average_current_minute];
		for (i = 0; i < minutes_in_a_day; i++)
		{
			g_moving_average_current_minute++;
			g_price_samples[g_moving_average_current_minute] = last_price;
		}
	}
	while (g_moving_average_current_minute < current_minute)
	{
		g_moving_average_current_minute++;
		intraday_minute = minute - (minute - g_moving_average_current_minute % minutes_in_a_day);
		g_price_samples[g_moving_average_current_minute] = intraday_price[intraday_minute];
	}
}



float
moving_average(float *price_samples, int minute)
{
	int i;
	double m_average = 0.0, num_samples = 0.0;
	
	if (minute >= MINUTES_IN_A_WEEK)
	{
		Erro("minute >= MINUTES_IN_A_WEEK in moving_average()", "", "");
		exit(1);
	}
		
	for (i = minute - MOVING_AVERAGE_NUM_PERIODS + 1; i <= minute; i++)
	{
		if (i > 0)
		{
			m_average += price_samples[i];
			num_samples += 1.0;
		}
		else
		{
			m_average += price_samples[0];
			num_samples += 1.0;
		}		
	}

	return (m_average / num_samples);
}

#endif


void
generate_graph(char *graph_dir, char *stock_name, int moving_average_first_day, int day, float *intraday_price, int trade_minute, float price, int buy)
{
	int i;
	char *dia;
	char graph_file_name[1000];
	char quotes_file_name[1000];
	char trade_file_name[1000];
	char moving_average_file_name[1000];
	FILE *graph_file, *quotes, *trade, *moving_average_file;
	
	sprintf(graph_file_name, "%s/graficos.gnuplot", graph_dir);
	graph_file = fopen(graph_file_name, "a");
	
	dia = long2isodate(g_train_data_set[day].date_l);	
	if (buy)
	{
		sprintf(quotes_file_name, "%s/%s-%s-%s.quotes", graph_dir, dia, stock_name, "buy");
		sprintf(trade_file_name, "%s/%s-%s-%s.trade", graph_dir, dia, stock_name, "buy");		
		sprintf(moving_average_file_name, "%s/%s-%s-%s.moving_average", graph_dir, dia, stock_name, "buy");		
		fprintf(graph_file, "plot '%s' using 1:2 title 'Precos %s' with lines, '%s' using 1:2 title 'Moving Average' with lines, '%s' using 1:2 title 'Compra %s %s' with impulses, %f title 'Referencia'\n", 
			quotes_file_name, stock_name, moving_average_file_name, trade_file_name, dia, long2dows(g_train_data_set[day].date_l), price);
	}
	else
	{
		sprintf(quotes_file_name, "%s/%s-%s-%s.quotes", graph_dir, dia, stock_name, "sell");
		sprintf(trade_file_name, "%s/%s-%s-%s.trade", graph_dir, dia, stock_name, "sell");		
		sprintf(moving_average_file_name, "%s/%s-%s-%s.moving_average", graph_dir, dia, stock_name, "sell");		
		fprintf(graph_file, "plot '%s' using 1:2 title 'Precos %s' with lines, '%s' using 1:2 title 'Moving Average' with lines, '%s' using 1:2 title 'Venda %s %s' with impulses, %f title 'Referencia'\n", 
			quotes_file_name, stock_name, moving_average_file_name, trade_file_name, dia, long2dows(g_train_data_set[day].date_l), price);
	}
	fprintf(graph_file, "pause -1  'press return'\n\n");
	
	quotes = fopen(quotes_file_name, "w");
	trade = fopen(trade_file_name, "w");
	moving_average_file = fopen(moving_average_file_name, "w");
	
	for (i = 0; i < MINUTES_IN_A_DAY; i++)
	{	// Dias sem contacoes (feriados?) tem o mesmo preco em todos os minutos
		(intraday_price != NULL)? fprintf(quotes, "%d, %f\n", i, intraday_price[i]): fprintf(quotes, "%d, %f\n", i, price);
		(intraday_price != NULL)? fprintf(moving_average_file, "%d, %f\n", i, moving_average(g_price_samples, (day - moving_average_first_day) * MINUTES_IN_A_DAY + i)): fprintf(moving_average_file, "%d, %f\n", i, price);
		if (i == trade_minute)
			(intraday_price != NULL)? fprintf(trade, "%d, %f\n", i, intraday_price[i]): fprintf(trade, "%d, %f\n", i, price);
	}
	
	fclose(graph_file);
	fclose(quotes);
	fclose(trade);
	fclose(moving_average_file);
}


float
search_for_good_opportunity_to_sell(int day, int *sell_minute, int first_sell_minute, float minimum_sell_price, float buy_price)
{
	int minute;
	float previous_maximum_intraday_price;
	float *intraday_price = NULL;
	float moving_average_price;
	
	intraday_price = g_train_data_set[day].intraday_price;
	
	get_moving_average_price_sample(intraday_price, day - g_moving_average_first_day, first_sell_minute);
	previous_maximum_intraday_price = moving_average(g_price_samples, (day - g_moving_average_first_day) * MINUTES_IN_A_DAY + first_sell_minute);
	for (minute = first_sell_minute + 1; minute < MINUTES_IN_A_DAY; minute++)
	{
		get_moving_average_price_sample(intraday_price, day - g_moving_average_first_day, minute);
		moving_average_price = moving_average(g_price_samples, (day - g_moving_average_first_day) * MINUTES_IN_A_DAY + minute);
		if ((intraday_price[minute] > minimum_sell_price) && (moving_average_price > minimum_sell_price))
		{
			if (moving_average_price < previous_maximum_intraday_price * (1.0 - TRADING_SYSTEM_BUY_SELL_DELTA_RETURN))
			{	// Achou a primeira descida depois de um maximo local
				*sell_minute = minute;
				get_moving_average_price_sample(intraday_price, day - g_moving_average_first_day, MINUTES_IN_A_DAY - 1);
				return (intraday_price[minute]);
			}
		}
		if (intraday_price[minute] < (buy_price * (1.0 + TRADING_SYSTEM_STOP_LOSS)))
		{	// Stop Loss
			*sell_minute = minute;
			get_moving_average_price_sample(intraday_price, day - g_moving_average_first_day, MINUTES_IN_A_DAY - 1);
			return (intraday_price[minute]);
		}
		if (moving_average_price > previous_maximum_intraday_price)
			previous_maximum_intraday_price = moving_average_price;
		if (moving_average_price < minimum_sell_price)
			previous_maximum_intraday_price = moving_average_price;
	}
	get_moving_average_price_sample(intraday_price, day - g_moving_average_first_day, MINUTES_IN_A_DAY - 1);
	
	return (-1.0); // Nao consegui vender
}



float
get_sell_price(int first_sell_day, int last_sell_day, int *sell_day, int *sell_minute, int first_sell_minute, float minimum_sell_price, float buy_price)
{
	int day;
	float sell_price = -1.0;

	for (day = first_sell_day; day <= last_sell_day; day++)
	{
#ifdef	TEST_TRADING_SYSTEM
		if (g_train_data_set[day].intraday_price == NULL) // @@@ Os price_sample deste dia vao ficar vazios...
			continue; // Feriado (ou outra razao para nao ter dados intraday)
#endif
		sell_price = search_for_good_opportunity_to_sell(day, sell_minute, first_sell_minute, minimum_sell_price, buy_price);
		if (sell_price > 0.0)
		{
			*sell_day = day;
			return (sell_price);
		}

		first_sell_minute = 0;
	}
	*sell_day = -1;
	return (sell_price); // Se sair por aqui nao conseguiu vender.
}



float
search_for_good_opportunity_to_buy(int day, int *buy_minute, float maximum_buy_price, int first_day)
{
	int minute;
	float previous_minimum_intraday_price;
	float *intraday_price = NULL;
	float moving_average_price;
	int first_minute;
	
	intraday_price = g_train_data_set[day].intraday_price;
	
	if (first_day)
		first_minute = MOVING_AVERAGE_NUM_PERIODS - 1;
	else
		first_minute = 0;
		
	get_moving_average_price_sample(intraday_price, day - g_moving_average_first_day, first_minute);
	previous_minimum_intraday_price = moving_average(g_price_samples, (day - g_moving_average_first_day) * MINUTES_IN_A_DAY + first_minute);
	for (minute = first_minute; minute < MINUTES_IN_A_DAY; minute++)
	{
		get_moving_average_price_sample(intraday_price, day - g_moving_average_first_day, minute);
		moving_average_price = moving_average(g_price_samples, (day - g_moving_average_first_day) * MINUTES_IN_A_DAY + minute);
		if ((intraday_price[minute] < maximum_buy_price) && (moving_average_price < maximum_buy_price))
		{	// @@@ Este if usa intraday_price[minute]
			if (moving_average_price > previous_minimum_intraday_price * (1.0 + TRADING_SYSTEM_BUY_SELL_DELTA_RETURN))
			{	// Achou a primeira subida depois de um minimo local
				*buy_minute = minute;
				return (intraday_price[minute]);
			}
		}
		if (moving_average_price < previous_minimum_intraday_price)
			previous_minimum_intraday_price = moving_average_price;
		if (moving_average_price > maximum_buy_price)
			previous_minimum_intraday_price = moving_average_price;
	}
	*buy_minute = -1;
	return (-1.0);
}



float
get_buy_price(int first_buy_day, int last_buy_day, int *buy_day, int *buy_minute, float maximum_buy_price)
{
	int day;
	float buy_price = -1.0;
	
	for (day = first_buy_day; day <= last_buy_day; day++)
	{
#ifdef	TEST_TRADING_SYSTEM
		if (g_train_data_set[day].intraday_price == NULL)
			continue; // Feriado (ou outra razao para nao ter dados intraday)
#endif
		buy_price = search_for_good_opportunity_to_buy(day, buy_minute, maximum_buy_price, day == first_buy_day);
		if (buy_price > 0.0)
		{
			*buy_day = day;
			return(buy_price);
		}
	}
	return (buy_price); // Se sair por aqui nao consguiu comprar.
}



int
enougth_intraday_data(int target_day)
{
#ifdef	IPC_ON_LINE
	return (1);
#else
	int first_buy_day, last_buy_day;
	int first_sell_day, last_sell_day;
	int i, num_buy_days = 0, num_sell_days = 0;

	first_buy_day = target_day - 4;
	last_buy_day =  target_day - 3;
	for (i = first_buy_day; i <= last_buy_day; i++)
		if (g_train_data_set[i].intraday_price != NULL)
			num_buy_days++;

	first_sell_day = target_day - 2;
	last_sell_day =  target_day;
	for (i = first_sell_day; i <= last_sell_day; i++)
		if (g_train_data_set[i].intraday_price != NULL)
			num_sell_days++;
	
	if ((num_buy_days >= 1) && (num_sell_days >= 1))
		return (1);
	else
		return (0);
#endif
}


float
get_return_trading_system (int target_day, char *stock_name)
{
	float buy_price, maximum_buy_price, sell_price, minimum_sell_price;
	int first_buy_day, last_buy_day, buy_day = -1,  buy_minute = -1, sell_minute = -1;
	int first_sell_day, last_sell_day, sell_day = -1;
	float week_return;
#if VERBOSE == 1
	char first_buy_date[100], buy_date[100], first_sell_date[100], sell_date[100];
#endif
		
	if (!enougth_intraday_data(target_day))
	{
		//printf ("@@@");
		return (0.0);
	}

	g_moving_average_current_minute = -1;
	g_moving_average_first_day = first_buy_day = target_day - 4;
	last_buy_day =  target_day - 3;
	maximum_buy_price = g_train_data_set[target_day - 5].clo * (1.0 + TRADING_SYSTEM_START_BUYING_RETURN);
	buy_price = get_buy_price(first_buy_day, last_buy_day, &buy_day, &buy_minute, maximum_buy_price);
#if VERBOSE == 1
	strcpy (first_buy_date, long2isodate (g_train_data_set[first_buy_day].date_l));
#endif
	if (buy_price < 0.0) // Nao conseguiu comprar
	{
#if VERBOSE == 1
		printf ("#FBD %s- nao comprou\n", first_buy_date);
#endif
		return (0.0);
	}		
		
	first_sell_day = buy_day;
#if VERBOSE == 1
	strcpy (buy_date, long2isodate (g_train_data_set[buy_day].date_l));
	strcpy (first_sell_date, long2isodate (g_train_data_set[first_sell_day].date_l));
#endif
	last_sell_day =  target_day;
	minimum_sell_price = buy_price * (1.0 + TRADING_SYSTEM_STOP_GAIN);	
	sell_price = get_sell_price(first_sell_day, last_sell_day, &sell_day, &sell_minute, buy_minute + 1, minimum_sell_price, buy_price);
				
	if (sell_price < 0.0) // Nao conseguiu vender
	{
		printf("buy price = R$%.2f, sell price = R$%.2f\n", buy_price, g_train_data_set[last_sell_day].clo);
		week_return = (g_train_data_set[last_sell_day].clo - buy_price) / buy_price;
#if VERBOSE == 1
		strcpy (sell_date, long2isodate (g_train_data_set[last_sell_day].date_l));
		printf ("#FBD %s - BD %s / FSD %s - SD %s\n", first_buy_date, buy_date, first_sell_date, sell_date);
#endif
	}
	else
	{
		printf("buy price = R$%.2f, sell price = R$%.2f\n", buy_price, sell_price);
		week_return = (sell_price - buy_price) / buy_price;
#if VERBOSE == 1
		strcpy (sell_date, long2isodate (g_train_data_set[sell_day].date_l));
		printf ("#FBD %s - BD %s / FSD %s - SD %s\n", first_buy_date, buy_date, first_sell_date, sell_date);
#endif
	}
	generate_graph("GRAFICOS", stock_name, g_moving_average_first_day, buy_day, g_train_data_set[buy_day].intraday_price, buy_minute, maximum_buy_price, BUY);
	if (sell_price < 0.0)
		generate_graph("GRAFICOS", stock_name, g_moving_average_first_day, last_sell_day, g_train_data_set[last_sell_day].intraday_price, MINUTES_IN_A_DAY - 1, minimum_sell_price, SELL);
	else
		generate_graph("GRAFICOS", stock_name, g_moving_average_first_day, sell_day, g_train_data_set[sell_day].intraday_price, sell_minute, minimum_sell_price, SELL);

	//printf("buy_price = %f, sell_price = %f, week_return = %f\n", buy_price, sell_price, week_return);

	return (week_return);
}

