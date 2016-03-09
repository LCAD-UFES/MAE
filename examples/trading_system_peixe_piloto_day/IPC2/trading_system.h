#define CAPITAL	40000.0

#define	MyCAP_TRADING_COSTS

#define STATE_FILE_NAME	"state.bin"

#define BUY	1
#define SELL	0

#define	ORDER_EXECUTED		1
#define	ORDER_NOT_EXECUTED	0
#define	ORDER_CANCELED		-1

#define	SECONDS_IN_A_DAY	(24*60*60)
#define	MINUTES_IN_A_DAY	(7*60)


// Types

struct _tt_data_set
{
	long date_l;
	float target_return;
	float wnn_predicted_return;
	float wnn_prediction_error;
	int file_number;
};

typedef struct _tt_data_set TT_DATA_SET;


struct _train_data_set
{
	long date_l;
	float clo;
	float opn;
	float max;
	float min;
	float vol;
	float buy;
	float sell;
	float *intraday_price;
	float *intraday_volume;
};

typedef struct _train_data_set TRAIN_DATA_SET;


struct _buy_sell_files
{
	FILE *buy_file;
	FILE *sell_file;
};

typedef struct _buy_sell_files BUY_SELL_FILES;




enum STATE_NAME
{
	INITIALIZE,
	WAIT_BEGIN_DAY,
	SELECT_STOCK,
	WAIT_MIN_TO_BUY,
	SEND_BUY_ORDER,
	WAIT_BUY,
	WAIT_BUY_CANCEL,
	WAIT_MAX_TO_SELL,
	SEND_SELL_ORDER,
	WAIT_SELL,
	WAIT_SELL_CANCEL,
	QUIT,
	INVALID_STATE
};



struct _state
{
	int state;

	FILE *log_file;

	char now[1000];

	float capital;
	
	char  best_stock_name[1000];
	float best_stock_predicted_return;
	float best_stock_maximum_buy_price;
	float best_stock_minimum_sell_price;

	float price;
	float buy_price;
	float sell_price;
	float order_price;
	float moving_average_price;
	float previous_minimum_intraday_price;
	float previous_maximum_intraday_price;
	int quantity;
	int order_result;
	
	int buy_day;
	int buy_minute;
	int sell_day;
	int sell_minute;
	
	int current_minute;
	int buy_order_minute;
	int buy_cancel_minute;
	int sell_order_minute;
	int sell_cancel_minute;

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
extern int g_train_num_samples;
extern char g_current_data_set_name[1000];

extern int FIRST_SAMPLE;
extern int LAST_SAMPLE;
extern int g_TrainWeekNumber;

extern TT_DATA_SET **g_tt_data_set; // train and test data set
extern int g_tt_num_samples;
extern int g_no_extra_predicted_sample;

extern float g_price_samples[MINUTES_IN_A_DAY];
extern int g_moving_average_current_minute;

extern int g_moving_average_first_day;

extern int g_trading_system_current_day;

extern char *g_state_string[];

extern int g_target_day_index;

extern int g_stop_trading_system;


// Prototypes

void init_data_sets ();
void save_data_sets(char *dir_name, char *stock_name);
int find_day_index_by_date_l(long date_l);
void compute_target_return_and_avg ();
float moving_average(int minute);
void get_moving_average_price_sample(float *intraday_price, int day, int minute);
int intraday_data_available(int target_day);
float get_return_trading_system (int week, char *stock_name);
void generate_graph(char *graph_dir, char *stock_name, int day, float *intraday_price, int trade_minute, float price, int buy);
void free_g_train_data_set();
STATE get_state();
void save_state(STATE state);
int trading_system_state_machine(int moving_average_current_minute, int target_day_index, int trading_system_current_day);
int get_current_day(int target_day_index);
char *get_best_stock_name(int trading_system_current_day);
float get_best_stock_volume(int target_day_index, int moving_average_current_minute);
char *get_trading_system_time(char *str_now, int trading_system_current_day, int moving_average_current_minute);
STATE get_state_in_a_hot_start();
void process_one_file(int file_number);
void free_g_tt_data_set(int argc);
void load_intraday_data(char *stock_name, int semana);
void build_daily_buy_sell_files(int num_stocks, char **stocks_names);
STATE init_state();
void get_best_stock_quote(int target_day_index);
void load_daily_data(char **stocks_names, int stock_index);
double trade_cost(double capital);
float get_best_stock_maximum_buy_price(int target_day_index, int trading_system_current_day);
void load_intraday_data2(char **stocks_names, int stock_index);
int get_num_days_of_intraday_data(int stock, char **stocks_names);
void build_fake_tt_data_set(int stock, int num_days);
BUY_SELL_FILES open_buy_sell_files(char *base_file_name);
void moving_average_warm_up(int target_day_index);
void save_buy_sell_file_line(BUY_SELL_FILES buy_sell_files, int day, STATE state);
void close_buy_sell_files(BUY_SELL_FILES buy_sell_files);
