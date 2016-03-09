#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <time.h>
#include <sys/types.h>
#include <dirent.h>
#ifdef	USE_IPC
#include "ipc.h"
#include "messages.h"
#endif
#include "timeutil.h"
#include "trading_system.h"

/* Global variables */

double LEVERAGE = 2.0;
double BROKERAGE = 0.0003;

int MOVING_AVERAGE_NUM_PERIODS = 20;
int SLOW_MOVING_AVERAGE_NUM_PERIODS = 120;

// Currently the ideal is 0.03
double TRADING_SYSTEM_STOP_GAIN = 0.03;
double TRADING_SYSTEM_STOP_LOSS	= -0.1;
double TRADING_SYSTEM_BUY_SELL_DELTA_RETURN = 0.0015;

// Mon=1, Tue=2, Wed=3, Thu=4, Fri=5
int FISRT_DAY_OF_TRADING_WEEK = 4;

int TRADING_OPENING_HOUR = 10;

int SAMPLE_SIZE = 3;
int DAYS_BETWEEN_SAMPLES = 5;

// Currently the ideal is 0
int NUM_DAYS_BEFORE_BEFORE_PREDICTION = 0;


int NUM_SAMPLES_FOR_PREDICTION = 3 + 1;

STATE g_state;

int g_reference_stock_index;
char *g_begin_date;
char *g_end_date;
long g_trading_system_current_day_date_l;

DAILY_DATA **g_daily_data = NULL;
int g_num_stocks_of_interest = 0;


double g_best_stock_price[MINUTES_IN_A_WEEK];
double g_ref_stock_price[MINUTES_IN_A_WEEK];

int g_moving_average_current_minute = -1;

int g_stop_trading_system = 0;

char *g_state_string[] = 
{
	"INITIALIZE",
	"WAIT_BEGIN_WEEK",
	"SELECT_STOCK",
	"WAIT_MOVING_AVERAGE",
	"WAIT_FIRST_ORDERS_TRIGGER",
	"SEND_FIRST_ORDERS",
	"WAIT_FIRST_ORDERS",
	"WAIT_FIRST_ORDERS_CANCEL",
	"WAIT_SECOND_ORDERS_TRIGGER",
	"SEND_SECOND_ORDERS",
	"WAIT_SECOND_ORDERS",
	"WAIT_SECOND_ORDERS_CANCEL",
	"QUIT",
	"INVALID_STATE"
};



/*
*********************************************************************************
* Print error and terminate							*
*********************************************************************************
*/

void
Erro(char *msg1, char *msg2, char *msg3)
{
#ifndef	USE_IPC	
	fprintf(stderr, "Error: %s%s%s\n", msg1, msg2, msg3);
	exit(1);
#else
	char description[1000];
	time_t now;
	struct tm *brokentime;

	fprintf(stderr, "Error: %s%s%s\n", msg1, msg2, msg3);
	
	sprintf(description, "Error: %s%s%s\n", msg1, msg2, msg3);
	now = time(NULL);
	brokentime = localtime(&now);
	publish_control_message(1, description, asctime(brokentime), TS_NAME);
#endif
}



/*
*********************************************************************************
* Alloc memory at building time							*
*********************************************************************************
*/

void *
alloc_mem(size_t size)
{    
	void *pointer;

	if ((pointer = malloc(size)) == (void *) NULL)
		Erro("cannot alloc more memory.", "", "");
	return (pointer);
}



double
compute_current_week_return(STATE *state)
{
	double current_total_capital, transaction_amount;

	switch (state->trade_order)
	{
		case BUY_SELL:
			transaction_amount = (double) (state->best_stock_quantity) * state->best_stock_price;
			current_total_capital = state->capital + transaction_amount - trade_cost(transaction_amount);

			transaction_amount = (double) (state->ref_stock_quantity) * state->ref_stock_price;
			current_total_capital = current_total_capital - transaction_amount - trade_cost(transaction_amount);	
			break;
			
		case SELL_BUY:
			transaction_amount = (double) (state->best_stock_quantity) * state->best_stock_price;
			current_total_capital = state->capital - transaction_amount - trade_cost(transaction_amount);

			transaction_amount = (double) (state->ref_stock_quantity) * state->ref_stock_price;
			current_total_capital = current_total_capital + transaction_amount - trade_cost(transaction_amount);	
			break;
	}
	
	return ((current_total_capital - state->previous_capital) / state->previous_capital);
}



