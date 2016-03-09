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
#include "ipc.h"
#include "messages.h"
#include "trading_system.h"

#define NUM_MINUTES_FOR_ORDER_EXECUTION		5
#define NUM_MINUTES_FOR_ORDER_CANCELATION	5
#define	MINUTES_BEFORE_CLOSING_DAY		10



STATE
init_state()
{
	STATE state;
	
	state.state = INITIALIZE;
	state.capital = CAPITAL;
	strcpy(state.now, "");
	strcpy(state.best_stock_name, "");
	state.previous_moving_average_current_minute = 0;
#ifdef	USE_IPC
	state.order_execution_message_received = 0;
#endif
	return (state);
}



STATE
get_state_in_a_hot_start()
{
	FILE *state_file;
	STATE state;
	
	if ((state_file = fopen(STATE_FILE_NAME, "r")) == NULL)
		Erro("Could not open state file ", STATE_FILE_NAME, " in get_state_in_a_hot_start()");
		
	fread(&state, sizeof(STATE), 1, state_file);

	fread(&g_target_day_index, 			sizeof(int), 1, state_file);
	fread(&g_trading_system_current_day, 		sizeof(int), 1, state_file);
	fread(&g_moving_average_current_minute, 	sizeof(int), 1, state_file);
	if (g_moving_average_current_minute > MOVING_AVERAGE_NUM_PERIODS)
		fread(&(g_price_samples[g_moving_average_current_minute - MOVING_AVERAGE_NUM_PERIODS]), sizeof(float), MOVING_AVERAGE_NUM_PERIODS + 1, state_file);
	else
		fread(g_price_samples, sizeof(float), g_moving_average_current_minute + 1, state_file);

	fclose(state_file);

	state.log_file = fopen("log.txt", "a");
	
	save_state(state);	// Tem que salvar para guardar o novo ponteiro para o arquivo log_file

	return (state);
}



STATE
get_state()
{
	FILE *state_file;
	STATE state;
	
	if ((state_file = fopen(STATE_FILE_NAME, "r")) == NULL)
	{
		state = init_state();
	}
	else
	{
		fread(&state, sizeof(STATE), 1, state_file);
		fclose(state_file);
	}
	return (state);
}



void
save_state(STATE state)
{
	FILE *state_file;
	
	if ((state_file = fopen(STATE_FILE_NAME, "w")) == NULL)
		Erro("Could not open state_file named ", STATE_FILE_NAME, " in save_state()");

	fwrite(&state, sizeof(STATE), 1, state_file);

	fwrite(&g_target_day_index, 			sizeof(int), 1, state_file);
	fwrite(&g_trading_system_current_day, 		sizeof(int), 1, state_file);
	fwrite(&g_moving_average_current_minute, 	sizeof(int), 1, state_file);
	if (g_moving_average_current_minute > MOVING_AVERAGE_NUM_PERIODS)
		fwrite(&(g_price_samples[g_moving_average_current_minute - MOVING_AVERAGE_NUM_PERIODS]), sizeof(float), MOVING_AVERAGE_NUM_PERIODS + 1, state_file);
	else
		fwrite(g_price_samples, sizeof(float), g_moving_average_current_minute + 1, state_file);
	
	fclose(state_file);
}


char *
get_best_stock_name(int trading_system_current_day)
{
	static char stock[1000];
	char *stock_name;
	char *aux;

	strcpy(stock, g_argv[g_tt_data_set[0][trading_system_current_day].file_number]);
	stock_name = strrchr(stock, '/') + 1;
	aux = strrchr(stock_name, '.');
	aux[0] = '\0';

	return (stock_name);
}



float
get_best_stock_predicted_return(int trading_system_current_day)
{
	return (g_tt_data_set[0][trading_system_current_day].wnn_predicted_return);
}



#ifdef	BUILD_DAILY_BUY_SELL_FILES

float
get_best_stock_maximum_buy_price(int target_day_index, int trading_system_current_day)
{
	int previous_day;
	float price;
	float maximum_buy_price;
	
	previous_day = target_day_index - 1;
	price = g_train_data_set[previous_day].clo;
	maximum_buy_price = price * (1.0 + TRADING_SYSTEM_START_BUYING_RETURN);
	
	return (maximum_buy_price);
}


#else

