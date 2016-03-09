#include <stdlib.h>
#include <locale.h>
#include "forms.h"
#include "trading_system_control.h"
#include "ipc.h"
#include "messages.h"
#include "trading_system.h"


STATE g_state;
extern double g_best_stock_price[MINUTES_IN_A_WEEK];
extern double g_ref_stock_price[MINUTES_IN_A_WEEK];


void
clear_form()
{
	char *aux;
	
	aux = "";
	fl_set_object_label(state_val, aux);
	fl_set_object_label(date_and_time_val, aux);
	fl_set_object_label(best_stock_name_val, aux);
	fl_set_object_label(ref_stock_name_val, aux);
	fl_set_object_label(best_stock_price_val, aux);
	fl_set_object_label(ref_stock_price_val, aux);
	fl_set_object_label(predicted_week_return_val, aux);
	fl_set_object_label(current_week_return_val, aux);
	fl_set_object_label(current_week_gross_return_val, aux);
	fl_set_object_label(moving_average_of_delta_val, aux);
	fl_set_object_label(trade_order_val, aux);
	fl_set_object_label(first_orders_trigger_moving_average_of_delta_val, aux);
	fl_set_object_label(best_stock_first_order_quantity_val, aux);
	fl_set_object_label(best_stock_first_order_price_val, aux);
	fl_set_object_label(final_week_return_val, aux);
	fl_set_object_label(second_orders_trigger_moving_average_of_delta_val, aux);
	fl_set_object_label(ref_stock_first_order_quantity_val, aux);
	fl_set_object_label(ref_stock_first_order_price_val, aux);
	fl_set_object_label(best_stock_name_val2, aux);
	fl_set_object_label(ref_stock_name_val2, aux);
	fl_set_object_label(best_stock_second_order_quantity_val, aux);
	fl_set_object_label(best_stock_second_order_price_val, aux);
	fl_set_object_label(ref_stock_second_order_quantity_val, aux);
	fl_set_object_label(ref_stock_second_order_price_val, aux);
	fl_set_object_label(best_stock_name_val3, aux);
	fl_set_object_label(ref_stock_name_val3, aux);
}


