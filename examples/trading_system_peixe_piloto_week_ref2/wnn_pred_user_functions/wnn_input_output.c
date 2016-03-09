#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <sys/types.h>
#include <dirent.h>
#include <math.h>
#include "mae.h"
#include "../wnn_pred.h"
#include "wnn_pred_user_functions.h"
#include "timeutil.h"



extern DATA_SET *g_train_data_set;
extern DATA_SET *g_ref_data_set;
extern int FIRST_SAMPLE;
extern int LAST_SAMPLE;
extern int g_TargetWeekNumber;
extern int g_train_num_samples;

extern double g_train_return_average;
extern double g_train_return_standard_deviation;


#define BUY	1
#define SELL	0



double
moving_average(int day)
{
	int i;
	double m_average = 0.0, num_samples = 0.0;
	
	for (i = day - (MOVING_AVERAGE_PRED_NUM_PERIODS + 1) * NUM_DAY_IN_A_WEEK; i <= day; i += NUM_DAY_IN_A_WEEK)
	{
		if (i > 0)
		{
			m_average += g_train_data_set[i].clo;
			num_samples += 1.0;
		}
		else
		{
			m_average += g_train_data_set[i].clo;
			num_samples += 1.0;
		}		
	}

	return (m_average / num_samples);
}



#if TYPE_OF_WNN_INPUT == 1

// retorno diario com relacao a quinta anterior 
double
compute_sample_return (int sample)
{
	double sample_return;
	double previous_previous_thursday;
	double sample_value;
	int previous_previous_thursday_index, sample_index;
	
	previous_previous_thursday_index = FIRST_SAMPLE + NUM_DAY_IN_A_WEEK * g_TargetWeekNumber - 2*NUM_DAY_IN_A_WEEK + 1;
	sample_index = FIRST_SAMPLE + NUM_DAY_IN_A_WEEK * g_TargetWeekNumber + sample - (NUM_DAY_IN_A_WEEK-1);
	
	previous_previous_thursday = g_train_data_set[previous_previous_thursday_index].clo;
	sample_value = g_train_data_set[sample_index].clo;
	sample_return = (sample_value - previous_previous_thursday) / previous_previous_thursday;

#if VERBOSE == 1
	char Tdate[100], Sdate[100], Tdow[100], Sdow[100]; // Tdata = target date; Sdata = sample date

	strcpy (Tdate, long2isodate (g_train_data_set[previous_previous_thursday_index].date_l));
	strcpy (Sdate, long2isodate (g_train_data_set[sample_index].date_l));
	strcpy (Tdow, long2dows (g_train_data_set[previous_previous_thursday_index].date_l));
	strcpy (Sdow, long2dows (g_train_data_set[sample_index].date_l));
	printf ("#|%s-%s/%s-%s ", Tdate, Tdow, Sdate, Sdow);
#endif
	return (sample_return);
}


#elif TYPE_OF_WNN_INPUT == 2

// retorno semanal com referencia movel
double
compute_sample_return (int sample)
{
	int moving_reference_index;
	int sample_index;
	double moving_reference_price;
	double sample_price;
	double sample_return;
	
	moving_reference_index = FIRST_SAMPLE + NUM_DAY_IN_A_WEEK * g_TargetWeekNumber - (SAMPLE_SIZE + 1) * NUM_DAY_IN_A_WEEK;
	sample_index = FIRST_SAMPLE + NUM_DAY_IN_A_WEEK * g_TargetWeekNumber + NUM_DAY_IN_A_WEEK * sample;
	moving_reference_price = g_train_data_set[moving_reference_index].clo;
	sample_price =    g_train_data_set[sample_index].clo;
	
	sample_return = (sample_price - moving_reference_price) / moving_reference_price;

#if VERBOSE == 1
	char Rdate[100], Sdate[100], Rdow[100], Sdow[100]; // R -> moving_reference_day; S -> sample_day

	strcpy (Rdate, long2isodate (g_train_data_set[moving_reference_index].date_l));
	strcpy (Sdate, long2isodate (g_train_data_set[sample_index].date_l));
	strcpy (Rdow, long2dows (g_train_data_set[moving_reference_index].date_l));
	strcpy (Sdow, long2dows (g_train_data_set[sample_index].date_l));
	printf ("#|%s-%s/%s-%s ", Rdate, Rdow, Sdate, Sdow);
#endif

	return (sample_return);
}


#elif TYPE_OF_WNN_INPUT == 3

// razao entre o preco da acao e uma referencia
double
compute_sample_return(int sample)
{
	int moving_reference_index;
	int sample_index;
	double sample_return;
	double factor;
	
	moving_reference_index = FIRST_SAMPLE + NUM_DAY_IN_A_WEEK * g_TargetWeekNumber - (SAMPLE_SIZE + 1) * NUM_DAY_IN_A_WEEK;
	sample_index = FIRST_SAMPLE + NUM_DAY_IN_A_WEEK * g_TargetWeekNumber + NUM_DAY_IN_A_WEEK * sample;
	
	factor = g_ref_data_set[moving_reference_index].clo / g_train_data_set[moving_reference_index].clo;

	sample_return = factor * (g_train_data_set[sample_index].clo / g_ref_data_set[sample_index].clo) - 1.0;

#if VERBOSE == 1
	char Rdate[100], Sdate[100], Rdow[100], Sdow[100]; // R -> moving_reference_day; S -> sample_day

	strcpy (Rdate, long2isodate (g_ref_data_set[moving_reference_index].date_l));
	strcpy (Sdate, long2isodate (g_train_data_set[sample_index].date_l));
	strcpy (Rdow, long2dows (g_ref_data_set[moving_reference_index].date_l));
	strcpy (Sdow, long2dows (g_train_data_set[sample_index].date_l));
	printf ("#|%s-%s/%s-%s ", Rdate, Rdow, Sdate, Sdow);
#endif

	return (sample_return);
}


#elif TYPE_OF_WNN_INPUT == 4