double
prediction_errors_standard_deviation(int file_number, int trading_system_current_day)
{
	int i;
	double err, average_wnn_prediction_error = 0.0, standard_deviation_wnn_error = 0.0;
	
	for (i = trading_system_current_day - 1; i >= (trading_system_current_day - SAMPLES2CALIBRATE); i--)
		average_wnn_prediction_error += g_tt_data_set[file_number][i].wnn_prediction_error;
	average_wnn_prediction_error /= (double) (SAMPLES2CALIBRATE-1);

	for (i = trading_system_current_day - 1; i >= (trading_system_current_day - SAMPLES2CALIBRATE); i--)
	{
		err = g_tt_data_set[file_number][i].wnn_prediction_error;

		standard_deviation_wnn_error += (err - average_wnn_prediction_error) *
					        (err - average_wnn_prediction_error);
	}
	standard_deviation_wnn_error = sqrt(standard_deviation_wnn_error / (double) (SAMPLES2CALIBRATE-1));

	return (standard_deviation_wnn_error);
}



float
get_best_stock_maximum_buy_price(int target_day_index, int trading_system_current_day)
{
	float maximum_buy_price;
	
	maximum_buy_price = g_train_data_set[target_day_index - (SAMPLE_SIZE + 1)].clo * 
	  (1.0 + g_tt_data_set[0][trading_system_current_day].wnn_predicted_return - 
	   prediction_errors_standard_deviation(g_tt_data_set[0][trading_system_current_day].file_number, trading_system_current_day) +
	   TRADING_SYSTEM_STOP_GAIN);
	
	return (maximum_buy_price);
}

#endif


int
get_current_day(int target_day_index)
{
	return (target_day_index);
}



