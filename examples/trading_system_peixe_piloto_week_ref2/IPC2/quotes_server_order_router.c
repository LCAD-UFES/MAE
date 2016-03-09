/* ********************************************************
* PROJETO: Trading System
*	   quotes_server_order_router
*
* ARQUIVO: quotes_server_order_router.c		
*
* DATA: 28/01/2011
*
* REFERENCIA: messages.h
*
* RESUMO 
*	      Publishes: quotes, order execution, control.
*             Subscreve a: ipc_trading_system (ipc_trading_system.c) order, state, control.
*             
*
***********************************************************/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <locale.h>
#include "ipc.h"
#include "messages.h"
#include "trading_system.h"

unsigned long x_ipc_timeInMsecs(void);
unsigned long realStart;



/*
*********************************************************************************
* Print error and terminate							*
*********************************************************************************
*/

void
Erro(char *msg1, char *msg2, char *msg3)
{
	char description[1000];
	time_t now;
	struct tm *brokentime;

	fprintf(stderr, "Error: %s%s%s\n", msg1, msg2, msg3);
	
	sprintf(description, "Error: %s%s%s\n", msg1, msg2, msg3);
	now = time(NULL);
	brokentime = localtime(&now);
	publish_control_message(1, description, asctime(brokentime), QSOR_NAME);
}



/******************** Order Handler **********************/
void 
multi_order_handler (MSG_INSTANCE msgRef, void *callData, void *clientData)
{
	FILE *multi_order_file;
	FILE *control_file;
	FORMATTER_PTR formatter;
	ts_multi_order_message* received_multi_order;
	int i;
	
	if (fileExists(multi_orderControltxt) == 1)
		Erro("New multi_order received but previous multi_order control file ", multi_orderControltxt, " still not deleted (in multi_order_handler())");

	received_multi_order = (ts_multi_order_message *) callData;

	formatter = IPC_msgInstanceFormatter(msgRef);
	// IPC_printData(formatter, stdout, received_multi_order);
	
	/* build the file with the multi_order struct */
	if ((multi_order_file = fopen(multi_ordertxt, "w")) != NULL) 		
	{	
		fprintf(multi_order_file, "%d\n", received_multi_order->num_orders);
		for (i = 0; i < received_multi_order->num_orders; i++)
		{
			/* multi_order type buy == 1, sell == 0 */
			fprintf(multi_order_file, "%d\n", received_multi_order->buy[i]);
			fprintf(multi_order_file, "%s\n", received_multi_order->stock_symbol[i]);
			fprintf(multi_order_file, "%lf\n", received_multi_order->price[i]);
			fprintf(multi_order_file, "%d\n", received_multi_order->quantity[i]);
		}
		fprintf(multi_order_file, "%s\n", received_multi_order->date_and_time);
		fprintf(multi_order_file, "%s\n", received_multi_order->host);
		
		fclose(multi_order_file);
	}
	else
	{
		Erro("Could not open multi_order file ", multi_ordertxt, " in multi_order_handler()");
	}
	
	/* build the control file */
	if ((control_file = fopen(multi_orderControltxt, "w")) != NULL)
	{
		fprintf (control_file, "valid multi_order available\n");
		
		fclose(control_file);	
	}					
	else
	{
		Erro("Could not open multi_order control file ", multi_orderControltxt, " in multi_order_handler()");
	}

	IPC_freeData(formatter, callData);

#ifdef	IPC_TEST
	char command[1000];

	strcpy(command, "cp ");
	strcat(command, multi_ordertxt);
	strcat(command, " ");
	strcat(command, multi_orderExecutiontxt);
	system(command);

	strcpy(command, "cp ");
	strcat(command, multi_orderControltxt);
	strcat(command, " ");
	strcat(command, multi_orderExecutionControltxt);
	system(command);


	strcpy(command, "rm -f ");
	strcat(command, multi_ordertxt);
	system(command);
	strcpy(command, "rm -f ");
	strcat(command, multi_orderControltxt);
	system(command);
#endif
}