// diferenca entre a acao e sua moving average
double
compute_sample_return(int sample)
{
	int sample_index;
	double sample_return;
	
	sample_index = FIRST_SAMPLE + NUM_DAY_IN_A_WEEK * g_TargetWeekNumber + NUM_DAY_IN_A_WEEK * sample;

	sample_return = g_train_data_set[sample_index].clo - moving_average(sample_index);

#if VERBOSE == 1
	char Rdate[100], Sdate[100], Rdow[100], Sdow[100]; // R -> moving_reference_day; S -> sample_day

	strcpy (Rdate, long2isodate (g_ref_data_set[moving_reference_index].date_l));
	strcpy (Sdate, long2isodate (g_train_data_set[sample_index].date_l));
	strcpy (Rdow, long2dows (g_ref_data_set[moving_reference_index].date_l));
	strcpy (Sdow, long2dows (g_train_data_set[sample_index].date_l));
	printf ("#|%s-%s/%s-%s ", Rdate, Rdow, Sdate, Sdow);
#endif

	return (sample_return);
}


#elif TYPE_OF_WNN_INPUT == 5

// retorno da media movel semanal com referencia movel
double
compute_sample_return(int sample)
{
	int moving_reference_index;
	int sample_index;
	double moving_reference_price;
	double sample_price;
	double sample_return;
	
	moving_reference_index = FIRST_SAMPLE + NUM_DAY_IN_A_WEEK * g_TargetWeekNumber - (SAMPLE_SIZE + 1) * NUM_DAY_IN_A_WEEK;
	sample_index = FIRST_SAMPLE + NUM_DAY_IN_A_WEEK * g_TargetWeekNumber + NUM_DAY_IN_A_WEEK * sample;
	moving_reference_price = moving_average(moving_reference_index);
	sample_price = moving_average(sample_index);
	
	sample_return = (sample_price - moving_reference_price) / moving_reference_price;

#if VERBOSE == 1
	char Rdate[100], Sdate[100], Rdow[100], Sdow[100]; // R -> moving_reference_day; S -> sample_day

	strcpy (Rdate, long2isodate (g_train_data_set[moving_reference_index].date_l));
	strcpy (Sdate, long2isodate (g_train_data_set[sample_index].date_l));
	strcpy (Rdow, long2dows (g_train_data_set[moving_reference_index].date_l));
	strcpy (Sdow, long2dows (g_train_data_set[sample_index].date_l));
	printf ("#|%s-%s/%s-%s ", Rdate, Rdow, Sdate, Sdow);
#endif

	return (sample_return);
}


#elif TYPE_OF_WNN_INPUT == 6

// diferenca semanal entre o preco de uma acao e uma acao/indice de referencia (normalizados por referencia movel)
double
compute_sample_return(int sample)
{
	int moving_reference_index;
	int sample_index;
	double sample_return;
	double factor_ref;
	double factor_stock;
	
	moving_reference_index = FIRST_SAMPLE + NUM_DAY_IN_A_WEEK * g_TargetWeekNumber - (SAMPLE_SIZE + 1) * NUM_DAY_IN_A_WEEK;
	sample_index = FIRST_SAMPLE + NUM_DAY_IN_A_WEEK * g_TargetWeekNumber + NUM_DAY_IN_A_WEEK * sample;
	
	factor_ref = g_ref_data_set[moving_reference_index].clo;
	factor_stock = g_train_data_set[moving_reference_index].clo;

	sample_return = (g_train_data_set[sample_index].clo / factor_stock) - (g_ref_data_set[sample_index].clo / factor_ref);

#if VERBOSE == 1
	char Rdate[100], Sdate[100], Rdow[100], Sdow[100]; // R -> moving_reference_day; S -> sample_day

	strcpy (Rdate, long2isodate (g_ref_data_set[moving_reference_index].date_l));
	strcpy (Sdate, long2isodate (g_train_data_set[sample_index].date_l));
	strcpy (Rdow, long2dows (g_ref_data_set[moving_reference_index].date_l));
	strcpy (Sdow, long2dows (g_train_data_set[sample_index].date_l));
	printf ("#|%s-%s/%s-%s ", Rdate, Rdow, Sdate, Sdow);
#endif

	return (sample_return);
}


#elif TYPE_OF_WNN_INPUT == 7

// diferenca diaria entre o preco de uma acao e uma acao/indice de referencia (normalizados pela quinta anterior)
double
compute_sample_return(int sample)
{
	int previous_previous_thursday_index;
	int sample_index;
	double sample_return;
	double factor_ref;
	double factor_stock;
	
	previous_previous_thursday_index = FIRST_SAMPLE + NUM_DAY_IN_A_WEEK * g_TargetWeekNumber - 2*NUM_DAY_IN_A_WEEK + 1;
	sample_index = FIRST_SAMPLE + NUM_DAY_IN_A_WEEK * g_TargetWeekNumber + sample - (NUM_DAY_IN_A_WEEK-1);
	
	factor_ref = g_ref_data_set[previous_previous_thursday_index].clo;
	factor_stock = g_train_data_set[previous_previous_thursday_index].clo;

	sample_return = (g_train_data_set[sample_index].clo / factor_stock) - (g_ref_data_set[sample_index].clo / factor_ref);

#if VERBOSE == 1
	char Rdate[100], Sdate[100], Rdow[100], Sdow[100]; // R -> moving_reference_day; S -> sample_day

	strcpy (Rdate, long2isodate (g_ref_data_set[previous_previous_thursday_index].date_l));
	strcpy (Sdate, long2isodate (g_train_data_set[sample_index].date_l));
	strcpy (Rdow, long2dows (g_ref_data_set[previous_previous_thursday_index].date_l));
	strcpy (Sdow, long2dows (g_train_data_set[sample_index].date_l));
	printf ("#|%s-%s/%s-%s ", Rdate, Rdow, Sdate, Sdow);
#endif

	return (sample_return);
}


#else

