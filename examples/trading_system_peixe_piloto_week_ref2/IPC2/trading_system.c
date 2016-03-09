#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#include <timeutil.h> 
#include <locale.h>
#include <math.h>
#include <unistd.h>
#include <ctype.h>
#ifdef USE_IPC
#include "ipc.h"
#endif
#include "messages.h"
#include "trading_system.h"


//#define	TEST_MISSING_QUOTES
extern double INITIAL_CAPITAL;


struct _performace
{
	double performance;
	int stock;
};

typedef struct _performace PERFORMANCE;



int
main_signal_buy_sell()
{
	int stock;
	int sum_signals = 0;
	
	for (stock = 0; stock < g_num_stocks_of_interest; stock++)
	{
		if (g_daily_data[stock]->current_predicted_return_buy_sell > 0.0)
			sum_signals++;
		else
			sum_signals--;
	}
	return (sum_signals);
}



int
main_signal_sell_buy()
{
	int stock;
	int sum_signals = 0;
	
	for (stock = 0; stock < g_num_stocks_of_interest; stock++)
	{
		if (g_daily_data[stock]->current_predicted_return_sell_buy > 0.0)
			sum_signals++;
		else
			sum_signals--;
	}
	return (sum_signals);
}



double
predict_buy_sell_return(STATE *state, int stock)
{
	int i;
	double ref_stock_price, stock_base_price, average_spread, buy_sell_return;
#ifdef	VERBOSE
	char date[1000];
	char dates[1000];
	char dates_full[1000];
#endif
	
	stock_base_price = g_daily_data[stock]->stock_close_price_for_prediction[0];
	ref_stock_price = g_daily_data[g_reference_stock_index]->stock_close_price_for_prediction[0];

#ifdef	VERBOSE
	sprintf(dates, "stock = %s, base date/base price/base ref_stock_price = %s/%.4lf/%.4lf\n  dates/prices/ref_stock_price:\n", 
			g_daily_data[stock]->stock_name, 
			long2isodate(g_daily_data[stock]->stock_close_price_for_prediction_date_l[0]),
			stock_base_price,
			ref_stock_price);
	strcpy(dates_full, dates);
#endif
	average_spread = 0.0;
	for (i = 0; i < SAMPLE_SIZE; i++)
	{
		average_spread += (g_daily_data[stock]->stock_close_price_for_prediction[i] / stock_base_price) - (g_daily_data[g_reference_stock_index]->stock_close_price_for_prediction[i] / ref_stock_price);
#ifdef	VERBOSE
		sprintf(dates, "  %s/%.4lf/%.4lf\n", long2isodate(g_daily_data[stock]->stock_close_price_for_prediction_date_l[i]), 
						    g_daily_data[stock]->stock_close_price_for_prediction[i],
						    g_daily_data[g_reference_stock_index]->stock_close_price_for_prediction[i]);
		strcat(dates_full, dates);
#endif
	}
	average_spread /= (double) SAMPLE_SIZE;
	buy_sell_return = ((g_daily_data[stock]->stock_close_price_for_prediction[i] / stock_base_price) - (g_daily_data[g_reference_stock_index]->stock_close_price_for_prediction[i] / ref_stock_price) - average_spread) / 2.0;
	
#ifdef	VERBOSE
	strcpy(date, long2isodate(g_trading_system_current_day_date_l));
	sprintf(dates, "current date %s, last sample date/price/ref_stock_price = %s/%.4lf/%.4lf, average_spread %.6lf,  return %.6lf", 
			date, 
			long2isodate(g_daily_data[stock]->stock_close_price_for_prediction_date_l[i]), 
			g_daily_data[stock]->stock_close_price_for_prediction[i],
			g_daily_data[g_reference_stock_index]->stock_close_price_for_prediction[i],
			average_spread, buy_sell_return);
	strcat(dates_full, dates);
	printf("%s\n\n", dates_full);
#endif
	
	g_daily_data[stock]->current_predicted_return_buy_sell = buy_sell_return;
	g_daily_data[stock]->current_predicted_return_sell_buy = -buy_sell_return;
	
	return (buy_sell_return);
}