/******************* State Handler **********************
void 
state_handler (MSG_INSTANCE msgRef, void *callData, void *clientData)
{
	ts_state_message* received_state;
	FORMATTER_PTR formatter;
	STATE state;
	double *stock_price;
	double *ref_price;

	received_state = (ts_state_message*) callData;

	formatter = IPC_msgInstanceFormatter(msgRef);

	state = *((STATE*) received_state->state);
	stock_price = received_state->stock_price;
	ref_price = received_state->ref_price;
	
	IPC_freeData(formatter, callData);
}
*/


/******************** Control Handler ********************/
void 
control_handler (MSG_INSTANCE msgRef, void *callData, void *clientData)
{
	
	ts_control_message *received_control;
	FORMATTER_PTR formatter;
	char command[1000];

	// printf("\control_handler: Receiving message %s and message size: %d .\n", IPC_msgInstanceName(msgRef), IPC_dataLength(msgRef));
  
  	received_control = (ts_control_message *) callData;

	/* Get the formatter for this message instance */
	formatter = IPC_msgInstanceFormatter(msgRef);
	// IPC_printData(formatter, stdout, received_control);
	
	if (received_control->type == 1) // serious error: all stop
	{
		printf("%s - Serious error! Error message: %s\n", received_control->date_and_time, received_control->description);
		printf("System exit!\n");

		strcpy(command, "rm -f ");
		strcat(command, alarmtxt);
		system(command);
		strcpy(command, "rm -f ");
		strcat(command, alarmControltxt);
		system(command);

		IPC_disconnect();
		exit(1);
	}
	else if (received_control->type == 2) // graciously stop
	{
		printf("%s - Stop trading system request! Associated message: %s\n", received_control->date_and_time, received_control->description);

		strcpy(command, "rm -f ");
		strcat(command, alarmtxt);
		system(command);
		strcpy(command, "rm -f ");
		strcat(command, alarmControltxt);
		system(command);

		IPC_disconnect();
		exit(1);
	}
	else if (received_control->type == 3) // buy/sell control order
	{
		fprintf(stderr, "Control buy/sell order received: %s\n", received_control->description);
	}
	else 
	{
		printf("%s - Unknown control message type %d! Associated text: %s\n", received_control->date_and_time, received_control->type, received_control->description);

		strcpy(command, "rm -f ");
		strcat(command, alarmtxt);
		system(command);
		strcpy(command, "rm -f ");
		strcat(command, alarmControltxt);
		system(command);
	}

	IPC_freeData(formatter, callData);
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
			IPC_disconnect();
			exit(0);

		default: 
			printf("stdinHnd [%s]: Received %s", (char *)clientData, inputLine);
			fflush(stdout);
	}
}



void 
handlerChangeHnd (const char *msgName, int num, void *clientData)
{
 	fprintf(stderr, "HANDLER CHANGE: %s: %d\n", msgName, num);
}



void 
handlerChangeHnd2 (const char *msgName, int num, void *clientData)
{
	fprintf(stderr, "HANDLER CHANGE2: %s: %d\n", msgName, num);
}



void 
connect1Hnd (const char *moduleName, void *clientData)
{
	fprintf(stderr, "CONNECT1: Connection from %s\n", moduleName);
	fprintf(stderr, "          Confirming connection (%d)\n", IPC_isModuleConnected(moduleName));

}



void 
connect2Hnd (const char *moduleName, void *clientData)
{
	fprintf(stderr, "CONNECT2: Connection from %s\n", moduleName);
	fprintf(stderr, "          Number of handlers quotes: %d\n", IPC_numHandlers("quotes"));
	fprintf(stderr, "          Number of handlers orderExecution: %d\n", IPC_numHandlers("orderExecution"));

}



void 
disconnect1Hnd (const char *moduleName, void *clientData)
{
	static int first = 1;

	fprintf(stderr, "DISCONNECT: %s\n", moduleName);
	if (first) 
		IPC_unsubscribeConnect(connect1Hnd);
	else 
		IPC_unsubscribeConnect(connect2Hnd);
		
	if (first)
	{
		IPC_unsubscribeHandlerChange("quotes", handlerChangeHnd2);
		IPC_unsubscribeHandlerChange("orderExecution", handlerChangeHnd2);
	}
	else
	{
		IPC_unsubscribeHandlerChange("quotes", handlerChangeHnd);
		IPC_unsubscribeHandlerChange("orderExecuion", handlerChangeHnd);
	}
	first = 0;
}



