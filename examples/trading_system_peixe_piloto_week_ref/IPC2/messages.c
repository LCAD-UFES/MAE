#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "ipc.h"
#include "messages.h"



void 
build_quotes_symbols(qsor_quotes_message *quotes, char *quotes_symbols)
{	
	int i;
	char *quote_symbol;
	char *strtok_aux;

	quotes->stock_symbol = (char **) malloc(quotes->num_stocks * sizeof(char *));
	quote_symbol = strtok_r(quotes_symbols, "; \t\n\r", &strtok_aux);
	if (quote_symbol == NULL)
		Erro("Could not find first quote_symbol in build_quotes_symbols()", "", "");
	
	quotes->stock_symbol[0] = (char *) malloc((strlen(quote_symbol) + 1) * sizeof(char));
	strcpy(quotes->stock_symbol[0], quote_symbol);
	
	for (i = 1; i < quotes->num_stocks; i++)
	{
		quote_symbol = strtok_r(NULL, "; \t\n\r", &strtok_aux);
		if (quote_symbol == NULL)
			Erro("Could not find next quote_symbol in build_quotes_symbols()", "", "");
		quotes->stock_symbol[i] = (char *) malloc((strlen(quote_symbol) + 1) * sizeof(char));
		strcpy(quotes->stock_symbol[i], quote_symbol);
	}
}


void 
free_quotes_symbols(qsor_quotes_message *quotes)
{	
	int i;

	for (i = 0; i < quotes->num_stocks; i++)
		free(quotes->stock_symbol[i]);

	free(quotes->stock_symbol);
}


void 
build_quotes_prices(qsor_quotes_message *quotes, char *quotes_prices)
{
	int i;
	char *quote_price;
	char *strtok_aux;

	quotes->price = (double *) malloc(quotes->num_stocks * sizeof(double));
	quote_price = strtok_r(quotes_prices, "; \t\n\r", &strtok_aux);
	if (quote_price == NULL)
		Erro("Could not find first quote_price in build_quotes_prices()", "", "");
	
	quotes->price[0] = atof(quote_price);	
	for (i = 1; i < quotes->num_stocks; i++)
	{
		quote_price = strtok_r(NULL, "; \t\n\r", &strtok_aux);
		if (quote_price == NULL)
			Erro("Could not find next quote_price in build_quotes_prices()", "", "");
		quotes->price[i] = atof(quote_price);
	}
}


void 
build_quotes_volumes(qsor_quotes_message *quotes, char *quotes_volumes)
{
	int i;
	char *quote_volume;
	char *strtok_aux;

	quotes->volume = (double *) malloc(quotes->num_stocks * sizeof(double));
	quote_volume = strtok_r(quotes_volumes, "; \t\n\r", &strtok_aux);
	if (quote_volume == NULL)
		Erro("Could not find first quote_volume in build_quotes_volumes()", "", "");
	
	quotes->volume[0] = atof(quote_volume);
	for (i = 1; i < quotes->num_stocks; i++)
	{
		quote_volume = strtok_r(NULL, "; \t\n\r", &strtok_aux);
		if (quote_volume == NULL)
			Erro("Could not find next quote_volume in build_quotes_volumes()", "", "");
		quotes->volume[i] = atof(quote_volume);
	}
}


int 
fileExists(char *cpfileName)
{
	FILE *fp;

	if ((fp=fopen(cpfileName, "r")) == NULL) 
	{
		return(0);
	}
	fclose(fp);
	return(1);
}


int 
verify_client_connection(char *moduleName)
{
	if (IPC_isModuleConnected(moduleName) == 0)
		return(0);

	return(1);
}


void 
free_quotes(qsor_quotes_message* quotes)
{
	free_quotes_symbols(quotes);
	free(quotes->price);
	free(quotes->volume);
	free(quotes->date_and_time);
	free(quotes->host);
}


int 
publish_quotes_message(char *moduleName)
{
	FILE *quotes_file;
#ifdef	IPC_ON_LINE
	char command[100];
#endif
	char str_buffer[5000];
	qsor_quotes_message quotes;

	//if (alarm_connection (verify_client_connection(moduleName), "ipc_trading_system NOT CONNECTED", "quotes_server_order_router") == 0)
	//{
	//	return(0);
	//}

	
	/* Rotine to check the directory and if control.txt exists publish the quotes data */
	if (fileExists(quotesControltxt) == 1) 
	{
		if ((quotes_file = fopen(quotestxt, "r")) == NULL) 
			Erro("Could not open quotes file ", quotestxt, " in publish_quotes_message().");

		fgets(str_buffer, 5000, quotes_file);
		quotes.num_stocks = atoi(str_buffer);		

		fgets(str_buffer, 5000, quotes_file);
		quotes.date_and_time = (char *) malloc((strlen(str_buffer) + 1) * sizeof(char));
		strcpy(quotes.date_and_time, str_buffer);

		fgets(str_buffer, 5000, quotes_file);
		build_quotes_symbols(&quotes, str_buffer);

		fgets(str_buffer, 5000, quotes_file);
		build_quotes_prices(&quotes, str_buffer);				

		fgets(str_buffer, 5000, quotes_file);
		build_quotes_volumes(&quotes, str_buffer);				

		quotes.host = (char *) malloc((strlen(QSOR_NAME) + 1) * sizeof(char));
		strcpy(quotes.host, QSOR_NAME);

		fclose(quotes_file);

		/* after published delete the files */
#ifdef	IPC_ON_LINE
		strcpy(command, "sleep 1s");
		system(command);
#endif
		remove(quotestxt);

#ifdef	IPC_ON_LINE
		strcpy(command, "sleep 1s");
		system(command);
#endif
		remove(quotesControltxt);
		
		IPC_publishData(QSOR_QUOTES_MESSAGE_NAME, &quotes);
		free_quotes(&quotes);

		return(1);
	}
	return(0);
}


