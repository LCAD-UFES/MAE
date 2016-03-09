#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#include <timeutil.h> 
#include <math.h>
#ifdef USE_IPC
#include "ipc.h"
#include "messages.h"
#endif
#include "trading_system.h"

#define NUM_MINUTES_FOR_ORDER_EXECUTION		5
#define NUM_MINUTES_FOR_ORDER_CANCELATION	5
#define	MINUTES_BEFORE_CLOSING_WEEK		10
#define	MINUTES_BEFORE_CLOSING_DAY		10
#define MINUTES_WAITING_MOVING_AVERAGE		(2*MOVING_AVERAGE_NUM_PERIODS)


FILE *g_log_file = NULL;
double INITIAL_CAPITAL = 0.0;


STATE
init_state()
{
	STATE state;
	
	state.state = INITIALIZE;
	state.capital = state.previous_capital = INITIAL_CAPITAL;
	strcpy(state.now, "");
	strcpy(state.best_stock_name, "");
	strcpy(state.ref_stock_name, "");
	state.previous_moving_average_current_minute = 0;
#ifdef	USE_IPC
	state.multi_order_execution_message_received = 0;
#endif
	return (state);
}



void
get_state_in_a_hot_start()
{
	FILE *state_file;

	if ((state_file = fopen(STATE_FILE_NAME, "r")) == NULL)
		Erro("Could not open state file ", STATE_FILE_NAME, " in get_state_in_a_hot_start()");

	fread(&g_trading_system_current_day_date_l, 	sizeof(long), 1, state_file);
	fread(&g_moving_average_current_minute, 	sizeof(int), 1, state_file);

	fread(g_best_stock_price, sizeof(double), g_moving_average_current_minute + 1, state_file);
	fread(g_ref_stock_price, sizeof(double), g_moving_average_current_minute + 1, state_file);
		
	fread(&g_state, sizeof(STATE), 1, state_file);

	fclose(state_file);

	g_log_file = fopen("log.txt", "a");
}



STATE
get_state()
{
	FILE *state_file;
	STATE state;
	long trading_system_current_day_date_l;
	int moving_average_current_minute;
	double best_stock_price[MINUTES_IN_A_WEEK];
	double ref_stock_price[MINUTES_IN_A_WEEK];
	
	if ((state_file = fopen(STATE_FILE_NAME, "r")) == NULL)
	{
		state = init_state();
	}
	else
	{
		fread(&trading_system_current_day_date_l, sizeof(long), 1, state_file);
		fread(&moving_average_current_minute, sizeof(int), 1, state_file);

		fread(best_stock_price, sizeof(double), moving_average_current_minute + 1, state_file);
		fread(ref_stock_price, sizeof(double), moving_average_current_minute + 1, state_file);
		
		fread(&state, sizeof(STATE), 1, state_file);

		fclose(state_file);
	}
	return (state);
}



void
save_current_state(STATE state)
{
	FILE *state_file;
	
	if ((state_file = fopen(STATE_FILE_NAME, "w")) == NULL)
		Erro("Could not open state_file named ", STATE_FILE_NAME, " in save_state()");

	fwrite(&g_trading_system_current_day_date_l, 	sizeof(long), 1, state_file);
	fwrite(&g_moving_average_current_minute, 	sizeof(int), 1, state_file);

	fwrite(g_best_stock_price, sizeof(double), g_moving_average_current_minute + 1, state_file);
	fwrite(g_ref_stock_price, sizeof(double), g_moving_average_current_minute + 1, state_file);

	fwrite(&state, sizeof(STATE), 1, state_file);
	
	fclose(state_file);
}



void
save_current_state_log(STATE state)
{
	char state_log_file_name[1014];
	time_t t;
	FILE *state_file;
	
	strcpy(state_log_file_name, "STATE_LOG/state.bin-");
	time(&t);
	strcat(state_log_file_name, ctime(&t));
	state_log_file_name[strlen(state_log_file_name) - 1] = '\0';

	if ((state_file = fopen(state_log_file_name, "w")) == NULL)
		Erro("Could not open state_file_log named ", state_log_file_name, " in save_current_state_log()");

	fwrite(&g_trading_system_current_day_date_l, 	sizeof(long), 1, state_file);
	fwrite(&g_moving_average_current_minute, 	sizeof(int), 1, state_file);

	fwrite(g_best_stock_price, sizeof(double), g_moving_average_current_minute + 1, state_file);
	fwrite(g_ref_stock_price, sizeof(double), g_moving_average_current_minute + 1, state_file);

	fwrite(&state, sizeof(STATE), 1, state_file);
	
	fclose(state_file);
}


void
save_state(STATE state)
{
	save_current_state(state);
	save_current_state_log(state);
}



char *
get_system_time()
{
	time_t now;
	char *str_now;
	
	now = time(NULL);
	str_now = ctime(&now);
	str_now[strlen(str_now) - 1] = '\0';
	
	return (str_now);
}



char *
get_trading_system_time(char *str_now, int moving_average_current_minute)
{
	char *day, *week_day;
	int hour, minute;
	
	day = long2isodate(g_trading_system_current_day_date_l);
	week_day = long2dows(g_trading_system_current_day_date_l);
	hour = (moving_average_current_minute / 60) % 7 + TRADING_OPENING_HOUR;
	minute = moving_average_current_minute % 60;
	sprintf(str_now, "%s %s %02d:%02d", day, week_day, hour, minute);
	
	return (str_now);
}



double
trade_cost(double capital)
{
	double cost;

#ifdef	MyCAP_TRADING_COSTS
	cost = 5.0; // Custo de compra (corretagem) MyCAP
	cost += cost * (2.0 / 100.0);// ISS sobre a corretagem
	cost += capital * (0.0285 / 100.0); // Emolumentos Bovespa
	cost += capital * (0.006 / 100.0); // Liquida��o
#else
	cost = 0.0; // Custo fixo de compra (corretagem) 
	cost += capital * (BROKERAGE); // Correntagem
	cost += capital * (0.0285 / 100.0); // Emolumentos Bovespa
	cost += capital * (0.006 / 100.0); // Liquida��o
#endif
	return (cost);
}



#ifdef	USE_IPC