// retorno semanal simples
double
compute_sample_return (int sample)
{
	double sample_return;
	
	sample_return = g_train_data_set[FIRST_SAMPLE + NUM_DAY_IN_A_WEEK * g_TargetWeekNumber + NUM_DAY_IN_A_WEEK * sample].week_return_buy_sell;

#if VERBOSE == 1
	char Tdate[100], Sdate[100], Tdow[100], Sdow[100]; // T -> target_day; S -> sample_day

	strcpy (Tdate, long2isodate (g_train_data_set[FIRST_SAMPLE + NUM_DAY_IN_A_WEEK * g_TargetWeekNumber].date_l));
	strcpy (Sdate, long2isodate (g_train_data_set[FIRST_SAMPLE + NUM_DAY_IN_A_WEEK * g_TargetWeekNumber + NUM_DAY_IN_A_WEEK * sample].date_l));
	strcpy (Tdow, long2dows (g_train_data_set[FIRST_SAMPLE + NUM_DAY_IN_A_WEEK * g_TargetWeekNumber].date_l));
	strcpy (Sdow, long2dows (g_train_data_set[FIRST_SAMPLE + NUM_DAY_IN_A_WEEK * g_TargetWeekNumber + NUM_DAY_IN_A_WEEK * sample].date_l));
	printf ("#|%s-%s/%s-%s ", Tdate, Tdow, Sdate, Sdow);
#endif

	return (sample_return);
}


#endif


#if TYPE_OF_WNN_INPUT2 == 1


double
compute_sample_return2(int sample)
{
	double sample_return;
	double previous_previous_thursday;
	double sample_value;
	int base_day_index, sample_index;
	
	base_day_index = FIRST_SAMPLE + NUM_DAY_IN_A_WEEK*g_TargetWeekNumber - 2*NUM_DAY_IN_A_WEEK + 1;
	sample_index = FIRST_SAMPLE + NUM_DAY_IN_A_WEEK*g_TargetWeekNumber + sample - (NUM_DAY_IN_A_WEEK-1);
	
	previous_previous_thursday = g_train_data_set[base_day_index].avg;
	sample_value = g_train_data_set[sample_index].avg;
	sample_return = (sample_value - previous_previous_thursday) / previous_previous_thursday;

#if VERBOSE == 1
	char Tdate[100], Sdate[100], Tdow[100], Sdow[100]; // Tdata = target date; Sdata = sample date

	strcpy (Tdate, long2isodate (g_train_data_set[base_day_index].date_l));
	strcpy (Sdate, long2isodate (g_train_data_set[sample_index].date_l));
	strcpy (Tdow, long2dows (g_train_data_set[base_day_index].date_l));
	strcpy (Sdow, long2dows (g_train_data_set[sample_index].date_l));
	printf ("#|%s-%s/%s-%s ", Tdate, Tdow, Sdate, Sdow);
#endif
	return (sample_return);
}


#elif TYPE_OF_WNN_INPUT2 == 2


double
compute_sample_return2(int sample)
{
	int moving_reference_index;
	int sample_index;
	double moving_reference_price;
	double sample_price;
	double sample_return;
	
	moving_reference_index = FIRST_SAMPLE + NUM_DAY_IN_A_WEEK * g_TargetWeekNumber - (SAMPLE_SIZE + 1) * NUM_DAY_IN_A_WEEK;
	sample_index = FIRST_SAMPLE + NUM_DAY_IN_A_WEEK * g_TargetWeekNumber + NUM_DAY_IN_A_WEEK * sample;
	moving_reference_price = g_train_data_set[moving_reference_index].clo;
	sample_price =    g_train_data_set[sample_index].clo;
	
	sample_return = (sample_price - moving_reference_price) / moving_reference_price;

#if VERBOSE == 1
	char Rdate[100], Sdate[100], Rdow[100], Sdow[100]; // R -> moving_reference_day; S -> sample_day

	strcpy (Rdate, long2isodate (g_train_data_set[moving_reference_index].date_l));
	strcpy (Sdate, long2isodate (g_train_data_set[sample_index].date_l));
	strcpy (Rdow, long2dows (g_train_data_set[moving_reference_index].date_l));
	strcpy (Sdow, long2dows (g_train_data_set[sample_index].date_l));
	printf ("#|%s-%s/%s-%s ", Rdate, Rdow, Sdate, Sdow);
#endif

	return (sample_return);
}


#else


double
compute_sample_return2(int sample)
{
	double sample_return;
	
	sample_return = g_train_data_set[FIRST_SAMPLE + NUM_DAY_IN_A_WEEK * g_TargetWeekNumber + NUM_DAY_IN_A_WEEK * sample].week_return;

#if VERBOSE == 1
	char Tdate[100], Sdate[100], Tdow[100], Sdow[100]; // T -> target_day; S -> sample_day

	strcpy (Tdate, long2isodate (g_train_data_set[FIRST_SAMPLE + NUM_DAY_IN_A_WEEK * g_TargetWeekNumber].date_l));
	strcpy (Sdate, long2isodate (g_train_data_set[FIRST_SAMPLE + NUM_DAY_IN_A_WEEK * g_TargetWeekNumber + NUM_DAY_IN_A_WEEK * sample].date_l));
	strcpy (Tdow, long2dows (g_train_data_set[FIRST_SAMPLE + NUM_DAY_IN_A_WEEK * g_TargetWeekNumber].date_l));
	strcpy (Sdow, long2dows (g_train_data_set[FIRST_SAMPLE + NUM_DAY_IN_A_WEEK * g_TargetWeekNumber + NUM_DAY_IN_A_WEEK * sample].date_l));
	printf ("#|%s-%s/%s-%s ", Tdate, Tdow, Sdate, Sdow);
#endif

	return (sample_return);
}


#endif



double
normalized_sigmoid(double x)
{
	double sigmoid;
	
	//sigmoid = 1.0 / (1.0 + exp(-((x - g_train_return_average) / g_train_return_standard_deviation)));
	sigmoid = (x - g_train_return_average) / (2.0 * M_PI * g_train_return_standard_deviation) + 0.5;
	
	return (sigmoid);
}



#if POINTS == 1

/*
***********************************************************
* Function: ReadSampleInput
* Description: Writes a set of returns into the MAE input
* Inputs: input - input image
* Output: 0 if OK, -1 otherwise
***********************************************************
*/