double
predict_sell_buy_return(STATE *state, int stock)
{
	return (g_daily_data[stock]->current_predicted_return_sell_buy);
}



double
get_current_performance_buy_sell(STATE *state, int stock)
{
	double performance;
	
	performance = predict_buy_sell_return(state, stock);
	
	return (performance);
}



double
get_current_performance_sell_buy(STATE *state, int stock)
{
	double performance;
	
	performance = predict_sell_buy_return(state, stock);
	
	return (performance);
}



void
print_vector(double *vetor, int size)
{
	int a, i;
	
	i = 0;
	for (a = 1; a < size; a++)
	{
		printf("% .4f ", vetor[a]);
		i++;
		if (i > 10)
		{
			printf("\n");
			i = 0;
		}
	}
	printf("\n");
}



void
normalize_vector(double *vector_out, double *vector_in, int size)
{
	int a;
	double max = -1000.0;
	double min = 1000.0;
	
	for (a = 1; a < size; a++)
	{
		if (vector_in[a] > max)
			max = vector_in[a];
		if (vector_in[a] < min)
			min = vector_in[a];
	}
	
	for (a = 1; a < size; a++)
	{
		if ((max - min) == 0.0)
			vector_out[a] = 0.0000001;
		else
			vector_out[a] = 0.0000001 + (vector_in[a] - min) / (max - min);
	}
}



void
compute_average_performance_buy_sell(double *average_performance, double *performance, double *current_performance, int size)
{
	int a;
	
	for (a = 1; a < size; a++)
	{
//		average_performance[a] = 1.0 / ((1.0 / performance[a]) + (1.0 / current_performance[a]));
		average_performance[a] = current_performance[a];
//		average_performance[a] = (performance[a] + 3.0 * current_performance[a]) / 4.0;
//		average_performance[a] = current_performance[a] - 2.0 * performance[a];
//		average_performance[a] = current_performance[a] / performance[a];
//		if (current_performance[a] < TRADING_SYSTEM_STOP_GAIN)
//			average_performance[a] = (100.0 * (performance[a] - 0.5) * current_performance[a]);
//		else
//			average_performance[a] = (100.0 * (performance[a] - 0.5) * TRADING_SYSTEM_STOP_GAIN);
//		printf("current_performance = %f, performance = %f\n", current_performance[a], performance[a]);
	}
}



void
compute_average_performance_sell_buy(double *average_performance, double *performance, double *current_performance, int size)
{
	int a;
	
	for (a = 1; a < size; a++)
	{
//		average_performance[a] = 1.0 / ((1.0 / performance[a]) + (1.0 / current_performance[a]));
		average_performance[a] = current_performance[a];
//		average_performance[a] = (performance[a] + 3.0 * current_performance[a]) / 4.0;
//		average_performance[a] = current_performance[a] - 2.0 * performance[a];
//		average_performance[a] = current_performance[a] / performance[a];
//		if (current_performance[a] < TRADING_SYSTEM_STOP_GAIN)
//			average_performance[a] = (100.0 * (performance[a] - 0.5) * current_performance[a]);
//		else
//			average_performance[a] = (100.0 * (performance[a] - 0.5) * TRADING_SYSTEM_STOP_GAIN);
//		printf("current_performance = %f, performance = %f\n", current_performance[a], performance[a]);
	}
}


int
compare_performances(const void *p1, const void *p2)
{
	PERFORMANCE *performance1;
	PERFORMANCE *performance2;
	
	performance1 = (PERFORMANCE *) p1;
	performance2 = (PERFORMANCE *) p2;
	
	if (performance1->performance > performance2->performance)
		return (-1);
	else if (performance1->performance < performance2->performance)
		return (1);
	else
		return (0);
}



int
get_stock_of_a_given_rank(int rank, double *performance, int size)
{
	PERFORMANCE *stock_performance;
	int i, stock;
	
	stock_performance = (PERFORMANCE *) malloc(size * sizeof(PERFORMANCE));
	for (i = 0; i < size; i++)
	{
		stock_performance[i].performance = performance[i];
		stock_performance[i].stock = i;
	}
	qsort((void *) stock_performance, size, sizeof(PERFORMANCE), compare_performances);
	stock = stock_performance[rank].stock;

	free(stock_performance);
	
	return (stock);
}