double
compute_possible_week_return(STATE *state)
{
	double current_total_capital, transaction_amount;

	switch (state->trade_order)
	{
		case BUY_SELL:
			transaction_amount = (double) (state->best_stock_quantity) * state->best_stock_price;
			current_total_capital = state->previous_capital + transaction_amount - trade_cost(transaction_amount);

			transaction_amount = (double) (state->ref_stock_quantity) * state->ref_stock_price;
			current_total_capital = current_total_capital - transaction_amount - trade_cost(transaction_amount);	
			break;
			
		case SELL_BUY:
			transaction_amount = (double) (state->best_stock_quantity) * state->best_stock_price;
			current_total_capital = state->previous_capital - transaction_amount - trade_cost(transaction_amount);

			transaction_amount = (double) (state->ref_stock_quantity) * state->ref_stock_price;
			current_total_capital = current_total_capital + transaction_amount - trade_cost(transaction_amount);	
			break;
	}
	
	return ((current_total_capital - state->previous_capital) / state->previous_capital);
}


double
compute_current_week_gross_return(STATE *state)
{
	double current_delta;

	switch (state->trade_order)
	{
		case BUY_SELL:
			current_delta = (g_best_stock_price[state->moving_average_current_minute] / state->best_stock_base_price) - (g_ref_stock_price[state->moving_average_current_minute] / state->ref_stock_base_price);
			break;
			
		case SELL_BUY:
			current_delta = (g_ref_stock_price[state->moving_average_current_minute] / state->ref_stock_base_price) - (g_best_stock_price[state->moving_average_current_minute] / state->best_stock_base_price);
			break;
	}
	
	return (LEVERAGE * (current_delta / 2.0));
}


double
compute_final_week_return(STATE *state)
{
	return ((state->capital - state->previous_capital) / state->previous_capital);
}


/*
*********************************************************************************
* Plot graph to screen or to file						*
*********************************************************************************
*/

