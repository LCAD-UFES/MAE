#include "forms.h"
#include "trading_system_control.h"
#include "mae.h"
#include "../wnn_pred.h"
#include "ipc.h"
#include "messages.h"
#include "trading_system.h"


STATE state;
double *price_samples = NULL;


void
clear_form()
{
	char *aux;
	
	aux = "";
	fl_set_object_label(state_val, aux);
	fl_set_object_label(date_and_time_val, aux);
	fl_set_object_label(best_stock_name_val, aux);
	fl_set_object_label(price_val, aux);
	fl_set_object_label(moving_average_price_val, aux);
	fl_set_object_label(predicted_return_val, aux);
	fl_set_object_label(order_price_val, aux);
	fl_set_object_label(quantity_val, aux);
	fl_set_object_label(maximum_buy_price_val, aux);
	fl_set_object_label(previous_minimum_intraday_price_val, aux);
	fl_set_object_label(possible_buy_order_price_val, aux);
	fl_set_object_label(order_price_val, aux);
	fl_set_object_label(buy_price_val, aux);
	fl_set_object_label(minimum_sell_price_val, aux);
	fl_set_object_label(previous_maximum_intraday_price_val, aux);
	fl_set_object_label(possible_sell_order_price_val, aux);
	fl_set_object_label(current_return_val, aux);
	fl_set_object_label(buy_price_val, aux);
	fl_set_object_label(sell_price_val, aux);
	fl_set_object_label(final_return_val, aux);
}


void
update_interface()
{
	char aux[1000];
	
	fl_freeze_form(Trading_System);
	clear_form();
	
	fl_set_object_label(state_val, g_state_string[state.state]);
	fl_set_object_label(date_and_time_val, state.now);
	fl_set_object_label(best_stock_name_val, state.best_stock_name);
	sprintf(aux, "R$%.2f", state.stock_price);
	fl_set_object_label(price_val, aux);
	sprintf(aux, "R$%.2f", state.moving_average_price);
	fl_set_object_label(moving_average_price_val, aux);
	sprintf(aux, "%f%%", 100.0 * state.best_stock_predicted_return);
	fl_set_object_label(predicted_return_val, aux);
	
	switch(state.state)
	{
		case SEND_BUY_ORDER:
		case WAIT_BUY:
		case WAIT_BUY_CANCEL:
			sprintf(aux, "R$%.2f", state.order_price);
			fl_set_object_label(order_price_val, aux);
			sprintf(aux, "%d", state.quantity);
			fl_set_object_label(quantity_val, aux);
			
		case WAIT_MIN_TO_BUY:
			sprintf(aux, "R$%.2f", state.best_stock_maximum_buy_price);
			fl_set_object_label(maximum_buy_price_val, aux);
			sprintf(aux, "R$%.2f", state.previous_minimum_intraday_price);
			fl_set_object_label(previous_minimum_intraday_price_val, aux);
			sprintf(aux, "R$%.2f", state.previous_minimum_intraday_price * (1.0 + TRADING_SYSTEM_BUY_SELL_DELTA_RETURN));
			fl_set_object_label(possible_buy_order_price_val, aux);
			break;
			       
		case SEND_SELL_ORDER:
		case WAIT_SELL:
		case WAIT_SELL_CANCEL:
			sprintf(aux, "R$%.2f", state.order_price);
			fl_set_object_label(order_price_val, aux);
			sprintf(aux, "%d", state.quantity);
			fl_set_object_label(quantity_val, aux);
			
		case WAIT_MAX_TO_SELL:
			sprintf(aux, "R$%.2f", state.buy_price);
			fl_set_object_label(buy_price_val, aux);
			sprintf(aux, "R$%.2f", state.best_stock_minimum_sell_price);
			fl_set_object_label(minimum_sell_price_val, aux);
			sprintf(aux, "R$%.2f", state.previous_maximum_intraday_price);
			fl_set_object_label(previous_maximum_intraday_price_val, aux);
			sprintf(aux, "R$%.2f", state.previous_maximum_intraday_price * (1.0 - TRADING_SYSTEM_BUY_SELL_DELTA_RETURN));
			fl_set_object_label(possible_sell_order_price_val, aux);
			
			sprintf(aux, "%f%%", 100.0 * (state.stock_price - state.buy_price) / state.buy_price);
			fl_set_object_label(current_return_val, aux);
			break;
			       
		case WAIT_BEGIN_WEEK:
			sprintf(aux, "R$%.2f", state.buy_price);
			fl_set_object_label(buy_price_val, aux);
			sprintf(aux, "R$%.2f", state.sell_price);
			fl_set_object_label(sell_price_val, aux);
			
			sprintf(aux, "%f%%", 100.0 * (state.sell_price - state.buy_price) / state.buy_price);
			fl_set_object_label(final_return_val, aux);
			break;
	}
	fl_unfreeze_form(Trading_System);
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
	
	state = *((STATE*) received_state->state);
	if (price_samples == NULL)
		price_samples = (double *) malloc(sizeof(double) * MINUTES_IN_A_WEEK);
		
	for (i = 0; i < MINUTES_IN_A_WEEK; i++)
		price_samples[i] = received_state->price_samples[i];
	
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