void
select_stock_and_trade_order(STATE *state, double *current_performance_buy_sell, double *current_performance_sell_buy, int num_stocks)
{
	int best_stock_buy_sell, best_stock_sell_buy;
	
	//print_vector(current_performance_buy_sell, num_stocks);
	best_stock_buy_sell = get_stock_of_a_given_rank(0, current_performance_buy_sell, num_stocks);
	
	best_stock_sell_buy = get_stock_of_a_given_rank(0, current_performance_sell_buy, num_stocks);

	//printf("main_signal_buy_sell = %d, main_signal_sell_buy = %d\n", main_signal_buy_sell(), main_signal_sell_buy());
	if (main_signal_buy_sell() > main_signal_sell_buy())
	//if (((double) main_signal_buy_sell() * current_performance_buy_sell[best_stock_buy_sell]) >= 
	//    ((double) main_signal_sell_buy() * current_performance_sell_buy[best_stock_sell_buy]))
	//if (performance_buy_sell[best_stock_buy_sell] * current_performance_buy_sell[best_stock_buy_sell] >
	//    performance_sell_buy[best_stock_sell_buy] * current_performance_sell_buy[best_stock_sell_buy])
	//if (average_performance_buy_sell[best_stock_buy_sell] < average_performance_sell_buy[best_stock_sell_buy])
	//if (current_performance_buy_sell[best_stock_buy_sell] > current_performance_sell_buy[best_stock_sell_buy])
	//if (performance_buy_sell[best_stock_buy_sell] < performance_sell_buy[best_stock_sell_buy])
	//if ((performance_buy_sell[best_stock_buy_sell] + 5.0 * current_performance_buy_sell[best_stock_buy_sell]) <
	//    (performance_sell_buy[best_stock_sell_buy] + 5.0 * current_performance_sell_buy[best_stock_sell_buy]))
	//if (g_tt_data_set[best_stock_buy_sell][day].confidence > g_tt_data_set[best_stock_sell_buy][day].confidence)
	//if (g_tt_data_set[best_stock_buy_sell][day].confidence * current_performance_buy_sell[best_stock_buy_sell] > 
	//    g_tt_data_set[best_stock_sell_buy][day].confidence * current_performance_sell_buy[best_stock_sell_buy])
	//if ((g_tt_data_set[best_stock_buy_sell][day].confidence * 
	//    current_performance_buy_sell[best_stock_buy_sell] * (double) main_signal_buy_sell(day) *
	//    performance_buy_sell[best_stock_buy_sell]) > 
	//    (g_tt_data_set[best_stock_sell_buy][day].confidence * 
	//    current_performance_sell_buy[best_stock_sell_buy] * (double) main_signal_sell_buy(day) *
	//    performance_sell_buy[best_stock_sell_buy]))
	{
		state->trade_order = BUY_SELL;
		state->best_stock_index = best_stock_buy_sell;
		state->ref_stock_index = g_reference_stock_index;
		strcpy(state->best_stock_name, g_daily_data[best_stock_buy_sell]->stock_name);
		strcpy(state->ref_stock_name, g_daily_data[g_reference_stock_index]->stock_name);
		state->predicted_return = g_daily_data[best_stock_buy_sell]->current_predicted_return_buy_sell;
	}
	else
	{
		state->trade_order = SELL_BUY;
		state->best_stock_index = best_stock_sell_buy;
		state->ref_stock_index = g_reference_stock_index;
		strcpy(state->best_stock_name, g_daily_data[best_stock_sell_buy]->stock_name);
		strcpy(state->ref_stock_name, g_daily_data[g_reference_stock_index]->stock_name);
		state->predicted_return = g_daily_data[best_stock_sell_buy]->current_predicted_return_sell_buy;
	}
}