int 
ReadSampleInput(INPUT_DESC *input)
{
	int x, y, x_w, y_h, stride, sample;
	NEURON *neuron;
	double sample_return, normalized_sample_return;
	int position;

	neuron = input->neuron_layer->neuron_vector;
	x_w = input->neuron_layer->dimentions.x;
	y_h = input->neuron_layer->dimentions.y;
	stride = x_w / SAMPLE_SIZE;
	sample = - SAMPLE_SIZE;
	// printf ("\n%03d: %s %s ; ", 
	//	g_TargetWeekNumber, 
	//	long2isodate (timePlusDays(g_train_data_set[FIRST_SAMPLE + NUM_DAY_IN_A_WEEK * (g_TargetWeekNumber-1)].date_l, NUM_DAY_IN_A_WEEK)),
	//	long2dows (timePlusDays(g_train_data_set[FIRST_SAMPLE + NUM_DAY_IN_A_WEEK * (g_TargetWeekNumber-1)].date_l, NUM_DAY_IN_A_WEEK)));
	for (x = 0; x < x_w; x++)
	{
		if (((x - stride/2) % stride) == 0)
		{
			sample_return = compute_sample_return (sample);
			// printf ("% .6f  ; ", sample_return);
			// normalized_sample_return = (sample_return - MIN_RETURN) / (MAX_RETURN - MIN_RETURN);
			normalized_sample_return = normalized_sigmoid(sample_return);
			for (y = 0; y < y_h; y++)
			{
				position = (int) (normalized_sample_return * (double) y_h);
				if (position >= y_h)
					position = y_h - 1;
				else if (position < 0)
					position = 0;

				if (position == y)
					neuron[y * x_w + x].output.fval = (float) 1.0;
				else
					neuron[y * x_w + x].output.fval = (float) 0.0;
			}
			sample += 1;
			if (sample >= 0)
				break;
		}
		else
		{
			for (y = 0; y < y_h; y++)
				neuron[y * x_w + x].output.fval = (float) 0.0;
		}
	}
#if VERBOSE == 1
	/* Target return for sample g_TargetWeekNumber */
	printf("-> % .6f\n", g_train_data_set[FIRST_SAMPLE + NUM_DAY_IN_A_WEEK * g_TargetWeekNumber].week_return);
	printf("week id = %d\n", g_TargetWeekNumber);
#endif

	return (0);
}


#else


/*
***********************************************************
* Function: ReadSampleInput
* Description: Writes a set of returns into the MAE input
* Inputs: input - input image
* Output: 0 if OK, -1 otherwise
***********************************************************
*/

int 
ReadSampleInput(INPUT_DESC *input)
{
	int x, y, x_w, y_h, stride, sample;
	NEURON *neuron;
	double sample_return, normalized_sample_return;
	int position;

	neuron = input->neuron_layer->neuron_vector;
	x_w = input->neuron_layer->dimentions.x;
	y_h = input->neuron_layer->dimentions.y;
	stride = x_w / SAMPLE_SIZE;
	sample = - SAMPLE_SIZE;
	// printf ("\n%03d: %s %s ; ", 
	//	g_TargetWeekNumber, 
	//	long2isodate (timePlusDays(g_train_data_set[FIRST_SAMPLE + NUM_DAY_IN_A_WEEK * (g_TargetWeekNumber-1)].date_l, NUM_DAY_IN_A_WEEK)),
	//	long2dows (timePlusDays(g_train_data_set[FIRST_SAMPLE + NUM_DAY_IN_A_WEEK * (g_TargetWeekNumber-1)].date_l, NUM_DAY_IN_A_WEEK)));
	for (x = 0; x < x_w; x++)
	{
		if (((x - stride/2) % stride) == 0)
		{
			sample_return = compute_sample_return (sample);
			// printf ("% .6f  ; ", sample_return);
			// normalized_sample_return = (sample_return - MIN_RETURN) / (MAX_RETURN - MIN_RETURN);
			normalized_sample_return = normalized_sigmoid(sample_return);
			for (y = 0; y < y_h; y++)
			{
				position = (int) (normalized_sample_return * (double) y_h);
				if (position >= y_h)
					position = y_h - 1;
				else if (position < 0)
					position = 0;

				if ((y >= y_h/2) && (y <= position && position != y_h/2))
					neuron[y * x_w + x].output.fval = (float) 1.0;
				else if ((y < y_h/2) && (y >= position))
					neuron[y * x_w + x].output.fval = (float) 1.0;
				else
					neuron[y * x_w + x].output.fval = (float) 0.0;
			}
			sample += 1;
			if (sample >= 0)
				break;
		}
		else
		{
			for (y = 0; y < y_h; y++)
				neuron[y * x_w + x].output.fval = (float) 0.0;
		}
	}
#if VERBOSE == 1
	/* Target return for sample g_TargetWeekNumber */
	printf ("-> % .6f\n", g_train_data_set[FIRST_SAMPLE + NUM_DAY_IN_A_WEEK * g_TargetWeekNumber].week_return_buy_sell);
#endif

	return (0);
}

#endif



#if POINTS2 == 1

/*
***********************************************************
* Function: ReadSampleInput
* Description: Writes a set of returns into the MAE input
* Inputs: input - input image
* Output: 0 if OK, -1 otherwise
***********************************************************
*/