void
publish_control_message(int type, char *description, char *date_and_time, char *host)
{
	ts_control_message control_message;

	control_message.type = type;
	control_message.description = description;
	control_message.date_and_time = date_and_time;
	control_message.host = host;
	
	IPC_publishData(TS_CONTROL_MESSAGE_NAME, &control_message);
}


int 
publish_order_execution_message(char *moduleName)
{
	char strBuffer[5000];
#ifdef	IPC_ON_LINE
	char command[100];
#endif
	FILE *order_execution_file;	
	qsor_order_execution_message order_execution_message;

	//if (alarm_connection (verify_client_connection(moduleName), "ipc_trading_system NOT CONNECTED", "quotes_server_order_router") == 0)
	//{
	//	return(0);
	//}

	/* message struct variable */

	/* Rotine to check the directory and if control.txt exists publish the orderExecution data */
	if (fileExists(orderExecutionControltxt) == 1) 
	{
		if ((order_execution_file = fopen(orderExecutiontxt, "r")) == NULL)
			Erro("Could not open order execution file ", orderExecutiontxt, " in publish_order_execution_message().");

		fgets(strBuffer, 5000, order_execution_file);
		order_execution_message.buy = atoi(strBuffer);

		fgets(strBuffer, 5000, order_execution_file);
		order_execution_message.stock_symbol = (char *) malloc(strlen(strBuffer) * sizeof(char) + 1);
		strcpy(order_execution_message.stock_symbol, strBuffer);

		fgets(strBuffer, 5000, order_execution_file);
		order_execution_message.price = atof(strBuffer);

		fgets(strBuffer, 5000, order_execution_file);
		order_execution_message.quantity = atoi(strBuffer);

		fgets(strBuffer, 5000, order_execution_file);
		order_execution_message.date_and_time = (char *) malloc(strlen(strBuffer) * sizeof(char) + 1);
		strcpy(order_execution_message.date_and_time, strBuffer);

		fgets(strBuffer, 5000, order_execution_file);
		order_execution_message.host = (char *) malloc(strlen(strBuffer) * sizeof(char) + 1);
		strcpy(order_execution_message.host, strBuffer);

		fclose(order_execution_file);

		/* after published delete the files */
#ifdef	IPC_ON_LINE
		strcpy(command, "sleep 1s");
		system(command);
#endif
		remove(orderExecutiontxt);

#ifdef	IPC_ON_LINE
		strcpy(command, "sleep 1s");
		system(command);
#endif
		remove(orderExecutionControltxt);

		IPC_publishData(QSOR_ORDER_EXECUTION_MESSAGE_NAME, &order_execution_message);

		free(order_execution_message.stock_symbol);
		free(order_execution_message.date_and_time);
		free(order_execution_message.host);
		
		return(1);
	}
	return(0);
}


int 
alarm_connection (int connection, char *control_description, char* control_host)
{
	FILE *fp;

	ts_control_message* control;

	if ((connection == 1) && (fileExists(alarmControltxt) == 1))
	{
		remove(alarmtxt);
		remove(alarmControltxt);
		return 1;
	}
	else if ((connection == 1) && (fileExists(alarmControltxt) == 0))
	{	
		return 1;
	}
	else if ((connection == 0)  && (fileExists(alarmControltxt) == 0))
	{

		control = newControlMessage();
		control->type = 1;
		control->description = (char *) malloc(strlen(control_description) * sizeof(char) + 1);
		strcpy(control->description, control_description);
		control->date_and_time = (char *) malloc(strlen(timeNowFormat()) * sizeof(char) + 1);
		strcpy(control->date_and_time, timeNowFormat());
		control->host = (char *) malloc(strlen(control_host) * sizeof(char) + 1);
		strcpy(control->host, control_host);
		IPC_publishData(TS_CONTROL_MESSAGE_NAME, control);

		if ((fp=fopen(alarmtxt, "w+")) != NULL) 
		{
			fprintf(fp, "%d\n", control->type);
			fprintf(fp, "%s\n", control->description);
			fprintf(fp, "%s\n", control->date_and_time);
			fprintf(fp, "%s\n", control->host);	
		}
		fclose(fp);
		if ((fp=fopen(alarmControltxt, "w+")) != NULL) 
		{
			fclose(fp);	
		}
		free_control(control);
		return 0;
	}
	else if ((connection == 0) && (fileExists(alarmControltxt) == 1))
	{
		return 0;
	}
	
	return 1; /* nao deveria nunca ocorrer? */ 
}


char * 
timeNowFormat()
{
	time_t rawtime;
	struct tm * timeinfo;
	static char buffer[51];
	char *string;

	time (&rawtime);
	timeinfo = localtime(&rawtime);

	strftime (buffer, 50, "%Y-%m-%d %a %H:%M", timeinfo);
	string = buffer;
	
	return string;
}


void 
free_order(ts_order_message* order)
{
	free(order->stock_symbol);
	free(order->date_and_time);
	free(order->host);
	free(order);
}

void 
free_control(ts_control_message* order)
{
	free(order->description);
	free(order->date_and_time);
	free(order->host);
	free(order);
}


ts_order_message* 
newOrderMessage(void) 
{
	ts_order_message* order;
	order = (ts_order_message*) malloc(sizeof(ts_order_message));
	return order;
}


ts_control_message* 
newControlMessage(void) 
{
	ts_control_message* control;
	control = (ts_control_message*) malloc(sizeof(ts_control_message));
	return control;
}


ts_state_message* 
newStateMessage(void) 
{
	ts_state_message* state;
	state = (ts_state_message*) malloc(sizeof(ts_state_message));
	return state;
}