void
select_stock(STATE *state)
{
	int stock;
	double *current_performance_buy_sell;
	double *current_performance_sell_buy;

	current_performance_buy_sell = (double *) malloc(g_num_stocks_of_interest * sizeof(double));
	current_performance_sell_buy = (double *) malloc(g_num_stocks_of_interest * sizeof(double));
	for (stock = 0; stock < g_num_stocks_of_interest; stock++)
	{
		current_performance_buy_sell[stock] = get_current_performance_buy_sell(state, stock);
		current_performance_sell_buy[stock] = get_current_performance_sell_buy(state, stock);
	}
	select_stock_and_trade_order(state, current_performance_buy_sell, current_performance_sell_buy, g_num_stocks_of_interest);

	free(current_performance_buy_sell);
	free(current_performance_sell_buy);
}



#ifdef	USE_IPC

void
send_quote(double best_stock_quote, double reference_quote, int moving_average_current_minute)
{
	FILE *quote_file;
	FILE *control_file;
	char now[1000];
#define	VOLUME	0
	
	usleep(1000000/5);
	
	// Espera que seja lido o arquivo de quotes existente
	while (fileExists(quotesControltxt))
		;
		
	// Cria e escreve no arquivo de quotes para que o quotes_server_order_router leia
	quote_file = fopen(quotestxt, "w");
	
	get_trading_system_time(now, moving_average_current_minute);
	fprintf(quote_file, "2\n%s\n%s; %s\n%lf; %lf\n%d; %d", now, 
								 g_state.best_stock_name, g_state.ref_stock_name, 
								 best_stock_quote, reference_quote, 
								 VOLUME, VOLUME);
	
	fclose(quote_file);
	
	// Cria o arquivo de controle que fecha o processo de envio da quote para o quotes_server_order_router
	control_file = fopen(quotesControltxt, "w");
	fprintf(control_file, "quote posted\n");
	fclose(control_file);
}



void
get_stocks_quote_from_quotes_message(qsor_quotes_message* received_quotes, double *best_stock_quote, double *ref_stock_quote)
{
	int i;
	
	for (i = 0; (i < received_quotes->num_stocks) && (strcmp(received_quotes->stock_symbol[i], g_state.best_stock_name) != 0); i++)
		;
	if (i == received_quotes->num_stocks)
		*best_stock_quote = -1.0;
	else
		*best_stock_quote = received_quotes->price[i];

	for (i = 0; (i < received_quotes->num_stocks) && (strcmp(received_quotes->stock_symbol[i], g_state.ref_stock_name) != 0); i++)
		;
	if (i == received_quotes->num_stocks)
		*ref_stock_quote = -1.0;
	else
		*ref_stock_quote = received_quotes->price[i];
}



int
minutes_within_this_week(char *date_and_time)
{
	long message_day_date_l;
	int days_within_this_week;
	int hour, minute;
	
	message_day_date_l = isodate2long(date_and_time);

	days_within_this_week = (message_day_date_l - g_state.first_day_of_week_date_l) / SECONDS_IN_A_DAY;
	if (days_within_this_week > (6 - FISRT_DAY_OF_TRADING_WEEK))
		days_within_this_week = days_within_this_week - 2;
	
	// formato de date_and_time: 2011-03-26 Sat 22:34
	hour = atoi(date_and_time + 15);
	minute = atoi(date_and_time + 18);
	
	return (days_within_this_week * MINUTES_IN_A_DAY + (hour - TRADING_OPENING_HOUR) * 60 + minute);
}



int
fill_in_eventual_missing_quotes(int message_minute, int trading_system_minute, double best_stock_quote, double ref_stock_quote)
{
	double fill_in_best_stock_quote;
	double fill_in_ref_stock_quote;
	int i;
	
	if ((message_minute - trading_system_minute) == 1) // no missing quotes
		return (message_minute);
		
	if (trading_system_minute == -1) // first minutes not available: fill in with best stock current quote
	{
		fill_in_best_stock_quote = best_stock_quote;
		fill_in_ref_stock_quote = ref_stock_quote;
	}
	else
	{
		fill_in_best_stock_quote = g_best_stock_price[trading_system_minute];
		fill_in_ref_stock_quote = g_ref_stock_price[trading_system_minute];
	}
	
	for (i = trading_system_minute + 1; i < message_minute; i++)
	{
		g_best_stock_price[i] = fill_in_best_stock_quote;
		g_ref_stock_price[i] = fill_in_ref_stock_quote;
	}
	g_trading_system_current_day_date_l = timePlusDays(g_state.first_day_of_week_date_l, message_minute / MINUTES_IN_A_DAY);
		
	return (message_minute);
}