int 
ReadSampleInput2(INPUT_DESC *input)
{
	int x, y, x_w, y_h, stride, sample;
	NEURON *neuron;
	double sample_return, normalized_sample_return;
	int position;

	neuron = input->neuron_layer->neuron_vector;
	x_w = input->neuron_layer->dimentions.x;
	y_h = input->neuron_layer->dimentions.y;
	stride = x_w / SAMPLE_SIZE;
	sample = - SAMPLE_SIZE;
	// printf ("\n%03d: %s %s ; ", 
	//	g_TargetWeekNumber, 
	//	long2isodate (timePlusDays(g_train_data_set[FIRST_SAMPLE + NUM_DAY_IN_A_WEEK * (g_TargetWeekNumber-1)].date_l, NUM_DAY_IN_A_WEEK)),
	//	long2dows (timePlusDays(g_train_data_set[FIRST_SAMPLE + NUM_DAY_IN_A_WEEK * (g_TargetWeekNumber-1)].date_l, NUM_DAY_IN_A_WEEK)));
	for (x = 0; x < x_w; x++)
	{
		if (((x - stride/2) % stride) == 0)
		{
			sample_return = compute_sample_return2(sample);
			// printf ("% .6f  ; ", sample_return);
			// normalized_sample_return = (sample_return - MIN_RETURN) / (MAX_RETURN - MIN_RETURN);
			normalized_sample_return = normalized_sigmoid(sample_return);
			for (y = 0; y < y_h; y++)
			{
				position = (int) (normalized_sample_return * (double) y_h);
				if (position >= y_h)
					position = y_h - 1;
				else if (position < 0)
					position = 0;

				if (position == y)
					neuron[y * x_w + x].output.fval = (float) 1.0;
				else
					neuron[y * x_w + x].output.fval = (float) 0.0;
			}
			sample += 1;
			if (sample >= 0)
				break;
		}
		else
		{
			for (y = 0; y < y_h; y++)
				neuron[y * x_w + x].output.fval = (float) 0.0;
		}
	}
#if VERBOSE == 1
	/* Target return for sample g_TargetWeekNumber */
	printf("-> % .6f\n", g_train_data_set[FIRST_SAMPLE + NUM_DAY_IN_A_WEEK * g_TargetWeekNumber].week_return);
	printf("week id = %d\n", g_TargetWeekNumber);
#endif

	return (0);
}


#else


/*
***********************************************************
* Function: ReadSampleInput
* Description: Writes a set of returns into the MAE input
* Inputs: input - input image
* Output: 0 if OK, -1 otherwise
***********************************************************
*/

int 
ReadSampleInput2(INPUT_DESC *input)
{
	int x, y, x_w, y_h, stride, sample;
	NEURON *neuron;
	double sample_return, normalized_sample_return;
	int position;

	neuron = input->neuron_layer->neuron_vector;
	x_w = input->neuron_layer->dimentions.x;
	y_h = input->neuron_layer->dimentions.y;
	stride = x_w / SAMPLE_SIZE;
	sample = - SAMPLE_SIZE;
	// printf ("\n%03d: %s %s ; ", 
	//	g_TargetWeekNumber, 
	//	long2isodate (timePlusDays(g_train_data_set[FIRST_SAMPLE + NUM_DAY_IN_A_WEEK * (g_TargetWeekNumber-1)].date_l, NUM_DAY_IN_A_WEEK)),
	//	long2dows (timePlusDays(g_train_data_set[FIRST_SAMPLE + NUM_DAY_IN_A_WEEK * (g_TargetWeekNumber-1)].date_l, NUM_DAY_IN_A_WEEK)));
	for (x = 0; x < x_w; x++)
	{
		if (((x - stride/2) % stride) == 0)
		{
			sample_return = compute_sample_return2(sample);
			// printf ("% .6f  ; ", sample_return);
			// normalized_sample_return = (sample_return - MIN_RETURN) / (MAX_RETURN - MIN_RETURN);
			normalized_sample_return = normalized_sigmoid(sample_return);
			for (y = 0; y < y_h; y++)
			{
				position = (int) (normalized_sample_return * (double) y_h);
				if (position >= y_h)
					position = y_h - 1;
				else if (position < 0)
					position = 0;

				if ((y >= y_h/2) && (y <= position && position != y_h/2))
					neuron[y * x_w + x].output.fval = (float) 1.0;
				else if ((y < y_h/2) && (y >= position))
					neuron[y * x_w + x].output.fval = (float) 1.0;
				else
					neuron[y * x_w + x].output.fval = (float) 0.0;
			}
			sample += 1;
			if (sample >= 0)
				break;
		}
		else
		{
			for (y = 0; y < y_h; y++)
				neuron[y * x_w + x].output.fval = (float) 0.0;
		}
	}
#if VERBOSE == 1
	/* Target return for sample g_TargetWeekNumber */
	printf ("-> % .6f\n", g_train_data_set[FIRST_SAMPLE + NUM_DAY_IN_A_WEEK * g_TargetWeekNumber].week_return_buy_sell);
#endif

	return (0);
}

#endif



#if TYPE_OF_RETURN_AND_AVERAGE == 1

void
compute_target_return_and_avg ()
{
	int i, j;
	double max;
	
	for (i = 0; i < g_train_num_samples; i++)
		g_train_data_set[i].avg = (g_train_data_set[i].max + g_train_data_set[i].min) / 2.0;
		
	for (i = 0; i < g_train_num_samples; i++)
	{
		if (i >= FIRST_SAMPLE)
		{
			max = g_train_data_set[i].avg;
			for (j = 1; j < NUM_DAY_IN_A_WEEK; j++)
			{
				if (g_train_data_set[i-j].avg > max)
					max = g_train_data_set[i-j].avg;
			}
			/* quando a refer�ncia � quarta,	 g_train_data_set[i-NUM_DAY_IN_A_WEEK].max = quarta passada */
			g_train_data_set[i].week_return = (max - g_train_data_set[i-NUM_DAY_IN_A_WEEK].avg) / g_train_data_set[i-NUM_DAY_IN_A_WEEK].avg;
		}
		else
			g_train_data_set[i].week_return = 0.0;
			
		g_train_data_set[i].neuron_return = g_train_data_set[i].week_return;
	}
}

#elif TYPE_OF_RETURN_AND_AVERAGE == 2

void
compute_target_return_and_avg ()
{
	int i, reference_index;
	
	for (i = 0; i < g_train_num_samples; i++)
		g_train_data_set[i].avg = (g_train_data_set[i].max + g_train_data_set[i].min) / 2.0;
		
	for (i = (SAMPLE_SIZE + 1) * NUM_DAY_IN_A_WEEK; i < g_train_num_samples; i++)
	{
		reference_index = i - (SAMPLE_SIZE + 1) * NUM_DAY_IN_A_WEEK;

		g_train_data_set[i].week_return = (g_train_data_set[i].clo - g_train_data_set[reference_index].clo) / g_train_data_set[reference_index].clo;
	}
}


#elif TYPE_OF_RETURN_AND_AVERAGE == 3

