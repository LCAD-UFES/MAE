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
#ifdef USE_IPC
#include "ipc.h"
#include "messages.h"
#endif
#include "trading_system.h"

#define NUM_MINUTES_FOR_ORDER_EXECUTION		5
#define NUM_MINUTES_FOR_ORDER_CANCELATION	5
#define	MINUTES_BEFORE_CLOSING_WEEK		10
#define	MINUTES_BEFORE_CLOSING_DAY		10
#define MINUTES_WAITING_MOVING_AVERAGE		MOVING_AVERAGE_NUM_PERIODS


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
	fread(&g_trading_system_current_week, 		sizeof(int), 1, state_file);
	fread(&g_moving_average_first_day, 		sizeof(int), 1, state_file);
	fread(&g_moving_average_current_minute, 	sizeof(int), 1, state_file);
	if (g_moving_average_current_minute > MOVING_AVERAGE_NUM_PERIODS)
		fread(&(g_price_samples[g_moving_average_current_minute - MOVING_AVERAGE_NUM_PERIODS]), sizeof(float), MOVING_AVERAGE_NUM_PERIODS + 1, state_file);
	else
		fread(g_price_samples, sizeof(float), g_moving_average_current_minute + 1, state_file);

	fclose(state_file);

	g_log_file = fopen("log.txt", "a");
	
	//save_state(state);	// Tem que salvar para guardar o novo ponteiro para o arquivo log_file

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
	fwrite(&g_trading_system_current_week, 		sizeof(int), 1, state_file);
	fwrite(&g_moving_average_first_day, 		sizeof(int), 1, state_file);
	fwrite(&g_moving_average_current_minute, 	sizeof(int), 1, state_file);
	if (g_moving_average_current_minute > MOVING_AVERAGE_NUM_PERIODS)
		fwrite(&(g_price_samples[g_moving_average_current_minute - MOVING_AVERAGE_NUM_PERIODS]), sizeof(float), MOVING_AVERAGE_NUM_PERIODS + 1, state_file);
	else
		fwrite(g_price_samples, sizeof(float), g_moving_average_current_minute + 1, state_file);
	
	fclose(state_file);
}


char *
get_best_stock_name(int trading_system_current_week)
{
	static char stock[1000];
	char *stock_name;
	char *aux;

	strcpy(stock, g_argv[g_tt_data_set[0][trading_system_current_week].file_number]);
	stock_name = strrchr(stock, '/') + 1;
	aux = strrchr(stock_name, '.');
	aux[0] = '\0';

	return (stock_name);
}



float
get_best_stock_predicted_return(int trading_system_current_week)
{
	return (g_tt_data_set[0][trading_system_current_week].wnn_predicted_return);
}


float
get_best_stock_maximum_buy_price(int target_day_index, int trading_system_current_week)
{
	int previous_day;
	float price;
	float maximum_buy_price;
	
	previous_day = target_day_index - 5;
	price = g_train_data_set[previous_day].clo;
	maximum_buy_price = price * (1.0 + TRADING_SYSTEM_START_BUYING_RETURN);
	
	return (maximum_buy_price);
}



double
prediction_errors_standard_deviation(int file_number, int trading_system_current_week)
{
	int i;
	double err, average_wnn_prediction_error = 0.0, standard_deviation_wnn_error = 0.0;
	
	for (i = trading_system_current_week - 1; i >= (trading_system_current_week - SAMPLES2CALIBRATE); i--)
		average_wnn_prediction_error += g_tt_data_set[file_number][i].wnn_prediction_error;
	average_wnn_prediction_error /= (double) (SAMPLES2CALIBRATE-1);

	for (i = trading_system_current_week - 1; i >= (trading_system_current_week - SAMPLES2CALIBRATE); i--)
	{
		err = g_tt_data_set[file_number][i].wnn_prediction_error;

		standard_deviation_wnn_error += (err - average_wnn_prediction_error) *
					        (err - average_wnn_prediction_error);
	}
	standard_deviation_wnn_error = sqrt(standard_deviation_wnn_error / (double) (SAMPLES2CALIBRATE-1));

	return (standard_deviation_wnn_error);
}



