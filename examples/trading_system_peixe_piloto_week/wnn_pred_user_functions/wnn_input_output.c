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



extern TRAIN_DATA_SET *g_train_data_set;
extern int FIRST_SAMPLE;
extern int LAST_SAMPLE;
extern int g_TargetWeekNumber;
extern int g_train_num_samples;

extern double g_train_return_average;
extern double g_train_return_standard_deviation;


#define BUY	1
#define SELL	0


#if TYPE_OF_WNN_INPUT == 1


float
compute_sample_return (int sample)
{
	float sample_return;
	float previous_previous_thursday;
	float sample_value;
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


#elif TYPE_OF_WNN_INPUT == 2


float
compute_sample_return (int sample)
{
	int reference_index;
	int sample_index;
	float reference_price;
	float sample_price;
	float sample_return;
	
	reference_index = FIRST_SAMPLE + NUM_DAY_IN_A_WEEK * g_TargetWeekNumber - (SAMPLE_SIZE + 1) * NUM_DAY_IN_A_WEEK;
	sample_index = FIRST_SAMPLE + NUM_DAY_IN_A_WEEK * g_TargetWeekNumber + NUM_DAY_IN_A_WEEK * sample;
	reference_price = g_train_data_set[reference_index].clo;
	sample_price =    g_train_data_set[sample_index].clo;
	
	sample_return = (sample_price - reference_price) / reference_price;

#if VERBOSE == 1
	char Rdate[100], Sdate[100], Rdow[100], Sdow[100]; // R -> reference_day; S -> sample_day

	strcpy (Rdate, long2isodate (g_train_data_set[reference_index].date_l));
	strcpy (Sdate, long2isodate (g_train_data_set[sample_index].date_l));
	strcpy (Rdow, long2dows (g_train_data_set[reference_index].date_l));
	strcpy (Sdow, long2dows (g_train_data_set[sample_index].date_l));
	printf ("#|%s-%s/%s-%s ", Rdate, Rdow, Sdate, Sdow);
#endif

	return (sample_return);
}


#else


float
compute_sample_return (int sample)
{
	float sample_return;
	
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


#if TYPE_OF_WNN_INPUT2 == 1


float
compute_sample_return2(int sample)
{
	float sample_return;
	float previous_previous_thursday;
	float sample_value;
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


float
compute_sample_return2(int sample)
{
	int reference_index;
	int sample_index;
	float reference_price;
	float sample_price;
	float sample_return;
	
	reference_index = FIRST_SAMPLE + NUM_DAY_IN_A_WEEK * g_TargetWeekNumber - (SAMPLE_SIZE + 1) * NUM_DAY_IN_A_WEEK;
	sample_index = FIRST_SAMPLE + NUM_DAY_IN_A_WEEK * g_TargetWeekNumber + NUM_DAY_IN_A_WEEK * sample;
	reference_price = g_train_data_set[reference_index].clo;
	sample_price =    g_train_data_set[sample_index].clo;
	
	sample_return = (sample_price - reference_price) / reference_price;

#if VERBOSE == 1
	char Rdate[100], Sdate[100], Rdow[100], Sdow[100]; // R -> reference_day; S -> sample_day

	strcpy (Rdate, long2isodate (g_train_data_set[reference_index].date_l));
	strcpy (Sdate, long2isodate (g_train_data_set[sample_index].date_l));
	strcpy (Rdow, long2dows (g_train_data_set[reference_index].date_l));
	strcpy (Sdow, long2dows (g_train_data_set[sample_index].date_l));
	printf ("#|%s-%s/%s-%s ", Rdate, Rdow, Sdate, Sdow);
#endif

	return (sample_return);
}


#else


float
compute_sample_return2(int sample)
{
	float sample_return;
	
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
	float sample_return, normalized_sample_return;
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
				position = (int) (normalized_sample_return * (float) y_h);
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
	float sample_return, normalized_sample_return;
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
				position = (int) (normalized_sample_return * (float) y_h);
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
	printf ("-> % .6f\n", g_train_data_set[FIRST_SAMPLE + NUM_DAY_IN_A_WEEK * g_TargetWeekNumber].week_return);
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
	float sample_return, normalized_sample_return;
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
				position = (int) (normalized_sample_return * (float) y_h);
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
	float sample_return, normalized_sample_return;
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
				position = (int) (normalized_sample_return * (float) y_h);
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
	printf ("-> % .6f\n", g_train_data_set[FIRST_SAMPLE + NUM_DAY_IN_A_WEEK * g_TargetWeekNumber].week_return);
#endif

	return (0);
}

#endif



#if TYPE_OF_RETURN_AND_AVERAGE == 1

void
compute_target_return_and_avg ()
{
	int i, j;
	float max;
	
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
			buy_day = last_buy_day = first_buy_day + 1;
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
					sell_price = g_train_data_set[i].clo;
					g_train_data_set[i].week_return = (sell_price - buy_price) / buy_price;
				}
			}
		}
		else
			g_train_data_set[i].week_return = 0.0;

		g_train_data_set[i].neuron_return = g_train_data_set[i].week_return;
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
					g_train_data_set[i].week_return = -STOP_GAIN;
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
	float previous_thu_price;
	
	for (i = 0; i < g_train_num_samples; i++)
		g_train_data_set[i].avg = (g_train_data_set[i].max + g_train_data_set[i].min) / 2.0;
		
	for (i = (SAMPLE_SIZE + 1) * NUM_DAY_IN_A_WEEK; i < g_train_num_samples; i++)
	{
		previous_thu_price = g_train_data_set[i-NUM_DAY_IN_A_WEEK+1].avg;

		g_train_data_set[i].week_return = (g_train_data_set[i].clo - previous_thu_price) / previous_thu_price;
		g_train_data_set[i].neuron_return = g_train_data_set[i].week_return;
	}
}

#else

void
compute_target_return_and_avg ()
{
	int i;
	
	for (i = 0; i < g_train_num_samples; i++)
		g_train_data_set[i].avg = (g_train_data_set[i].max + g_train_data_set[i].min) / 2.0;
		
	for (i = 0; i < g_train_num_samples; i++)
	{
		if (i >= FIRST_SAMPLE)
			g_train_data_set[i].week_return = (g_train_data_set[i].clo - g_train_data_set[i-NUM_DAY_IN_A_WEEK].clo) / g_train_data_set[i-NUM_DAY_IN_A_WEEK].clo;
		else
			g_train_data_set[i].week_return = 0.0;

		g_train_data_set[i].neuron_return = g_train_data_set[i].week_return;
	}
}

#endif