void
update_interface()
{
	char aux[1000];
	FILE *output_file;
	
	fl_freeze_form(Trading_System);
	clear_form();
	
	fl_set_object_label(state_val, g_state_string[g_state.state]);
	fl_set_object_label(date_and_time_val, g_state.now);
	
	sprintf(aux, "%.2lf%%", 100.0 * g_state.predicted_return);
	fl_set_object_label(predicted_week_return_val, aux);
	
	fl_set_object_label(best_stock_name_val, g_state.best_stock_name);
	fl_set_object_label(best_stock_name_val2, g_state.best_stock_name);
	fl_set_object_label(best_stock_name_val3, g_state.best_stock_name);
	fl_set_object_label(ref_stock_name_val, g_state.ref_stock_name);
	fl_set_object_label(ref_stock_name_val2, g_state.ref_stock_name);
	fl_set_object_label(ref_stock_name_val3, g_state.ref_stock_name);
	
	sprintf(aux, "R$%.2lf", g_state.best_stock_price);
	fl_set_object_label(best_stock_price_val, aux);
	sprintf(aux, "R$%.2lf", g_state.ref_stock_price);
	fl_set_object_label(ref_stock_price_val, aux);
	
	sprintf(aux, "%s", ((g_state.trade_order == BUY_SELL)? "BUY_SELL": "SELL_BUY"));
	fl_set_object_label(trade_order_val, aux);
	sprintf(aux, "%.4lf", g_state.moving_average_of_delta);
	fl_set_object_label(moving_average_of_delta_val, aux);

	switch (g_state.state)
	{
		case WAIT_FIRST_ORDERS_TRIGGER:
			sprintf(aux, "%.4lf", g_state.previous_minimum_delta + (2.0 * TRADING_SYSTEM_BUY_SELL_DELTA_RETURN) / LEVERAGE);
			fl_set_object_label(first_orders_trigger_moving_average_of_delta_val, aux);
			break;
			       
		case SEND_FIRST_ORDERS:
		case WAIT_FIRST_ORDERS:
		case WAIT_FIRST_ORDERS_CANCEL:
			sprintf(aux, "R$%.2lf", g_state.best_stock_order_price);
			fl_set_object_label(best_stock_first_order_price_val, aux);
			sprintf(aux, "%d", g_state.best_stock_quantity);
			fl_set_object_label(best_stock_first_order_quantity_val, aux);
			sprintf(aux, "R$%.2lf", g_state.ref_stock_order_price);
			fl_set_object_label(ref_stock_first_order_price_val, aux);
			sprintf(aux, "%d", g_state.ref_stock_quantity);
			fl_set_object_label(ref_stock_first_order_quantity_val, aux);
			break;
			
		case WAIT_SECOND_ORDERS_TRIGGER:
			sprintf(aux, "%.2lf%%", 100.0 * compute_current_week_return(&g_state));
			fl_set_object_label(current_week_return_val, aux);
			sprintf(aux, "%.2lf%%", 100.0 * compute_current_week_gross_return(&g_state));
			fl_set_object_label(current_week_gross_return_val, aux);

			sprintf(aux, "R$%.2lf", ((g_state.trade_order == BUY_SELL)? g_state.best_stock_buy_price: g_state.best_stock_sell_price));
			fl_set_object_label(best_stock_first_order_price_val, aux);
			sprintf(aux, "%d", g_state.best_stock_quantity);
			fl_set_object_label(best_stock_first_order_quantity_val, aux);
			sprintf(aux, "R$%.2lf", ((g_state.trade_order == SELL_BUY)? g_state.ref_stock_buy_price: g_state.ref_stock_sell_price));
			fl_set_object_label(ref_stock_first_order_price_val, aux);
			sprintf(aux, "%d", g_state.ref_stock_quantity);
			fl_set_object_label(ref_stock_first_order_quantity_val, aux);
			
			sprintf(aux, "%.4lf", slow_delta_moving_average(&g_state, g_best_stock_price, g_ref_stock_price, g_state.moving_average_current_minute));
			fl_set_object_label(second_orders_trigger_moving_average_of_delta_val, aux);
			break;
			       
		case SEND_SECOND_ORDERS:
		case WAIT_SECOND_ORDERS:
		case WAIT_SECOND_ORDERS_CANCEL:
			sprintf(aux, "%.2lf%%", 100.0 * compute_current_week_return(&g_state));
			fl_set_object_label(current_week_return_val, aux);
			sprintf(aux, "%.2lf%%", 100.0 * compute_current_week_gross_return(&g_state));
			fl_set_object_label(current_week_gross_return_val, aux);

			sprintf(aux, "R$%.2lf", ((g_state.trade_order == BUY_SELL)? g_state.best_stock_buy_price: g_state.best_stock_sell_price));
			fl_set_object_label(best_stock_first_order_price_val, aux);
			sprintf(aux, "R$%.2lf", ((g_state.trade_order == SELL_BUY)? g_state.ref_stock_buy_price: g_state.ref_stock_sell_price));
			fl_set_object_label(ref_stock_first_order_price_val, aux);
			
			sprintf(aux, "R$%.2lf", g_state.best_stock_order_price);
			fl_set_object_label(best_stock_second_order_price_val, aux);
			sprintf(aux, "%d", g_state.best_stock_quantity);
			fl_set_object_label(best_stock_second_order_quantity_val, aux);
			sprintf(aux, "R$%.2lf", g_state.ref_stock_order_price);
			fl_set_object_label(ref_stock_second_order_price_val, aux);
			sprintf(aux, "%d", g_state.ref_stock_quantity);
			fl_set_object_label(ref_stock_second_order_quantity_val, aux);
			break;
			
		case WAIT_BEGIN_WEEK:
		case QUIT:
			sprintf(aux, "R$%.2lf", ((g_state.trade_order == BUY_SELL)? g_state.best_stock_buy_price: g_state.best_stock_sell_price));
			fl_set_object_label(best_stock_first_order_price_val, aux);
			sprintf(aux, "R$%.2lf", ((g_state.trade_order == SELL_BUY)? g_state.ref_stock_buy_price: g_state.ref_stock_sell_price));
			fl_set_object_label(ref_stock_first_order_price_val, aux);
			
			sprintf(aux, "R$%.2lf", ((g_state.trade_order == SELL_BUY)? g_state.best_stock_buy_price: g_state.best_stock_sell_price));
			fl_set_object_label(best_stock_second_order_price_val, aux);
			sprintf(aux, "%d", g_state.best_stock_quantity);
			fl_set_object_label(best_stock_second_order_quantity_val, aux);
			sprintf(aux, "R$%.2lf", ((g_state.trade_order == BUY_SELL)? g_state.ref_stock_buy_price: g_state.ref_stock_sell_price));
			fl_set_object_label(ref_stock_second_order_price_val, aux);
			sprintf(aux, "%d", g_state.ref_stock_quantity);
			fl_set_object_label(ref_stock_second_order_quantity_val, aux);
						
			sprintf(aux, "%.2lf%%", 100.0 * compute_final_week_return(&g_state));
			fl_set_object_label(final_week_return_val, aux);
			break;
	}
	fl_unfreeze_form(Trading_System);

	output_file = fopen("/var/www/html/state.txt", "w");	
	print_state(output_file, g_state, g_best_stock_price, g_ref_stock_price);
	plot_graph(TO_FILE, g_best_stock_price, g_ref_stock_price, "/var/www/html/graph.png");
	fclose(output_file); 
}