float
get_best_stock_maximum_buy_price_new(int target_day_index, int trading_system_current_week)
{
	float maximum_buy_price;
	
	maximum_buy_price = g_train_data_set[target_day_index - (SAMPLE_SIZE + 1) * DAYS_IN_A_WEEK].clo * 
	  (1.0 + g_tt_data_set[0][trading_system_current_week].wnn_predicted_return - 
	   prediction_errors_standard_deviation(g_tt_data_set[0][trading_system_current_week].file_number, trading_system_current_week) +
	   TRADING_SYSTEM_STOP_GAIN);
	
	return (maximum_buy_price);
}



int
get_current_day(int target_day_index, int moving_average_current_minute)
{
	return (target_day_index - 4 + moving_average_current_minute / MINUTES_IN_A_DAY);
}



float
get_best_stock_volume(int target_day_index, int moving_average_current_minute)
{
	int current_day;
	float *intraday_volume;
	
	current_day = get_current_day(target_day_index, moving_average_current_minute);
	intraday_volume = g_train_data_set[current_day].intraday_volume;
	if (intraday_volume != NULL)
		return (intraday_volume[moving_average_current_minute % MINUTES_IN_A_DAY]);
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
get_trading_system_time(char *str_now, int trading_system_current_week, int moving_average_current_minute)
{
	long best_stock_date_l; // Na quarta
	long current_date_l_without_weekend;
	char *day, *week_day;
	int hour, minute;
	
	// g_tt_data_set[0][trading_system_current_week].date_l contem a data para a qual temos a predicao: temos que subtrair uma semana
	best_stock_date_l = g_tt_data_set[0][trading_system_current_week].date_l - 7 * SECONDS_IN_A_DAY;
	
	if (strcmp(long2dows(best_stock_date_l), "Wed") != 0)
		Erro("Trading system not ready to work on a day different of Wed. Current day = ", long2isodate(best_stock_date_l), " is not Wed");
	
	if ((moving_average_current_minute / MINUTES_IN_A_DAY) < 2) // N�o passou de sexta
		current_date_l_without_weekend = best_stock_date_l + SECONDS_IN_A_DAY * (1 + moving_average_current_minute / MINUTES_IN_A_DAY);
	else
		current_date_l_without_weekend = best_stock_date_l + SECONDS_IN_A_DAY * (3 + moving_average_current_minute / MINUTES_IN_A_DAY);
	
	day = long2isodate(current_date_l_without_weekend);
	week_day = long2dows(current_date_l_without_weekend);
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
	cost += capital * (0.006 / 100.0); // Liquida��o
#else
	cost = 0.0; // Custo fixo de compra (corretagem) 
	cost += capital * (0.070 / 100.0); // Correntagem
	cost += capital * (0.019 / 100.0); // Emolumentos Bovespa
	cost += capital * (0.006 / 100.0); // Liquida��o
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
	
	trading_system_state_machine(g_moving_average_current_minute, g_target_day_index, g_trading_system_current_week);
}



void
send_order(STATE state, int buy)
{
	ts_order_message order;
	
	order.buy = buy;
	order.stock_symbol = state.best_stock_name;
	order.price = state.order_price;
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


void
send_state(STATE state)
{
	ts_state_message state_message;

	state_message.state_size = sizeof(state);
	state_message.price_samples_size = MINUTES_IN_A_WEEK;
	state_message.state = (void *) (&state);
	state_message.price_samples = g_price_samples;
	state_message.date_and_time = state.now;
	state_message.host = TS_NAME;

	IPC_publishData(TS_STATE_MESSAGE_NAME, &state_message);	
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
trading_system_state_machine(int moving_average_current_minute, int target_day_index, int trading_system_current_week)
{	
	STATE state;

	state = get_state();

	if (trading_system_current_week < g_tt_num_samples)
	{
		get_trading_system_time(state.now, trading_system_current_week, moving_average_current_minute);
		state.moving_average_current_minute = moving_average_current_minute;
	}
	
	if (state.state == INITIALIZE)
	{
		printf("Initial state = %s\n", g_state_string[state.state]);
		fflush(stdout);

		g_log_file = fopen("log.txt", "a");
		fprintf(g_log_file, "%s - INITIALIZE\n", get_trading_system_time(state.now, trading_system_current_week, moving_average_current_minute)); fflush(g_log_file);

		state.state = SELECT_STOCK;
	}
	
	if (state.state == WAIT_MOVING_AVERAGE)
	{
		state.price = g_price_samples[moving_average_current_minute];
		state.moving_average_price = moving_average(g_price_samples, moving_average_current_minute);
		
		if (moving_average_current_minute >= MINUTES_WAITING_MOVING_AVERAGE)
		{
			state.previous_minimum_intraday_price = moving_average(g_price_samples, moving_average_current_minute);

			fprintf(g_log_file, "%s - WAIT_MOVING_AVERAGE: \t\tcurrent quote = %d\n", get_trading_system_time(state.now, trading_system_current_week, moving_average_current_minute), moving_average_current_minute); fflush(g_log_file);

			state.state = WAIT_MIN_TO_BUY;
		}
	}

	if (state.state == WAIT_MIN_TO_BUY)
	{
		state.price = g_price_samples[moving_average_current_minute];
		state.moving_average_price = moving_average(g_price_samples, moving_average_current_minute);
		
		if (moving_average_current_minute > 2 * MINUTES_IN_A_DAY) // Time out para a compra
		{
			state.buy_day = state.sell_day = -1;
			
			fprintf(g_log_file, "%s - WAIT_MIN_TO_BUY:\t\t\ttime out to buy - skip week!\n", get_trading_system_time(state.now, trading_system_current_week, moving_average_current_minute)); fflush(g_log_file);
			
			state.state = WAIT_BEGIN_WEEK;
		}
		else if ((state.price < state.best_stock_maximum_buy_price) && (state.moving_average_price < state.best_stock_maximum_buy_price))
		{
			if (state.moving_average_price > state.previous_minimum_intraday_price * (1.0 + TRADING_SYSTEM_BUY_SELL_DELTA_RETURN))
			{	// Achou a primeira subida depois de um minimo local
				fprintf(g_log_file, "%s - WAIT_MIN_TO_BUY: min. found,\t%s price = R$%.2f, moving aver. price = R$%.2f, max. buy price = R$%.2f\n", get_trading_system_time(state.now, trading_system_current_week, moving_average_current_minute), state.best_stock_name, state.price, state.moving_average_price, state.best_stock_maximum_buy_price); fflush(g_log_file);
	
				state.previous_capital = state.capital;
				
				state.order_price = state.price = g_price_samples[moving_average_current_minute];
				state.quantity = (state.capital / state.price);
				state.quantity = state.quantity - state.quantity % 100; // Lotes de 100 acoes
				
				get_trading_system_time(state.now, trading_system_current_week, moving_average_current_minute);
				
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
		send_buy_order(state);

		state.buy_order_minute = moving_average_current_minute % MINUTES_IN_A_DAY;
			
		fprintf(g_log_file, "%s - SEND_BUY_ORDER: \t\t\t%s price = R$%.2f, quant. = %d\n", get_trading_system_time(state.now, trading_system_current_week, moving_average_current_minute), state.best_stock_name, state.order_price, state.quantity); fflush(g_log_file);

		state.state = WAIT_BUY;
	}
	
	if (state.state == WAIT_BUY)
	{
		state.order_result = check_buy_order(&state);
		state.day_current_minute = moving_average_current_minute % MINUTES_IN_A_DAY;
		
		state.price = g_price_samples[moving_average_current_minute];
		if (state.order_result == ORDER_EXECUTED)
		{
			state.best_stock_minimum_sell_price = state.buy_price * (1.0 + TRADING_SYSTEM_STOP_GAIN);
			
			state.buy_day = get_current_day(target_day_index, moving_average_current_minute);
			state.buy_minute = moving_average_current_minute % MINUTES_IN_A_DAY;
			
			state.previous_maximum_intraday_price = moving_average(g_price_samples, moving_average_current_minute);

			fprintf(g_log_file, "%s - WAIT_BUY: buy order exec., \t%s price = R$%.2f, order price = R$%.2f, buy price = R$%.2f, quant. = %d\n", get_trading_system_time(state.now, trading_system_current_week, moving_average_current_minute), state.best_stock_name, state.price, state.order_price, state.buy_price, state.quantity); fflush(g_log_file);
	
			state.state = WAIT_MAX_TO_SELL;
		}
		else if (state.order_result == ORDER_CANCELED)
		{
			fprintf(g_log_file, "%s - WAIT_BUY: buy CANCELED!    \t%s\n", get_trading_system_time(state.now, trading_system_current_week, moving_average_current_minute), state.best_stock_name); fflush(g_log_file);

			state.state = WAIT_MIN_TO_BUY;			
		}
		else if (((state.day_current_minute - state.buy_order_minute) > NUM_MINUTES_FOR_ORDER_EXECUTION) || ((state.day_current_minute - state.buy_order_minute) < 0))
		{
			fprintf(g_log_file, "%s - WAIT_BUY: buy time out!    \t%s\n", get_trading_system_time(state.now, trading_system_current_week, moving_average_current_minute), state.best_stock_name); fflush(g_log_file);

			state.buy_cancel_minute = moving_average_current_minute % MINUTES_IN_A_DAY;
			
			state.state = WAIT_BUY_CANCEL;			
		}
		else 
		{
			fprintf(g_log_file, "%s - WAIT_BUY: wait buy order,  \t%s, price = R$%.2f, order price = R$%.2f, quant. = %d\n", get_trading_system_time(state.now, trading_system_current_week, moving_average_current_minute), state.best_stock_name, state.price, state.order_price, state.quantity); fflush(g_log_file);
		}
	}
	
	if (state.state == WAIT_BUY_CANCEL)
	{
		state.order_result = check_buy_order(&state);
		state.day_current_minute = moving_average_current_minute % MINUTES_IN_A_DAY;
		
		state.price = g_price_samples[moving_average_current_minute];
		if (state.order_result == ORDER_EXECUTED)
		{
			state.best_stock_minimum_sell_price = state.buy_price * (1.0 + TRADING_SYSTEM_STOP_GAIN);
			
			state.buy_day = get_current_day(target_day_index, moving_average_current_minute);
			state.buy_minute = moving_average_current_minute % MINUTES_IN_A_DAY;
			
			state.previous_maximum_intraday_price = moving_average(g_price_samples, moving_average_current_minute);

			fprintf(g_log_file, "%s - WAIT_BUY_CANCEL: buy order exec.,\t%s price = R$%.2f, order price = R$%.2f, buy price = R$%.2f, quant. = %d\n", get_trading_system_time(state.now, trading_system_current_week, moving_average_current_minute), state.best_stock_name, state.price, state.order_price, state.buy_price, state.quantity); fflush(g_log_file);
	
			state.state = WAIT_MAX_TO_SELL;
		}
		else if (state.order_result == ORDER_CANCELED)
		{
			fprintf(g_log_file, "%s - WAIT_BUY_CANCEL: buy CANCELED! \t%s\n", get_trading_system_time(state.now, trading_system_current_week, moving_average_current_minute), state.best_stock_name); fflush(g_log_file);

			state.state = WAIT_MIN_TO_BUY;			
		}
		else if (((state.day_current_minute - state.buy_cancel_minute) > NUM_MINUTES_FOR_ORDER_CANCELATION) || ((state.day_current_minute - state.buy_cancel_minute) < 0))
		{
			fprintf(g_log_file, "%s - WAIT_BUY_CANCEL: buy cancel time out! USER INTERVENTION REQUIRED!\t%s\n", get_trading_system_time(state.now, trading_system_current_week, moving_average_current_minute), state.best_stock_name); fflush(g_log_file);

			state.state = QUIT;
		}
		else 
		{
			fprintf(g_log_file, "%s - WAIT_BUY_CANCEL: wait buy cancel, \t%s, price = R$%.2f, order price = R$%.2f, quant. = %d\n", get_trading_system_time(state.now, trading_system_current_week, moving_average_current_minute), state.best_stock_name, state.price, state.order_price, state.quantity); fflush(g_log_file);
		}
	}
	
	if (state.state == WAIT_MAX_TO_SELL)
	{
		state.price = g_price_samples[moving_average_current_minute];
		state.moving_average_price = moving_average(g_price_samples, moving_average_current_minute);

		if (moving_average_current_minute >= (MINUTES_IN_A_WEEK - MINUTES_BEFORE_CLOSING_WEEK)) // Time out para a venda @@@ tem que colocar antes quando on-line
		{
			if (moving_average_current_minute > (MINUTES_IN_A_WEEK - 1)) // pode ocorrer de nao haver negocios na quinta
				g_moving_average_current_minute = moving_average_current_minute = MINUTES_IN_A_WEEK - 2;
				
			fprintf(g_log_file, "%s - WAIT_MAX_TO_SELL:\t\ttime out to sell!\n", get_trading_system_time(state.now, trading_system_current_week, moving_average_current_minute)); fflush(g_log_file);

			state.order_price = state.price = g_price_samples[moving_average_current_minute];
			
			state.state = SEND_SELL_ORDER;
		}
		else if (state.price < (state.buy_price * (1.0 + TRADING_SYSTEM_STOP_LOSS)))
		{	// Stop Loss
			fprintf(g_log_file, "%s - WAIT_MAX_TO_SELL:\t\tstop loss!\n", get_trading_system_time(state.now, trading_system_current_week, moving_average_current_minute)); fflush(g_log_file);

			state.order_price = state.price = g_price_samples[moving_average_current_minute];
			
			state.state = SEND_SELL_ORDER;
		}
		else if ((moving_average_current_minute > (MINUTES_IN_A_WEEK - MINUTES_IN_A_DAY)) || ((state.price > state.best_stock_minimum_sell_price) && (state.moving_average_price > state.best_stock_minimum_sell_price)))
		{
			if (state.moving_average_price < state.previous_maximum_intraday_price * (1.0 - TRADING_SYSTEM_BUY_SELL_DELTA_RETURN))
			{	// Achou a primeira descida depois de um maximo local
				fprintf(g_log_file, "%s - WAIT_MAX_TO_SELL: max. found,\t%s price = R$%.2f, moving aver. price = R$%.2f, min. sell price = R$%.2f\n", get_trading_system_time(state.now, trading_system_current_week, moving_average_current_minute), state.best_stock_name, state.price, state.moving_average_price, state.best_stock_minimum_sell_price); fflush(g_log_file);
	
				state.order_price = state.price = g_price_samples[moving_average_current_minute];
				
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
		get_trading_system_time(state.now, trading_system_current_week, moving_average_current_minute);
		send_sell_order(state);

		state.sell_order_minute = moving_average_current_minute % MINUTES_IN_A_DAY;
			
		fprintf(g_log_file, "%s - SEND_SELL_ORDER:\t\t\t%s price = R$%.2f, quant. = %d\n", get_trading_system_time(state.now, trading_system_current_week, moving_average_current_minute), state.best_stock_name, state.order_price, state.quantity); fflush(g_log_file);

		state.state = WAIT_SELL;
	}
	
	if (state.state == WAIT_SELL)
	{
		state.order_result = check_sell_order(&state);
		state.day_current_minute = moving_average_current_minute % MINUTES_IN_A_DAY;

		state.price = g_price_samples[moving_average_current_minute];
		if (state.order_result == ORDER_EXECUTED)
		{
			state.sell_day = get_current_day(target_day_index, moving_average_current_minute);
			state.sell_minute = moving_average_current_minute % MINUTES_IN_A_DAY;
	
			fprintf(g_log_file, "%s - WAIT_SELL: sell order exec.,\t%s price = R$%.2f, order price = R$%.2f, quant. = %d\n", get_trading_system_time(state.now, trading_system_current_week, moving_average_current_minute), state.best_stock_name, state.price, state.order_price, state.quantity); fflush(g_log_file);
			fprintf(g_log_file, "%s - WAIT_SELL: current capital, \tR$%.2f\tbuy price = R$%.2f\tsell price = R$%.2f\n", get_trading_system_time(state.now, trading_system_current_week, moving_average_current_minute), state.capital, state.buy_price, state.sell_price); fflush(g_log_file);
			printf("# %s - traded stock %s,  current capital: R$%.2f,  week return: %+0.2f%%,  accumulated return: %+0.2f%%\n", get_trading_system_time(state.now, trading_system_current_week, moving_average_current_minute), state.best_stock_name, state.capital, 100.0 * (state.capital - state.previous_capital) / state.previous_capital, 100.0 * (state.capital - INITIAL_CAPITAL) / INITIAL_CAPITAL);
			state.state = WAIT_BEGIN_WEEK;
		}
		else if (state.order_result == ORDER_CANCELED)
		{
			fprintf(g_log_file, "%s - WAIT_SELL: sell CANCELED!  \t%s\n", get_trading_system_time(state.now, trading_system_current_week, moving_average_current_minute), state.best_stock_name); fflush(g_log_file);

			state.state = WAIT_MAX_TO_SELL;
		}
		else if (((state.day_current_minute - state.sell_order_minute) > NUM_MINUTES_FOR_ORDER_EXECUTION) || ((state.day_current_minute - state.sell_order_minute) < 0))
		{
			fprintf(g_log_file, "%s - WAIT_SELL: sell time out!    \t%s\n", get_trading_system_time(state.now, trading_system_current_week, moving_average_current_minute), state.best_stock_name); fflush(g_log_file);

			state.sell_cancel_minute = moving_average_current_minute % MINUTES_IN_A_DAY;
			
			state.state = WAIT_SELL_CANCEL;			
		}
		else
		{
			fprintf(g_log_file, "%s - WAIT_SELL: wait sell order, \t%s, price = R$%.2f, order price = R$%.2f, quant. = %d\n", get_trading_system_time(state.now, trading_system_current_week, moving_average_current_minute), state.best_stock_name, state.price, state.order_price, state.quantity); fflush(g_log_file);
		}
	}
		
	if (state.state == WAIT_SELL_CANCEL)
	{
		state.order_result = check_sell_order(&state);
		state.day_current_minute = moving_average_current_minute % MINUTES_IN_A_DAY;

		state.price = g_price_samples[moving_average_current_minute];
		if (state.order_result == ORDER_EXECUTED)
		{
			state.sell_day = get_current_day(target_day_index, moving_average_current_minute);
			state.sell_minute = moving_average_current_minute % MINUTES_IN_A_DAY;
	
			fprintf(g_log_file, "%s - WAIT_SELL_CANCEL: sell order exec.,\t%s price = R$%.2f, order price = R$%.2f, quant. = %d\n", get_trading_system_time(state.now, trading_system_current_week, moving_average_current_minute), state.best_stock_name, state.price, state.order_price, state.quantity); fflush(g_log_file);
			fprintf(g_log_file, "%s - WAIT_SELL_CANCEL: current capital, \tR$%.2f\tbuy price = R$%.2f\tsell price = R$%.2f\n", get_trading_system_time(state.now, trading_system_current_week, moving_average_current_minute), state.capital, state.buy_price, state.sell_price); fflush(g_log_file);
			
			state.state = WAIT_BEGIN_WEEK;
		}
		else if (state.order_result == ORDER_CANCELED)
		{
			fprintf(g_log_file, "%s - WAIT_SELL_CANCEL: sell CANCELED!  \t%s\n", get_trading_system_time(state.now, trading_system_current_week, moving_average_current_minute), state.best_stock_name); fflush(g_log_file);

			state.state = WAIT_MAX_TO_SELL;
		}
		else if (moving_average_current_minute >= (MINUTES_IN_A_WEEK - MINUTES_BEFORE_CLOSING_WEEK)) // Time out para cancelar...
		{
			if (moving_average_current_minute > (MINUTES_IN_A_WEEK - 1)) // pode ocorrer de nao haver negocios na quinta
				moving_average_current_minute = MINUTES_IN_A_WEEK - 1;
				
			fprintf(g_log_file, "%s - WAIT_SELL_CANCEL:\t\ttime out to sell cancel! USER INTERVENTION REQUIRED!\n", get_trading_system_time(state.now, trading_system_current_week, moving_average_current_minute)); fflush(g_log_file);

			state.state = QUIT;
		}
		else if (((state.day_current_minute - state.sell_cancel_minute) > NUM_MINUTES_FOR_ORDER_CANCELATION) || ((state.day_current_minute - state.sell_order_minute) < 0))
		{
			fprintf(g_log_file, "%s - WAIT_SELL_CANCEL: sell cancel time out! USER INTERVENTION REQUIRED! %s\n", get_trading_system_time(state.now, trading_system_current_week, moving_average_current_minute), state.best_stock_name); fflush(g_log_file);

			state.state = QUIT;			
		}
		else
		{
			fprintf(g_log_file, "%s - WAIT_SELL_CANCEL: wait sell cancel,\t%s, price = R$%.2f, order price = R$%.2f, quant. = %d\n", get_trading_system_time(state.now, trading_system_current_week, moving_average_current_minute), state.best_stock_name, state.price, state.order_price, state.quantity); fflush(g_log_file);
		}
	}
		
	if (state.state == WAIT_BEGIN_WEEK)
	{
		if (trading_system_current_week >= g_tt_num_samples)
		{
			trading_system_current_week = g_tt_num_samples - 1; // apenas para conseguir imprimir a data atual no estado QUIT
			state.state = QUIT;
		}
		else if (moving_average_current_minute == (MINUTES_IN_A_WEEK - 1))
		{
			if (state.buy_day != -1)
				generate_graph("GRAFICOS", state.best_stock_name, state.moving_average_first_day, state.buy_day, g_train_data_set[state.buy_day].intraday_price, state.buy_minute, g_train_data_set[target_day_index - DAYS_IN_A_WEEK].clo, BUY);
			if (state.sell_day != -1)
				generate_graph("GRAFICOS", state.best_stock_name, state.moving_average_first_day, state.sell_day, g_train_data_set[state.sell_day].intraday_price, state.sell_minute, state.best_stock_minimum_sell_price, SELL);
		}
		else if (moving_average_current_minute < state.previous_moving_average_current_minute)
		{
			fprintf(g_log_file, "%s - WAIT_BEGIN_WEEK: starting a week.\n", get_trading_system_time(state.now, trading_system_current_week, moving_average_current_minute)); fflush(g_log_file);

			state.state = SELECT_STOCK;
		}
	}
		
	if (state.state == SELECT_STOCK)
	{
		strcpy (state.best_stock_name, get_best_stock_name(trading_system_current_week));
		state.best_stock_maximum_buy_price = get_best_stock_maximum_buy_price(target_day_index, trading_system_current_week);
		state.best_stock_predicted_return = get_best_stock_predicted_return(trading_system_current_week);

		state.moving_average_first_day = target_day_index - 4;

		fprintf(g_log_file, "%s - SELECT_STOCK: best stock \t%s, max. buy price = R$%.2f\n", get_trading_system_time(state.now, trading_system_current_week, moving_average_current_minute), state.best_stock_name, state.best_stock_maximum_buy_price); fflush(g_log_file);

		state.state = WAIT_MOVING_AVERAGE;
	}

	if (state.state == QUIT)
	{
		fprintf(g_log_file, "%s - QUIT:  Final Capital\t\tR$%.2f  accumulated return: %.2f%%\n", get_trading_system_time(state.now, trading_system_current_week, moving_average_current_minute), state.capital, 100.0 * (state.capital - INITIAL_CAPITAL) / INITIAL_CAPITAL); fflush(g_log_file);
		printf("### %s - QUIT: Final Capital\t\tR$%.2f\t accumulated return: %.2f%%\n", get_trading_system_time(state.now, trading_system_current_week, moving_average_current_minute), state.capital, 100.0 * (state.capital - INITIAL_CAPITAL) / INITIAL_CAPITAL); fflush(g_log_file);
		
		fclose(g_log_file);
	}

	state.previous_moving_average_current_minute = moving_average_current_minute;
	
	save_state(state);
	
	if (g_stop_trading_system == 1)
	{
		printf("Trading System stopped!!!\n");
#ifdef	USE_IPC
		IPC_disconnect();
#endif
		exit(0);
	}
	

#ifdef	USE_IPC
#ifdef	IPC_ON_LINE
	printf("%s - %s: %s price = R$%.2f\n", get_trading_system_time(state.now, trading_system_current_week, moving_average_current_minute), g_state_string[state.state], state.best_stock_name, g_price_samples[moving_average_current_minute]); 
#endif
	send_state(state);
#endif
	return (state.state);
}