float
get_best_stock_volume(int target_day_index, int moving_average_current_minute)
{
	int current_day;
	float *intraday_volume;
	
	current_day = get_current_day(target_day_index);
	intraday_volume = g_train_data_set[current_day].intraday_volume;
	if (intraday_volume != NULL)
		return (intraday_volume[moving_average_current_minute]);
	else
		return (0.0);
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
get_trading_system_time(char *str_now, int trading_system_current_day, int moving_average_current_minute)
{
	long best_stock_date_l;
	char *day, *week_day;
	int hour, minute;
	
	// g_tt_data_set[0][trading_system_current_day].date_l contem a data para a qual temos a predicao
	best_stock_date_l = g_tt_data_set[0][trading_system_current_day].date_l;
	
	day = long2isodate(best_stock_date_l);
	week_day = long2dows(best_stock_date_l);
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
	cost += capital * (0.019 / 100.0); // Emolumentos Bovespa
	cost += capital * (0.006 / 100.0); // Liquidação
#else
	cost = 0.0; // Custo fixo de compra (corretagem) 
	cost += capital * (0.070 / 100.0); // Correntagem
	cost += capital * (0.019 / 100.0); // Emolumentos Bovespa
	cost += capital * (0.006 / 100.0); // Liquidação
#endif
	return (cost);
}



#ifdef	USE_IPC


void
receive_order_execution(qsor_order_execution_message* received_order_execution)
{
	STATE state;

	state = get_state();
	state.received_order_execution = *received_order_execution;
	state.order_execution_message_received = 1;
	save_state(state);
	
	trading_system_state_machine(g_moving_average_current_minute, g_target_day_index, g_trading_system_current_day);
}



void
send_order(STATE state, int buy)
{
	ts_order_message order;
	
	order.buy = buy;
	order.stock_symbol = state.best_stock_name;
	order.price = state.price;
	order.quantity = state.quantity;
	order.date_and_time = state.now;
	order.host = TS_NAME;

	IPC_publishData(TS_ORDER_MESSAGE_NAME, &order);
}



void
send_buy_order(STATE state)
{
	send_order(state, 1);
}



void
send_sell_order(STATE state)
{
	send_order(state, 0);
}



int
check_order(STATE *state, int buy)
{
	double transaction_amount;
	
	if (state->order_execution_message_received && (state->received_order_execution.buy == -1))
	{
		state->order_execution_message_received = 0;
		
		return (ORDER_CANCELED);
	}
	else if (state->order_execution_message_received && (state->received_order_execution.buy == buy))
	{
		state->order_execution_message_received = 0;
		
		if (buy)
		{
			state->buy_price = state->received_order_execution.price;
			state->quantity = state->received_order_execution.quantity;

			transaction_amount = (float) (state->quantity) * state->buy_price;
			state->capital = state->capital - transaction_amount - trade_cost(transaction_amount);
		}
		else
		{
			state->sell_price = state->received_order_execution.price;
			state->quantity = state->received_order_execution.quantity;

			transaction_amount = (float) (state->quantity) * state->sell_price;
			state->capital = state->capital + transaction_amount - trade_cost(transaction_amount);
		}
		
		return (ORDER_EXECUTED);
	}
	else
	{
		return (ORDER_NOT_EXECUTED);
	}
}



int
check_buy_order(STATE *state)
{
	return (check_order(state, 1));
}


int
check_sell_order(STATE *state)
{
	return (check_order(state, 0));
}


#else


void
send_buy_order(STATE state)
{
}



void
send_sell_order(STATE state)
{
}


int
check_buy_order(STATE *state)
{
	double transaction_amount;
	
	state->buy_price = g_price_samples[g_moving_average_current_minute];
	state->quantity = (state->capital / state->buy_price);
	state->quantity = state->quantity - state->quantity % 100; // Lotes de 100 acoes

	transaction_amount = (float) (state->quantity) * state->buy_price;
	state->capital = state->capital - transaction_amount - trade_cost(transaction_amount);
	
	return (ORDER_EXECUTED);
}


int
check_sell_order(STATE *state)
{
	double transaction_amount;
	
	state->sell_price = g_price_samples[g_moving_average_current_minute];
	// state->quantity = (state->capital / state->sell_price);
	// state->quantity = state->quantity - state->quantity % 100; // Lotes de 100 acoes

	transaction_amount = (float) (state->quantity) * state->sell_price;
	state->capital = state->capital + transaction_amount - trade_cost(transaction_amount);
	
	return (ORDER_EXECUTED);
}

#endif


int
trading_system_state_machine(int moving_average_current_minute, int target_day_index, int trading_system_current_day)
{	
	STATE state;

	state = get_state();

	if (state.state == INITIALIZE)
	{
		printf("Initial state = %s\n", g_state_string[state.state]);
		fflush(stdout);

		state.log_file = fopen("log.txt", "a");
		fprintf(state.log_file, "%s - INITIALIZE\n", get_trading_system_time(state.now, trading_system_current_day, moving_average_current_minute)); fflush(state.log_file);

		state.buy_day = state.sell_day = -1;
		state.buy_price = state.sell_price = 0.0;
		
		state.state = SELECT_STOCK;
	}
	
	if (state.state == WAIT_MIN_TO_BUY)
	{
		state.price = g_price_samples[moving_average_current_minute];
		state.moving_average_price = moving_average(moving_average_current_minute);
		
		if (moving_average_current_minute > MINUTES_IN_A_DAY / 2) // Time out para a compra
		{
			fprintf(state.log_file, "%s - WAIT_MIN_TO_BUY:\t\t\ttime out to buy - skip day!\n", get_trading_system_time(state.now, trading_system_current_day, moving_average_current_minute)); fflush(state.log_file);
			
			state.state = WAIT_BEGIN_DAY;
		}
		else if ((state.price < state.best_stock_maximum_buy_price) && (state.moving_average_price < state.best_stock_maximum_buy_price))
		{
			if (state.moving_average_price > state.previous_minimum_intraday_price * (1.0 + TRADING_SYSTEM_BUY_SELL_DELTA_RETURN))
			{	// Achou a primeira subida depois de um minimo local
				fprintf(state.log_file, "%s - WAIT_MIN_TO_BUY: min. found,\t%s price = R$%.2f, moving aver. price = R$%.2f, max. buy price = R$%.2f\n", get_trading_system_time(state.now, trading_system_current_day, moving_average_current_minute), state.best_stock_name, state.price, state.moving_average_price, state.best_stock_maximum_buy_price); fflush(state.log_file);
	
				state.state = SEND_BUY_ORDER;
			}
		}

		if (state.moving_average_price < state.previous_minimum_intraday_price) // search for minimum
			state.previous_minimum_intraday_price = state.moving_average_price;
		if (state.moving_average_price > state.best_stock_maximum_buy_price) // reset search for minimum
			state.previous_minimum_intraday_price = state.moving_average_price;
	}

	if (state.state == SEND_BUY_ORDER)
	{
		state.order_price = state.price = g_price_samples[moving_average_current_minute];
		state.quantity = (state.capital / state.price);
		state.quantity = state.quantity - state.quantity % 100; // Lotes de 100 acoes
		
		get_trading_system_time(state.now, trading_system_current_day, moving_average_current_minute);
		send_buy_order(state);

		state.buy_order_minute = moving_average_current_minute;
			
		fprintf(state.log_file, "%s - SEND_BUY_ORDER: \t\t\t%s price = R$%.2f, quant. = %d\n", get_trading_system_time(state.now, trading_system_current_day, moving_average_current_minute), state.best_stock_name, state.order_price, state.quantity); fflush(state.log_file);

		state.state = WAIT_BUY;
	}
	
	if (state.state == WAIT_BUY)
	{
		state.order_result = check_buy_order(&state);
		state.current_minute = moving_average_current_minute;
		
		state.price = g_price_samples[moving_average_current_minute];
		if (state.order_result == ORDER_EXECUTED)
		{
			state.best_stock_minimum_sell_price = state.buy_price * (1.0 + TRADING_SYSTEM_STOP_GAIN);

			state.buy_day = get_current_day(target_day_index);
			state.buy_minute = moving_average_current_minute;

			state.previous_maximum_intraday_price = moving_average(moving_average_current_minute);

			fprintf(state.log_file, "%s - WAIT_BUY: buy order exec., \t%s price = R$%.2f, order price = R$%.2f, buy price = R$%.2f, quant. = %d\n", get_trading_system_time(state.now, trading_system_current_day, moving_average_current_minute), state.best_stock_name, state.price, state.order_price, state.buy_price, state.quantity); fflush(state.log_file);
	
			state.state = WAIT_MAX_TO_SELL;
		}
		else if (state.order_result == ORDER_CANCELED)
		{
			fprintf(state.log_file, "%s - WAIT_BUY: buy CANCELED!    \t%s\n", get_trading_system_time(state.now, trading_system_current_day, moving_average_current_minute), state.best_stock_name); fflush(state.log_file);

			state.state = WAIT_MIN_TO_BUY;			
		}
		else if (((state.current_minute - state.buy_order_minute) > NUM_MINUTES_FOR_ORDER_EXECUTION) || ((state.current_minute - state.buy_order_minute) < 0))
		{
			fprintf(state.log_file, "%s - WAIT_BUY: buy time out!    \t%s\n", get_trading_system_time(state.now, trading_system_current_day, moving_average_current_minute), state.best_stock_name); fflush(state.log_file);

			state.buy_cancel_minute = moving_average_current_minute;
			
			state.state = WAIT_BUY_CANCEL;			
		}
		else 
		{
			fprintf(state.log_file, "%s - WAIT_BUY: wait buy order,  \t%s, price = R$%.2f, order price = R$%.2f, quant. = %d\n", get_trading_system_time(state.now, trading_system_current_day, moving_average_current_minute), state.best_stock_name, state.price, state.order_price, state.quantity); fflush(state.log_file);
		}
	}
	
	if (state.state == WAIT_BUY_CANCEL)
	{
		state.order_result = check_buy_order(&state);
		state.current_minute = moving_average_current_minute;
		
		state.price = g_price_samples[moving_average_current_minute];
		if (state.order_result == ORDER_EXECUTED)
		{
			state.best_stock_minimum_sell_price = state.buy_price * (1.0 + TRADING_SYSTEM_STOP_GAIN);
			
			state.buy_day = get_current_day(target_day_index);
			state.buy_minute = moving_average_current_minute;
			
			state.previous_maximum_intraday_price = moving_average(moving_average_current_minute);

			fprintf(state.log_file, "%s - WAIT_BUY_CANCEL: buy order exec.,\t%s price = R$%.2f, order price = R$%.2f, buy price = R$%.2f, quant. = %d\n", get_trading_system_time(state.now, trading_system_current_day, moving_average_current_minute), state.best_stock_name, state.price, state.order_price, state.buy_price, state.quantity); fflush(state.log_file);
	
			state.state = WAIT_MAX_TO_SELL;
		}
		else if (state.order_result == ORDER_CANCELED)
		{
			fprintf(state.log_file, "%s - WAIT_BUY_CANCEL: buy CANCELED! \t%s\n", get_trading_system_time(state.now, trading_system_current_day, moving_average_current_minute), state.best_stock_name); fflush(state.log_file);

			state.state = WAIT_MIN_TO_BUY;			
		}
		else if (((state.current_minute - state.buy_cancel_minute) > NUM_MINUTES_FOR_ORDER_CANCELATION) || ((state.current_minute - state.buy_cancel_minute) < 0))
		{
			fprintf(state.log_file, "%s - WAIT_BUY_CANCEL: buy cancel time out! USER INTERVENTION REQUIRED!\t%s\n", get_trading_system_time(state.now, trading_system_current_day, moving_average_current_minute), state.best_stock_name); fflush(state.log_file);

			state.state = QUIT;
		}
		else 
		{
			fprintf(state.log_file, "%s - WAIT_BUY_CANCEL: wait buy cancel, \t%s, price = R$%.2f, order price = R$%.2f, quant. = %d\n", get_trading_system_time(state.now, trading_system_current_day, moving_average_current_minute), state.best_stock_name, state.price, state.order_price, state.quantity); fflush(state.log_file);
		}
	}
	
	if (state.state == WAIT_MAX_TO_SELL)
	{
		state.price = g_price_samples[moving_average_current_minute];
		state.moving_average_price = moving_average(moving_average_current_minute);

		if (moving_average_current_minute >= (MINUTES_IN_A_DAY - MINUTES_BEFORE_CLOSING_DAY)) // Time out para a venda @@@ tem que colocar antes quando on-line
		{
			if (moving_average_current_minute > (MINUTES_IN_A_DAY - 1)) // Erro?
				g_moving_average_current_minute = moving_average_current_minute = MINUTES_IN_A_DAY - 2;
				
			fprintf(state.log_file, "%s - WAIT_MAX_TO_SELL:\t\ttime out to sell!\n", get_trading_system_time(state.now, trading_system_current_day, moving_average_current_minute)); fflush(state.log_file);

			state.state = SEND_SELL_ORDER;
		}
		else if (state.price < (state.buy_price * (1.0 + TRADING_SYSTEM_STOP_LOSS)))
		{	// Stop Loss
			fprintf(state.log_file, "%s - WAIT_MAX_TO_SELL:\t\tstop loss!\n", get_trading_system_time(state.now, trading_system_current_day, moving_average_current_minute)); fflush(state.log_file);

			state.state = SEND_SELL_ORDER;
		}
		else if ((state.price > state.best_stock_minimum_sell_price) && (state.moving_average_price > state.best_stock_minimum_sell_price))
		{
			if (state.moving_average_price < state.previous_maximum_intraday_price * (1.0 - TRADING_SYSTEM_BUY_SELL_DELTA_RETURN))
			{	// Achou a primeira descida depois de um maximo local
				fprintf(state.log_file, "%s - WAIT_MAX_TO_SELL: max. found,\t%s price = R$%.2f, moving aver. price = R$%.2f, min. sell price = R$%.2f\n", get_trading_system_time(state.now, trading_system_current_day, moving_average_current_minute), state.best_stock_name, state.price, state.moving_average_price, state.best_stock_minimum_sell_price); fflush(state.log_file);
	
				state.state = SEND_SELL_ORDER;
			}
		}

		if (state.moving_average_price > state.previous_maximum_intraday_price) // search for maximum
			state.previous_maximum_intraday_price = state.moving_average_price;
		if (state.moving_average_price < state.best_stock_minimum_sell_price) // reset search for maximum
			state.previous_maximum_intraday_price = state.moving_average_price;
	}

	if (state.state == SEND_SELL_ORDER)
	{
		state.order_price = state.price = g_price_samples[moving_average_current_minute];
		
		get_trading_system_time(state.now, trading_system_current_day, moving_average_current_minute);
		send_sell_order(state);

		state.sell_order_minute = moving_average_current_minute;
			
		fprintf(state.log_file, "%s - SEND_SELL_ORDER:\t\t\t%s price = R$%.2f, quant. = %d\n", get_trading_system_time(state.now, trading_system_current_day, moving_average_current_minute), state.best_stock_name, state.order_price, state.quantity); fflush(state.log_file);

		state.state = WAIT_SELL;
	}
	
	if (state.state == WAIT_SELL)
	{
		state.order_result = check_sell_order(&state);
		state.current_minute = moving_average_current_minute;

		state.price = g_price_samples[moving_average_current_minute];
		if (state.order_result == ORDER_EXECUTED)
		{
			state.sell_day = get_current_day(target_day_index);
			state.sell_minute = moving_average_current_minute;
	
			fprintf(state.log_file, "%s - WAIT_SELL: sell order exec.,\t%s price = R$%.2f, order price = R$%.2f, quant. = %d\n", get_trading_system_time(state.now, trading_system_current_day, moving_average_current_minute), state.best_stock_name, state.price, state.order_price, state.quantity); fflush(state.log_file);
			fprintf(state.log_file, "%s - WAIT_SELL: current capital, \tR$%.2f\tbuy price = R$%.2f\tsell price = R$%.2f\n", get_trading_system_time(state.now, trading_system_current_day, moving_average_current_minute), state.capital, state.buy_price, state.sell_price); fflush(state.log_file);
			
			state.state = WAIT_BEGIN_DAY;
		}
		else if (state.order_result == ORDER_CANCELED)
		{
			fprintf(state.log_file, "%s - WAIT_SELL: sell CANCELED!  \t%s\n", get_trading_system_time(state.now, trading_system_current_day, moving_average_current_minute), state.best_stock_name); fflush(state.log_file);

			state.state = WAIT_MAX_TO_SELL;
		}
		else if (((state.current_minute - state.sell_order_minute) > NUM_MINUTES_FOR_ORDER_EXECUTION) || ((state.current_minute - state.sell_order_minute) < 0))
		{
			fprintf(state.log_file, "%s - WAIT_SELL: sell time out!    \t%s\n", get_trading_system_time(state.now, trading_system_current_day, moving_average_current_minute), state.best_stock_name); fflush(state.log_file);

			state.sell_cancel_minute = moving_average_current_minute % MINUTES_IN_A_DAY;
			
			state.state = WAIT_SELL_CANCEL;			
		}
		else
		{
			fprintf(state.log_file, "%s - WAIT_SELL: wait sell order, \t%s, price = R$%.2f, order price = R$%.2f, quant. = %d\n", get_trading_system_time(state.now, trading_system_current_day, moving_average_current_minute), state.best_stock_name, state.price, state.order_price, state.quantity); fflush(state.log_file);
		}
	}
		
	if (state.state == WAIT_SELL_CANCEL)
	{
		state.order_result = check_sell_order(&state);
		state.current_minute = moving_average_current_minute;

		state.price = g_price_samples[moving_average_current_minute];
		if (state.order_result == ORDER_EXECUTED)
		{
			state.sell_day = get_current_day(target_day_index);
			state.sell_minute = moving_average_current_minute;
	
			fprintf(state.log_file, "%s - WAIT_SELL_CANCEL: sell order exec.,\t%s price = R$%.2f, order price = R$%.2f, quant. = %d\n", get_trading_system_time(state.now, trading_system_current_day, moving_average_current_minute), state.best_stock_name, state.price, state.order_price, state.quantity); fflush(state.log_file);
			fprintf(state.log_file, "%s - WAIT_SELL_CANCEL: current capital, \tR$%.2f\tbuy price = R$%.2f\tsell price = R$%.2f\n", get_trading_system_time(state.now, trading_system_current_day, moving_average_current_minute), state.capital, state.buy_price, state.sell_price); fflush(state.log_file);
			
			state.state = WAIT_BEGIN_DAY;
		}
		else if (state.order_result == ORDER_CANCELED)
		{
			fprintf(state.log_file, "%s - WAIT_SELL_CANCEL: sell CANCELED!  \t%s\n", get_trading_system_time(state.now, trading_system_current_day, moving_average_current_minute), state.best_stock_name); fflush(state.log_file);

			state.state = WAIT_MAX_TO_SELL;
		}
		else if (moving_average_current_minute >= (MINUTES_IN_A_DAY - MINUTES_BEFORE_CLOSING_DAY)) // Time out para cancelar...
		{
			if (moving_average_current_minute > (MINUTES_IN_A_DAY - 1)) // Erro?
				moving_average_current_minute = MINUTES_IN_A_DAY - 1;
				
			fprintf(state.log_file, "%s - WAIT_SELL_CANCEL:\t\ttime out to sell cancel! USER INTERVENTION REQUIRED!\n", get_trading_system_time(state.now, trading_system_current_day, moving_average_current_minute)); fflush(state.log_file);

			state.state = QUIT;
		}
		else if (((state.current_minute - state.sell_cancel_minute) > NUM_MINUTES_FOR_ORDER_CANCELATION) || ((state.current_minute - state.sell_order_minute) < 0))
		{
			fprintf(state.log_file, "%s - WAIT_SELL_CANCEL: sell cancel time out! USER INTERVENTION REQUIRED! %s\n", get_trading_system_time(state.now, trading_system_current_day, moving_average_current_minute), state.best_stock_name); fflush(state.log_file);

			state.state = QUIT;			
		}
		else
		{
			fprintf(state.log_file, "%s - WAIT_SELL_CANCEL: wait sell cancel,\t%s, price = R$%.2f, order price = R$%.2f, quant. = %d\n", get_trading_system_time(state.now, trading_system_current_day, moving_average_current_minute), state.best_stock_name, state.price, state.order_price, state.quantity); fflush(state.log_file);
		}
	}
		
	if (state.state == WAIT_BEGIN_DAY)
	{
		if (trading_system_current_day >= g_tt_num_samples)
		{
			trading_system_current_day = g_tt_num_samples - 1; // apenas para conseguir imprimir a data atual no estado QUIT
			state.state = QUIT;
		}
		else if (moving_average_current_minute == (MINUTES_IN_A_DAY - 1))
		{
			if (state.buy_day != -1)
				generate_graph("GRAFICOS", state.best_stock_name, state.buy_day, g_train_data_set[state.buy_day].intraday_price, state.buy_minute, state.best_stock_maximum_buy_price, BUY);
			if (state.sell_day != -1)
				generate_graph("GRAFICOS", state.best_stock_name, state.sell_day, g_train_data_set[state.sell_day].intraday_price, state.sell_minute, state.best_stock_minimum_sell_price, SELL);
		}
		else if (moving_average_current_minute < state.previous_moving_average_current_minute)
		{
			fprintf(state.log_file, "%s - WAIT_BEGIN_DAY: starting a day.\n", get_trading_system_time(state.now, trading_system_current_day, moving_average_current_minute)); fflush(state.log_file);

			state.state = SELECT_STOCK;
		}
	}
		
	if (state.state == SELECT_STOCK)
	{
		strcpy (state.best_stock_name, get_best_stock_name(trading_system_current_day));
		state.best_stock_maximum_buy_price = get_best_stock_maximum_buy_price(target_day_index, trading_system_current_day);
		state.best_stock_predicted_return = get_best_stock_predicted_return(trading_system_current_day);

		state.previous_minimum_intraday_price = moving_average(moving_average_current_minute);

		state.buy_day = state.sell_day = -1;
		state.buy_price = state.sell_price = 0.0;
				
		fprintf(state.log_file, "%s - SELECT_STOCK: best stock \t%s, max. buy price = R$%.2f\n", get_trading_system_time(state.now, trading_system_current_day, moving_average_current_minute), state.best_stock_name, state.best_stock_maximum_buy_price); fflush(state.log_file);

#ifndef	BUILD_DAILY_BUY_SELL_FILES
		if (state.best_stock_predicted_return <= 0.0)
		{
			fprintf(state.log_file, "%s - SELECT_STOCK: pessimist prediction; skip day. \t, predicted return = %f%%\n", get_trading_system_time(state.now, trading_system_current_day, moving_average_current_minute), state.best_stock_predicted_return); fflush(state.log_file);
			state.state = WAIT_BEGIN_DAY;
		}
		else
			state.state = WAIT_MIN_TO_BUY;
#else
		state.state = WAIT_MIN_TO_BUY;
#endif
	}

	if (state.state == QUIT)
	{
		fprintf(state.log_file, "%s - QUIT: Final Capital\t\tR$%.2f  retorno acumulado: %.2f%%\n", get_trading_system_time(state.now, trading_system_current_day, moving_average_current_minute), state.capital, 100.0 * (state.capital - CAPITAL) / CAPITAL); fflush(state.log_file);
		printf("### %s - QUIT: Final Capital\t\tR$%.2f  retorno acumulado: %.2f%%\n", get_trading_system_time(state.now, trading_system_current_day, moving_average_current_minute), state.capital, 100.0 * (state.capital - CAPITAL) / CAPITAL); fflush(state.log_file);
		
		fclose(state.log_file);
	}

	state.previous_moving_average_current_minute = moving_average_current_minute;
	
	save_state(state);
	
	if (g_stop_trading_system == 1)
	{
		printf("Trading System stopped!!!\n");
		IPC_disconnect();
		exit(0);
	}
	

#ifdef	IPC_ON_LINE
	printf("%s - %s: %s price = R$%.2f\n", get_trading_system_time(state.now, trading_system_current_day, moving_average_current_minute), g_state_string[state.state], state.best_stock_name, g_price_samples[moving_average_current_minute]); 
#endif

	return (state.state);
}