/******************* State Handler ***********************/
void 
state_handler (MSG_INSTANCE msgRef, void *callData, void *clientData)
{
	ts_state_message* received_state;
	FORMATTER_PTR formatter;
	int i;
	
	received_state = (ts_state_message*) callData;
	/* Get the formatter for this message instance */
	formatter = IPC_msgInstanceFormatter(msgRef);
	
	g_state = *((STATE*) received_state->state);
		
	for (i = 0; i < MINUTES_IN_A_WEEK; i++)
	{
		g_best_stock_price[i] = received_state->best_stock_price[i];
		g_ref_stock_price[i] = received_state->ref_stock_price[i];
	}
	
	update_interface();
	
	IPC_freeData(formatter, callData);
}


int 
xform_idle_callback(XEvent *ev, void *data)
{
	IPC_handleMessage(1);
	return 0;
}


int
run_at_application_close(FL_FORM *form, void *data)
{
	system("pkill gnuplot");
	return (FL_OK);
}


int 
main(int argc, char *argv[])
{
	/* Locale Setting */
	setlocale (LC_ALL, "C");

	/* IPC initialization */
	/* Connect trading_system_control to the central server */
	printf("\nIPC_connect(%s)\n", TSCONTROL_NAME);
	if (argc == 2)
		IPC_connectModule(TSCONTROL_NAME, argv[1]);
	else
		IPC_connect(TSCONTROL_NAME);

	IPC_defineFormat(TS_CONTROL_MESSAGE_NAME, TS_CONTROL_MESSAGE_FORMAT);
	IPC_defineFormat(TS_STATE_MESSAGE_NAME, TS_STATE_MESSAGE_FORMAT);

	/* Define the messages that this module publishes */
	IPC_defineMsg(TS_CONTROL_MESSAGE_NAME, IPC_VARIABLE_LENGTH, TS_CONTROL_MESSAGE_FORMAT);

	/* Subscribe to the messages that this module listens to. */
	IPC_subscribeData(TS_STATE_MESSAGE_NAME, state_handler, TSCONTROL_NAME);
	/* End of IPC initialization */
	
	read_trading_system_parameters();
	
	/* xforms initialization */
	fl_initialize(&argc, argv, 0, 0, 0);

	create_the_forms();

	/* fill-in form initialization code */

	/* show the first form */
	fl_show_form(Trading_System,FL_PLACE_CENTER,FL_FULLBORDER,"Trading_System");
	
	fl_set_idle_callback(xform_idle_callback, NULL);
	
	fl_set_atclose(run_at_application_close, NULL);
	
	fl_do_forms();
	/* End xforms initialization */
	
	IPC_disconnect();
	
	return 0;
}
