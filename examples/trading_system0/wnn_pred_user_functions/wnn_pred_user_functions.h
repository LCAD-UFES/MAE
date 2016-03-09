

struct _train_data_set
{
	long date_l;
	float avg;
	float clo;
	float max;
	float min;
	float opn;
	float trd;
	float vol;
	float week_return;
	float wnn_predicted_return;
	float wnn_prediction_error;
};

typedef struct _train_data_set TRAIN_DATA_SET;



// Prototypes
void compute_target_return_and_avg ();
int ReadSampleInput(INPUT_DESC *input);
float EvaluateOutputRiRj (OUTPUT_DESC *output, int *num_neurons);
