#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <sys/types.h>
#include <dirent.h>
#include "mae.h"
#include "../wnn_pred.h"
#include "timeutil.h"
#include "ipc.h"
#include "messages.h"
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
moving_average(int minute)
{
	int i;
	double m_average = 0.0, num_samples = 0.0;
	
	if (minute >= MINUTES_IN_A_DAY)
	{
		Erro("minute >= NUM_MINUTES_IN_A_DAY in moving_average()", "", "");
		exit(1);
	}
		
	for (i = minute - MOVING_AVERAGE_NUM_PERIODS + 1; i <= minute; i++)
	{
		if (i > 0)
		{
			m_average += g_price_samples[i];
			num_samples += 1.0;
		}
		else
		{
			m_average += g_price_samples[0];
			num_samples += 1.0;
		}		
	}

	return (m_average / num_samples);
}

#endif


void
generate_graph(char *graph_dir, char *stock_name, int day, float *intraday_price, int trade_minute, float price, int buy)
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
		(intraday_price != NULL)? fprintf(moving_average_file, "%d, %f\n", i, moving_average(i)): fprintf(moving_average_file, "%d, %f\n", i, price);
		if (i == trade_minute)
			(intraday_price != NULL)? fprintf(trade, "%d, %f\n", i, intraday_price[i]): fprintf(trade, "%d, %f\n", i, price);
	}
	
	fclose(graph_file);
	fclose(quotes);
	fclose(trade);
	fclose(moving_average_file);
}



int
intraday_data_available(int target_day)
{
#ifdef	IPC_ON_LINE
	return (1);
#else
	if (target_day == g_train_num_samples)
	{
		Erro("No intraday data available in intraday_data_available(). ", "Shouldn't it be an on-line trading system compilation?", "");
		exit(1);
	}
		
	if (g_train_data_set[target_day].intraday_price != NULL)
		return (1);
	else
		return (0);
#endif
}