// Retorno aproximado com compra no preco medio na quinta/sexta; 
// so compra com retorno contra a quarta anterior menor que START_BUYING_RETURN (se nao ocorrer, pula a semana);
// vende se o preco medio no periodo de sexta/segunda levar a um retorno > STOP_GAIN ou < STOP_LOSS;
// se nao sair por STOP_GAIN ou STOP_LOSS vende no fechamento da semana.

void
compute_target_return_and_avg ()
{
	int i, j;
	double buy_price, sell_price, previous_wed_price, trade_price;
	int first_buy_day, last_buy_day, buy_day, first_sell_day, last_sell_day, sell_day;
	
	for (i = 0; i < g_train_num_samples; i++)
		g_train_data_set[i].avg = (g_train_data_set[i].max + g_train_data_set[i].min) / 2.0;
		
	for (i = 0; i < g_train_num_samples; i++)
	{
		if (i >= FIRST_SAMPLE)
		{
			// buy -> sell
			previous_wed_price = g_train_data_set[i-NUM_DAY_IN_A_WEEK].clo;
			first_buy_day = i - (NUM_DAY_IN_A_WEEK-1);
			buy_day = last_buy_day = first_buy_day + 1;
			buy_price = 0.0;
			for (j = first_buy_day; j <= last_buy_day; j++)
			{
				trade_price = g_train_data_set[j].avg - (g_train_data_set[j].max - g_train_data_set[j].min) / BUY_SELL_GAIN_DIVIDER;
				if (trade_price < (previous_wed_price * (1.0 + START_BUYING_RETURN)))
				{
					buy_price = trade_price;
					buy_day = j;
					break;
				}
			}
			if (buy_price == 0.0) // skip week
			{
				g_train_data_set[i].week_return_buy_sell = 0.0;
			}
			else
			{
				first_sell_day = buy_day + 1;
				last_sell_day =  i;
				for (j = first_sell_day; j <= last_sell_day; j++)
				{
					trade_price = g_train_data_set[j].avg + (g_train_data_set[j].max - g_train_data_set[j].min) / BUY_SELL_GAIN_DIVIDER;
					if (trade_price > (buy_price * (1.0 + STOP_GAIN)))
					{
						sell_price = trade_price;
						g_train_data_set[i].week_return_buy_sell = (sell_price - buy_price) / buy_price;
						break;
					}
					if (trade_price < (buy_price * (1.0 + STOP_LOSS)))
					{
						sell_price = trade_price;
						g_train_data_set[i].week_return_buy_sell = (sell_price - buy_price) / buy_price;
						break;
					}
				}
				if (j == (last_sell_day + 1)) // Nao conseguiu vender
				{
					sell_price = g_train_data_set[i].clo;
					g_train_data_set[i].week_return_buy_sell = (sell_price - buy_price) / buy_price;
				}
			}
			
			// sell -> buy
			previous_wed_price = g_train_data_set[i-NUM_DAY_IN_A_WEEK].clo;
			first_sell_day = i - (NUM_DAY_IN_A_WEEK-1);
			sell_day = last_sell_day = first_sell_day + 1;
			sell_price = 0.0;
			for (j = first_sell_day; j <= last_sell_day; j++)
			{
				trade_price = g_train_data_set[j].avg + (g_train_data_set[j].max - g_train_data_set[j].min) / SELL_BUY_GAIN_DIVIDER;
				if (trade_price > (previous_wed_price * (1.0 + START_SELLING_RETURN)))
				{
					sell_price = trade_price;
					sell_day = j;
					break;
				}
			}
			if (sell_price == 0.0) // skip week
			{
				g_train_data_set[i].week_return_sell_buy = 0.0;
			}
			else
			{
				first_buy_day = sell_day + 1;
				last_buy_day =  i;
				for (j = first_buy_day; j <= last_buy_day; j++)
				{
					trade_price = g_train_data_set[j].avg - (g_train_data_set[j].max - g_train_data_set[j].min) / SELL_BUY_GAIN_DIVIDER;
					if (trade_price < (sell_price * (1.0 - STOP_GAIN)))
					{
						buy_price = trade_price;
						g_train_data_set[i].week_return_sell_buy = (sell_price - buy_price) / sell_price;
						break;
					}
					if (trade_price > (sell_price * (1.0 - STOP_LOSS)))
					{
						buy_price = trade_price;
						g_train_data_set[i].week_return_sell_buy = (sell_price - buy_price) / sell_price;
						break;
					}
				}
				if (j == (last_buy_day + 1)) // Nao conseguiu comprar
				{
					buy_price = g_train_data_set[i].clo;
					g_train_data_set[i].week_return_sell_buy = (sell_price - buy_price) / sell_price;
				}
			}
		}
		else
		{
			g_train_data_set[i].week_return_buy_sell = 0.0;
			g_train_data_set[i].week_return_sell_buy = 0.0;
		}

		g_train_data_set[i].neuron_return_buy_sell = g_train_data_set[i].week_return_buy_sell;
		g_train_data_set[i].neuron_return_sell_buy = g_train_data_set[i].week_return_sell_buy;
	}
}


#elif TYPE_OF_RETURN_AND_AVERAGE == 4

