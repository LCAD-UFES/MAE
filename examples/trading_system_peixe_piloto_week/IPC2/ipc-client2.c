/* ********************************************************
* PROJETO: Trading System
*	   IpcClient #2	
*
* ARQUIVO: ipc-client2.c		
*
* DATA: 28/01/2011
*
* REFERENCIA: messages.h
*
* RESUMO 
*	      Publishes: quotes, order execution, control.
*             Subscreve a: Ipc Client #2 (ipc-client2.c) order, state, control.
*             
*
***********************************************************/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>

#include "ipc.h"
#include "messages.h"

unsigned long x_ipc_timeInMsecs(void);
unsigned long realStart;



/******************** Order Handler **********************/
void 
orderHandler (MSG_INSTANCE msgRef, BYTE_ARRAY callData, void *clientData)
{
	FILE *fp;
	FORMATTER_PTR formatter;

	/* Check control and order files */
	if (fileExists(orderControltxt) == 1)
	{
		
		/* gerar mensagem de error */
		return;
	}
	if (fileExists(orderControltxt) == 1)
	{
		return;
	}	

	
	trading_system_order_message* received_order = (trading_system_order_message*) callData;
	printf("\norderHandler: Receiving message %s and message size: %d .\n", IPC_msgInstanceName(msgRef), IPC_dataLength(msgRef));
	
	/* Get the formatter for this message instance */
	formatter = IPC_msgInstanceFormatter(msgRef);
	IPC_unmarshall(formatter, callData, (void **)(&received_order));
	IPC_printData(formatter, stdout, received_order);
	
	if (received_order != NULL) 
	{	
		printf ("\n\n#DEBUG# - RECEIVED_ORDER != NULL\n\n");	
		/* build the file with the order struct */
		if ((fp=fopen(ordertxt, "w+")) != NULL) 		
		{	
			printf ("\n\n#DEBUG# - WRITE == TRUE\n\n");
			/* order type buy==1, sell ==0 */
			fprintf(fp, "%d\n", received_order -> buy);
			/* stock */
			fprintf(fp, "%s\n", received_order -> stock);
			/* price */
			fprintf(fp, "%.2f\n", received_order -> price);
			/* quantity */
			fprintf(fp, "%d\n", received_order -> quantity);
			/* date and time */
			fprintf(fp, "%s\n", received_order -> date_and_time);
			/* host */
			fprintf(fp, "%s\n", received_order -> host);	
			fclose(fp);
		}
	      		
		/* build the control file */
		if ((fp=fopen(orderControltxt, "w+")) != NULL)
		{
			printf ("\n\n#DEBUG# - CLOSE_CONTROL == TRUE\n\n");
			fclose(fp);	
		}					
	}

	//IPC_freeDataElements(IPC_msgInstanceFormatter(msgRef), &received_quotes);
	free_order(received_order);
	IPC_freeByteArray(callData);
}



/******************* State Handler ***********************/
void 
stateHandler (MSG_INSTANCE msgRef, BYTE_ARRAY callData, void *clientData)
{
	
	trading_system_state_message* received_state = (trading_system_state_message*) callData;
	FORMATTER_PTR formatter;
	printf("\nquotesHanlder: Receiving message %s and message size: %d .\n", IPC_msgInstanceName(msgRef), IPC_dataLength(msgRef));

	/* Get the formatter for this message instance */
	formatter = IPC_msgInstanceFormatter(msgRef);
	IPC_unmarshall(formatter, callData, (void **)(&received_state));
	IPC_printData(formatter, stdout, received_state);
	
	/* Link to the trading system */
	//IPC_freeDataElements(IPC_msgInstanceFormatter(msgRef), &received_quotes);
	free_state(received_state);
	IPC_freeByteArray(callData);
}



/******************** Control Handler ********************/
void 
controlHandler (MSG_INSTANCE msgRef, BYTE_ARRAY callData, void *clientData)
{
	
	trading_system_control_message* received_control = (trading_system_control_message*) callData;
	FORMATTER_PTR formatter;
	printf("\ncontrolHanlder: Receiving message %s and message size: %d .\n", IPC_msgInstanceName(msgRef), IPC_dataLength(msgRef));

	/* Get the formatter for this message instance */
	formatter = IPC_msgInstanceFormatter(msgRef);
	IPC_unmarshall(formatter, callData, (void **)(&received_control));
	IPC_printData(formatter, stdout, received_control);
	
	/* Link to the trading system */

	//IPC_freeDataElements(IPC_msgInstanceFormatter(msgRef), &received_quotes);
	free_control(received_control);
	IPC_freeByteArray(callData);
}



