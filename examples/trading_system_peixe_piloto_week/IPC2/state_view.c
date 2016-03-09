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
		fread(&(g_price_samples[g_moving_average_current_minute - MOVING_AVERAGE_NUM_PERIODS]), sizeof(float), MOVING_AVERAGE_NUM_PERIODS + 1, state_file);
	else
		fread(g_price_samples, sizeof(float), g_moving_average_current_minute + 1, state_file);

	fclose(state_file);

	return (state);
}



void
show_state(STATE state)
{
	printf("state = %s\n", g_state_string[state.state]);
	printf("stock = %s\n", state.best_stock_name);
	printf("price			R$%.2f\n", state.price);
	printf("moving_average_price  	R$%.2f\n", state.moving_average_price);
	switch(state.state)
	{
		case SEND_BUY_ORDER:
		case WAIT_BUY:
		case WAIT_BUY_CANCEL:
			printf("order_price	 		R$%.2f\n", state.order_price);
			
		case WAIT_MIN_TO_BUY:
			printf("maximum_buy_price  		R$%.2f\n", state.best_stock_maximum_buy_price);
			printf("previous_minimum_intraday_price R$%.2f\n", state.previous_minimum_intraday_price);
			printf("the system send a buy order if moving_average_price > previous_minimum_intraday_price*(1.0+TRADING_SYSTEM_BUY_SELL_DELTA_RETURN) = R$%.2f\n", 
			       state.previous_minimum_intraday_price * (1.0 + TRADING_SYSTEM_BUY_SELL_DELTA_RETURN));
			break;

		case SEND_SELL_ORDER:
		case WAIT_SELL:
		case WAIT_SELL_CANCEL:
			printf("order_price	 		R$%.2f\n", state.order_price);
			
		case WAIT_MAX_TO_SELL:
			printf("buy_price	  		R$%.2f\n", state.buy_price);
			printf("best_stock_minimum_sell_price	R$%.2f\n", state.best_stock_minimum_sell_price);
			printf("previous_maximum_intraday_price R$%.2f\n", state.previous_maximum_intraday_price);
			printf("predicted_return		%f%%\n", 100.0 * state.best_stock_predicted_return);
			printf("current possible return		%f%%\n", 100.0 * (state.price - state.buy_price) / state.buy_price);
			printf("the system send a sell order if moving_average_price < previous_maximum_intraday_price*(1.0-TRADING_SYSTEM_BUY_SELL_DELTA_RETURN) = R$%.2f\n", 
			       state.previous_maximum_intraday_price * (1.0 - TRADING_SYSTEM_BUY_SELL_DELTA_RETURN));
			break;

		case WAIT_BEGIN_WEEK:
			printf("buy_price	  		R$%.2f\n", state.buy_price);
			printf("sell_price			R$%.2f\n", state.sell_price);
			printf("return				%f%%\n", 100.0 * (state.sell_price - state.buy_price) / state.buy_price);
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