void 
plot_graph(int where_to, double *best_stock_price, double *ref_stock_price, char *output_file_name)
{
	int i;
	char first_trades_day[1000];
	char first_trades_dow[1000];
	char second_trades_day[1000];
	char second_trades_dow[1000];
	char quotes_file_name[1000];
	char reference_file_name[1000];
	char first_trades_file_name[1000];
	char second_trades_file_name[1000];
	char moving_average_file_name[1000];
	char slow_moving_average_file_name[1000];
	FILE *quotes, *reference, *first_trades, *second_trades, *moving_average_file, *slow_moving_average_file, *gnuplot_pipe;
	STATE *state;

	char *graph_dir = "GRAFICO_ONLINE";
	
	setlocale (LC_ALL, "C");
	
	state = &g_state;
	
	if (best_stock_price == NULL)
	{
		printf("Waiting a quote for initializing internal data\n");
		return;
	}
	
	if (where_to == TO_SCREEN)
		system("pkill gnuplot");
	
	sprintf(quotes_file_name, "%s/%s.quotes", graph_dir, state->best_stock_name);
	sprintf(reference_file_name, "%s/%s.quotes", graph_dir, state->ref_stock_name);
	sprintf(first_trades_file_name, "%s/%s.first_trades", graph_dir, state->best_stock_name);		
	sprintf(second_trades_file_name, "%s/%s.second_trades", graph_dir, state->best_stock_name);		
	sprintf(moving_average_file_name, "%s/%s.moving_average", graph_dir, state->best_stock_name);
	sprintf(slow_moving_average_file_name, "%s/%s.slow_moving_average", graph_dir, state->best_stock_name);

	strcpy(first_trades_day, long2isodate(state->first_orders_day_date_l));
	strcpy(first_trades_dow, long2dows(state->first_orders_day_date_l));
	strcpy(second_trades_day, long2isodate(state->second_orders_day_date_l));
	strcpy(second_trades_dow, long2dows(state->second_orders_day_date_l));

	quotes = fopen(quotes_file_name, "w");
	reference = fopen(reference_file_name, "w");
	first_trades = fopen(first_trades_file_name, "w");
	second_trades = fopen(second_trades_file_name, "w");
	moving_average_file = fopen(moving_average_file_name, "w");
	slow_moving_average_file = fopen(slow_moving_average_file_name, "w");
	
	for (i = 0; i < state->moving_average_current_minute; i++)
	{	// Dias sem contacoes (feriados?) tem o mesmo preco em todos os minutos
		(best_stock_price != NULL)? fprintf(quotes, "%d %lf\n", i, best_stock_price[i] / state->best_stock_base_price - 1.0): fprintf(quotes, "%d %f\n", i, 0.0);
		(best_stock_price != NULL)? fprintf(reference, "%d %lf\n", i, ref_stock_price[i] / state->ref_stock_base_price - 1.0): fprintf(reference, "%d %f\n", i, 0.0);
		(best_stock_price != NULL)? fprintf(moving_average_file, "%d %lf\n", i, (LEVERAGE * delta_moving_average(state, best_stock_price, ref_stock_price, i)) / 2.0): fprintf(moving_average_file, "%d %f\n", i, 0.0);
		(best_stock_price != NULL)? fprintf(slow_moving_average_file, "%d %f\n", i, (LEVERAGE * slow_delta_moving_average(state, best_stock_price, ref_stock_price, i)) / 2.0): fprintf(slow_moving_average_file, "%d %f\n", i, 0.0);
		if (i == state->first_orders_minute)
			(best_stock_price != NULL)? fprintf(first_trades, "%d %lf\n", i, -0.05): fprintf(first_trades, "%d %lf\n", i, 0.0);
		if (i == state->second_orders_minute)
			(best_stock_price != NULL)? fprintf(second_trades, "%d %lf\n", i, -0.05): fprintf(second_trades, "%d %lf\n", i, 0.0);
	}

	fclose(quotes);
	fclose(reference);
	fclose(first_trades);
	fclose(second_trades);
	fclose(moving_average_file);
	fclose(slow_moving_average_file);

	if (where_to == TO_SCREEN)
	{
		gnuplot_pipe = popen("gnuplot -persist", "w");
		fprintf(gnuplot_pipe, "set grid\n");
	}
	else
	{
		gnuplot_pipe = popen("gnuplot", "w");
		fprintf(gnuplot_pipe, "set terminal png\nset output \"%s\"\nset grid\n", output_file_name);
	}
	
	switch (state->state)
	{
		case WAIT_MOVING_AVERAGE:
		case WAIT_FIRST_ORDERS_TRIGGER:
		case SEND_FIRST_ORDERS:
		case WAIT_FIRST_ORDERS:
		case WAIT_FIRST_ORDERS_CANCEL:
			if (state->trade_order == BUY_SELL)
				fprintf(gnuplot_pipe, "plot '%s' using 1:2 title 'Buy %s' w lines, '%s' using 1:2 title 'Sell %s' w lines, '%s' using 1:2 title 'Moving Average of Delta' w lines, '%s' using 1:2 title 'Slow Moving Average of Delta' w lines\n", 
					quotes_file_name, state->best_stock_name, reference_file_name, state->ref_stock_name, 
					moving_average_file_name, slow_moving_average_file_name);
			else
				fprintf(gnuplot_pipe, "plot '%s' using 1:2 title 'Sell %s' w lines, '%s' using 1:2 title 'Buy %s' w lines, '%s' using 1:2 title 'Moving Average of Delta' w lines, '%s' using 1:2 title 'Slow Moving Average of Delta' w lines\n", 
					quotes_file_name, state->best_stock_name, reference_file_name, state->ref_stock_name, 
					moving_average_file_name, slow_moving_average_file_name);
			break;

		case WAIT_SECOND_ORDERS_TRIGGER:
		case SEND_SECOND_ORDERS:
		case WAIT_SECOND_ORDERS:
		case WAIT_SECOND_ORDERS_CANCEL:
			if (state->trade_order == BUY_SELL)
				fprintf(gnuplot_pipe, "plot '%s' using 1:2 title 'Buy %s' w lines, '%s' using 1:2 title 'Sell %s' w lines, '%s' using 1:2 title 'Moving Average of Delta' w lines, '%s' using 1:2 title 'Slow Moving Average of Delta' w lines, '%s' using 1:2 title \"First Trades %s %s\" w impulses\n", 
					quotes_file_name, state->best_stock_name, reference_file_name, state->ref_stock_name, 
					moving_average_file_name, slow_moving_average_file_name, 
					first_trades_file_name, first_trades_day, first_trades_dow);
			else
				fprintf(gnuplot_pipe, "plot '%s' using 1:2 title 'Sell %s' w lines, '%s' using 1:2 title 'Buy %s' w lines, '%s' using 1:2 title 'Moving Average of Delta' w lines, '%s' using 1:2 title 'Slow Moving Average of Delta' w lines, '%s' using 1:2 title \"First Trades %s %s\" w impulses\n", 
					quotes_file_name, state->best_stock_name, reference_file_name, state->ref_stock_name, 
					moving_average_file_name, slow_moving_average_file_name, 
					first_trades_file_name, first_trades_day, first_trades_dow);
			break;
			
		case WAIT_BEGIN_WEEK:
		case QUIT:
			if (state->trade_order == BUY_SELL)
				fprintf(gnuplot_pipe, "plot '%s' using 1:2 title 'Buy %s' w lines, '%s' using 1:2 title 'Sell %s' w lines, '%s' using 1:2 title 'Moving Average of Delta' w lines, '%s' using 1:2 title 'Slow Moving Average of Delta' w lines, '%s' using 1:2 title \"First Trades %s %s\" w impulses, '%s' using 1:2 title 'Second Trades %s %s' w impulses\n", 
					quotes_file_name, state->best_stock_name, reference_file_name, state->ref_stock_name, 
					moving_average_file_name, slow_moving_average_file_name, 
					first_trades_file_name, first_trades_day, first_trades_dow, 
					second_trades_file_name, second_trades_day, second_trades_dow);
			else
				fprintf(gnuplot_pipe, "plot '%s' using 1:2 title 'Sell %s' w lines, '%s' using 1:2 title 'Buy %s' w lines, '%s' using 1:2 title 'Moving Average of Delta' w lines, '%s' using 1:2 title 'Slow Moving Average of Delta' w lines, '%s' using 1:2 title \"First Trades %s %s\" w impulses, '%s' using 1:2 title 'Second Trades %s %s' w impulses\n", 
					quotes_file_name, state->best_stock_name, reference_file_name, state->ref_stock_name, 
					moving_average_file_name, slow_moving_average_file_name, 
					first_trades_file_name, first_trades_day, first_trades_dow, 
					second_trades_file_name, second_trades_day, second_trades_dow);

			break;
	}
	pclose(gnuplot_pipe);
}



