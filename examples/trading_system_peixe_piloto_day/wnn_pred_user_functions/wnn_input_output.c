#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <sys/types.h>
#include <dirent.h>
#include "mae.h"
#include "../wnn_pred.h"
#include "wnn_pred_user_functions.h"
#include "timeutil.h"



extern TRAIN_DATA_SET *g_train_data_set;
extern int g_TargetDayNumber;
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
	
	base_day_index = g_TargetDayNumber - 2 + 1;
	sample_index = g_TargetDayNumber + sample;
	
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
	
	reference_index = g_TargetDayNumber - (SAMPLE_SIZE + 1);
	sample_index = g_TargetDayNumber + sample;
	reference_price = g_train_data_set[reference_index].clo;
	if (sample == -1)
		sample_price = g_train_data_set[sample_index].clo;
	else
		sample_price = g_train_data_set[sample_index].clo;
	
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




#elif TYPE_OF_WNN_INPUT == 3


float
compute_sample_return (int sample)
{
	int sample_index;
	float sample_return;
	
	sample_index = g_TargetDayNumber + sample;
	sample_return = g_train_data_set[sample_index].day_return;

#if VERBOSE == 1
	char Rdate[100], Sdate[100], Rdow[100], Sdow[100]; // R -> reference_day; S -> sample_day

	strcpy (Rdate, long2isodate (g_train_data_set[sample_index].date_l));
	strcpy (Sdate, long2isodate (g_train_data_set[sample_index].date_l));
	strcpy (Rdow, long2dows (g_train_data_set[sample_index].date_l));
	strcpy (Sdow, long2dows (g_train_data_set[sample_index].date_l));
	printf ("#|%s-%s/%s-%s ", Rdate, Rdow, Sdate, Sdow);
#endif

	return (sample_return);
}


#elif TYPE_OF_WNN_INPUT == 4


float
compute_sample_return (int sample)
{
	int reference_index;
	float reference_price;
	float sample_price;
	float sample_return;
	
	reference_index = g_TargetDayNumber - 1;
	reference_price = g_train_data_set[reference_index].opn;
	if (sample == -4)
		sample_price = g_train_data_set[reference_index].buy;
	if (sample == -3)
		sample_price = g_train_data_set[reference_index].sell;
	if (sample == -2)
		sample_price = g_train_data_set[reference_index].clo;
	if (sample == -1)
		sample_price = g_train_data_set[reference_index+1].opn;
	
	if (sample_price != 0.0)
		sample_return = (sample_price - reference_price) / reference_price;
	else
		sample_return = 0.0;

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
	
	sample_return = g_train_data_set[g_TargetDayNumber + sample].day_return;

#if VERBOSE == 1
	char Tdate[100], Sdate[100], Tdow[100], Sdow[100]; // T -> target_day; S -> sample_day

	strcpy (Tdate, long2isodate (g_train_data_set[g_TargetDayNumber].date_l));
	strcpy (Sdate, long2isodate (g_train_data_set[g_TargetDayNumber + sample].date_l));
	strcpy (Tdow, long2dows (g_train_data_set[g_TargetDayNumber].date_l));
	strcpy (Sdow, long2dows (g_train_data_set[g_TargetDayNumber + sample].date_l));
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
	sigmoid = (x - g_train_return_average) / (2.0 * 3.1415 * g_train_return_standard_deviation) + 0.5;
	
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
	//	g_TargetDayNumber, 
	//	long2isodate (timePlusDays(g_train_data_set[(g_TargetDayNumber-1)].date_l, 1)),
	//	long2dows (timePlusDays(g_train_data_set[(g_TargetDayNumber-1)].date_l, 1)));
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
	/* Target return for sample g_TargetDayNumber */
	printf("-> % .6f\n", g_train_data_set[g_TargetDayNumber].day_return);
	printf("week id = %d\n", g_TargetDayNumber);
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
	//	g_TargetDayNumber, 
	//	long2isodate (timePlusDays(g_train_data_set[(g_TargetDayNumber-1)].date_l, 1)),
	//	long2dows (timePlusDays(g_train_data_set[(g_TargetDayNumber-1)].date_l, 1)));
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

				if ((y >= y_h/2) && ((y <= position) && (position != y_h/2)))
					neuron[y * x_w + x].output.fval = (float) 1.0;
				else if ((y < y_h/2) && (y > position))
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
	/* Target return for sample g_TargetDayNumber */
	printf ("-> % .6f\n", g_train_data_set[g_TargetDayNumber].day_return);
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
		max = g_train_data_set[i].avg;
		for (j = 1; j < 5; j++)
		{
			if (g_train_data_set[i-j].avg > max)
				max = g_train_data_set[i-j].avg;
		}
		/* quando a referência é quarta,	 g_train_data_set[i-5].max = quarta passada */
		g_train_data_set[i].day_return = (max - g_train_data_set[i-5].avg) / g_train_data_set[i-5].avg;
			
		g_train_data_set[i].neuron_return = g_train_data_set[i].day_return;
	}
}

#elif TYPE_OF_RETURN_AND_AVERAGE == 2

void
compute_target_return_and_avg ()
{
	int i, reference_index;
	
	for (i = 0; i < g_train_num_samples; i++)
		g_train_data_set[i].avg = (g_train_data_set[i].max + g_train_data_set[i].min) / 2.0;
		
	for (i = SAMPLE_SIZE + 1; i < g_train_num_samples; i++)
	{
		reference_index = i - (SAMPLE_SIZE + 1);

		g_train_data_set[i].day_return = (g_train_data_set[i].clo - g_train_data_set[reference_index].clo) / g_train_data_set[reference_index].clo;

		g_train_data_set[i].neuron_return = g_train_data_set[i].day_return;
	}
}

#elif TYPE_OF_RETURN_AND_AVERAGE == 3

void
compute_target_return_and_avg ()
{
	int i;
	
	for (i = 0; i < g_train_num_samples; i++)
		g_train_data_set[i].avg = (g_train_data_set[i].max + g_train_data_set[i].min) / 2.0;
		
	for (i = 0; i < g_train_num_samples; i++)
	{
		if (g_train_data_set[i].buy != 0.0)
			g_train_data_set[i].day_return = (g_train_data_set[i].sell - g_train_data_set[i].buy) / g_train_data_set[i].buy;
		else
			g_train_data_set[i].day_return = 0.0;

		g_train_data_set[i].neuron_return = g_train_data_set[i].day_return;
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
		g_train_data_set[i].day_return = (g_train_data_set[i].clo - g_train_data_set[i-1].clo) / g_train_data_set[i-1].clo;

		g_train_data_set[i].neuron_return = g_train_data_set[i].day_return;
	}
}

#endif