int 
publish_multi_order_execution_message(char *moduleName)
{
	char strBuffer[5000];
#ifdef	IPC_ON_LINE
	char command[100];
#endif
	FILE *multi_order_execution_file;	
	qsor_multi_order_execution_message multi_order_execution_message;
	int i;

	/* Rotine to check the directory and if control.txt exists publish the multi_orderExecution data */
	if (fileExists(multi_orderExecutionControltxt) == 1) 
	{
		if ((multi_order_execution_file = fopen(multi_orderExecutiontxt, "r")) == NULL)
			Erro("Could not open multi_order execution file ", multi_orderExecutiontxt, " in publish_multi_order_execution_message().");

		fgets(strBuffer, 5000, multi_order_execution_file);
		multi_order_execution_message.num_orders = atoi(strBuffer);
		multi_order_execution_message.buy = (int *) malloc(multi_order_execution_message.num_orders * sizeof(int));
		multi_order_execution_message.stock_symbol = (char **) malloc(multi_order_execution_message.num_orders * sizeof(char *));
		multi_order_execution_message.price = (double *) malloc(multi_order_execution_message.num_orders * sizeof(double));
		multi_order_execution_message.quantity = (int *) malloc(multi_order_execution_message.num_orders * sizeof(int));
		for (i = 0; i < multi_order_execution_message.num_orders; i++)
		{
			fgets(strBuffer, 5000, multi_order_execution_file);
			multi_order_execution_message.buy[i] = atoi(strBuffer);

			fgets(strBuffer, 5000, multi_order_execution_file);
			multi_order_execution_message.stock_symbol[i] = (char *) malloc(strlen(strBuffer) * sizeof(char) + 1);
			strcpy(multi_order_execution_message.stock_symbol[i], strBuffer);

			fgets(strBuffer, 5000, multi_order_execution_file);
			multi_order_execution_message.price[i] = atof(strBuffer);

			fgets(strBuffer, 5000, multi_order_execution_file);
			multi_order_execution_message.quantity[i] = atoi(strBuffer);
		}

		fgets(strBuffer, 5000, multi_order_execution_file);
		multi_order_execution_message.date_and_time = (char *) malloc(strlen(strBuffer) * sizeof(char) + 1);
		strcpy(multi_order_execution_message.date_and_time, strBuffer);

		fgets(strBuffer, 5000, multi_order_execution_file);
		multi_order_execution_message.host = (char *) malloc(strlen(strBuffer) * sizeof(char) + 1);
		strcpy(multi_order_execution_message.host, strBuffer);

		fclose(multi_order_execution_file);

		/* after published delete the files */
#ifdef	IPC_ON_LINE
		strcpy(command, "sleep 1s");
		system(command);
#endif
		remove(multi_orderExecutiontxt);

#ifdef	IPC_ON_LINE
		strcpy(command, "sleep 1s");
		system(command);
#endif
		remove(multi_orderExecutionControltxt);

		IPC_publishData(QSOR_MULTI_ORDER_EXECUTION_MESSAGE_NAME, &multi_order_execution_message);

		free(multi_order_execution_message.buy);
		for (i = 0; i < multi_order_execution_message.num_orders; i++)
			free(multi_order_execution_message.stock_symbol[i]);
		free(multi_order_execution_message.stock_symbol);
		free(multi_order_execution_message.price);
		free(multi_order_execution_message.quantity);
		free(multi_order_execution_message.date_and_time);
		free(multi_order_execution_message.host);
		
		return(1);
	}
	return(0);
}



