
#define	LAST_SAMPLE_IN_THE_GROUP(x)	(x + SAMPLE_GROUP_SIZE-1)

struct _data_set
{
	float sample_price;
	float sample_volume;
	float sample_return;
	float sample_group_return;
	float wnn_predicted_return;
	float wnn_prediction_error;
	char minute[11];
};

typedef struct _data_set DATA_SET;



// Prototypes
void compute_target_return_and_avg ();
int ReadSampleInput(INPUT_DESC *input, int input_sample);
int EvaluateOutput(OUTPUT_DESC *output, int *num_neurons);

#define PWB_PRED_EPSILON	0.000001 /* for camparing with zero */


/* Function prototypes */
int CalculateErrors(int n);
int doubleeqcmp(double d1, double d2, double epsilon);
