#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <sys/types.h>
#include <dirent.h>
#include "timeutil.h"
#ifdef	USE_IPC
#include "ipc.h"
#include "messages.h"
#endif
#include "trading_system.h"



#if EXPONENTIAL_MOVING_AVERAGE == 1

double
moving_average(double *intraday_price, int current_minute)
{
	int i;
	int num_periods = MOVING_AVERAGE_NUM_PERIODS;
	double em_average;
	double alpha;
	
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

double
moving_average_old(double *intraday_price, int current_minute)
{
	int i;
	int num_periods = MOVING_AVERAGE_NUM_PERIODS;
	double m_average = 0.0;
	
	if ((current_minute - MOVING_AVERAGE_NUM_PERIODS + 1) < 0)
		num_periods = current_minute + 1;
	else
		num_periods = MOVING_AVERAGE_NUM_PERIODS;

	for (i = current_minute - num_periods + 1; i <= current_minute; i++)
		m_average += intraday_price[i];

	return (m_average / num_periods);
}



void
get_moving_average_price_sample(double *intraday_price, int day, int minute)
{
	int current_minute;
	int minutes_in_a_day = MINUTES_IN_A_DAY;
	int intraday_minute;
	int i;
	double last_price;
	
	current_minute = day * minutes_in_a_day + minute;
	while ((current_minute - g_moving_average_current_minute) >= minutes_in_a_day)
	{	// Tem um dia ou mais de dados faltando -> repete a última cotacao
		if (g_moving_average_current_minute == -1) // Nao tem ultima cotacao (feriado quinta) -> pega a corrente
			last_price = intraday_price[minute];
		else
			last_price = g_best_stock_price[g_moving_average_current_minute];
		for (i = 0; i < minutes_in_a_day; i++)
		{
			g_moving_average_current_minute++;
			g_best_stock_price[g_moving_average_current_minute] = last_price;
		}
	}
	while (g_moving_average_current_minute < current_minute)
	{
		g_moving_average_current_minute++;
		intraday_minute = minute - (minute - g_moving_average_current_minute % minutes_in_a_day);
		g_best_stock_price[g_moving_average_current_minute] = intraday_price[intraday_minute];
	}
}



double
moving_average(double *price_samples, int minute)
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



double
delta_moving_average(STATE *state, double *stock_price, double *ref_price, int minute)
{
	int i;
	double m_average = 0.0, num_samples = 0.0, delta;
	
	if (minute >= MINUTES_IN_A_WEEK)
	{
		Erro("minute >= MINUTES_IN_A_WEEK in delta_moving_average()", "", "");
		exit(1);
	}

	for (i = minute - MOVING_AVERAGE_NUM_PERIODS + 1; i <= minute; i++)
	{
		switch (state->trade_order)
		{
			case BUY_SELL:
				if (i > 0)
				{
					delta = (stock_price[i] / state->best_stock_base_price) - (ref_price[i] / state->ref_stock_base_price);
					m_average += delta;
					num_samples += 1.0;
				}
				else
				{
					delta = (stock_price[0] / state->best_stock_base_price) - (ref_price[0] / state->ref_stock_base_price);
					m_average += delta;
					num_samples += 1.0;
				}
				break;
			case SELL_BUY:
				if (i > 0)
				{
					delta = (ref_price[i] / state->ref_stock_base_price) - (stock_price[i] / state->best_stock_base_price);
					m_average += delta;
					num_samples += 1.0;
				}
				else
				{
					delta = (ref_price[0] / state->ref_stock_base_price) - (stock_price[0] / state->best_stock_base_price);
					m_average += delta;
					num_samples += 1.0;
				}
				break;
		}
	}

	return (m_average / num_samples);
}



double
slow_delta_moving_average(STATE *state, double *stock_price, double *ref_price, int minute)
{
	int i;
	double m_average = 0.0, num_samples = 0.0, delta;
	
	if (minute >= MINUTES_IN_A_WEEK)
	{
		Erro("minute >= MINUTES_IN_A_WEEK in delta_moving_average()", "", "");
		exit(1);
	}

	for (i = minute - SLOW_MOVING_AVERAGE_NUM_PERIODS + 1; i <= minute; i++)
	{
		switch (state->trade_order)
		{
			case BUY_SELL:
				if (i > 0)
				{
					delta = (stock_price[i] / state->best_stock_base_price) - (ref_price[i] / state->ref_stock_base_price);
					m_average += delta;
					num_samples += 1.0;
				}
				else
				{
					delta = (stock_price[0] / state->best_stock_base_price) - (ref_price[0] / state->ref_stock_base_price);
					m_average += delta;
					num_samples += 1.0;
				}
				break;
			case SELL_BUY:
				if (i > 0)
				{
					delta = (ref_price[i] / state->ref_stock_base_price) - (stock_price[i] / state->best_stock_base_price);
					m_average += delta;
					num_samples += 1.0;
				}
				else
				{
					delta = (ref_price[0] / state->ref_stock_base_price) - (stock_price[0] / state->best_stock_base_price);
					m_average += delta;
					num_samples += 1.0;
				}
				break;
		}
	}

	return (m_average / num_samples);
}

#endif


void
generate_graph(char *graph_dir, STATE *state, double *stock_price, double *ref_price)
{
	int i;
	char *base_day;
	char first_trades_day[1000];
	char first_trades_dow[1000];
	char second_trades_day[1000];
	char second_trades_dow[1000];
	char graph_file_name[1000];
	char quotes_file_name[1000];
	char reference_file_name[1000];
	char first_trades_file_name[1000];
	char second_trades_file_name[1000];
	char moving_average_file_name[1000];
	char slow_moving_average_file_name[1000];
	FILE *graph_file, *quotes, *reference, *first_trades, *second_trades, *moving_average_file, *slow_moving_average_file;
	
	sprintf(graph_file_name, "%s/graficos.gnuplot", graph_dir);
	graph_file = fopen(graph_file_name, "a");
	
	base_day = long2isodate(state->first_orders_day_date_l);
	sprintf(quotes_file_name, "%s/%s-%s.quotes", graph_dir, base_day, state->best_stock_name);
	sprintf(reference_file_name, "%s/%s-%s.quotes", graph_dir, base_day, state->ref_stock_name);
	sprintf(first_trades_file_name, "%s/%s-%s.first_trades", graph_dir, base_day, state->best_stock_name);		
	sprintf(second_trades_file_name, "%s/%s-%s.second_trades", graph_dir, base_day, state->best_stock_name);		
	sprintf(moving_average_file_name, "%s/%s-%s.moving_average", graph_dir, base_day, state->best_stock_name);
	sprintf(slow_moving_average_file_name, "%s/%s-%s.slow_moving_average", graph_dir, base_day, state->best_stock_name);

	strcpy(first_trades_day, long2isodate(state->first_orders_day_date_l));
	strcpy(first_trades_dow, long2dows(state->first_orders_day_date_l));
	strcpy(second_trades_day, long2isodate(state->second_orders_day_date_l));
	strcpy(second_trades_dow, long2dows(state->second_orders_day_date_l));
	if (state->trade_order == BUY_SELL)
		fprintf(graph_file, "plot '%s' using 1:2 title 'Buy %s' with lines, '%s' using 1:2 title 'Sell %s' with lines, '%s' using 1:2 title 'Moving Average of Delta' with lines, '%s' using 1:2 title 'Slow Moving Average of Delta' with lines, '%s' using 1:2 title 'First Trades %s %s' with impulses, '%s' using 1:2 title 'Second Trades %s %s' with impulses\n", 
			quotes_file_name, state->best_stock_name, reference_file_name, state->ref_stock_name, 
			moving_average_file_name, slow_moving_average_file_name, 
			first_trades_file_name, first_trades_day, first_trades_dow, 
			second_trades_file_name, second_trades_day, second_trades_dow);
	else
		fprintf(graph_file, "plot '%s' using 1:2 title 'Sell %s' with lines, '%s' using 1:2 title 'Buy %s' with lines, '%s' using 1:2 title 'Moving Average of Delta' with lines, '%s' using 1:2 title 'Slow Moving Average of Delta' with lines, '%s' using 1:2 title 'First Trades %s %s' with impulses, '%s' using 1:2 title 'Second Trades %s %s' with impulses\n", 
			quotes_file_name, state->best_stock_name, reference_file_name, state->ref_stock_name, 
			moving_average_file_name, slow_moving_average_file_name, 
			first_trades_file_name, first_trades_day, first_trades_dow, 
			second_trades_file_name, second_trades_day, second_trades_dow);

	fprintf(graph_file, "pause -1  'press return'\n\n");
	
	quotes = fopen(quotes_file_name, "w");
	reference = fopen(reference_file_name, "w");
	first_trades = fopen(first_trades_file_name, "w");
	second_trades = fopen(second_trades_file_name, "w");
	moving_average_file = fopen(moving_average_file_name, "w");
	slow_moving_average_file = fopen(slow_moving_average_file_name, "w");
	
	for (i = 0; i < MINUTES_IN_A_WEEK; i++)
	{	// Dias sem contacoes (feriados?) tem o mesmo preco em todos os minutos
		(stock_price != NULL)? fprintf(quotes, "%d, %f\n", i, stock_price[i] / state->best_stock_base_price - 1.0): fprintf(quotes, "%d, %f\n", i, 0.0);
		(stock_price != NULL)? fprintf(reference, "%d, %f\n", i, ref_price[i] / state->ref_stock_base_price - 1.0): fprintf(reference, "%d, %f\n", i, 0.0);
		(stock_price != NULL)? fprintf(moving_average_file, "%d, %f\n", i, (LEVERAGE * delta_moving_average(state, stock_price, ref_price, i)) / 2.0): fprintf(moving_average_file, "%d, %f\n", i, 0.0);
		(stock_price != NULL)? fprintf(slow_moving_average_file, "%d, %f\n", i, (LEVERAGE * slow_delta_moving_average(state, stock_price, ref_price, i)) / 2.0): fprintf(slow_moving_average_file, "%d, %f\n", i, 0.0);
		if (i == state->first_orders_minute)
			(stock_price != NULL)? fprintf(first_trades, "%d, %f\n", i, -0.05): fprintf(first_trades, "%d, %f\n", i, 0.0);
		if (i == state->second_orders_minute)
			(stock_price != NULL)? fprintf(second_trades, "%d, %f\n", i, -0.05): fprintf(second_trades, "%d, %f\n", i, 0.0);
	}
	
	fclose(graph_file);
	fclose(quotes);
	fclose(reference);
	fclose(first_trades);
	fclose(second_trades);
	fclose(moving_average_file);
	fclose(slow_moving_average_file);
}
