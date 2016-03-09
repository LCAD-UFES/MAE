#include "forms.h"
#include "trading_system_control.h"
#include "mae.h"
#include "../wnn_pred.h"
#include "ipc.h"
#include "messages.h"
#include "trading_system.h"
#include "timeutil.h"

#define CONTROL_ORDER 	3 /* buy/sell order control message code */

extern STATE state;
extern double *price_samples;

/*** callbacks and freeobj handles for form Trading_System ***/

void order_buy_price_callback(FL_OBJECT *ob, long data)
{
	/* fill-in code for callback */
}

void order_buy_quantity_callback(FL_OBJECT *ob, long data)
{
	/* fill-in code for callback */
}

void order_sell_price_callback(FL_OBJECT *ob, long data)
{
	/* fill-in code for callback */
}

void order_sell_quantity_callback(FL_OBJECT *ob, long data)
{
	/* fill-in code for callback */
}


/* Function responsable by the check up on the input value */
int input_check_up(char *input_val) 
{
	char aux[1000];
	int i;
	int input_val_size;
	
	strcpy(aux, input_val);
	
	input_val_size = strlen(aux);
	for (i=0;i<input_val_size;i++)
	{
		if (aux[i] != '1' || aux[i] != '2' || aux[i] != '3' || aux[i] != '4' || aux[i] != '5' || aux[i] != '6' || aux[i] != '7' || aux[i] != '8' || aux[i] != '9' || aux[i] != '0' || aux[i] != '.')  
		{
			return 0;
		}
	}
	return 1;
}


void buy_button_callback(FL_OBJECT *ob, long data)
{
	char description[1000];
	char question[2000];
	const char *input_val;
	
	input_val = fl_get_input(order_buy_price_val);
	if (input_check_up((char *)input_val) == 0) return;
	if (input_val == NULL)
	{
		fprintf(stderr, "Could not read input val from order_buy_price_val in buy_button_callback()\n");
		return;
	}
	state.order_price = atof(input_val);
	
	input_val = fl_get_input(order_buy_quantity_val);
	if (input_val == NULL)
	{
		fprintf(stderr, "Could not read input val from order_quantity_price_val in buy_button_callback()\n");
		return;
	}
	state.quantity = atoi(input_val);
	sprintf(description, "B %d %s %.2f", state.quantity, state.best_stock_name, state.order_price); // Ex: B 400 PETR4 43.10  ou B 400 PETR4 43.10
	
	sprintf(question, "Are you shure that you want to send the order below?\n%s", description);
	if(fl_show_question (question, 0))
		publish_control_message(CONTROL_ORDER, description, state.now, TSCONTROL_NAME);
}


void fast_buy_button_callback(FL_OBJECT *ob, long data)
{
	char description[1000];
	char question[2000];
	
	state.order_price = price_samples[state.moving_average_current_minute];
	state.quantity = (state.capital / state.order_price);
	state.quantity = state.quantity - state.quantity % 100; // Lotes de 100 acoes
	sprintf(description, "B %d %s %.2f", state.quantity, state.best_stock_name, state.order_price); // Ex: B 400 PETR4 43.10  ou B 400 PETR4 43.10
	
	sprintf(question, "Are you shure that you want to send the order below?\n%s", description);
	if(fl_show_question (question, 0))
		publish_control_message(CONTROL_ORDER, description, state.now, TSCONTROL_NAME);
}


void sell_button_callback(FL_OBJECT *ob, long data)
{
	char description[1000];
	char question[2000];
	const char *input_val;
	
	input_val = fl_get_input(order_sell_price_val);
	if (input_check_up((char *)input_val) == 0) return;
	if (input_val == NULL)
	{
		fprintf(stderr, "Could not read input val from order_sell_price_val in sell_button_callback()\n");
		return;
	}
	state.order_price = atof(input_val);
	
	input_val = fl_get_input(order_sell_quantity_val);
	if (input_val == NULL)
	{
		fprintf(stderr, "Could not read input val from order_sell_quantity_val in sell_button_callback()\n");
		return;
	}
	state.quantity = atoi(input_val);
	sprintf(description, "S %d %s %.2f", state.quantity, state.best_stock_name, state.order_price); // Ex: B 400 PETR4 43.10  ou B 400 PETR4 43.10
	
	sprintf(question, "Are you shure that you want to send the order below?\n%s", description);
	if(fl_show_question (question, 0))
		publish_control_message(CONTROL_ORDER, description, state.now, TSCONTROL_NAME);
}


void fast_sell_button_callback(FL_OBJECT *ob, long data)
{
	char description[1000];
	char question[2000];
	
	state.order_price = price_samples[state.moving_average_current_minute];
	sprintf(description, "S %d %s %.2f", state.quantity, state.best_stock_name, state.order_price); // Ex: S 400 PETR4 43.10  ou S 400 PETR4 43.10
	
	sprintf(question, "Are you shure that you want to send the order below?\n%s", description);
	if(fl_show_question (question, 0))
		publish_control_message(CONTROL_ORDER, description, state.now, TSCONTROL_NAME);
}


