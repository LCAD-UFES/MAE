/* ********************************************************
* PROJETO: Trading System
*	   IpcClient #1	
*
* ARQUIVO: ipc-client1.c		
*
* DATA: 28/01/2011
*
* REFERENCIA: messages.h
*
* RESUMO 
*	      Publishes: order, state, control.
*             Subscreve a: Ipc Client #1 (ipc-client1.c) quotes, order execution, control.
*             Funcao: 
*
***********************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "ipc.h"
#include "messages.h"



void 
quotesHandler (MSG_INSTANCE msgRef, BYTE_ARRAY callData, void *clientData)
{
	trading_system_quotes_message* received_quotes = (trading_system_quotes_message*) callData;
	FORMATTER_PTR formatter;

	printf("\nquotesHandler: Receiving message %s and message size: %d.\n", IPC_msgInstanceName(msgRef), IPC_dataLength(msgRef));
	
	/* Get the formatter for this message instance */
	formatter = IPC_msgInstanceFormatter(msgRef);
	IPC_unmarshall(formatter, callData, (void **) (&received_quotes));
	IPC_printData(formatter, stdout, received_quotes);
	printf("\nQuotes Time: %s.\n", received_quotes->date_and_time);
	/* ORDEM SIMULACAO */

	/* Rotina para teste do order handler 
	trading_system_order_message* order;
	order = newOrderMessage();
	order -> buy = 1;
	order -> stock = aloca_string(8);
	strcpy(order -> stock,"AMBV4");
	printf ("\n\n DEBUG MAKE ORDER MESSAGE TESTE \n\n");
	order -> price = received_quotes -> quote[3];
	order -> quantity = 300;
	order -> date_and_time = aloca_string(30);
	strcpy(order -> date_and_time ,received_quotes -> date_and_time);
	order -> host = aloca_string(15);
	strcpy(order -> host, received_quotes -> host);
	IPC_publishData("order",order);
	IPC_freeDataElements(IPC_msgInstanceFormatter(msgRef), &received_quotes);
	free_order(order); */


	free_quotes(received_quotes);
	IPC_freeByteArray(callData);
}



void 
orderExecutionHandler (MSG_INSTANCE msgRef, BYTE_ARRAY callData,void *clientData)
{

	trading_system_order_execution_message* received_order_execution = (trading_system_order_execution_message*) callData;
	FORMATTER_PTR formatter;
	printf("\norderExecutionHandler: Receiving message %s and message size: %d .\n", IPC_msgInstanceName(msgRef), IPC_dataLength(msgRef));

	/* Get the formatter for this message instance */ 
	formatter = IPC_msgInstanceFormatter(msgRef);
	IPC_unmarshall(formatter, callData, (void **)(&received_order_execution));
	IPC_printData(formatter, stdout, received_order_execution);
	
	
	/* Link to the trading system 

	IPC_freeDataElements(IPC_msgInstanceFormatter(msgRef), &received_quotes); */
	free_order_execution(received_order_execution);
	IPC_freeByteArray(callData); 
}



void 
controlHandler (MSG_INSTANCE msgRef, BYTE_ARRAY callData, void *clientData)
{
	
	trading_system_control_message* received_control = (trading_system_control_message*) callData;
	FORMATTER_PTR formatter;
	printf("\ncontrolHandler: Receiving message %s and message size: %d .\n", IPC_msgInstanceName(msgRef), IPC_dataLength(msgRef));
  
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



int 
main (void)
{
	/* Connect to the central server */
	printf("\nIPC_connect(%s)\n", IPCCLIENT1_NAME);
	IPC_connect(IPCCLIENT1_NAME);

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
	printf("\nIPC_defineMsg(%s, IPC_VARIABLE_LENGTH, %s)\n", "order",  TRADING_SYSTEM_ORDER_MESSAGE_FORMAT);
	IPC_defineMsg("order", IPC_VARIABLE_LENGTH,  TRADING_SYSTEM_ORDER_MESSAGE_FORMAT);

	printf("\nIPC_defineMsg(%s, IPC_VARIABLE_LENGTH, %s)\n", "state", TRADING_SYSTEM_STATE_MESSAGE_FORMAT);
	IPC_defineMsg("state", IPC_VARIABLE_LENGTH, TRADING_SYSTEM_STATE_MESSAGE_FORMAT);

	printf("\nIPC_defineMsg(%s, IPC_VARIABLE_LENGTH, %s)\n", "control", TRADING_SYSTEM_CONTROL_MESSAGE_FORMAT);
	IPC_defineMsg("control", IPC_VARIABLE_LENGTH, TRADING_SYSTEM_CONTROL_MESSAGE_FORMAT);

	/* Subscribe to the messages that this module listens to. */
	printf("\nIPC_subscribe(%s, quotesHandler, %s)\n", "quotes", IPCCLIENT1_NAME);
	IPC_subscribe("quotes", quotesHandler, IPCCLIENT1_NAME);

	printf("\nIPC_subscribe(%s, orderExecutionHandler, %s)\n", "orderExecution", IPCCLIENT1_NAME);
	IPC_subscribe("orderExecution", orderExecutionHandler, IPCCLIENT1_NAME);

	printf("\nIPC_subscribe(%s, controlHandler, %s)\n", "control", IPCCLIENT1_NAME);
	IPC_subscribe("control", controlHandler, IPCCLIENT1_NAME);

	/* Subscribe a handler for tty input. Typing "q" will quit the program. */
	printf("\nIPC_subscribeFD(%d, stdinHnd, %s)\n", fileno(stdin), IPCCLIENT1_NAME);
	IPC_subscribeFD(fileno(stdin), stdinHnd, IPCCLIENT1_NAME);

	printf("\nType 'q' to quit\n");

	IPC_dispatch();
	IPC_disconnect();

	return 0;
}

