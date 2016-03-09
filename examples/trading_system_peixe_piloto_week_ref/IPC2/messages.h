/* ********************************************************
* PROJETO: Trading System 
* 
* ARQUIVO: messages.h		
*
* DATA: 28/01/2011
*
* REFERENCIA: -	
*
* RESUMO 
*	   Declara as estruturas utilizadas em
*	   ipc-client1.c e ipc-client2.c
*
***********************************************************/



/* Definindo o nome dos modulos */
#define TASK_NAME 	"Handshaking System"
#define TS_NAME		"trading_system"
#define QSOR_NAME 	"quotes_server_order_router"
#define TSCONTROL_NAME 	"trading_system_control"
/***************************************************/



/******************** Timers ***********************/
#ifndef	IPC_ON_LINE
#define TIMER1_PERIOD (1) /* Msecs */
#else
#define TIMER1_PERIOD (100) /* Msecs */
#endif

#define TIMER2_PERIOD (1000) /* Msecs */
#define TIMER2_COUNT  TRIGGER_FOREVER
/***************************************************/



/******** TAD da mensagem de cotacao ********/
typedef struct 
{
	int num_stocks;
	char **stock_symbol;
	double *price;
	double *volume;
	char *date_and_time;
	char *host;
} qsor_quotes_message;

#define QSOR_QUOTES_MESSAGE_FORMAT		"{int,<string:1>,<double:1>,<double:1>,string,string}"
#define QSOR_QUOTES_MESSAGE_NAME		"quotes"
/*********************************************/


/******* TAD da mensagem de execucao *********/
typedef struct 
{
	int buy;	// buy == 1; sell == 0; canceled == -1
	char *stock_symbol;
	double price;
	int quantity;	// num stocks (not lotes)
	char *date_and_time;
	char *host;
} qsor_order_execution_message;

#define QSOR_ORDER_EXECUTION_MESSAGE_FORMAT	"{int,string,double,int,string,string}"
#define QSOR_ORDER_EXECUTION_MESSAGE_NAME	"order_execution"
/*********************************************/


/********* TAD da mensagem de ordem **********/
typedef struct 
{
	int buy;	// buy == 1; sell == 0
	char *stock_symbol;
	double price;
	int quantity;	// num stocks (not lotes)
	char *date_and_time;
	char *host;
} ts_order_message;

#define TS_ORDER_MESSAGE_FORMAT			"{int,string,double,int,string,string}"
#define TS_ORDER_MESSAGE_NAME			"order"
/*********************************************/


/******* TAD da mensagem de controle *********/
typedef struct 
{
	int type;		// all stop (serious error) == 1; graciously stop == 2; buy/sell order == 3
	char *description;
	char *date_and_time;
	char *host;
} ts_control_message;

#define TS_CONTROL_MESSAGE_FORMAT		"{int,string,string,string}"
#define TS_CONTROL_MESSAGE_NAME			"control"
/*********************************************/


/* TAD da mensagem de estado do trading system */
typedef struct 
{
	int state_size;		// sizeof(STATE)
	int price_samples_size;	// MINUTES_IN_A_WEEK in a week trading system
	void *state;		// contains a struct whose type is STATE
	double *price_samples;	// contains all stored samples of a week; at each minute a sample is added
	char *date_and_time;
	char *host;
} ts_state_message;

#define TS_STATE_MESSAGE_FORMAT			"{int,int,<ubyte: 1>,<double:2>,string,string}"
#define TS_STATE_MESSAGE_NAME			"state"
/***********************************************/



/* Globals */
extern char *quotesControltxt;
extern char *quotestxt;
extern char *orderExecutionControltxt;
extern char *orderExecutiontxt;
extern char *orderControltxt;
extern char *ordertxt;
extern char *alarmControltxt;
extern char *alarmtxt;

#ifdef	USE_IPC
extern qsor_order_execution_message* g_received_order_execution;
extern int g_order_execution_message_received;
#endif



/**************** Prototypes ****************/
char** aloca_array_string(int vet_tam, int string_tam);
double* aloca_array_double(int array_tam);
char* aloca_string(int string_tam);
void build_array_string(char** array_string, char* file_string);
void build_array_double(double* array_double, char* file_string);
int verify_client_connection(char* moduleName);
int publish_quotes_message(char* moduleName);
int publish_order_execution_message(char* moduleName);
int fileExists(char *cpfileName);
char* timeNowFormat (void);
int alarm_connection (int connection, char *control_description, char* control_host);
void free_quotes(qsor_quotes_message* quotes);
void free_order(ts_order_message* order);
void free_order_execution(qsor_order_execution_message* order);
void free_control(ts_control_message* order);
void free_state(ts_state_message* state);
qsor_quotes_message* newQuotesMessage(void);
ts_order_message* newOrderMessage(void); 
qsor_order_execution_message* newOrderExecutionMessage(void); 
ts_control_message* newControlMessage(void); 
ts_state_message* newStateMessage(void); 
int init_trading_system(int argc, char **argv);
void call_trading_system_state_machine();
int receive_quote(qsor_quotes_message* received_quotes);
void Erro(char *msg1, char *msg2, char *msg3);
int trading_system_state_machine_busy();
void receive_order_execution(qsor_order_execution_message* received_order_execution);
void publish_control_message(int type, char *description, char *date_and_time, char *host);

/***************************************************/
