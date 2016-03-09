//#define	MyCAP_TRADING_COSTS

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
#define DAYS_IN_A_WEEK		5


// Types

struct _tt_data_set
{
	long date_l;
	double target_return_buy_sell;
	double wnn_predicted_return_buy_sell;
	double wnn_prediction_error_buy_sell;
	double target_return_sell_buy;
	double wnn_predicted_return_sell_buy;
	double wnn_prediction_error_sell_buy;
	double confidence;
	int file_number;
	int file_number2;
	int trade_order;
};

typedef struct _tt_data_set TT_DATA_SET;


struct _train_data_set
{
	long date_l;
	double clo;
	double opn;
	double max;
	double min;
	double vol;
	double *intraday_price;
	double *intraday_volume;
};

typedef struct _train_data_set TRAIN_DATA_SET;


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
	int state;

	char now[1000];
	int moving_average_current_minute;

	int trade_order;
	
	double capital;
	double previous_capital;
	
	int moving_average_first_day;
	
	char  best_stock_name[1000];
	char  reference_name[1000];
	double best_stock_predicted_return;
	double best_stock_maximum_buy_price;
	double minimum_delta_for_second_order;

	double base_stock_price;
	double stock_price;
	double stock_buy_price;
	double stock_sell_price;
	double stock_order_price;
	int stock_quantity;

	double base_ref_price;
	double ref_price;
	double ref_buy_price;
	double ref_sell_price;
	double ref_order_price;
	int ref_quantity;

	double moving_average_of_delta;
	double previous_minimum_delta;
	double previous_maximum_delta;
	int order_result;
	
	int first_orders_day;
	int first_orders_minute;
	int second_orders_day;
	int second_orders_minute;
	
	int day_current_minute;

	int first_orders_execution_minute;
	int first_orders_cancel_minute;
	int second_orders_execution_minute;
	int second_orders_cancel_minute;

	int previous_moving_average_current_minute;

#ifdef	USE_IPC
	qsor_order_execution_message received_order_execution;
	int order_execution_message_received;
#endif


};

typedef struct _state STATE;



// Global variables

extern char **g_argv;
extern int g_argc;

extern TRAIN_DATA_SET *g_train_data_set;
extern TRAIN_DATA_SET *g_ref_data_set;

extern int g_train_num_samples;
extern char g_current_data_set_name[1000];

extern int FIRST_SAMPLE;
extern int LAST_SAMPLE;
extern int g_TrainWeekNumber;

extern TT_DATA_SET **g_tt_data_set; // train and test data set
extern int g_tt_num_samples;
extern int g_no_extra_predicted_sample;

extern double g_stock_price[MINUTES_IN_A_WEEK];
extern double g_ref_price[MINUTES_IN_A_WEEK];

extern int g_moving_average_current_minute;

extern int g_moving_average_first_day;

extern int g_trading_system_current_week;

extern char *g_state_string[];

extern int g_target_day_index;

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
void free_data_set(TRAIN_DATA_SET *data_set);
STATE get_state();
void save_state(STATE state);
int trading_system_state_machine(int moving_average_current_minute, int target_day_index, int trading_system_current_week);
int get_current_day(int target_day_index, int moving_average_current_minute);
double get_best_stock_maximum_buy_price(int target_day_index, int trading_system_current_week);
char *get_best_stock_name(int trading_system_current_week);
double get_best_stock_volume(int target_day_index, int moving_average_current_minute);
char *get_trading_system_time(char *str_now, int trading_system_current_week, int moving_average_current_minute);
STATE get_state_in_a_hot_start();
void process_one_file(int file_number);
void free_g_tt_data_set(int argc);
void load_intraday_data(char *stock_name, int semana);
void send_state();
