#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#include <timeutil.h> 
#include <math.h>
#ifdef	USE_IPC
#include "ipc.h"
#include "messages.h"
#endif
#include "trading_system.h"


STATE g_state;
extern double g_best_stock_price[MINUTES_IN_A_WEEK];
extern double g_ref_stock_price[MINUTES_IN_A_WEEK];
long g_trading_system_current_day_date_l;
int g_moving_average_current_minute;

void
Error(char *mess1, char *mess2, char *mess3)
{
	printf("%s%s%s\n", mess1, mess2, mess3);
	exit(1);
}


void
get_state_file_content()
{
	FILE *state_file;

	if ((state_file = fopen(STATE_FILE_NAME, "r")) == NULL)
		Error("Could not open state file ", STATE_FILE_NAME, " in get_state_file_content()");

	fread(&g_trading_system_current_day_date_l, 	sizeof(long), 1, state_file);
	fread(&g_moving_average_current_minute, 	sizeof(int), 1, state_file);

	fread(g_best_stock_price, sizeof(double), g_moving_average_current_minute + 1, state_file);
	fread(g_ref_stock_price, sizeof(double), g_moving_average_current_minute + 1, state_file);
		
	fread(&g_state, sizeof(STATE), 1, state_file);

	fclose(state_file); 
}


void
daily_data_repair()
{
	int i;
	
	for (i = 0; i <= g_moving_average_current_minute; i++)
	{
		g_best_stock_price[i] = g_state.best_stock_price;
		g_ref_stock_price[i] = g_state.ref_stock_price;
	}
}



int 
main ()
{
	/* Locale Setting */
	setlocale (LC_ALL, "C");

	get_state_file_content();

	print_state(stdout, g_state, g_best_stock_price, g_ref_stock_price);
	plot_graph(TO_FILE, g_best_stock_price, g_ref_stock_price, "graph.png");
	
	//g_state.state = WAIT_FIRST_ORDERS;
	//g_state.best_stock_base_price = g_state.best_stock_sell_price = 63.77;
        //g_state.ref_stock_base_price = g_state.ref_stock_buy_price = 24.94;
	//g_state.first_orders_day_date_l = g_trading_system_current_day_date_l;
        //g_state.first_orders_minute = 60+49;
	//g_state.previous_maximum_delta = g_state.moving_average_of_delta;
	//daily_data_repair();

	//save_state(g_state);
	
	return (0);
}