void
save_multi_order_execution_message_log(qsor_multi_order_execution_message *received_multi_order_execution)
{
	char multi_order_execution_log_file_name[1014];
	time_t t;
	FILE *message_log_file;
	
	strcpy(multi_order_execution_log_file_name, "MESSAGE_LOG/multi_order_execution_message-");
	time(&t);
	strcat(multi_order_execution_log_file_name, ctime(&t));
	multi_order_execution_log_file_name[strlen(multi_order_execution_log_file_name) - 1] = '\0';

	if ((message_log_file = fopen(multi_order_execution_log_file_name, "w")) == NULL)
		Erro("Could not open multi_order_execution_log_file_name named ", multi_order_execution_log_file_name, " in save_multi_order_execution_message_log()");

	fprintf(message_log_file, "%d\n%s\n%lf\n%d\n", received_multi_order_execution->buy[0], received_multi_order_execution->stock_symbol[0], received_multi_order_execution->price[0], received_multi_order_execution->quantity[0]);
	fprintf(message_log_file, "%d\n%s\n%lf\n%d\n", received_multi_order_execution->buy[1], received_multi_order_execution->stock_symbol[1], received_multi_order_execution->price[1], received_multi_order_execution->quantity[1]);
	fprintf(message_log_file, "%s%s", received_multi_order_execution->date_and_time, received_multi_order_execution->host);
	
	fclose(message_log_file);
}



void
receive_multi_order_execution(qsor_multi_order_execution_message *received_multi_order_execution)
{
	save_multi_order_execution_message_log(received_multi_order_execution);
	
	if (received_multi_order_execution->num_orders != 2)
		Erro("num_orders != 2 in receive_multi_order_execution()", "", "");
		
	g_state.received_multi_order_execution.buy[0] = received_multi_order_execution->buy[0];
	g_state.received_multi_order_execution.buy[1] = received_multi_order_execution->buy[1];
	strcpy(g_state.received_multi_order_execution.stock_symbol[0], received_multi_order_execution->stock_symbol[0]);
	strcpy(g_state.received_multi_order_execution.stock_symbol[1], received_multi_order_execution->stock_symbol[1]);
	g_state.received_multi_order_execution.price[0] = received_multi_order_execution->price[0];
	g_state.received_multi_order_execution.price[1] = received_multi_order_execution->price[1];
	g_state.received_multi_order_execution.quantity[0] = received_multi_order_execution->quantity[0];
	g_state.received_multi_order_execution.quantity[1] = received_multi_order_execution->quantity[1];
	strcpy(g_state.received_multi_order_execution.date_and_time, received_multi_order_execution->date_and_time);
	strcpy(g_state.received_multi_order_execution.host, received_multi_order_execution->host);

	g_state.multi_order_execution_message_received = 1;
	save_state(g_state);
	
	trading_system_state_machine(g_moving_average_current_minute);
}



void
send_multi_order(STATE *state, int first_orders)
{
	ts_multi_order_message order;
	int buy[2];
	char *stock_symbol[2];
	double price[2];
	int quantity[2];
	
	order.num_orders = 2;
	if (first_orders == 1)
	{
		if (state->trade_order == BUY_SELL)
		{
			buy[0] = BUY;
			buy[1] = SELL;
		}
		else
		{
			buy[0] = SELL;
			buy[1] = BUY;
		}
	}
	else
	{
		if (state->trade_order == SELL_BUY)
		{
			buy[0] = BUY;
			buy[1] = SELL;
		}
		else
		{
			buy[0] = SELL;
			buy[1] = BUY;
		}
	}
	order.buy = buy;
	
	stock_symbol[0] = state->best_stock_name;
	stock_symbol[1] = state->ref_stock_name;
	order.stock_symbol = stock_symbol;
	
	price[0] = state->best_stock_order_price;
	price[1] = state->ref_stock_order_price;
	order.price = price;
	
	quantity[0] = state->best_stock_quantity;
	quantity[1] = state->ref_stock_quantity;
	order.quantity = quantity;
	
	order.date_and_time = state->now;
	order.host = TS_NAME;

	IPC_publishData(TS_MULTI_ORDER_MESSAGE_NAME, &order);
}



void
send_first_orders(STATE *state)
{
#ifndef	TEST_TRADING_SYSTEM_ON_LINE
	send_multi_order(state, 1);
#endif
}



void
send_second_orders(STATE *state)
{
#ifndef	TEST_TRADING_SYSTEM_ON_LINE
	send_multi_order(state, 0);
#endif
}



int
check_multi_order(STATE *state, int first_orders)
{
	double transaction_amount;
	
	if (state->multi_order_execution_message_received && 
	    ((state->received_multi_order_execution.buy[0] == -1) || (state->received_multi_order_execution.buy[1] == -1)))
	{
		state->multi_order_execution_message_received = 0;
		
		return (ORDER_CANCELED);
	}
	else if (state->multi_order_execution_message_received && 
		 ((state->received_multi_order_execution.buy[0] >= 0) || (state->received_multi_order_execution.buy[1] >= 0)))
	{
		state->multi_order_execution_message_received = 0;
		
		if (first_orders == 1)
		{
			switch (state->trade_order)
			{
				case BUY_SELL:
					if ((state->received_multi_order_execution.buy[0] != BUY) || (state->received_multi_order_execution.buy[1] != SELL))
						Erro("Unexpected order pair received (BUY_SELL instead of SELL_BUY or vice-versa) in check_multi_order()", "", "");

					state->best_stock_buy_price = state->received_multi_order_execution.price[0];
					state->best_stock_quantity = state->received_multi_order_execution.quantity[0];
					transaction_amount = (double) (state->best_stock_quantity) * state->best_stock_buy_price;
					state->capital = state->capital - transaction_amount - trade_cost(transaction_amount);

					state->ref_stock_sell_price = state->received_multi_order_execution.price[1];
					state->ref_stock_quantity = state->received_multi_order_execution.quantity[1];
					transaction_amount = (double) (state->ref_stock_quantity) * state->ref_stock_sell_price;
					state->capital = state->capital + transaction_amount - trade_cost(transaction_amount);	
					break;

				case SELL_BUY:
					if ((state->received_multi_order_execution.buy[0] != SELL) || (state->received_multi_order_execution.buy[1] != BUY))
						Erro("Unexpected order pair received (BUY_SELL instead of SELL_BUY or vice-versa) in check_multi_order()", "", "");

					state->best_stock_sell_price = state->received_multi_order_execution.price[0];
					state->best_stock_quantity = state->received_multi_order_execution.quantity[0];
					transaction_amount = (double) (state->best_stock_quantity) * state->best_stock_sell_price;
					state->capital = state->capital + transaction_amount - trade_cost(transaction_amount);

					state->ref_stock_buy_price = state->received_multi_order_execution.price[1];
					state->ref_stock_quantity = state->received_multi_order_execution.quantity[1];
					transaction_amount = (double) (state->ref_stock_quantity) * state->ref_stock_buy_price;
					state->capital = state->capital - transaction_amount - trade_cost(transaction_amount);	
					break;
			}
		}
		else
		{
			switch (state->trade_order)
			{
				case BUY_SELL:
					if ((state->received_multi_order_execution.buy[0] != SELL) || (state->received_multi_order_execution.buy[1] != BUY))
						Erro("Unexpected order pair received (BUY_SELL instead of SELL_BUY or vice-versa) in check_multi_order()", "", "");

					state->best_stock_sell_price = state->received_multi_order_execution.price[0];
					state->best_stock_quantity = state->received_multi_order_execution.quantity[0];
					transaction_amount = (double) (state->best_stock_quantity) * state->best_stock_sell_price;
					state->capital = state->capital + transaction_amount - trade_cost(transaction_amount);

					state->ref_stock_buy_price = state->received_multi_order_execution.price[1];
					state->ref_stock_quantity = state->received_multi_order_execution.quantity[1];
					transaction_amount = (double) (state->ref_stock_quantity) * state->ref_stock_buy_price;
					state->capital = state->capital - transaction_amount - trade_cost(transaction_amount);	
					break;

				case SELL_BUY:
					if ((state->received_multi_order_execution.buy[0] != BUY) || (state->received_multi_order_execution.buy[1] != SELL))
						Erro("Unexpected order pair received (BUY_SELL instead of SELL_BUY or vice-versa) in check_multi_order()", "", "");

					state->best_stock_buy_price = state->received_multi_order_execution.price[0];
					state->best_stock_quantity = state->received_multi_order_execution.quantity[0];
					transaction_amount = (double) (state->best_stock_quantity) * state->best_stock_buy_price;
					state->capital = state->capital - transaction_amount - trade_cost(transaction_amount);

					state->ref_stock_sell_price = state->received_multi_order_execution.price[1];
					state->ref_stock_quantity = state->received_multi_order_execution.quantity[1];
					transaction_amount = (double) (state->ref_stock_quantity) * state->ref_stock_sell_price;
					state->capital = state->capital + transaction_amount - trade_cost(transaction_amount);	
					break;
			}
		}
		
		return (ORDER_EXECUTED);
	}
	else
	{
		return (ORDER_NOT_EXECUTED);
	}
}



