/* ********************************************************
* PROJETO: Trading System
*	   ipc_trading_system	
*
* ARQUIVO: ipc_trading_system.c		
*
* DATA: 28/01/2011
*
* REFERENCIA: messages.h
*
* RESUMO 
*	      Publishes: order, state, control.
*             Subscreve a: quotes_server_order_router (quotes_server_order_router.c) quotes, order execution, control.
*             Funcao: 
*
***********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include "ipc.h"
#include "messages.h"
#include "trading_system.h"



void 
quotes_handler(MSG_INSTANCE msgRef, void *callData, void *clientData)
{
	qsor_quotes_message* received_quotes;
	FORMATTER_PTR formatter;

	received_quotes = (qsor_quotes_message *) callData;
	
	/* Get the formatter for this message instance */
	formatter = IPC_msgInstanceFormatter(msgRef);
	// IPC_printData(formatter, stdout, received_quotes);
	
#ifdef	USE_IPC
	if (receive_quote(received_quotes))
		call_trading_system_state_machine();
#endif

	IPC_freeData(formatter, callData);
}



void 
multi_order_execution_handler (MSG_INSTANCE msgRef, void *callData, void *clientData)
{
	qsor_multi_order_execution_message *received_multi_order_execution;
	FORMATTER_PTR formatter;
	
	// printf("\nmulti_order_execution_handler(): Receiving message %s and message size: %d .\n", IPC_msgInstanceName(msgRef), IPC_dataLength(msgRef));

	received_multi_order_execution = (qsor_multi_order_execution_message *) callData;
	
	/* Get the formatter for this message instance */ 
	formatter = IPC_msgInstanceFormatter(msgRef);
	// IPC_printData(formatter, stdout, received_multi_order_execution);
	
#ifdef	USE_IPC
	receive_multi_order_execution(received_multi_order_execution);
#endif

	IPC_freeData(formatter, callData);
}



void 
control_handler (MSG_INSTANCE msgRef, void *callData, void *clientData)
{
	ts_control_message *received_control;
	FORMATTER_PTR formatter;

	// printf("\control_handler: Receiving message %s and message size: %d .\n", IPC_msgInstanceName(msgRef), IPC_dataLength(msgRef));
  
  	received_control = (ts_control_message *) callData;

	/* Get the formatter for this message instance */
	formatter = IPC_msgInstanceFormatter(msgRef);
	// IPC_printData(formatter, stdout, received_control);
	
	if (received_control->type == 1) // serious error: all stop
	{
		printf("%s - Serious error! Error message: %s\n", received_control->date_and_time, received_control->description);
		printf("System exit!\n");
		IPC_disconnect();
		exit(1);
	}
	else if (received_control->type == 2) // graciously stop
	{
		printf("%s - Stop trading system request! Associated message: %s\n", received_control->date_and_time, received_control->description);
		g_stop_trading_system = 1;
	}
	else  if (received_control->type == 3) // buy/sell order
	{
		if ((received_control->description[0] == 'B') && ((g_state.state == WAIT_MOVING_AVERAGE) || (g_state.state == WAIT_FIRST_ORDERS_TRIGGER)))
		{
			g_state.state = SEND_FIRST_ORDERS;
		}
		else if ((received_control->description[0] == 'S') && (g_state.state == WAIT_SECOND_ORDERS_TRIGGER))
		{
			//g_state.best_stock_order_price = g_state.best_stock_price = g_best_stock_price[g_moving_average_current_minute];
			//g_state.ref_stock_order_price = g_state.ref_stock_price = g_ref_stock_price[g_moving_average_current_minute];
			g_state.state = SEND_SECOND_ORDERS;
		}
		else
		{
			fprintf(stderr, "Could not accept the buy or sell order: %s\n", received_control->description);
		}
		save_state(g_state);
	}
	else
	{
		printf("%s - Unknown control message type %d! Associated text: %s\n", received_control->date_and_time, received_control->type, received_control->description);
	}

	IPC_freeData(formatter, callData);
	call_trading_system_state_machine();
}



void 
stdinHnd (int fd, void *clientData)
{
	char inputLine[81];

	fgets(inputLine, 80, stdin);

	switch (inputLine[0]) 
	{
		case 'q': 
		case 'Q': 
#ifdef	USE_IPC
			g_stop_trading_system = 1;
			break;
		default: 
#endif
			printf("%s: unknown key pressed = %s", (char *)clientData, inputLine);
			fflush(stdout);
	}
}



void 
send_state_timer(void *string, unsigned long currentTime, unsigned long scheduledTime)
{
#ifdef	USE_IPC
	send_state(&g_state);
#endif
}



int
main (int argc, char **argv)
{	
	/* Locale setting */
	setlocale (LC_ALL, "C");	
	/* Connect the trading_system to the central server */
	printf("IPC_connect(%s)\n", TS_NAME);
	IPC_connect(TS_NAME);

	/* Define the named formats that the modules need */
	IPC_defineFormat(QSOR_QUOTES_MESSAGE_NAME, QSOR_QUOTES_MESSAGE_FORMAT);
	IPC_defineFormat(QSOR_MULTI_ORDER_EXECUTION_MESSAGE_NAME, QSOR_MULTI_ORDER_EXECUTION_MESSAGE_FORMAT);
	IPC_defineFormat(TS_MULTI_ORDER_MESSAGE_NAME, TS_MULTI_ORDER_MESSAGE_FORMAT);
	IPC_defineFormat(TS_CONTROL_MESSAGE_NAME, TS_CONTROL_MESSAGE_FORMAT);
	IPC_defineFormat(TS_STATE_MESSAGE_NAME, TS_STATE_MESSAGE_FORMAT);

	/* Define the messages that this module publishes */
	IPC_defineMsg(TS_MULTI_ORDER_MESSAGE_NAME, IPC_VARIABLE_LENGTH, TS_MULTI_ORDER_MESSAGE_FORMAT);
	IPC_defineMsg(TS_STATE_MESSAGE_NAME, IPC_VARIABLE_LENGTH, TS_STATE_MESSAGE_FORMAT);
	IPC_defineMsg(TS_CONTROL_MESSAGE_NAME, IPC_VARIABLE_LENGTH, TS_CONTROL_MESSAGE_FORMAT);

	/* Subscribe to the messages that this module listens to. */
	IPC_subscribeData(QSOR_QUOTES_MESSAGE_NAME, quotes_handler, NULL);
	IPC_subscribeData(QSOR_MULTI_ORDER_EXECUTION_MESSAGE_NAME, multi_order_execution_handler, NULL);
	IPC_subscribeData(TS_CONTROL_MESSAGE_NAME, control_handler, TS_NAME);

	/* Subscribe for tty input. Typing "q" will quit the program. */
	IPC_subscribeFD(fileno(stdin), stdinHnd, TS_NAME);
	
	/* Add timer for sending the state */
	IPC_addTimer(TIME_BETWEEN_STATE_MESSAGES, TRIGGER_FOREVER, send_state_timer, NULL);

#ifdef	USE_IPC
	init_trading_system(argc, argv);
#endif	
	printf("\nType 'q' to quit\n");

	IPC_dispatch();

	IPC_disconnect();

	return 0;
}

