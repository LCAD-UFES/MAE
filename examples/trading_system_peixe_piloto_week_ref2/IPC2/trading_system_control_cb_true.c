#include <stdlib.h>
#include <locale.h>
#include "forms.h"
#include "trading_system_control.h"
#include "ipc.h"
#include "messages.h"
#include "trading_system.h"
#include "timeutil.h"

#define CONTROL_ORDER 	3 /* buy/sell order control message code */


extern STATE g_state;
extern double g_best_stock_price[MINUTES_IN_A_WEEK];
extern double g_ref_stock_price[MINUTES_IN_A_WEEK];

/*** callbacks and freeobj handles for form Trading_System ***/

void send_first_orders_button_callback(FL_OBJECT *ob, long data)
{
	char description[1000];
	char question[2000];
	
	sprintf(description, "B code -> send first orders");
	
	sprintf(question, "Are you shure that you want to send the order below?\n%s", description);
	if (fl_show_question (question, 0))
		publish_control_message(CONTROL_ORDER, description, g_state.now, TSCONTROL_NAME);
}

void send_second_orders_button_callback(FL_OBJECT *ob, long data)
{
	char description[1000];
	char question[2000];
	
	sprintf(description, "S code -> send second orders");
	
	sprintf(question, "Are you shure that you want to send the order below?\n%s", description);
	if (fl_show_question (question, 0))
		publish_control_message(CONTROL_ORDER, description, g_state.now, TSCONTROL_NAME);
}


/* Function responsable by the check up on the input value */
/*
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
	//state.order_price = atof(input_val);
	
	input_val = fl_get_input(order_buy_quantity_val);
	if (input_val == NULL)
	{
		fprintf(stderr, "Could not read input val from order_quantity_price_val in buy_button_callback()\n");
		return;
	}
	//state.quantity = atoi(input_val);
	//sprintf(description, "B %d %s %.2f", state.quantity, state.best_stock_name, state.order_price); // Ex: B 400 PETR4 43.10  ou B 400 PETR4 43.10
	
	sprintf(question, "Are you shure that you want to send the order below?\n%s", description);
	if(fl_show_question (question, 0))
		publish_control_message(CONTROL_ORDER, description, g_state.now, TSCONTROL_NAME);
}


void fast_buy_button_callback(FL_OBJECT *ob, long data)
{
	char description[1000];
	char question[2000];
	
	//state.order_price = price_samples[state.moving_average_current_minute];
	//state.quantity = (state.capital / state.order_price);
	//state.quantity = state.quantity - state.quantity % 100; // Lotes de 100 acoes
	//sprintf(description, "B %d %s %.2f", state.quantity, state.best_stock_name, state.order_price); // Ex: B 400 PETR4 43.10  ou B 400 PETR4 43.10
	
	sprintf(question, "Are you shure that you want to send the order below?\n%s", description);
	if(fl_show_question (question, 0))
		publish_control_message(CONTROL_ORDER, description, g_state.now, TSCONTROL_NAME);
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
	//state.order_price = atof(input_val);
	
	input_val = fl_get_input(order_sell_quantity_val);
	if (input_val == NULL)
	{
		fprintf(stderr, "Could not read input val from order_sell_quantity_val in sell_button_callback()\n");
		return;
	}
	//state.quantity = atoi(input_val);
	//sprintf(description, "S %d %s %.2f", state.quantity, state.best_stock_name, state.order_price); // Ex: B 400 PETR4 43.10  ou B 400 PETR4 43.10
	
	sprintf(question, "Are you shure that you want to send the order below?\n%s", description);
	if(fl_show_question (question, 0))
		publish_control_message(CONTROL_ORDER, description, g_state.now, TSCONTROL_NAME);
}


void fast_sell_button_callback(FL_OBJECT *ob, long data)
{
	char description[1000];
	char question[2000];
	
	//state.order_price = price_samples[state.moving_average_current_minute];
	//sprintf(description, "S %d %s %.2f", state.quantity, state.best_stock_name, state.order_price); // Ex: S 400 PETR4 43.10  ou S 400 PETR4 43.10
	
	sprintf(question, "Are you shure that you want to send the order below?\n%s", description);
	if(fl_show_question (question, 0))
		publish_control_message(CONTROL_ORDER, description, g_state.now, TSCONTROL_NAME);
}
*/



void plot_graph_button_callback(FL_OBJECT *ob, long data)
{
	plot_graph(TO_SCREEN, g_best_stock_price, g_ref_stock_price, NULL);
}