void
compute_target_return_and_avg ()
{
	int i, j;
	double buy_price, sell_price, previous_wed_price;
	int first_buy_day, last_buy_day, buy_day, first_sell_day, last_sell_day;
	
	for (i = 0; i < g_train_num_samples; i++)
		g_train_data_set[i].avg = (g_train_data_set[i].max + g_train_data_set[i].min) / 2.0;
		
	for (i = 0; i < g_train_num_samples; i++)
	{
		if (i >= FIRST_SAMPLE)
		{
			previous_wed_price = g_train_data_set[i-NUM_DAY_IN_A_WEEK].clo;
			first_buy_day = i - (NUM_DAY_IN_A_WEEK-1);
			buy_day = last_buy_day = first_buy_day;
			buy_price = 0.0;
			for (j = first_buy_day; j <= last_buy_day; j++)
			{
				if (g_train_data_set[j].avg < (previous_wed_price * (1.0 + START_BUYING_RETURN)))
				{
					buy_price = g_train_data_set[j].avg;
					buy_day = j;
					break;
				}
			}
			if (buy_price == 0.0)
			{
				g_train_data_set[i].week_return = 0.0;
			}
			else
			{
				first_sell_day = buy_day + 1;
				last_sell_day =  i;
				for (j = first_sell_day; j <= last_sell_day; j++)
				{
					if (g_train_data_set[j].avg > (buy_price * (1.0 + STOP_GAIN)))
					{
						sell_price = g_train_data_set[j].avg;
						g_train_data_set[i].week_return = (sell_price - buy_price) / buy_price;
						break;
					}
					if (g_train_data_set[j].avg < (buy_price * (1.0 + STOP_LOSS)))
					{
						sell_price = g_train_data_set[j].avg;
						g_train_data_set[i].week_return = (sell_price - buy_price) / buy_price;
						break;
					}
				}
				if (j == (last_sell_day + 1)) // Nao conseguiu vender
				{
					buy_price = g_train_data_set[i].clo;
					g_train_data_set[i].week_return_sell_buy = (sell_price - buy_price) / sell_price;
				}
			}
		}
		else
			g_train_data_set[i].week_return = 0.0;

		g_train_data_set[i].neuron_return = g_train_data_set[i].week_return;
	}
}

#elif TYPE_OF_RETURN_AND_AVERAGE == 5

void
compute_target_return_and_avg ()
{
	int i;
	double previous_thu_price;
	
	for (i = 0; i < g_train_num_samples; i++)
		g_train_data_set[i].avg = (g_train_data_set[i].max + g_train_data_set[i].min) / 2.0;
		
	for (i = (SAMPLE_SIZE + 1) * NUM_DAY_IN_A_WEEK; i < g_train_num_samples; i++)
	{
		previous_thu_price = g_train_data_set[i-NUM_DAY_IN_A_WEEK+1].avg;

		g_train_data_set[i].week_return = (g_train_data_set[i].clo - previous_thu_price) / previous_thu_price;
		g_train_data_set[i].neuron_return = g_train_data_set[i].week_return;
	}
}

#elif TYPE_OF_RETURN_AND_AVERAGE == 6

void
compute_target_return_and_avg ()
{
	int i;
	int reference_index;
	int sample_index;
	double reference_price;
	double sample_price;
	double sample_return;
	
	for (i = 0; i < g_train_num_samples; i++)
		g_train_data_set[i].avg = (g_train_data_set[i].max + g_train_data_set[i].min) / 2.0;
		
	for (i = (SAMPLE_SIZE + 1) * NUM_DAY_IN_A_WEEK; i < g_train_num_samples; i++)
	{
		reference_index = i - (SAMPLE_SIZE + 1) * NUM_DAY_IN_A_WEEK;
		sample_index = i;
		reference_price = g_train_data_set[reference_index].clo;
		sample_price =    g_train_data_set[sample_index].clo;

		sample_return = (sample_price - reference_price) / reference_price;
	
		g_train_data_set[i].week_return_buy_sell = sample_return;
		g_train_data_set[i].week_return_sell_buy = sample_return;

		g_train_data_set[i].neuron_return_buy_sell = g_train_data_set[i].week_return_buy_sell;
		g_train_data_set[i].neuron_return_sell_buy = g_train_data_set[i].week_return_sell_buy;
	}
}


#elif TYPE_OF_RETURN_AND_AVERAGE == 7

void
compute_target_return_and_avg ()
{
	int i;
	double sample_return, factor;
	
	for (i = 0; i < g_train_num_samples; i++)
		g_train_data_set[i].avg = (g_train_data_set[i].max + g_train_data_set[i].min) / 2.0;
		
	for (i = 0; i < g_train_num_samples; i++)
	{
		if (i >= FIRST_SAMPLE)
		{
			factor = g_ref_data_set[i-NUM_DAY_IN_A_WEEK].clo / g_train_data_set[i-NUM_DAY_IN_A_WEEK].clo;

			sample_return = factor * (g_train_data_set[i].clo / g_ref_data_set[i].clo) - 1.0;
		}
		else
			sample_return = 0.0;

		g_train_data_set[i].week_return_buy_sell = sample_return;
		g_train_data_set[i].week_return_sell_buy = sample_return;

		g_train_data_set[i].neuron_return_buy_sell = g_train_data_set[i].week_return_buy_sell;
		g_train_data_set[i].neuron_return_sell_buy = g_train_data_set[i].week_return_sell_buy;
	}
}


#elif TYPE_OF_RETURN_AND_AVERAGE == 8

// Retorno aproximado com compra de uma acao e venda de uma acao/indice de referencia na quinta,
// e venda da acao e recompra da acao/indice se o preco medio no periodo de sexta/segunda levar a um retorno > STOP_GAIN ou < STOP_LOSS;
// se nao sair por STOP_GAIN ou STOP_LOSS vende/recompra no fechamento da semana.