/***************** Timers - quotes - order ***************/
void 
quotes_timer_handler (void *string, unsigned long currentTime, unsigned long scheduledTime)
{
	char command[100];
	
	publish_multi_order_execution_message(TS_NAME);
	publish_quotes_message(TS_NAME);

	// Modificacao para deletar os arquivos de alarm
 	if (verify_client_connection(TS_NAME) == 1)
	{
		if ((fileExists(alarmControltxt) == 1) && (fileExists(alarmtxt) == 1))
		{
#ifdef	IPC_ON_LINE
#ifndef TEST_TRADING_SYSTEM_ON_LINE2
			strcpy(command, "sleep ");
			strcat(command, "1s");
			system(command);
#endif
#endif
			strcpy(command, "rm -f ");
			strcat(command, alarmtxt);
			system(command);
#ifdef	IPC_ON_LINE
#ifndef TEST_TRADING_SYSTEM_ON_LINE2
			strcpy(command, "sleep ");
			strcat(command, "1s");
			system(command);
#endif
#endif
			strcpy(command, "rm -f ");
			strcat(command, alarmControltxt);
			system(command);
		}
	}

}



void 
start_timer(void)
{
	realStart = x_ipc_timeInMsecs();
	IPC_addTimer(TIMER1_PERIOD, TRIGGER_FOREVER, quotes_timer_handler, NULL);
}
/*********************************************************/



int 
main (int argc, char **argv)
{
	/* Locale Setting */
	setlocale (LC_ALL, "C");
	
	/* Connect the quotes_server_order_router to the central server */
	printf("\nIPC_connect(%s)\n", QSOR_NAME);
	
	//IPC_connectModule(QSOR_NAME,"192.168.254.151:1381");
	if (argc == 2)
		IPC_connectModule(QSOR_NAME, argv[1]);
	else
		IPC_connect(QSOR_NAME);
	//IPC_subscribeConnect(connect1Hnd, NULL);
	//IPC_subscribeConnect(connect2Hnd, NULL);
	//IPC_subscribeDisconnect(disconnect1Hnd, NULL);

	/* Define the named formats that the modules need */
	IPC_defineFormat(QSOR_QUOTES_MESSAGE_NAME, QSOR_QUOTES_MESSAGE_FORMAT);
	IPC_defineFormat(QSOR_MULTI_ORDER_EXECUTION_MESSAGE_NAME, QSOR_MULTI_ORDER_EXECUTION_MESSAGE_FORMAT);
	IPC_defineFormat(TS_MULTI_ORDER_MESSAGE_NAME, TS_MULTI_ORDER_MESSAGE_FORMAT);
	IPC_defineFormat(TS_CONTROL_MESSAGE_NAME, TS_CONTROL_MESSAGE_FORMAT);
	IPC_defineFormat(TS_STATE_MESSAGE_NAME, TS_STATE_MESSAGE_FORMAT);

	/* Define the messages that this module publishes */
	/* Quotes Message Format */
	IPC_defineMsg(QSOR_QUOTES_MESSAGE_NAME, IPC_VARIABLE_LENGTH, QSOR_QUOTES_MESSAGE_FORMAT);
	IPC_defineMsg(QSOR_MULTI_ORDER_EXECUTION_MESSAGE_NAME, IPC_VARIABLE_LENGTH, QSOR_MULTI_ORDER_EXECUTION_MESSAGE_FORMAT);
	IPC_defineMsg(TS_CONTROL_MESSAGE_NAME, IPC_VARIABLE_LENGTH, TS_CONTROL_MESSAGE_FORMAT);

	/* Subscribe to the messages that this module listens to. */
	IPC_subscribeData(TS_MULTI_ORDER_MESSAGE_NAME, multi_order_handler, QSOR_NAME);
	IPC_subscribeData(TS_CONTROL_MESSAGE_NAME, control_handler, QSOR_NAME);

//	IPC_subscribeData(TS_STATE_MESSAGE_NAME, state_handler, QSOR_NAME);

	/****************** Starting the timer *******************/

	start_timer();

	/*********************************************************/

	/* Subscribe for tty input. Typing "q" will quit the program. */
	IPC_subscribeFD(fileno(stdin), stdinHnd, QSOR_NAME);

	printf("Type 'q' to quit\n");

  	IPC_dispatch();

	IPC_disconnect();

	return 0;
}