int
check_first_orders(STATE *state)
{
	return (check_multi_order(state, 1));
}


int
check_second_orders(STATE *state)
{
	return (check_multi_order(state, 0));
}


void
send_state(STATE *state)
{
	ts_state_message state_message;

	state_message.state_size = sizeof(STATE);
	state_message.price_samples_size = MINUTES_IN_A_WEEK;
	state_message.state = (void *) state;
	state_message.best_stock_price = g_best_stock_price;
	state_message.ref_stock_price = g_ref_stock_price;
	state_message.date_and_time = state->now;
	state_message.host = TS_NAME;

	IPC_publishData(TS_STATE_MESSAGE_NAME, &state_message);	
}


#else


void
send_first_orders(STATE *state)
{
}



void
send_second_orders(STATE *state)
{
}


int
check_first_orders(STATE *state)
{
	double transaction_amount;
	
	switch (state->trade_order)
	{
		case BUY_SELL:
			state->best_stock_buy_price = g_best_stock_price[g_moving_average_current_minute];
			transaction_amount = (double) (state->best_stock_quantity) * state->best_stock_buy_price;
			state->capital = state->capital - transaction_amount - trade_cost(transaction_amount);

			state->ref_stock_sell_price = g_ref_stock_price[g_moving_average_current_minute];
			transaction_amount = (double) (state->ref_stock_quantity) * state->ref_stock_sell_price;
			state->capital = state->capital + transaction_amount - trade_cost(transaction_amount);	
			break;
			
		case SELL_BUY:
			state->best_stock_sell_price = g_best_stock_price[g_moving_average_current_minute];
			transaction_amount = (double) (state->best_stock_quantity) * state->best_stock_sell_price;
			state->capital = state->capital + transaction_amount - trade_cost(transaction_amount);

			state->ref_stock_buy_price = g_ref_stock_price[g_moving_average_current_minute];
			transaction_amount = (double) (state->ref_stock_quantity) * state->ref_stock_buy_price;
			state->capital = state->capital - transaction_amount - trade_cost(transaction_amount);	
			break;
	}

	return (ORDER_EXECUTED);
}


int
check_second_orders(STATE *state)
{
	double transaction_amount;
	
	switch (state->trade_order)
	{
		case BUY_SELL:
			state->best_stock_sell_price = g_best_stock_price[g_moving_average_current_minute];
			transaction_amount = (double) (state->best_stock_quantity) * state->best_stock_sell_price;
			state->capital = state->capital + transaction_amount - trade_cost(transaction_amount);

			state->ref_stock_buy_price = g_ref_stock_price[g_moving_average_current_minute];
			transaction_amount = (double) (state->ref_stock_quantity) * state->ref_stock_buy_price;
			state->capital = state->capital - transaction_amount - trade_cost(transaction_amount);	
			break;
			
		case SELL_BUY:
			state->best_stock_buy_price = g_best_stock_price[g_moving_average_current_minute];
			transaction_amount = (double) (state->best_stock_quantity) * state->best_stock_buy_price;
			state->capital = state->capital - transaction_amount - trade_cost(transaction_amount);

			state->ref_stock_sell_price = g_ref_stock_price[g_moving_average_current_minute];
			transaction_amount = (double) (state->ref_stock_quantity) * state->ref_stock_sell_price;
			state->capital = state->capital + transaction_amount - trade_cost(transaction_amount);	
			break;
	}

	return (ORDER_EXECUTED);
}

#endif



int
trigger_first_orders(STATE *state)
{
	int trigger = 0;

	if ((LEVERAGE * ((state->moving_average_of_delta - state->previous_minimum_delta) / 2.0)) > TRADING_SYSTEM_BUY_SELL_DELTA_RETURN)
	{	// Achou a primeira subida depois de um minimo local
		trigger = 1;
	}

	if (state->moving_average_of_delta < state->previous_minimum_delta) // search for minimum
		state->previous_minimum_delta = state->moving_average_of_delta;
	if (state->moving_average_of_delta > state->previous_maximum_delta) // search for minimum
		state->previous_maximum_delta = state->moving_average_of_delta;

	return (trigger);
}