void 
stdinHnd (int fd, void *clientData)
{
	char inputLine[81];

	fgets(inputLine, 80, stdin);

	switch (inputLine[0]) {
	case 'q': case 'Q': 
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
	if (first) IPC_unsubscribeConnect(connect1Hnd);
	else IPC_unsubscribeConnect(connect2Hnd);
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



/***************** Timers - quotes - order ***************/
void 
timerHandler (void *string, unsigned long currentTime, unsigned long scheduledTime)
{
	publish_quotes_message("IPCClient1");
	publish_orderExecution_message("IPCClient1");
}



void 
quotesTimerHandler (void *string, unsigned long currentTime, unsigned long scheduledTime)
{
	timerHandler ((char *)string, currentTime, scheduledTime);
}



void 
orderExecutionTimerHandler (void *string, unsigned long currentTime, unsigned long scheduledTime)
{
	timerHandler ((char *)string, currentTime, scheduledTime);
}



void 
timerTest(void)
{

	/* Add some timers */
	realStart = x_ipc_timeInMsecs();
	IPC_addTimer(TIMER1_PERIOD, TIMER1_COUNT, timerHandler, "Geral");
	//IPC_addTimer(TIMER2_PERIOD, TIMER2_COUNT, orderExecutionTimerHandler, "orderExecution");
}
/*********************************************************/



int 
main (void)
{
	/* Connect IPCCLIENT #2 to the central server */
	printf("\nIPC_connect(%s)\n", IPCCLIENT2_NAME);
	IPC_connect(IPCCLIENT2_NAME);

	IPC_subscribeConnect(connect1Hnd, NULL);
	IPC_subscribeConnect(connect2Hnd, NULL);
	IPC_subscribeDisconnect(disconnect1Hnd, NULL);

	/* Define the named formats that the modules need */
	/* Quotes Format */	
	printf("\nIPC_defineFormat(%s, %s)\n", TRADING_SYSTEM_QUOTES_MESSAGE_NAME, TRADING_SYSTEM_QUOTES_MESSAGE_FORMAT);
	IPC_defineFormat(TRADING_SYSTEM_QUOTES_MESSAGE_NAME, TRADING_SYSTEM_QUOTES_MESSAGE_FORMAT);
	/* Order Format */
	printf("\nIPC_defineFormat(%s, %s)\n", TRADING_SYSTEM_ORDER_MESSAGE_NAME, TRADING_SYSTEM_ORDER_MESSAGE_FORMAT);
	IPC_defineFormat(TRADING_SYSTEM_ORDER_MESSAGE_NAME, TRADING_SYSTEM_ORDER_MESSAGE_FORMAT);
	/* Order Execution Format */
	printf("\nIPC_defineFormat(%s, %s)\n", TRADING_SYSTEM_ORDER_EXECUTION_MESSAGE_NAME, TRADING_SYSTEM_ORDER_EXECUTION_MESSAGE_FORMAT);
	IPC_defineFormat(TRADING_SYSTEM_ORDER_EXECUTION_MESSAGE_NAME, TRADING_SYSTEM_ORDER_EXECUTION_MESSAGE_FORMAT);
	/* Control Format */
	printf("\nIPC_defineFormat(%s, %s)\n", TRADING_SYSTEM_CONTROL_MESSAGE_NAME, TRADING_SYSTEM_CONTROL_MESSAGE_FORMAT);
	IPC_defineFormat(TRADING_SYSTEM_CONTROL_MESSAGE_NAME, TRADING_SYSTEM_CONTROL_MESSAGE_FORMAT);
	/* State Format */
	printf("\nIPC_defineFormat(%s, %s)\n", TRADING_SYSTEM_STATE_MESSAGE_NAME, TRADING_SYSTEM_STATE_MESSAGE_FORMAT);
	IPC_defineFormat(TRADING_SYSTEM_STATE_MESSAGE_NAME, TRADING_SYSTEM_STATE_MESSAGE_FORMAT);

	/* Define the messages that this module publishes */
	/* Quotes Message Format */
	printf("\nIPC_defineMsg(%s, IPC_VARIABLE_LENGTH, %s)\n", "quotes", TRADING_SYSTEM_QUOTES_MESSAGE_FORMAT);
	IPC_defineMsg("quotes", IPC_VARIABLE_LENGTH, TRADING_SYSTEM_QUOTES_MESSAGE_FORMAT);
	/* Order Execution Message Format */
	printf("\nIPC_defineMsg(%s, IPC_VARIABLE_LENGTH, %s)\n", "order_execution", TRADING_SYSTEM_ORDER_EXECUTION_MESSAGE_FORMAT);
	IPC_defineMsg("orderExecution", IPC_VARIABLE_LENGTH,TRADING_SYSTEM_ORDER_EXECUTION_MESSAGE_FORMAT);
	/* Order Execution Message Format */
	printf("\nIPC_defineMsg(%s, IPC_VARIABLE_LENGTH, %s)\n", "control", TRADING_SYSTEM_CONTROL_MESSAGE_FORMAT);
	IPC_defineMsg("control", IPC_VARIABLE_LENGTH,TRADING_SYSTEM_CONTROL_MESSAGE_FORMAT);

	/* Subscribe to the messages that this module listens to. */
	printf("\nIPC_subscribe(%s, orderHandler, %s)\n", "order", IPCCLIENT2_NAME);
	IPC_subscribe("order", orderHandler, IPCCLIENT2_NAME);

	printf("\nIPC_subscribe(%s, stateHandler, %s)\n", "state", IPCCLIENT2_NAME);
	IPC_subscribe("state", stateHandler, IPCCLIENT2_NAME);

	printf("\nIPC_subscribe(%s, controlHandler, %s)\n", "control", IPCCLIENT2_NAME);
	IPC_subscribe("control", controlHandler, IPCCLIENT2_NAME);


	//IPC_subscribeHandlerChange("quotes", handlerChangeHnd, NULL);
	//IPC_subscribeHandlerChange("quotes", handlerChangeHnd2, NULL);
	//IPC_subscribeHandlerChange("orderExecution", handlerChangeHnd, NULL);
	//IPC_subscribeHandlerChange("orderExecution", handlerChangeHnd2, NULL);

//sleep(5);
/****************** Starting the timer *******************/

timerTest();
/*********************************************************/

	/* Subscribe a handler for tty input.*/
	//printf("\nIPC_subscribeFD(%d, stdinHnd, %s)\n", fileno(stdin), IPCCLIENT2_NAME);
	IPC_subscribeFD(fileno(stdin), stdinHnd, IPCCLIENT2_NAME);

	printf("Type 'q' to quit\n");

  	IPC_dispatch();

	IPC_dispatch();
	IPC_disconnect();

	return 0;
}
