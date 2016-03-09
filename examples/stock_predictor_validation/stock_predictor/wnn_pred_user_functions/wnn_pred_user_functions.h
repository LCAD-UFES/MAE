

struct _train_data_set
{
	float week_return;
	float wnn_predicted_return;
	float wnn_prediction_error;
	char date[11];
};

typedef struct _train_data_set TRAIN_DATA_SET;



// Prototypes
void compute_target_return_and_avg ();
int ReadSampleInput(INPUT_DESC *input);
float EvaluateOutputRiRj (OUTPUT_DESC *output, int *num_neurons);

#define PWB_PRED_EPSILON				0.000001 /* for camparing with zero */


/* Function prototypes */
int CalculateErrors(int n);
int doubleeqcmp(double d1, double d2, double epsilon);
