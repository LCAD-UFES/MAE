#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#include <timeutil.h> 
#include <math.h>
#include "mae.h"
#include "../wnn_pred.h"
#ifdef	USE_IPC
#include "ipc.h"
#include "messages.h"
#endif
#include "trading_system.h"


STATE
get_state_file_content()
{
	FILE *state_file;
	STATE state;
	
	if ((state_file = fopen(STATE_FILE_NAME, "r")) == NULL)
		Erro("Could not open state file ", STATE_FILE_NAME, " in get_state_file_content()");
		
	fread(&state, sizeof(STATE), 1, state_file);

	fread(&g_target_day_index, 			sizeof(int), 1, state_file);
	fread(&g_trading_system_current_week, 		sizeof(int), 1, state_file);
	fread(&g_moving_average_first_day, 		sizeof(int), 1, state_file);
	fread(&g_moving_average_current_minute, 	sizeof(int), 1, state_file);
	if (g_moving_average_current_minute > MOVING_AVERAGE_NUM_PERIODS)
		fread(&(g_stock_price[g_moving_average_current_minute - MOVING_AVERAGE_NUM_PERIODS]), sizeof(double), MOVING_AVERAGE_NUM_PERIODS + 1, state_file);
	else
		fread(g_stock_price, sizeof(double), g_moving_average_current_minute + 1, state_file);

	fclose(state_file);

	return (state);
}



void
show_state(STATE state)
{
	printf("state = %s\n", g_state_string[state.state]);
	printf("stock = %s\n", state.best_stock_name);
	printf("price			R$%.2f\n", state.stock_price);
	printf("moving_average_of_delta  	R$%.2f\n", state.moving_average_of_delta);
	switch(state.state)
	{
		case SEND_FIRST_ORDERS:
		case WAIT_FIRST_ORDERS:
		case WAIT_FIRST_ORDERS_CANCEL:
			printf("order_price	 		R$%.2f\n", state.stock_order_price);
			
		case WAIT_FIRST_ORDERS_TRIGGER:
			printf("maximum_buy_price  		R$%.2f\n", state.best_stock_maximum_buy_price);
			printf("previous_minimum_delta		R$%.2f\n", state.previous_minimum_delta);
			printf("the system send a buy order if moving_average_of_delta > (previous_minimum_delta + TRADING_SYSTEM_BUY_SELL_DELTA_RETURN) = %.3f\n", 
			       state.previous_minimum_delta + TRADING_SYSTEM_BUY_SELL_DELTA_RETURN);
			break;

		case SEND_SECOND_ORDERS:
		case WAIT_SECOND_ORDERS:
		case WAIT_SECOND_ORDERS_CANCEL:
			printf("order_price	 		R$%.2f\n", state.stock_order_price);
			
		case WAIT_SECOND_ORDERS_TRIGGER:
			printf("buy_price	  		R$%.2f\n", state.stock_buy_price);
			printf("minimum_delta_for_second_order	R$%.2f\n", state.minimum_delta_for_second_order);
			printf("previous_maximum_delta		R$%.2f\n", state.previous_maximum_delta);
			printf("predicted_return		%f%%\n", 100.0 * state.best_stock_predicted_return);
			printf("current possible return		%f%%\n", 100.0 * (state.stock_price - state.stock_buy_price) / state.stock_buy_price);
			printf("the system send a sell order if moving_average_of_delta < previous_maximum_delta -TRADING_SYSTEM_BUY_SELL_DELTA_RETURN) = %.3f\n", 
			       state.previous_maximum_delta - TRADING_SYSTEM_BUY_SELL_DELTA_RETURN);
			break;

		case WAIT_BEGIN_WEEK:
			printf("buy_price	  		R$%.2f\n", state.stock_buy_price);
			printf("sell_price			R$%.2f\n", state.stock_sell_price);
			printf("return				%f%%\n", 100.0 * (state.stock_sell_price - state.stock_buy_price) / state.stock_buy_price);
			printf("predicted_return		%f%%\n", state.best_stock_predicted_return);
			break;
	}
}



int 
main ()
{
	STATE state;
	
	state = get_state_file_content();
	show_state(state);
	
	return (0);
}