void
print_state(FILE *output_file, STATE state, double *best_stock_price, double *ref_stock_price)
{
	fprintf(output_file, "\ntrading system time = %s\n", state.now);
	fprintf(output_file, "state = %s\n\n", g_state_string[state.state]);

	fprintf(output_file, "trade order 		= %s\n", ((state.trade_order == BUY_SELL)? "BUY_SELL": "SELL_BUY"));
	fprintf(output_file, "best stock 		= %s\n", state.best_stock_name);
	fprintf(output_file, "best stock price 	= R$%.2f\n", state.best_stock_price);
	fprintf(output_file, "ref. stock  		= %s\n", state.ref_stock_name);
	fprintf(output_file, "ref. stock price 	= R$%.2f\n\n", state.ref_stock_price);
	switch(state.state)
	{
		case WAIT_FIRST_ORDERS_TRIGGER:
			fprintf(output_file, "current week return       = %.2lf%%\n", 100.0 * compute_current_week_return(&g_state));
			fprintf(output_file, "current week gross return = %.2lf%%\n\n", 100.0 * compute_current_week_gross_return(&g_state));

			fprintf(output_file, "moving_average_of_delta\t\t%.4lf\n", state.moving_average_of_delta);
			fprintf(output_file, "slow_moving_average_of_delta\t\t%.4lf\n\n", slow_delta_moving_average(&g_state, g_best_stock_price, g_ref_stock_price, g_state.moving_average_current_minute));

			fprintf(output_file, "the system will send the first orders if moving_average_of_delta > %.4lf\n", 
			       state.previous_minimum_delta + (2.0 * TRADING_SYSTEM_BUY_SELL_DELTA_RETURN) / LEVERAGE);
			break;

		case SEND_FIRST_ORDERS:
		case WAIT_FIRST_ORDERS:
		case WAIT_FIRST_ORDERS_CANCEL:
			fprintf(output_file, "current week return       = %.2lf%%\n", 100.0 * compute_current_week_return(&g_state));
			fprintf(output_file, "current week gross return = %.2lf%%\n\n", 100.0 * compute_current_week_gross_return(&g_state));

			fprintf(output_file, "moving_average_of_delta\t\t%.4lf\n", state.moving_average_of_delta);
			fprintf(output_file, "slow_moving_average_of_delta\t\t%.4lf\n\n", slow_delta_moving_average(&g_state, g_best_stock_price, g_ref_stock_price, g_state.moving_average_current_minute));

			fprintf(output_file, "first orders:\t\t\t%s %s @ R$%.2lf, order @ R$%.2lf, quant. = %d\n", 
				((state.trade_order == BUY_SELL)? "buy": "sell"), state.best_stock_name, 
				state.best_stock_price, state.best_stock_order_price, state.best_stock_quantity); 
			fprintf(output_file, "             \t\t\t%s %s @ R$%.2lf, order @ R$%.2lf, quant. = %d\n", 
				((state.trade_order == BUY_SELL)? "sell": "buy"), state.ref_stock_name, 
				state.ref_stock_price, state.ref_stock_order_price, state.ref_stock_quantity);
			break;
			
		case WAIT_SECOND_ORDERS_TRIGGER:
			fprintf(output_file, "current week return       = %.2lf%%\n", 100.0 * compute_current_week_return(&g_state));
			fprintf(output_file, "current week gross return = %.2lf%%\n\n", 100.0 * compute_current_week_gross_return(&g_state));

			fprintf(output_file, "moving_average_of_delta\t\t%.4lf\n", state.moving_average_of_delta);
			fprintf(output_file, "slow_moving_average_of_delta\t\t%.4lf\n\n", slow_delta_moving_average(&g_state, g_best_stock_price, g_ref_stock_price, g_state.moving_average_current_minute));

			fprintf(output_file, "first orders:\t\t\t%s %s @ R$%.2lf, order @ R$%.2lf, quant. = %d\n", 
				state.best_stock_name, ((state.trade_order == BUY_SELL)? "bought": "sold"), 
				((state.trade_order == BUY_SELL)? state.best_stock_buy_price: state.best_stock_sell_price), 
				state.best_stock_order_price, state.best_stock_quantity); 
			fprintf(output_file, "             \t\t\t%s %s @ R$%.2lf, order @ R$%.2lf, quant. = %d\n", 
				state.ref_stock_name, ((state.trade_order == BUY_SELL)? "sold": "bought"), 
				((state.trade_order == BUY_SELL)? state.ref_stock_sell_price: state.ref_stock_buy_price),  
				state.ref_stock_order_price, state.ref_stock_quantity); 
			break;

		case SEND_SECOND_ORDERS:
		case WAIT_SECOND_ORDERS:
		case WAIT_SECOND_ORDERS_CANCEL:
			fprintf(output_file, "current week return       = %.2lf%%\n", 100.0 * compute_current_week_return(&g_state));
			fprintf(output_file, "current week gross return = %.2lf%%\n\n", 100.0 * compute_current_week_gross_return(&g_state));

			fprintf(output_file, "moving_average_of_delta\t\t%.4lf\n", state.moving_average_of_delta);
			fprintf(output_file, "slow_moving_average_of_delta\t\t%.4lf\n\n", slow_delta_moving_average(&g_state, g_best_stock_price, g_ref_stock_price, g_state.moving_average_current_minute));

			fprintf(output_file, "first orders:\t\t\t%s %s @ R$%.2lf\n", 
				state.best_stock_name, ((state.trade_order == BUY_SELL)? "bought": "sold"), 
				((state.trade_order == BUY_SELL)? state.best_stock_buy_price: state.best_stock_sell_price)); 
			fprintf(output_file, "             \t\t\t%s %s @ R$%.2lf\n", 
				state.ref_stock_name, ((state.trade_order == BUY_SELL)? "sold": "bought"), 
				((state.trade_order == BUY_SELL)? state.ref_stock_sell_price: state.ref_stock_buy_price)); 

			fprintf(output_file, "\nsecond orders:\t\t\t%s %s @ R$%.2lf, order @ R$%.2lf, quant. = %d\n", 
				state.best_stock_name, ((state.trade_order == BUY_SELL)? "sell": "buy"), 
				((state.trade_order == BUY_SELL)? state.best_stock_sell_price: state.best_stock_buy_price),
				state.best_stock_order_price, state.best_stock_quantity); 
			fprintf(output_file, "             \t\t\t%s %s @ R$%.2lf, order @ R$%.2lf, quant. = %d\n", 
				state.ref_stock_name, ((state.trade_order == BUY_SELL)? "buy": "sell"), 
				((state.trade_order == BUY_SELL)? state.ref_stock_buy_price: state.ref_stock_sell_price),  
				state.ref_stock_order_price, state.ref_stock_quantity); 
			break;
			
		case WAIT_BEGIN_WEEK:
			fprintf(output_file, "final week return = %.2lf%%\n", 100.0 * compute_final_week_return(&g_state));
			fprintf(output_file, "current week gross return = %.2lf%%\n\n", 100.0 * compute_current_week_gross_return(&g_state));

			fprintf(output_file, "moving_average_of_delta\t\t%.4lf\n", state.moving_average_of_delta);
			fprintf(output_file, "slow_moving_average_of_delta\t\t%.4lf\n\n", slow_delta_moving_average(&g_state, g_best_stock_price, g_ref_stock_price, g_state.moving_average_current_minute));

			fprintf(output_file, "first orders:\t\t\t%s %s @ R$%.2lf\n", 
				state.best_stock_name, ((state.trade_order == BUY_SELL)? "bought": "sold"), 
				((state.trade_order == BUY_SELL)? state.best_stock_buy_price: state.best_stock_sell_price)); 
			fprintf(output_file, "             \t\t\t%s %s @ R$%.2lf\n", 
				state.ref_stock_name, ((state.trade_order == BUY_SELL)? "sold": "bought"), 
				((state.trade_order == BUY_SELL)? state.ref_stock_sell_price: state.ref_stock_buy_price)); 

			fprintf(output_file, "\nsecond orders:\t\t\t%s %s @ R$%.2lf, order @ R$%.2lf, quant. = %d\n", 
				state.best_stock_name, ((state.trade_order == BUY_SELL)? "sold": "bought"), 
				((state.trade_order == BUY_SELL)? state.best_stock_sell_price: state.best_stock_buy_price),
				state.best_stock_order_price, state.best_stock_quantity); 
			fprintf(output_file, "             \t\t\t%s %s @ R$%.2lf, order @ R$%.2lf, quant. = %d\n", 
				state.ref_stock_name, ((state.trade_order == BUY_SELL)? "bought": "sold"), 
				((state.trade_order == BUY_SELL)? state.ref_stock_buy_price: state.ref_stock_sell_price),  
				state.ref_stock_order_price, state.ref_stock_quantity); 

			fprintf(output_file, "\n# traded best stock %s, ref. stock %s,  previous capital = R$%.2lf,  current capital = R$%.2lf,  week return: %+0.2lf%%\n", 
				state.best_stock_name, state.ref_stock_name, 
				state.previous_capital, 
				state.capital, 
				100.0 * (state.capital - state.previous_capital) / state.previous_capital);
			break;
	}
}
