#define	NUM_DAY_IN_A_WEEK	5

struct _train_data_set
{
	long date_l;
	float avg;
	float clo;
	float max;
	float min;
	float opn;
	float vol;
	float week_return;
	float neuron_return;
	float wnn_predicted_return;
	float wnn_prediction_error;
	float *intraday_price;
	float *intraday_volume;
};

typedef struct _train_data_set TRAIN_DATA_SET;


// Prototypes
void compute_target_return_and_avg ();
int ReadSampleInput(INPUT_DESC *input);
int ReadSampleInput2(INPUT_DESC *input);
float EvaluateOutputRiRj (OUTPUT_DESC *output);
void init_data_sets ();
float compute_sample_return (int sample);