void
compute_target_return_and_avg ()
{
	int i, j;
	double factor_ref, factor_stock, sample_return;
	int buy_day, first_sell_day, last_sell_day;
	
	for (i = 0; i < g_train_num_samples; i++)
	{
		g_train_data_set[i].avg = (g_train_data_set[i].max + g_train_data_set[i].min) / 2.0;
		g_ref_data_set[i].avg = (g_ref_data_set[i].max + g_ref_data_set[i].min) / 2.0;
	}
		
	for (i = 0; i < g_train_num_samples; i++)
	{
		if (i >= FIRST_SAMPLE)
		{
			// buy -> sell
			buy_day = i - (NUM_DAY_IN_A_WEEK-1);

			factor_ref = g_ref_data_set[buy_day].avg;
			factor_stock = g_train_data_set[buy_day].avg;

			first_sell_day = buy_day + 1;
			last_sell_day =  i;
			for (j = first_sell_day; j <= last_sell_day; j++)
			{
				sample_return = (g_train_data_set[j].avg / factor_stock) - (g_ref_data_set[j].avg / factor_ref);
				if (sample_return > STOP_GAIN)
					break;
				if (sample_return < STOP_LOSS)
					break;
			}
			if (j == (last_sell_day + 1)) // Nao conseguiu vender
				sample_return = (g_train_data_set[i].clo / factor_stock) - (g_ref_data_set[i].clo / factor_ref);

			g_train_data_set[i].week_return_buy_sell = sample_return;
			g_train_data_set[i].neuron_return_buy_sell = g_train_data_set[i].week_return_buy_sell;


			// sell -> buy
			first_sell_day = buy_day + 1;
			last_sell_day =  i;
			for (j = first_sell_day; j <= last_sell_day; j++)
			{
				sample_return = (g_ref_data_set[j].avg / factor_ref) - (g_train_data_set[j].avg / factor_stock);
				if (sample_return > STOP_GAIN)
					break;
				if (sample_return < STOP_LOSS)
					break;
			}
			if (j == (last_sell_day + 1)) // Nao conseguiu vender
				sample_return = (g_ref_data_set[i].clo / factor_ref) - (g_train_data_set[i].clo / factor_stock);

			g_train_data_set[i].week_return_sell_buy = sample_return;
			g_train_data_set[i].neuron_return_sell_buy = g_train_data_set[i].week_return_sell_buy;
		}
		else
		{
			g_train_data_set[i].week_return_buy_sell = 0.0;
			g_train_data_set[i].week_return_sell_buy = 0.0;

			g_train_data_set[i].neuron_return_buy_sell = g_train_data_set[i].week_return_buy_sell;
			g_train_data_set[i].neuron_return_sell_buy = g_train_data_set[i].week_return_sell_buy;
		}
	}
}


#elif TYPE_OF_RETURN_AND_AVERAGE == 9

// Retorno aproximado com compra de uma acao e venda de uma acao/indice de referencia na abertura de quinta,
// e venda da acao e recompra da acao/indice se o preco de fechamento no periodo de quinta/segunda levar a um retorno > STOP_GAIN ou < STOP_LOSS;
// se nao sair por STOP_GAIN ou STOP_LOSS vende/recompra no fechamento da semana.

void
compute_target_return_and_avg ()
{
	int i, j;
	double factor_ref, factor_stock, sample_return;
	int buy_day, first_sell_day, last_sell_day;
	
	for (i = 0; i < g_train_num_samples; i++)
	{
		g_train_data_set[i].avg = (g_train_data_set[i].max + g_train_data_set[i].min) / 2.0;
		g_ref_data_set[i].avg = (g_ref_data_set[i].max + g_ref_data_set[i].min) / 2.0;
	}
		
	for (i = 0; i < g_train_num_samples; i++)
	{
		if (i >= FIRST_SAMPLE)
		{
			// buy -> sell
			buy_day = i - (NUM_DAY_IN_A_WEEK-1);

			factor_ref = g_ref_data_set[buy_day].opn;
			factor_stock = g_train_data_set[buy_day].opn;

			first_sell_day = buy_day;
			last_sell_day =  i;
			for (j = first_sell_day; j <= last_sell_day; j++)
			{
				sample_return = (g_train_data_set[j].opn / factor_stock) - (g_ref_data_set[j].opn / factor_ref);
				if (sample_return > STOP_GAIN)
					break;
				if (sample_return < STOP_LOSS)
					break;

				sample_return = (g_train_data_set[j].clo / factor_stock) - (g_ref_data_set[j].clo / factor_ref);
				if (sample_return > STOP_GAIN)
					break;
				if (sample_return < STOP_LOSS)
					break;
			}
			if (j == (last_sell_day + 1)) // Nao conseguiu vender
				sample_return = (g_train_data_set[i].clo / factor_stock) - (g_ref_data_set[i].clo / factor_ref);

			g_train_data_set[i].week_return_buy_sell = sample_return;
			g_train_data_set[i].neuron_return_buy_sell = g_train_data_set[i].week_return_buy_sell;


			// sell -> buy
			first_sell_day = buy_day;
			last_sell_day =  i;
			for (j = first_sell_day; j <= last_sell_day; j++)
			{
				sample_return = (g_ref_data_set[j].opn / factor_ref) - (g_train_data_set[j].opn / factor_stock);
				if (sample_return > STOP_GAIN)
					break;
				if (sample_return < STOP_LOSS)
					break;

				sample_return = (g_ref_data_set[j].clo / factor_ref) - (g_train_data_set[j].clo / factor_stock);
				if (sample_return > STOP_GAIN)
					break;
				if (sample_return < STOP_LOSS)
					break;
			}
			if (j == (last_sell_day + 1)) // Nao conseguiu comprar
				sample_return = (g_ref_data_set[i].clo / factor_ref) - (g_train_data_set[i].clo / factor_stock);

			g_train_data_set[i].week_return_sell_buy = sample_return;
			g_train_data_set[i].neuron_return_sell_buy = g_train_data_set[i].week_return_sell_buy;
		}
		else
		{
			g_train_data_set[i].week_return_buy_sell = 0.0;
			g_train_data_set[i].week_return_sell_buy = 0.0;

			g_train_data_set[i].neuron_return_buy_sell = g_train_data_set[i].week_return_buy_sell;
			g_train_data_set[i].neuron_return_sell_buy = g_train_data_set[i].week_return_sell_buy;
		}
	}
}


#else

void
compute_target_return_and_avg ()
{
	int i;
	double sample_return;
	
	for (i = 0; i < g_train_num_samples; i++)
		g_train_data_set[i].avg = (g_train_data_set[i].max + g_train_data_set[i].min) / 2.0;
		
	for (i = 0; i < g_train_num_samples; i++)
	{
		if (i >= FIRST_SAMPLE)
			sample_return = (g_train_data_set[i].clo - g_train_data_set[i-NUM_DAY_IN_A_WEEK].clo) / g_train_data_set[i-NUM_DAY_IN_A_WEEK].clo;
		else
			sample_return = 0.0;

		g_train_data_set[i].week_return_buy_sell = sample_return;
		g_train_data_set[i].week_return_sell_buy = sample_return;

		g_train_data_set[i].neuron_return_buy_sell = g_train_data_set[i].week_return_buy_sell;
		g_train_data_set[i].neuron_return_sell_buy = g_train_data_set[i].week_return_sell_buy;
	}
}

#endif
