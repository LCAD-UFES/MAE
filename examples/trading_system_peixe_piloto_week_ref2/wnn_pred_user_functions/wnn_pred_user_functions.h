#define	NUM_DAY_IN_A_WEEK	5

struct _train_data_set
{
	long date_l;
	double avg;
	double clo;
	double max;
	double min;
	double opn;
	double vol;
	double week_return_buy_sell;
	double neuron_return_buy_sell;
	double wnn_predicted_return_buy_sell;
	double wnn_prediction_error_buy_sell;
	double week_return_sell_buy;
	double neuron_return_sell_buy;
	double wnn_predicted_return_sell_buy;
	double wnn_prediction_error_sell_buy;
	double confidence;
	double *intraday_price;
	double *intraday_volume;
};

typedef struct _train_data_set DATA_SET;


// Prototypes
void compute_target_return_and_avg ();
int ReadSampleInput(INPUT_DESC *input);
int ReadSampleInput2(INPUT_DESC *input);
double EvaluateOutputRiRj (OUTPUT_DESC *output);
void init_data_sets ();
double compute_sample_return (int sample);