void
fill_in_missing_prices()
{
	int i, j, last_i;
	double last_price;
	
	last_i = j = i = 0;
	while (i < state.moving_average_current_minute)
	{
		for ( ; i < state.moving_average_current_minute; i++)
		{
			if (price_samples[i] != 0.0)
				break;
		}
		
		j = i;
		last_price = price_samples[j];
		for ( ; j >= last_i; j--)
		{
			if (price_samples[j] == 0.0)
				price_samples[j] = last_price;
		}
		last_i = i;

		for ( ; i < state.moving_average_current_minute; i++)
		{
			if (price_samples[i] == 0.0)
				break;
		}
	}
}


void plot_graph_button_callback(FL_OBJECT *ob, long data)
{
	int i;
	char quotes_file_name[1000];
	char trade_file_name[1000];
	char trade2_file_name[1000];
	char moving_average_file_name[1000];
	FILE *gnuplot_pipe, *quotes, *trade, *trade2, *moving_average_file;

	char *graph_dir = "GRAFICO_ONLINE";
	char *stock_name = state.best_stock_name;
	int moving_average_first_day = state.moving_average_first_day;
	int day;
	double *intraday_price = price_samples;
	double price;
	
	if (intraday_price == NULL)
	{
		printf("Waiting a quote for initializing internal data\n");
		return;
	}
	
	system("pkill gnuplot");
	
	sprintf(quotes_file_name, "%s/%s.quotes", graph_dir, stock_name);
	sprintf(trade_file_name, "%s/%s.trade", graph_dir, stock_name);		
	sprintf(trade2_file_name, "%s/%s.trade2", graph_dir, stock_name);		
	sprintf(moving_average_file_name, "%s/%s.moving_average", graph_dir, stock_name);
	
	quotes = fopen(quotes_file_name, "w");
	moving_average_file = fopen(moving_average_file_name, "w");

	fill_in_missing_prices();	
	for (i = 0; i < state.moving_average_current_minute; i++)
	{
		fprintf(quotes, "%d, %f\n", i, intraday_price[i]);
		fprintf(moving_average_file, "%d, %f\n", i, moving_average(intraday_price, i));
	}
	
	fclose(quotes);
	fclose(moving_average_file);

	gnuplot_pipe = popen("gnuplot -persist", "w");
	switch(state.state)
	{
		case WAIT_MOVING_AVERAGE:
		case WAIT_MIN_TO_BUY:
		case SEND_BUY_ORDER:
		case WAIT_BUY:
		case WAIT_BUY_CANCEL:
			price = state.best_stock_maximum_buy_price;
			fprintf(gnuplot_pipe, "plot '%s' using 1:2 title 'Prices %s' with lines, '%s' using 1:2 title 'Moving Average' with lines, %f title 'Max. Buy Price'\n", 
				quotes_file_name, stock_name, moving_average_file_name, price);
			break;

		case WAIT_MAX_TO_SELL:
		case SEND_SELL_ORDER:
		case WAIT_SELL:
		case WAIT_SELL_CANCEL:
			trade = fopen(trade_file_name, "w");
			day = state.buy_day;
			i = state.buy_minute + (day - moving_average_first_day) * MINUTES_IN_A_DAY;
			fprintf(trade, "%d, %f\n", i, intraday_price[i]);
			fclose(trade);
			
			price = state.best_stock_minimum_sell_price;
			fprintf(gnuplot_pipe, "plot '%s' using 1:2 title 'Prices %s' with lines, '%s' using 1:2 title 'Moving Average' with lines, '%s' using 1:2 title 'Buy' with impulses, %f title 'Min. Sell Price'\n", 
				quotes_file_name, stock_name, moving_average_file_name, trade_file_name, price);
			break;
			
		case WAIT_BEGIN_WEEK:
			trade = fopen(trade_file_name, "w");
			day = state.buy_day;
			i = state.buy_minute + (day - moving_average_first_day) * MINUTES_IN_A_DAY;
			fprintf(trade, "%d, %f\n", i, intraday_price[i]);
			fclose(trade);

			trade2 = fopen(trade2_file_name, "w");
			day = state.sell_day;
			i = state.sell_minute + (day - moving_average_first_day) * MINUTES_IN_A_DAY;
			fprintf(trade2, "%d, %f\n", i, intraday_price[i]);
			fclose(trade2);

			price = state.best_stock_minimum_sell_price;
			fprintf(gnuplot_pipe, "plot '%s' using 1:2 title 'Prices %s' with lines, '%s' using 1:2 title 'Moving Average' with lines, '%s' using 1:2 title 'Buy' with impulses, '%s' using 1:2 title 'Sell' with impulses, %f title 'Min. Sell Price'\n", 
				quotes_file_name, stock_name, moving_average_file_name, trade_file_name, trade2_file_name, price);
			break;
	}
	pclose(gnuplot_pipe);
}
