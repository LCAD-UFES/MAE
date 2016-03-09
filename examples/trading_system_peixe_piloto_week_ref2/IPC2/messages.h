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


/******* TAD da mensagem de multi execucao *********/
typedef struct 
{
	int num_orders;
	int *buy;	// buy == 1; sell == 0; canceled == -1
	char **stock_symbol;
	double *price;
	int *quantity;	// num stocks (not lotes)
	char *date_and_time;
	char *host;
} qsor_multi_order_execution_message;

#define QSOR_MULTI_ORDER_EXECUTION_MESSAGE_FORMAT	"{int,<int:1>,<string:1>,<double:1>,<int:1>,string,string}"
#define QSOR_MULTI_ORDER_EXECUTION_MESSAGE_NAME		"multi_order_execution"


typedef struct 
{
	int buy[2];	// buy == 1; sell == 0; canceled == -1
	char stock_symbol[2][100];
	double price[2];
	int quantity[2];	// num stocks (not lotes)
	char date_and_time[200];
	char host[1000];
} state_machine_multi_order_execution_message;
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


/********* TAD da mensagem de multi ordem **********/
typedef struct 
{
	int num_orders;
	int *buy;	// buy == 1; sell == 0
	char **stock_symbol;
	double *price;
	int *quantity;	// num stocks (not lotes)
	char *date_and_time;
	char *host;
} ts_multi_order_message;

#define TS_MULTI_ORDER_MESSAGE_FORMAT			"{int,<int:1>,<string:1>,<double:1>,<int:1>,string,string}"
#define TS_MULTI_ORDER_MESSAGE_NAME			"multi_order"
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
	int state_size;			// sizeof(STATE)
	int price_samples_size;		// MINUTES_IN_A_WEEK in a week trading system
	double *best_stock_price;	// contains all stock prices of a week; at each minute a sample is added
	double *ref_stock_price;	// contains all ref stock samples of a week; at each minute a sample is added
	char *date_and_time;
	char *host;
	void *state;			// contains a struct whose type is STATE
} ts_state_message;

#define TS_STATE_MESSAGE_FORMAT			"{int,int,<double:2>,<double:2>,string,string,<ubyte: 1>}"
#define TS_STATE_MESSAGE_NAME			"state"
/***********************************************/



/* Globals */
extern char *quotesControltxt;
extern char *quotestxt;
extern char *orderExecutionControltxt;
extern char *orderExecutiontxt;
extern char *multi_orderExecutionControltxt;
extern char *multi_orderExecutiontxt;
extern char *orderControltxt;
extern char *ordertxt;
extern char *multi_orderControltxt;
extern char *multi_ordertxt;
extern char *alarmControltxt;
extern char *alarmtxt;



/**************** Prototypes ****************/
char** aloca_array_string(int vet_tam, int string_tam);
double* aloca_array_double(int array_tam);
char* aloca_string(int string_tam);
void build_array_string(char** array_string, char* file_string);
void build_array_double(double* array_double, char* file_string);
int verify_client_connection(char* moduleName);
int publish_quotes_message(char* moduleName);
int publish_order_execution_message(char* moduleName);
int publish_multi_order_execution_message(char* moduleName);
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