void
save_quote_message_log(qsor_quotes_message* received_quotes)
{
	char quote_log_file_name[1014];
	time_t t;
	FILE *message_log_file;
	int i;
	
	strcpy(quote_log_file_name, "MESSAGE_LOG/quote-");
	time(&t);
	strcat(quote_log_file_name, ctime(&t));
	quote_log_file_name[strlen(quote_log_file_name) - 1] = '\0';

	if ((message_log_file = fopen(quote_log_file_name, "w")) == NULL)
		Erro("Could not open quote_log_file_name named ", quote_log_file_name, " in save_quote_message_log()");
	
	fprintf(message_log_file, "%d\n", received_quotes->num_stocks);
	for (i = 0; i < received_quotes->num_stocks; i++)
	{
		fprintf(message_log_file, "%s\n%lf\n%lf\n", received_quotes->stock_symbol[i], received_quotes->price[i], received_quotes->volume[i]);
	}
	fprintf(message_log_file, "%s%s", received_quotes->date_and_time, received_quotes->host);
	
	fclose(message_log_file);
}



int
receive_quote(qsor_quotes_message* received_quotes)
{
	double best_stock_quote;
	double ref_stock_quote;
	int message_minute;
	
	save_quote_message_log(received_quotes);
	
	get_stocks_quote_from_quotes_message(received_quotes, &best_stock_quote, &ref_stock_quote);
	if ((best_stock_quote == -1.0) || (ref_stock_quote == -1.0)) // A cotacao da acao ou referencia nao veio com o quotes message. Inicio do pregao?
		return (0);
	
	message_minute = minutes_within_this_week(received_quotes->date_and_time);
	
	g_moving_average_current_minute = fill_in_eventual_missing_quotes(message_minute, g_moving_average_current_minute, best_stock_quote, ref_stock_quote);
	
	if (g_moving_average_current_minute < MINUTES_IN_A_WEEK)
	{
		g_best_stock_price[g_moving_average_current_minute] = best_stock_quote;
		g_ref_stock_price[g_moving_average_current_minute] = ref_stock_quote;
	}
	else
		Erro("g_moving_average_current_minute >= MINUTES_IN_A_WEEK in receive_quote()", "", "");

	return (1);
}



void
get_stocks_quote()
{
#ifndef	IPC_ON_LINE
	double *intraday_price;
	double *intraday_ref_price;
#endif
	int new_moving_average_current_minute;
	
	new_moving_average_current_minute = g_moving_average_current_minute + 1;
	
	if (new_moving_average_current_minute < MINUTES_IN_A_WEEK)
	{
#ifndef	IPC_ON_LINE
		intraday_price = get_stock_current_day_intraday_prices(&g_state, new_moving_average_current_minute);
		intraday_ref_price = get_ref_current_day_intraday_prices(&g_state, new_moving_average_current_minute);
		g_best_stock_price[new_moving_average_current_minute] = intraday_price[new_moving_average_current_minute % MINUTES_IN_A_DAY];
		g_ref_stock_price[new_moving_average_current_minute] = intraday_ref_price[new_moving_average_current_minute % MINUTES_IN_A_DAY];
#endif
		g_trading_system_current_day_date_l = timePlusDays(g_state.first_day_of_week_date_l, new_moving_average_current_minute / MINUTES_IN_A_DAY);
	}
#ifndef	IPC_ON_LINE
	else
		new_moving_average_current_minute--;

	if ((new_moving_average_current_minute < MINUTES_IN_A_WEEK) && (intraday_price != NULL))
	{
		send_quote(intraday_price[new_moving_average_current_minute % MINUTES_IN_A_DAY], intraday_ref_price[new_moving_average_current_minute % MINUTES_IN_A_DAY], new_moving_average_current_minute);
	}
#endif
}

