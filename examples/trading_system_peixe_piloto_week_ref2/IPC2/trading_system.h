//#define	VERBOSE			1

//#define	MyCAP_TRADING_COSTS

extern double LEVERAGE;
extern double BROKERAGE;

extern int MOVING_AVERAGE_NUM_PERIODS;
extern int SLOW_MOVING_AVERAGE_NUM_PERIODS;

// Currently the ideal is 0.03
extern double TRADING_SYSTEM_STOP_GAIN;
extern double TRADING_SYSTEM_STOP_LOSS;
extern double TRADING_SYSTEM_BUY_SELL_DELTA_RETURN;

// Mon=1, Tue=2, Wed=3, Thu=4, Fri=5
extern int FISRT_DAY_OF_TRADING_WEEK;

extern int TRADING_OPENING_HOUR;

extern int SAMPLE_SIZE;
extern int DAYS_BETWEEN_SAMPLES;

// Currently the ideal is 0
extern int NUM_DAYS_BEFORE_BEFORE_PREDICTION;


extern int NUM_SAMPLES_FOR_PREDICTION;

#define DATA_PATH ("DATA/")
#define PEIXE_PILOTO_DATA_PATH ("DADOS_PEIXE_PILOTO/COM_VOLUME/")
#define STATE_FILE_NAME	"state.bin"

#define BUY	1
#define SELL	0

#define BUY_SELL		1
#define SELL_BUY		0

#define	ORDER_EXECUTED		1
#define	ORDER_NOT_EXECUTED	0
#define	ORDER_CANCELED		-1

#define	SECONDS_IN_A_DAY	(24*60*60)
#define	MINUTES_IN_A_DAY	(7*60)
#define	MINUTES_IN_A_WEEK	(MINUTES_IN_A_DAY*5)
#define DAYS_IN_A_WEEK		(MINUTES_IN_A_WEEK/MINUTES_IN_A_DAY)
#define NUM_SAMPLES_FOR_INTRADAY_CORRECTION	DAYS_IN_A_WEEK


//#define TEST_TRADING_SYSTEM_ON_LINE
#define TEST_TRADING_SYSTEM_ON_LINE2
/*
Para testar o trading_system como se ele estivesse on-line:
1- Crie um diretorio temp dentro de IPC2
2- Copie todo o conteudo de IPC2 para este diretorio
mkdir temp
cp * temp
mkdir temp/GRAFICO_ONLINE
mkdir temp/GRAFICOS
mkdir temp/DADOS_PEIXE_PILOTO
ln -s /home/alberto/dados_peixe_piloto/quant/COM_VOLUME temp/DADOS_PEIXE_PILOTO/
ln -s /home/alberto/dados_peixe_piloto/quant/DATA temp/DATA

3- Descomente as linhas:
 #define TEST_TRADING_SYSTEM_ON_LINE acima deste arquivo na versao temp
 #define TEST_TRADING_SYSTEM_ON_LINE2 acima deste arquivo neste versao
4- Compile a versao temp como as flags do Makefile:
#IPC_FLAGS =
IPC_FLAGS = -DUSE_IPC -DIPC_TEST
#IPC_FLAGS = -DUSE_IPC -DIPC_ON_LINE
5- Compile a versao IPC2 com as flags do Makefile:
#IPC_FLAGS =
#IPC_FLAGS = -DUSE_IPC -DIPC_TEST
IPC_FLAGS = -DUSE_IPC -DIPC_ON_LINE
6- Rode o quotes_server_order_router da versao temp e o central
7- Rode os dois trading_system com os mesmos parametros de linha
*/


/********************* Timers *********************/
#ifndef TEST_TRADING_SYSTEM_ON_LINE2
#define	TIME_BETWEEN_STATE_MESSAGES	(15*1000) 	/* Msecs */
#else
#define	TIME_BETWEEN_STATE_MESSAGES	(1*1000)	/* Msecs */
#endif

#ifndef	IPC_ON_LINE
#define TIMER1_PERIOD (1) 	/* Msecs */
#else
#ifndef TEST_TRADING_SYSTEM_ON_LINE2
#define TIMER1_PERIOD (100) 	/* Msecs */
#else
#define TIMER1_PERIOD (10) 	/* Msecs */
#endif
#endif
/***************************************************/


#define TO_SCREEN	1
#define TO_FILE		2


// Types

struct _daily_data
{
	char stock_name[100];
	double stock_price[MINUTES_IN_A_DAY];
	double *stock_close_price_for_prediction;
	double *stock_close_price_for_prediction_date_l;
	double stock_close_price_for_intraday_correction[NUM_SAMPLES_FOR_INTRADAY_CORRECTION];
	double stock_close_price_for_intraday_correction_date_l[NUM_SAMPLES_FOR_INTRADAY_CORRECTION];
	double current_predicted_return_buy_sell;
	double current_predicted_return_sell_buy;
};

typedef struct _daily_data DAILY_DATA;