int
trigger_second_orders(STATE *state, int moving_average_current_minute)
{
	int trigger = 0;
	double current_delta;
	//int last_week_day;

	switch (state->trade_order)
	{
		case BUY_SELL:
			current_delta = (g_best_stock_price[moving_average_current_minute] / state->best_stock_base_price) - (g_ref_stock_price[moving_average_current_minute] / state->ref_stock_base_price);
			break;
			
		case SELL_BUY:
			current_delta = (g_ref_stock_price[moving_average_current_minute] / state->ref_stock_base_price) - (g_best_stock_price[moving_average_current_minute] / state->best_stock_base_price);
			break;
	}

/*	last_week_day = 0;//((moving_average_current_minute - (MINUTES_IN_A_WEEK - MINUTES_IN_A_DAY)) > 0)? 1: 0;
	if ((state->previous_maximum_delta - state->moving_average_of_delta) > TRADING_SYSTEM_BUY_SELL_DELTA_RETURN)
	{	// Achou a primeira subida depois de um minimo local
		if (last_week_day || ((state->moving_average_of_delta > TRADING_SYSTEM_STOP_GAIN) && (current_delta > TRADING_SYSTEM_STOP_GAIN)))
			trigger = 1;
	}
*/
	if ((state->moving_average_of_delta < slow_delta_moving_average(state, g_best_stock_price, g_ref_stock_price, moving_average_current_minute)) && 
	    ((LEVERAGE * (current_delta / 2.0)) > TRADING_SYSTEM_STOP_GAIN) &&
	    ((LEVERAGE * (state->moving_average_of_delta / 2.0)) > TRADING_SYSTEM_STOP_GAIN))
	{
		trigger = 1;
	}

	if (state->moving_average_of_delta < state->previous_minimum_delta) // search for minimum
		state->previous_minimum_delta = state->moving_average_of_delta;
	if (state->moving_average_of_delta > state->previous_maximum_delta) // search for minimum
		state->previous_maximum_delta = state->moving_average_of_delta;

	return (trigger);
}



int
trigger_stop_loss(STATE *state, int moving_average_current_minute)
{
	int trigger = 0;
	double current_delta;

	switch (state->trade_order)
	{
		case BUY_SELL:
			current_delta = (g_best_stock_price[moving_average_current_minute] / state->best_stock_base_price) - (g_ref_stock_price[moving_average_current_minute] / state->ref_stock_base_price);
			break;
			
		case SELL_BUY:
			current_delta = (g_ref_stock_price[moving_average_current_minute] / state->ref_stock_base_price) - (g_best_stock_price[moving_average_current_minute] / state->best_stock_base_price);
			break;
	}

	if ((LEVERAGE * (current_delta / 2.0)) < TRADING_SYSTEM_STOP_LOSS)
	{
		trigger = 1;
	}

	return (trigger);
}


void
initialize_state_machine()
{
	printf("Initial state = %s\n", g_state_string[g_state.state]);
	fflush(stdout);

	g_log_file = fopen("log.txt", "a");
	fprintf(g_log_file, "%s - INITIALIZE\n", get_trading_system_time(g_state.now, g_moving_average_current_minute)); fflush(g_log_file);
}