#else


void
get_stocks_quote()
{
	double *intraday_price;
	double *intraday_ref_price;
	int new_moving_average_current_minute;
	
	new_moving_average_current_minute = g_moving_average_current_minute + 1;
	
	if (new_moving_average_current_minute < MINUTES_IN_A_WEEK)
	{
		intraday_price = get_stock_current_day_intraday_prices(&g_state, new_moving_average_current_minute);
		intraday_ref_price = get_ref_current_day_intraday_prices(&g_state, new_moving_average_current_minute);
		g_best_stock_price[new_moving_average_current_minute] = intraday_price[new_moving_average_current_minute % MINUTES_IN_A_DAY];
		g_ref_stock_price[new_moving_average_current_minute] = intraday_ref_price[new_moving_average_current_minute % MINUTES_IN_A_DAY];
		g_trading_system_current_day_date_l = timePlusDays(g_state.first_day_of_week_date_l, new_moving_average_current_minute / MINUTES_IN_A_DAY);
	}
	else
		new_moving_average_current_minute--;
	
	g_moving_average_current_minute = new_moving_average_current_minute;
}

#endif


void
print_week_stock_quotes(double *quotes)
{
	int i;

	for (i = 0; i < MINUTES_IN_A_WEEK; i++)
	{
		printf("%05d, %lf\n", i, quotes[i]);
	}
	printf("@@@@\n\n\n");
}


void
run_trading_system_state_machine()
{
	long last_current_day_date_l;
	
	g_state.first_day_of_week_date_l = g_trading_system_current_day_date_l = get_first_day();
	if (g_trading_system_current_day_date_l == -1)
	{
		// Not enough intraday data for running the experiment
		g_trading_system_current_day_date_l = isodate2long(g_begin_date);
		g_moving_average_current_minute = 0;
		initialize_state_machine();
		g_state.state = QUIT;
		trading_system_state_machine(g_moving_average_current_minute);
		return;
	}
	load_all_stocks_day_data(g_trading_system_current_day_date_l);
	select_stock(&g_state);
	
	g_moving_average_current_minute = -1;
	do
	{
		if ((g_moving_average_current_minute + 1) < MINUTES_IN_A_WEEK)
		{
			get_stocks_quote();
		}
		else
		{
			last_current_day_date_l = g_trading_system_current_day_date_l;
			g_state.first_day_of_week_date_l = g_trading_system_current_day_date_l = get_next_available_day(g_trading_system_current_day_date_l);
			if (g_trading_system_current_day_date_l == -1)
			{
				// Not enough intraday data for another week
				g_state.state = QUIT;
				g_trading_system_current_day_date_l = last_current_day_date_l;
				trading_system_state_machine(g_moving_average_current_minute);
				break;
			}
			load_all_stocks_day_data(g_trading_system_current_day_date_l); 
			select_stock(&g_state);
			
			g_moving_average_current_minute = -1;	// Esta linha e a de baixo garantem que havera contacao valida
			get_stocks_quote(); 			// no primeiro minuto da semana
		}
	} while (trading_system_state_machine(g_moving_average_current_minute) != QUIT);
}



double
get_initial_capital()
{
	FILE *file;
	double initial_capital;
	int ch;
	
	file = fopen("initial_capital.txt", "r");
	if (file == NULL)
		Erro("Could not open file initial_capital.txt in get_initial_capital()", "", "");
	do
	{
		ch = getc(file);
	} while (!isdigit(ch) && (ch != EOF));
	if (ch == EOF)
		Erro("Could not read a number from initial_capital.txt in get_initial_capital()", "", "");
	ungetc(ch, file);
	
	fscanf(file, "%lf", &initial_capital);
	if (initial_capital <= 0.0)
		Erro("Could not read a number > 0.0 from initial_capital.txt in get_initial_capital()", "", "");
	return (initial_capital);;
}


void
read_input_parameters(int argc, char **argv)
{
	read_trading_system_parameters();
	read_stock_list(argv[1]);
	g_reference_stock_index = get_reference_stock_index(argv[2]);
	g_begin_date = argv[3];
	g_end_date = argv[4];
}