enum STATE_NAME
{
	INITIALIZE,
	WAIT_BEGIN_WEEK,
	SELECT_STOCK,
	WAIT_MOVING_AVERAGE,
	WAIT_FIRST_ORDERS_TRIGGER,
	SEND_FIRST_ORDERS,
	WAIT_FIRST_ORDERS,
	WAIT_FIRST_ORDERS_CANCEL,
	WAIT_SECOND_ORDERS_TRIGGER,
	SEND_SECOND_ORDERS,
	WAIT_SECOND_ORDERS,
	WAIT_SECOND_ORDERS_CANCEL,
	QUIT,
	INVALID_STATE
};



struct _state
{
	int    state;

	char   now[1000];
	int    moving_average_current_minute;
	long   first_day_of_week_date_l;
	
	
	double capital;
	double previous_capital;
	double predicted_return;
	
	char   best_stock_name[1000];
	int    best_stock_index;
	double best_stock_base_price;
	double best_stock_price;
	double best_stock_buy_price;
	double best_stock_sell_price;
	double best_stock_order_price;
	int    best_stock_quantity;

	char   ref_stock_name[1000];
	int    ref_stock_index;
	double ref_stock_base_price;
	double ref_stock_price;
	double ref_stock_buy_price;
	double ref_stock_sell_price;
	double ref_stock_order_price;
	int    ref_stock_quantity;

	int    trade_order;

	double moving_average_of_delta;
	double previous_minimum_delta;
	double previous_maximum_delta;
	int    order_result;
	
	long   first_orders_day_date_l;
	int    first_orders_minute;
	long   second_orders_day_date_l;
	int    second_orders_minute;
	
	int    day_current_minute;

	int    first_orders_execution_minute;
	int    first_orders_cancel_minute;
	int    second_orders_execution_minute;
	int    second_orders_cancel_minute;

	int    previous_moving_average_current_minute;

#ifdef	USE_IPC
	state_machine_multi_order_execution_message 	received_multi_order_execution;
	int    multi_order_execution_message_received;
#endif


};

typedef struct _state STATE;



// Global variables

extern STATE g_state;

extern int g_reference_stock_index;
extern char *g_begin_date;
extern char *g_end_date;
extern long g_trading_system_current_day_date_l;

extern DAILY_DATA **g_daily_data;
extern int g_num_stocks_of_interest;


extern double g_best_stock_price[MINUTES_IN_A_WEEK];
extern double g_ref_stock_price[MINUTES_IN_A_WEEK];

extern int g_moving_average_current_minute;

extern char *g_state_string[];

extern int g_stop_trading_system;


// Prototypes

void init_data_sets ();
void save_data_sets(char *dir_name, char *stock_name);
int find_day_index_by_date_l(long date_l);
void compute_target_return_and_avg ();
double moving_average(double *price_samples, int minute);
double delta_moving_average(STATE *state, double *stock_price, double *ref_price, int minute);
double slow_delta_moving_average(STATE *state, double *stock_price, double *ref_price, int minute);
void get_moving_average_price_sample(double *intraday_price, int day, int minute);
int enougth_intraday_data(int target_day);
double get_return_trading_system (int week, char *stock_name);
void generate_graph(char *graph_dir, STATE *state, double *stock_price, double *ref_price);
STATE get_state();
void save_state(STATE state);
int trading_system_state_machine(int moving_average_current_minute);
void initialize_state_machine();
char *get_best_stock_name(int trading_system_current_week);
char *get_trading_system_time(char *str_now, int moving_average_current_minute);
void get_state_in_a_hot_start();
void process_one_file(int file_number);
void free_g_tt_data_set(int argc);
void load_intraday_data(char *stock_name, int semana);
void send_state();
void load_all_stocks_intraday_data(long date_l_trading_system_current_day);
void load_all_stocks_day_data(long date_l_trading_system_current_day);
int check_if_date_l_is_available_for_all_stocks(long date_l);
void select_stock(STATE *state);
void Erro(char *msg1, char *msg2, char *msg3);
void *alloc_mem(size_t size);
long get_first_day();
void read_stock_list(char *stock_list_file_name);
long get_next_available_day(long date_l_trading_system_current_day);
int get_reference_stock_index(char *stock_name);
void test_find_date_in_file();
double *get_stock_current_day_intraday_prices(STATE *state, int moving_average_current_minute);
double *get_ref_current_day_intraday_prices(STATE *state, int moving_average_current_minute);
void select_stock(STATE *state);
#ifdef	USE_IPC
void receive_multi_order_execution(qsor_multi_order_execution_message* received_multi_order_execution);
#endif
void get_stock_or_ref_current_day_intraday_prices_on_hot_start(int stock, long date_l, int moving_average_current_minute);
void send_state(STATE *state);
double trade_cost(double capital);
void read_trading_system_parameters();
void plot_graph(int where_to, double *best_stock_price, double *ref_stock_price, char *output_file_name);
void print_state(FILE *output_file, STATE state, double *best_stock_price, double *ref_stock_price);
double compute_current_week_return(STATE *state);
double compute_current_week_gross_return(STATE *state);
double compute_final_week_return(STATE *state);