int
trading_system_state_machine(int moving_average_current_minute)
{
	STATE state;

	state = g_state;

	get_trading_system_time(state.now, moving_average_current_minute);
	state.moving_average_current_minute = moving_average_current_minute;
	
	if (state.state == INITIALIZE)
	{
		initialize_state_machine();
		state.state = WAIT_MOVING_AVERAGE;
	}
	
	if (state.state == WAIT_MOVING_AVERAGE)
	{
		state.best_stock_base_price = g_best_stock_price[0];
		state.ref_stock_base_price = g_ref_stock_price[0];
		state.best_stock_price = g_best_stock_price[moving_average_current_minute];
		state.ref_stock_price = g_ref_stock_price[moving_average_current_minute];
		//state.moving_average_of_delta = 0.0;
		state.moving_average_of_delta = delta_moving_average(&state, g_best_stock_price, g_ref_stock_price, moving_average_current_minute);
		
		if (moving_average_current_minute >= MINUTES_WAITING_MOVING_AVERAGE)
		{
			state.best_stock_base_price = g_best_stock_price[moving_average_current_minute];
			state.ref_stock_base_price = g_ref_stock_price[moving_average_current_minute];

			state.previous_maximum_delta = state.previous_minimum_delta = delta_moving_average(&state, g_best_stock_price, g_ref_stock_price, moving_average_current_minute);

			fprintf(g_log_file, "%s - WAIT_MOVING_AVERAGE:\t\t\tcurrent quote = %d\n", get_trading_system_time(state.now, moving_average_current_minute), moving_average_current_minute); fflush(g_log_file);

			state.state = WAIT_FIRST_ORDERS_TRIGGER;
		}
	}

	if (state.state == WAIT_FIRST_ORDERS_TRIGGER)
	{
		state.best_stock_price = g_best_stock_price[moving_average_current_minute];
		state.ref_stock_price = g_ref_stock_price[moving_average_current_minute];
		state.moving_average_of_delta = delta_moving_average(&state, g_best_stock_price, g_ref_stock_price, moving_average_current_minute);
		
		if (moving_average_current_minute > 2 * MINUTES_IN_A_DAY) // Time out para a compra
		{
			state.first_orders_day_date_l = state.second_orders_day_date_l = -1;
			
			fprintf(g_log_file, "%s - WAIT_FIRST_ORDERS_TRIGGER:\t\t\ttime out to first orders - skip week!\n", get_trading_system_time(state.now, moving_average_current_minute)); fflush(g_log_file);
			
			state.state = WAIT_BEGIN_WEEK;
		}
		
		if (trigger_first_orders(&state))
		{
			fprintf(g_log_file, "%s - WAIT_FIRST_ORDERS_TRIGGER: min. found,\t%s = R$%.2lf, %s = R$%.2lf, moving aver. delta = %.3f\n", 
				get_trading_system_time(state.now, moving_average_current_minute), 
				state.best_stock_name, state.best_stock_price, 
				state.ref_stock_name, state.ref_stock_price, 
				state.moving_average_of_delta); 
				fflush(g_log_file);

			state.state = SEND_FIRST_ORDERS;
		}
	}

	if (state.state == SEND_FIRST_ORDERS)
	{
		state.previous_capital = state.capital;

		state.best_stock_order_price = state.best_stock_price = g_best_stock_price[moving_average_current_minute];
		state.best_stock_quantity = (LEVERAGE * (state.capital / 2.0)) / state.best_stock_price;
		state.best_stock_quantity = state.best_stock_quantity - state.best_stock_quantity % 100; // Lotes de 100 acoes

		state.ref_stock_order_price = state.ref_stock_price = g_ref_stock_price[moving_average_current_minute];
		state.ref_stock_quantity = (LEVERAGE * (state.capital / 2.0)) / state.ref_stock_price;
		state.ref_stock_quantity = state.ref_stock_quantity - state.ref_stock_quantity % 100; // Lotes de 100 acoes

		send_first_orders(&state);

		state.first_orders_execution_minute = moving_average_current_minute % MINUTES_IN_A_DAY;
			
		fprintf(g_log_file, "%s - SEND_FIRST_ORDERS: \t\t\t%s %s @ R$%.2lf, quant. = %d,  %s %s @ R$%.2lf, quant. = %d\n", 
			get_trading_system_time(state.now, moving_average_current_minute), 
			((state.trade_order == BUY_SELL)? "buy": "sell"), state.best_stock_name, state.best_stock_order_price, state.best_stock_quantity,
			((state.trade_order == BUY_SELL)? "sell": "buy"), state.ref_stock_name, state.ref_stock_order_price, state.ref_stock_quantity);
			fflush(g_log_file);

		state.state = WAIT_FIRST_ORDERS;
	}
	
	if (state.state == WAIT_FIRST_ORDERS)
	{
		state.order_result = check_first_orders(&state);
		state.day_current_minute = moving_average_current_minute % MINUTES_IN_A_DAY;
		
		state.best_stock_price = g_best_stock_price[moving_average_current_minute];
		state.ref_stock_price = g_ref_stock_price[moving_average_current_minute];
		if (state.order_result == ORDER_EXECUTED)
		{
			state.best_stock_base_price = (state.trade_order == BUY_SELL)? state.best_stock_buy_price: state.best_stock_sell_price;
			state.ref_stock_base_price = (state.trade_order == BUY_SELL)? state.ref_stock_sell_price: state.ref_stock_buy_price;
			
			state.first_orders_day_date_l = g_trading_system_current_day_date_l;
			state.first_orders_minute = moving_average_current_minute;
			
			state.previous_maximum_delta = delta_moving_average(&state, g_best_stock_price, g_ref_stock_price, moving_average_current_minute);

			fprintf(g_log_file, "%s - WAIT_FIRST_ORDERS: first orders exec.,\t%s %s @ R$%.2lf, order @ R$%.2lf, trade price = R$%.2lf, quant. = %d\n", 
				get_trading_system_time(state.now, moving_average_current_minute), 
				((state.trade_order == BUY_SELL)? "buy": "sell"), state.best_stock_name, 
				state.best_stock_price, state.best_stock_order_price, 
				((state.trade_order == BUY_SELL)? state.best_stock_buy_price: state.best_stock_sell_price), 
				state.best_stock_quantity); 
				fflush(g_log_file);
			fprintf(g_log_file, "%s - WAIT_FIRST_ORDERS: first orders exec.,\t%s %s @ R$%.2lf, order @ R$%.2lf, trade price = R$%.2lf, quant. = %d\n", 
				get_trading_system_time(state.now, moving_average_current_minute), 
				((state.trade_order == BUY_SELL)? "sell": "buy"), state.ref_stock_name, 
				state.ref_stock_price, state.ref_stock_order_price, 
				((state.trade_order == BUY_SELL)? state.ref_stock_sell_price: state.ref_stock_buy_price),  
				state.ref_stock_quantity); 
				fflush(g_log_file);
	
			state.state = WAIT_SECOND_ORDERS_TRIGGER;
		}
		else if (state.order_result == ORDER_CANCELED)
		{
			fprintf(g_log_file, "%s - WAIT_FIRST_ORDERS: first orders CANCELED!\t%s %s, %s %s\n", 
				get_trading_system_time(state.now, moving_average_current_minute), 
				((state.trade_order == BUY_SELL)? "buy": "sell"), state.best_stock_name,
				((state.trade_order == BUY_SELL)? "sell": "buy"), state.ref_stock_name); 
				fflush(g_log_file);

			state.state = WAIT_FIRST_ORDERS_TRIGGER;			
		}
		else if (((state.day_current_minute - state.first_orders_execution_minute) > NUM_MINUTES_FOR_ORDER_EXECUTION) || ((state.day_current_minute - state.first_orders_execution_minute) < 0))
		{
			fprintf(g_log_file, "%s - WAIT_FIRST_ORDERS: first orders time out!\t%s %s, %s %s\n", 
				get_trading_system_time(state.now, moving_average_current_minute), 
				((state.trade_order == BUY_SELL)? "buy": "sell"), state.best_stock_name,
				((state.trade_order == BUY_SELL)? "sell": "buy"), state.ref_stock_name); 
				fflush(g_log_file);

			state.first_orders_cancel_minute = moving_average_current_minute % MINUTES_IN_A_DAY;
			
			state.state = WAIT_FIRST_ORDERS_CANCEL;			
		}
		else 
		{
			fprintf(g_log_file, "%s - WAIT_FIRST_ORDERS: wait first orders,\t%s %s @ R$%.2lf, order @ R$%.2lf, quant. = %d\n", 
				get_trading_system_time(state.now, moving_average_current_minute), 
				((state.trade_order == BUY_SELL)? "buy": "sell"), state.best_stock_name, 
				state.best_stock_price, state.best_stock_order_price, state.best_stock_quantity); 
				fflush(g_log_file);
			fprintf(g_log_file, "%s - WAIT_FIRST_ORDERS: wait first orders,\t%s %s @ R$%.2lf, order @ R$%.2lf, quant. = %d\n", 
				get_trading_system_time(state.now, moving_average_current_minute), 
				((state.trade_order == BUY_SELL)? "sell": "buy"), state.ref_stock_name, 
				state.ref_stock_price, state.ref_stock_order_price, state.ref_stock_quantity); 
				fflush(g_log_file);
		}
	}
	
	if (state.state == WAIT_FIRST_ORDERS_CANCEL)
	{
		state.order_result = check_first_orders(&state);
		state.day_current_minute = moving_average_current_minute % MINUTES_IN_A_DAY;
		
		state.best_stock_price = g_best_stock_price[moving_average_current_minute];
		state.ref_stock_price = g_ref_stock_price[moving_average_current_minute];
		if (state.order_result == ORDER_EXECUTED)
		{
			state.best_stock_base_price = (state.trade_order == BUY_SELL)? state.best_stock_buy_price: state.best_stock_sell_price;
			state.ref_stock_base_price = (state.trade_order == BUY_SELL)? state.ref_stock_sell_price: state.ref_stock_buy_price;
			
			state.first_orders_day_date_l = g_trading_system_current_day_date_l;
			state.first_orders_minute = moving_average_current_minute;
			
			state.previous_maximum_delta = delta_moving_average(&state, g_best_stock_price, g_ref_stock_price, moving_average_current_minute);

			fprintf(g_log_file, "%s - WAIT_FIRST_ORDERS_CANCEL: first orders exec.,\t%s %s @ R$%.2lf, order @ R$%.2lf, trade price = R$%.2lf, quant. = %d\n", 
				get_trading_system_time(state.now, moving_average_current_minute), 
				((state.trade_order == BUY_SELL)? "buy": "sell"), state.best_stock_name, 
				state.best_stock_price, state.best_stock_order_price, 
				((state.trade_order == BUY_SELL)? state.best_stock_buy_price: state.best_stock_sell_price), 
				state.best_stock_quantity); 
				fflush(g_log_file);
			fprintf(g_log_file, "%s - WAIT_FIRST_ORDERS_CANCEL: first orders exec.,\t%s %s @ R$%.2lf, order @ R$%.2lf, trade price = R$%.2lf, quant. = %d\n", 
				get_trading_system_time(state.now, moving_average_current_minute), 
				((state.trade_order == BUY_SELL)? "sell": "buy"), state.ref_stock_name, 
				state.ref_stock_price, state.ref_stock_order_price, 
				((state.trade_order == BUY_SELL)? state.ref_stock_sell_price: state.ref_stock_buy_price),  
				state.ref_stock_quantity); 
				fflush(g_log_file);

			state.state = WAIT_SECOND_ORDERS_TRIGGER;
		}
		else if (state.order_result == ORDER_CANCELED)
		{
			fprintf(g_log_file, "%s - WAIT_FIRST_ORDERS_CANCEL: first orders CANCELED!\n", get_trading_system_time(state.now, moving_average_current_minute)); fflush(g_log_file);

			state.state = WAIT_FIRST_ORDERS_TRIGGER;			
		}
		else if (((state.day_current_minute - state.first_orders_cancel_minute) > NUM_MINUTES_FOR_ORDER_CANCELATION) || ((state.day_current_minute - state.first_orders_cancel_minute) < 0))
		{
			fprintf(g_log_file, "%s - WAIT_FIRST_ORDERS_CANCEL: first orders cancel time out! USER INTERVENTION REQUIRED!\n", get_trading_system_time(state.now, moving_average_current_minute)); fflush(g_log_file);

			state.state = QUIT;
		}
		else 
		{
			fprintf(g_log_file, "%s - WAIT_FIRST_ORDERS_CANCEL: wait cancelation,\t%s %s @ R$%.2lf, order @ R$%.2lf, quant. = %d\n", 
				get_trading_system_time(state.now, moving_average_current_minute), 
				((state.trade_order == BUY_SELL)? "buy": "sell"), state.best_stock_name, 
				state.best_stock_price, state.best_stock_order_price, state.best_stock_quantity); 
				fflush(g_log_file);
			fprintf(g_log_file, "%s - WAIT_FIRST_ORDERS_CANCEL: wait cancelation,\t%s %s @ R$%.2lf, order @ R$%.2lf, quant. = %d\n", 
				get_trading_system_time(state.now, moving_average_current_minute), 
				((state.trade_order == BUY_SELL)? "sell": "buy"), state.ref_stock_name, 
				state.ref_stock_price, state.ref_stock_order_price, state.ref_stock_quantity); 
				fflush(g_log_file);
		}
	}
	
	if (state.state == WAIT_SECOND_ORDERS_TRIGGER)
	{
		state.best_stock_price = g_best_stock_price[moving_average_current_minute];
		state.ref_stock_price = g_ref_stock_price[moving_average_current_minute];
		state.moving_average_of_delta = delta_moving_average(&state, g_best_stock_price, g_ref_stock_price, moving_average_current_minute);

		if (moving_average_current_minute >= (MINUTES_IN_A_WEEK - MINUTES_BEFORE_CLOSING_WEEK)) // Time out para a venda @@@ tem que colocar antes quando on-line
		{
			if (moving_average_current_minute > (MINUTES_IN_A_WEEK - 1)) // pode ocorrer de nao haver negocios na quinta
				g_moving_average_current_minute = moving_average_current_minute = MINUTES_IN_A_WEEK - 2;
				
			fprintf(g_log_file, "%s - WAIT_SECOND_ORDERS_TRIGGER:\t\ttime out to sell!\n", get_trading_system_time(state.now, moving_average_current_minute)); fflush(g_log_file);
			
			state.state = SEND_SECOND_ORDERS;
		}
		
		if (trigger_stop_loss(&state, moving_average_current_minute))
		{
			fprintf(g_log_file, "%s - WAIT_SECOND_ORDERS_TRIGGER:\t\tstop loss!\n", get_trading_system_time(state.now, moving_average_current_minute)); fflush(g_log_file);

			state.state = SEND_SECOND_ORDERS;
		}

		if (trigger_second_orders(&state, moving_average_current_minute))
		{
			fprintf(g_log_file, "%s - WAIT_SECOND_ORDERS_TRIGGER: max. found,\t%s = R$%.2lf, %s = R$%.2lf, moving aver. delta = %.3f\n", 
				get_trading_system_time(state.now, moving_average_current_minute), 
				state.best_stock_name, state.best_stock_price, 
				state.ref_stock_name, state.ref_stock_price, 
				state.moving_average_of_delta); 
				fflush(g_log_file);

			state.state = SEND_SECOND_ORDERS;
		}
	}

	if (state.state == SEND_SECOND_ORDERS)
	{
		state.best_stock_order_price = state.best_stock_price = g_best_stock_price[moving_average_current_minute];
		state.ref_stock_order_price = state.ref_stock_price = g_ref_stock_price[moving_average_current_minute];
			
		send_second_orders(&state);

		state.second_orders_execution_minute = moving_average_current_minute % MINUTES_IN_A_DAY;
			
		fprintf(g_log_file, "%s - SEND_SECOND_ORDERS: \t\t\t%s %s @ R$%.2lf, quant. = %d,  %s %s @ R$%.2lf, quant. = %d\n", 
			get_trading_system_time(state.now, moving_average_current_minute), 
			((state.trade_order == BUY_SELL)? "sell": "buy"), state.best_stock_name, state.best_stock_order_price, state.best_stock_quantity,
			((state.trade_order == BUY_SELL)? "buy": "sell"), state.ref_stock_name, state.ref_stock_order_price, state.ref_stock_quantity);
			fflush(g_log_file);

		state.state = WAIT_SECOND_ORDERS;
	}
	
	if (state.state == WAIT_SECOND_ORDERS)
	{
		state.order_result = check_second_orders(&state);
		state.day_current_minute = moving_average_current_minute % MINUTES_IN_A_DAY;

		state.best_stock_price = g_best_stock_price[moving_average_current_minute];
		state.ref_stock_price = g_ref_stock_price[moving_average_current_minute];
		if (state.order_result == ORDER_EXECUTED)
		{
			state.second_orders_day_date_l = g_trading_system_current_day_date_l;
			state.second_orders_minute = moving_average_current_minute;
	
			fprintf(g_log_file, "%s - WAIT_SECOND_ORDERS: second orders exec.,\t%s %s @ R$%.2lf, order @ R$%.2lf, trade price = R$%.2lf, quant. = %d\n", 
				get_trading_system_time(state.now, moving_average_current_minute), 
				((state.trade_order == BUY_SELL)? "sell": "buy"), state.best_stock_name, 
				state.best_stock_price, state.best_stock_order_price, 
				((state.trade_order == BUY_SELL)? state.best_stock_sell_price: state.best_stock_buy_price), 
				state.best_stock_quantity); 
				fflush(g_log_file);
			fprintf(g_log_file, "%s - WAIT_SECOND_ORDERS: second orders exec.,\t%s %s @ R$%.2lf, order @ R$%.2lf, trade price = R$%.2lf, quant. = %d\n", 
				get_trading_system_time(state.now, moving_average_current_minute), 
				((state.trade_order == BUY_SELL)? "buy": "sell"), state.ref_stock_name, 
				state.ref_stock_price, state.ref_stock_order_price, 
				((state.trade_order == BUY_SELL)? state.ref_stock_buy_price: state.ref_stock_sell_price),  
				state.ref_stock_quantity); 
				fflush(g_log_file);
			fprintf(g_log_file, "%s - WAIT_SECOND_ORDERS: current capital, \tR$%.2lf\n", 
				get_trading_system_time(state.now, moving_average_current_minute), 
				state.capital); 
				fflush(g_log_file);

			fprintf(g_log_file, "%s - %s %s, ref. %s,  current capital: R$%.2lf,  week return: %+0.2lf%%,  accumulated return: %+0.2lf%%\n", 
				get_trading_system_time(state.now, moving_average_current_minute), 
				state.best_stock_name, ((state.trade_order == BUY_SELL)? "buy ": "sell"), 
				state.ref_stock_name, 
				state.capital, 
				100.0 * (state.capital - state.previous_capital) / state.previous_capital, 
				100.0 * (state.capital - INITIAL_CAPITAL) / INITIAL_CAPITAL);
			printf("# %s - %s %s, ref. %s,  current capital: R$%.2lf,  week return: %+0.2lf%%,  accumulated return: %+0.2lf%%\n", 
				get_trading_system_time(state.now, moving_average_current_minute), 
				state.best_stock_name, ((state.trade_order == BUY_SELL)? "buy ": "sell"), 
				state.ref_stock_name, 
				state.capital, 
				100.0 * (state.capital - state.previous_capital) / state.previous_capital, 
				100.0 * (state.capital - INITIAL_CAPITAL) / INITIAL_CAPITAL);
/*			printf("%s - %s %s, ref. %s,  current capital: R$%.2lf,  week return: %+0.2lf%%,  accumulated return: %+0.2lf%%,  pr: %+0.2lf%%\n", 
				get_trading_system_time(state.now, moving_average_current_minute), 
				state.best_stock_name, ((state.trade_order == BUY_SELL)? "buy ": "sell"), 
				state.ref_stock_name, 
				state.capital, 
				100.0 * (state.capital - state.previous_capital) / state.previous_capital, 
				100.0 * (state.capital - INITIAL_CAPITAL) / INITIAL_CAPITAL,
				100.0 * state.predicted_return);
*/			state.state = WAIT_BEGIN_WEEK;
		}
		else if (state.order_result == ORDER_CANCELED)
		{
			fprintf(g_log_file, "%s - WAIT_SECOND_ORDERS: sell CANCELED!\n", get_trading_system_time(state.now, moving_average_current_minute)); fflush(g_log_file);

			state.state = WAIT_SECOND_ORDERS_TRIGGER;
		}
		else if (((state.day_current_minute - state.second_orders_execution_minute) > NUM_MINUTES_FOR_ORDER_EXECUTION) || ((state.day_current_minute - state.second_orders_execution_minute) < 0))
		{
			fprintf(g_log_file, "%s - WAIT_SECOND_ORDERS: sell time out!\n", get_trading_system_time(state.now, moving_average_current_minute)); fflush(g_log_file);

			state.second_orders_cancel_minute = moving_average_current_minute % MINUTES_IN_A_DAY;
			
			state.state = WAIT_SECOND_ORDERS_CANCEL;			
		}
		else
		{
			fprintf(g_log_file, "%s - WAIT_SECOND_ORDERS: wait second orders,\t%s %s @ R$%.2lf, order @ R$%.2lf, quant. = %d\n", 
				get_trading_system_time(state.now, moving_average_current_minute), 
				((state.trade_order == BUY_SELL)? "sell": "buy"), state.best_stock_name, 
				state.best_stock_price, state.best_stock_order_price, state.best_stock_quantity); 
				fflush(g_log_file);
			fprintf(g_log_file, "%s - WAIT_SECOND_ORDERS: wait second orders,\t%s %s @ R$%.2lf, order @ R$%.2lf, quant. = %d\n", 
				get_trading_system_time(state.now, moving_average_current_minute), 
				((state.trade_order == BUY_SELL)? "buy": "sell"), state.ref_stock_name, 
				state.ref_stock_price, state.ref_stock_order_price, state.ref_stock_quantity); 
				fflush(g_log_file);
		}
	}
		
	if (state.state == WAIT_SECOND_ORDERS_CANCEL)
	{
		state.order_result = check_second_orders(&state);
		state.day_current_minute = moving_average_current_minute % MINUTES_IN_A_DAY;

		state.best_stock_price = g_best_stock_price[moving_average_current_minute];
		state.ref_stock_price = g_ref_stock_price[moving_average_current_minute];
		if (state.order_result == ORDER_EXECUTED)
		{
			state.second_orders_day_date_l = g_trading_system_current_day_date_l;
			state.second_orders_minute = moving_average_current_minute;
	
			fprintf(g_log_file, "%s - WAIT_SECOND_ORDERS_CANCEL: second orders exec.,\t%s %s @ R$%.2lf, order @ R$%.2lf, trade price = R$%.2lf, quant. = %d\n", 
				get_trading_system_time(state.now, moving_average_current_minute), 
				((state.trade_order == BUY_SELL)? "sell": "buy"), state.best_stock_name, 
				state.best_stock_price, state.best_stock_order_price, 
				((state.trade_order == BUY_SELL)? state.best_stock_sell_price: state.best_stock_buy_price), 
				state.best_stock_quantity); 
				fflush(g_log_file);
			fprintf(g_log_file, "%s - WAIT_SECOND_ORDERS_CANCEL: second orders exec.,\t%s %s @ R$%.2lf, order @ R$%.2lf, trade price = R$%.2lf, quant. = %d\n", 
				get_trading_system_time(state.now, moving_average_current_minute), 
				((state.trade_order == BUY_SELL)? "buy": "sell"), state.ref_stock_name, 
				state.ref_stock_price, state.ref_stock_order_price, 
				((state.trade_order == BUY_SELL)? state.ref_stock_buy_price: state.ref_stock_sell_price),  
				state.ref_stock_quantity); 
				fflush(g_log_file);
			fprintf(g_log_file, "%s - WAIT_SECOND_ORDERS_CANCEL: current capital, \tR$%.2lf\n", 
				get_trading_system_time(state.now, moving_average_current_minute), 
				state.capital); 
				fflush(g_log_file);
			
			fprintf(g_log_file, "%s - %s %s, ref. %s,  current capital: R$%.2lf,  week return: %+0.2lf%%,  accumulated return: %+0.2lf%%\n", 
				get_trading_system_time(state.now, moving_average_current_minute), 
				state.best_stock_name, ((state.trade_order == BUY_SELL)? "buy ": "sell"), 
				state.ref_stock_name, 
				state.capital, 
				100.0 * (state.capital - state.previous_capital) / state.previous_capital, 
				100.0 * (state.capital - INITIAL_CAPITAL) / INITIAL_CAPITAL);
			printf("# %s - %s %s, ref. %s,  current capital: R$%.2lf,  week return: %+0.2lf%%,  accumulated return: %+0.2lf%%\n", 
				get_trading_system_time(state.now, moving_average_current_minute), 
				state.best_stock_name, ((state.trade_order == BUY_SELL)? "buy ": "sell"), 
				state.ref_stock_name, 
				state.capital, 
				100.0 * (state.capital - state.previous_capital) / state.previous_capital, 
				100.0 * (state.capital - INITIAL_CAPITAL) / INITIAL_CAPITAL);
			
			state.state = WAIT_BEGIN_WEEK;
		}
		else if (state.order_result == ORDER_CANCELED)
		{
			fprintf(g_log_file, "%s - WAIT_SECOND_ORDERS_CANCEL: second orders CANCELED!\n", get_trading_system_time(state.now, moving_average_current_minute)); fflush(g_log_file);

			state.state = WAIT_SECOND_ORDERS_TRIGGER;
		}
		else if (moving_average_current_minute >= (MINUTES_IN_A_WEEK - MINUTES_BEFORE_CLOSING_WEEK)) // Time out para cancelar...
		{
			if (moving_average_current_minute > (MINUTES_IN_A_WEEK - 1)) // pode ocorrer de nao haver negocios na quinta
				moving_average_current_minute = MINUTES_IN_A_WEEK - 1;
				
			fprintf(g_log_file, "%s - WAIT_SECOND_ORDERS_CANCEL:\t\ttime out to sell cancel! USER INTERVENTION REQUIRED!\n", get_trading_system_time(state.now, moving_average_current_minute)); fflush(g_log_file);

			state.state = QUIT;
		}
		else if (((state.day_current_minute - state.second_orders_cancel_minute) > NUM_MINUTES_FOR_ORDER_CANCELATION) || ((state.day_current_minute - state.second_orders_execution_minute) < 0))
		{
			fprintf(g_log_file, "%s - WAIT_SECOND_ORDERS_CANCEL: sell cancel time out! USER INTERVENTION REQUIRED! %s\n", get_trading_system_time(state.now, moving_average_current_minute), state.best_stock_name); fflush(g_log_file);

			state.state = QUIT;			
		}
		else
		{
			fprintf(g_log_file, "%s - WAIT_SECOND_ORDERS_CANCEL: wait second orders cancel,\t%s %s @ R$%.2lf, order @ R$%.2lf, quant. = %d\n", 
				get_trading_system_time(state.now, moving_average_current_minute), 
				((state.trade_order == BUY_SELL)? "sell": "buy"), state.best_stock_name, 
				state.best_stock_price, state.best_stock_order_price, state.best_stock_quantity); 
				fflush(g_log_file);
			fprintf(g_log_file, "%s - WAIT_SECOND_ORDERS_CANCEL: wait second orders cancel,\t%s %s @ R$%.2lf, order @ R$%.2lf, quant. = %d\n", 
				get_trading_system_time(state.now, moving_average_current_minute), 
				((state.trade_order == BUY_SELL)? "buy": "sell"), state.ref_stock_name, 
				state.ref_stock_price, state.ref_stock_order_price, state.ref_stock_quantity); 
				fflush(g_log_file);
		}
	}
		
	if (state.state == WAIT_BEGIN_WEEK)
	{
		state.best_stock_price = g_best_stock_price[moving_average_current_minute];
		state.ref_stock_price = g_ref_stock_price[moving_average_current_minute];
		state.moving_average_of_delta = delta_moving_average(&state, g_best_stock_price, g_ref_stock_price, moving_average_current_minute);
#ifdef	USE_IPC
		if ((moving_average_current_minute == (MINUTES_IN_A_WEEK - 2)) && (g_trading_system_current_day_date_l >= isodate2long(g_end_date)))
		{
			state.state = QUIT;
		}
#else
		if ((moving_average_current_minute == (MINUTES_IN_A_WEEK - 1)) && (g_trading_system_current_day_date_l >= isodate2long(g_end_date)))
		{
			generate_graph("GRAFICOS", &state, g_best_stock_price, g_ref_stock_price);
			state.state = QUIT;
		}
#endif
		else if (moving_average_current_minute == (MINUTES_IN_A_WEEK - 1))
		{
			generate_graph("GRAFICOS", &state, g_best_stock_price, g_ref_stock_price);
		}
		else if (moving_average_current_minute < state.previous_moving_average_current_minute)
		{
			fprintf(g_log_file, "%s - WAIT_BEGIN_WEEK: starting a week.\n", get_trading_system_time(state.now, moving_average_current_minute)); fflush(g_log_file);

			state.state = WAIT_MOVING_AVERAGE;
		}
	}

	
	if (g_stop_trading_system == 1)
	{
		generate_graph("GRAFICOS", &state, g_best_stock_price, g_ref_stock_price);

		printf("Trading System stopped graciously.\n");
#ifdef	USE_IPC
		IPC_disconnect();
#endif
		exit(0);
	}

	if (state.state == QUIT)
	{
		fprintf(g_log_file, "%s - QUIT:  Final Capital\t\tR$%.2lf  accumulated return: %.2lf%%\n", get_trading_system_time(state.now, moving_average_current_minute), state.capital, 100.0 * (state.capital - INITIAL_CAPITAL) / INITIAL_CAPITAL); 
		printf("### %s - QUIT: Final Capital\t\tR$%.2lf\t accumulated return: %.2lf%%\n", get_trading_system_time(state.now, moving_average_current_minute), state.capital, 100.0 * (state.capital - INITIAL_CAPITAL) / INITIAL_CAPITAL); 
		
		fclose(g_log_file);
	}

	state.previous_moving_average_current_minute = moving_average_current_minute;
	
	g_state = state;
	save_state(g_state);	

#ifdef	IPC_ON_LINE
	printf("%s - %s: %s price = R$%.2lf\n", get_trading_system_time(state.now, moving_average_current_minute), g_state_string[state.state], state.best_stock_name, g_best_stock_price[moving_average_current_minute]); 
#endif
	return (state.state);
}