#ifdef	USE_IPC

void
call_trading_system_state_machine()
{
	int state;

	state = trading_system_state_machine(g_moving_average_current_minute);

	if ((g_moving_average_current_minute + 1) < MINUTES_IN_A_WEEK)
	{
		get_stocks_quote();
	}
	else
	{
		g_state.first_day_of_week_date_l = g_trading_system_current_day_date_l = get_next_available_day(g_trading_system_current_day_date_l);
		if (g_trading_system_current_day_date_l != -1)
		{
			//print_week_stock_quotes(g_best_stock_price);
			//print_week_stock_quotes(g_ref_stock_price);
			
			load_all_stocks_day_data(g_trading_system_current_day_date_l); 
			select_stock(&g_state);

			g_moving_average_current_minute = -1;	// Esta linha e a de baixo garantem que havera contacao valida
			get_stocks_quote(); 			// no primeiro minuto da semana
		}
	}

	if (state == QUIT)
	{
#ifndef	USE_IPC	
		printf("The trading system has reached the QUIT state.\n");
		
		exit(0);
#else
		time_t now;
		struct tm *brokentime;

		now = time(NULL);
		brokentime = localtime(&now);
		publish_control_message(2, "The trading system has reached the QUIT state.\n", asctime(brokentime), TS_NAME);
#endif
	}
}


void
init_trading_system_state_machine()
{
	g_state.first_day_of_week_date_l = g_trading_system_current_day_date_l = get_first_day();
	if (g_trading_system_current_day_date_l == -1)
	{
		Erro("Could not find date ", g_begin_date, " in the data set.");
		exit(1);
	}
	load_all_stocks_day_data(g_trading_system_current_day_date_l);
	select_stock(&g_state);
	printf("\nSelected stock = %s, Trade order = %s\n", g_state.best_stock_name, ((g_state.trade_order == BUY_SELL)? "BUY_SELL": "SELL_BUY"));

	g_moving_average_current_minute = -1;
	get_stocks_quote();
}



void
hot_start_trading_system_state_machine()
{
	get_state_in_a_hot_start();
	
	load_all_stocks_day_data(g_state.first_day_of_week_date_l);

	get_stock_or_ref_current_day_intraday_prices_on_hot_start(g_state.best_stock_index, g_trading_system_current_day_date_l, g_moving_average_current_minute);
	get_stock_or_ref_current_day_intraday_prices_on_hot_start(g_state.ref_stock_index, g_trading_system_current_day_date_l, g_moving_average_current_minute);

	get_stocks_quote();
}



int
init_trading_system(int argc, char **argv)
{
	if (argc < 2)
	{
		printf("Erro: Missing input parameters.\n  Usage: trading_system <stock_list.txt> <reference> <yyyy-mm-dd yyyy-mm-dd>\n  Example: ./trading_system ../acoes_de_interesse.txt PETR3 2010-01-07 2010-01-20\n");

		IPC_disconnect();
		exit(1);
	}
	else
	{
		read_input_parameters(argc, argv);
	}
	INITIAL_CAPITAL = get_initial_capital();

	g_state = get_state();
	if (g_state.state == INITIALIZE)
	{
		save_state(g_state);
		init_trading_system_state_machine();
	}
	else
	{
		hot_start_trading_system_state_machine();
	}
	
	return (0);
}

#else


int
main(int argc, char **argv)
{
	/* Locale Setting */
	setlocale (LC_ALL, "C");

	if (argc < 2)
	{
		printf("Erro: Missing input parameters.\n  Usage: trading_system <stock_list.txt> <reference> <yyyy-mm-dd yyyy-mm-dd>\n  Example: ./trading_system ../acoes_de_interesse.txt PETR3 2010-01-07 2010-01-20\n");
		exit(1);
	}
	else
	{
		read_input_parameters(argc, argv);
	}
	INITIAL_CAPITAL = get_initial_capital();
	
	g_state = get_state();
	if (g_state.state == INITIALIZE)
		save_state(g_state);

	run_trading_system_state_machine();
	
	return (0);
}

#endif
