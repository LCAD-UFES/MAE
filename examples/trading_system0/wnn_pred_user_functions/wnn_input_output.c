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
extern int FIRST_SAMPLE;
extern int g_TrainWeekNumber;
extern int g_train_num_samples;


#if INPUT_PREVIUOS_WEEK_RETURNS == 1


float
compute_sample_return (int sample)
{
	float sample_return;
	float previous_previous_thursday;
	float sample_value;
	// char Tdate[100], Sdate[100], Tdow[100], Sdow[100];
	
	previous_previous_thursday = g_train_data_set[FIRST_SAMPLE + 5*g_TrainWeekNumber - 2*5 + 1].clo;
	sample_value = g_train_data_set[FIRST_SAMPLE + 5*g_TrainWeekNumber + sample - 4].clo;
	sample_return = (sample_value - previous_previous_thursday) / previous_previous_thursday;

	// strcpy (Tdate, long2isodate (g_train_data_set[FIRST_SAMPLE + 5*g_TrainWeekNumber - 2*5 + 1].date_l));
	// strcpy (Sdate, long2isodate (g_train_data_set[FIRST_SAMPLE + 5*g_TrainWeekNumber + sample - 4].date_l));
	// strcpy (Tdow, long2dows (g_train_data_set[FIRST_SAMPLE + 5*g_TrainWeekNumber - 2*5 + 1].date_l));
	// strcpy (Sdow, long2dows (g_train_data_set[FIRST_SAMPLE + 5*g_TrainWeekNumber + sample - 4].date_l));
	// printf ("|%s-%s/%s-%s", Tdate, Tdow, Sdate, Sdow);
	
	return (sample_return);
}


#else


float
compute_sample_return (int sample)
{
	float sample_return;
	
	sample_return = g_train_data_set[FIRST_SAMPLE + 5 * g_TrainWeekNumber + 5 * sample].week_return;
	
	return (sample_return);
}


#endif


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
	//	g_TrainWeekNumber, 
	//	long2isodate (timePlusDays(g_train_data_set[FIRST_SAMPLE + 5 * (g_TrainWeekNumber-1)].date_l, 5)),
	//	long2dows (timePlusDays(g_train_data_set[FIRST_SAMPLE + 5 * (g_TrainWeekNumber-1)].date_l, 5)));
	for (x = 0; x < x_w; x++)
	{
		if (((x - stride/2) % stride) == 0)
		{
			sample_return = compute_sample_return (sample);
			// printf ("% .6f  ; ", sample_return);
			normalized_sample_return = (sample_return - MIN_RETURN) / (MAX_RETURN - MIN_RETURN);
			for (y = 0; y < y_h; y++)
			{
				position = (int) (normalized_sample_return * (float) y_h + 0.5);
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
		}
		else
		{
			for (y = 0; y < y_h; y++)
				neuron[y * x_w + x].output.fval = (float) 0.0;
		}
	}
	/* Target return for sample g_TrainWeekNumber */
	// printf ("-> % .6f\n", g_train_data_set[FIRST_SAMPLE + 5 * g_TrainWeekNumber].week_return);
	update_input_image (input);
	check_input_bounds (input, input->wx + input->ww/2, input->wy + input->wh/2);	

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
	//	g_TrainWeekNumber, 
	//	long2isodate (timePlusDays(g_train_data_set[FIRST_SAMPLE + 5 * (g_TrainWeekNumber-1)].date_l, 5)),
	//	long2dows (timePlusDays(g_train_data_set[FIRST_SAMPLE + 5 * (g_TrainWeekNumber-1)].date_l, 5)));
	for (x = 0; x < x_w; x++)
	{
		if (((x - stride/2) % stride) == 0)
		{
			sample_return = compute_sample_return (sample);
			// printf ("% .6f  ; ", sample_return);
			normalized_sample_return = (sample_return - MIN_RETURN) / (MAX_RETURN - MIN_RETURN);
			for (y = 0; y < y_h; y++)
			{
				position = (int) (normalized_sample_return * (float) y_h + 0.5);
				if (position >= y_h)
					position = y_h - 1;
				else if (position < 0)
					position = 0;

				if ((y >= y_h/2) && (y <= position))
					neuron[y * x_w + x].output.fval = (float) 1.0;
				else if ((y < y_h/2) && (y >= position))
					neuron[y * x_w + x].output.fval = (float) 1.0;
				else
					neuron[y * x_w + x].output.fval = (float) 0.0;
			}
			sample += 1;
		}
		else
		{
			for (y = 0; y < y_h; y++)
				neuron[y * x_w + x].output.fval = (float) 0.0;
		}
	}
	/* Target return for sample g_TrainWeekNumber */
	// printf ("-> % .6f\n", g_train_data_set[FIRST_SAMPLE + 5 * g_TrainWeekNumber].week_return);
	update_input_image (input);
	check_input_bounds (input, input->wx + input->ww/2, input->wy + input->wh/2);	

	return (0);
}

#endif





#if MAX_PREVIOUS_WEEK_RETURN == 1

void
compute_target_return_and_avg ()
{
	int i, j;
	float max;
	
	for (i = 0; i < g_train_num_samples; i++)
		g_train_data_set[i].avg = (g_train_data_set[i].max + g_train_data_set[i].min + g_train_data_set[i].clo) / 3.0;
		
	for (i = 0; i < g_train_num_samples; i++)
	{
		if (i >= 5)
		{
			max = g_train_data_set[i].avg;
			for (j = 1; j < 5; j++)
			{
				if (g_train_data_set[i-j].avg > max)
					max = g_train_data_set[i-j].avg;
			}
			/* quando a referência é quarta,	 g_train_data_set[i-5].max = quarta passada */
			g_train_data_set[i].week_return = (max - g_train_data_set[i-5].avg) / g_train_data_set[i-5].avg;
		}
		else
			g_train_data_set[i].week_return = 0.0;
	}
}

#else

void
compute_target_return_and_avg ()
{
	int i;
	
	for (i = 0; i < g_train_num_samples; i++)
		g_train_data_set[i].avg = (g_train_data_set[i].max + g_train_data_set[i].min + g_train_data_set[i].clo) / 3.0;
	for (i = 0; i < g_train_num_samples; i++)
	{
		if (i >= 5)
			g_train_data_set[i].week_return = (g_train_data_set[i].clo - g_train_data_set[i-5].clo) / g_train_data_set[i-5].clo;
		else
			g_train_data_set[i].week_return = 0.0;
	}
}

#endif


int
get_week_j (OUTPUT_DESC *output, int *num_neurons)
{
	int i, j;
	int current_week, selected_week = 0;
	int nAux, nMax = 0;
	NEURON *neuron_vector;

	neuron_vector = output->neuron_layer->neuron_vector;
	
	// Percorre a camada de saida (output), neuronio por neuronio, contando
	// o numero de retornos distintos. O retorno escolhido pelo maior numero de neuronios 
	// sera considerado como o valor da camada de saida.
	for (i = 0; i < (output->wh * output->ww); i++)
	{
		current_week = neuron_vector[i].output.ival;
		nAux = 1;
		
		for (j = i + 1; j < (output->wh * output->ww); j++)
		{
			if (neuron_vector[j].output.ival == current_week)
				nAux++;
		}

		// Verifica se eh a classe com mais neuronios setados ateh agora
		// Se for, seta a classe e atualiza o valor maximo
		if (nAux > nMax)
		{
			nMax = nAux;
			selected_week = current_week;
		}
	}
	
	*num_neurons = nMax;
	return (selected_week);
}



#if BY_ON_CLOSE_PRICE == 1

float
get_return (int week)
{
	float buy_price;
	int first_sell_day, last_sell_day;
	int i;
	float current_min_return;
	float current_max_return;
	
	buy_price = g_train_data_set[FIRST_SAMPLE + (5 * week) - 5].clo;
	first_sell_day = FIRST_SAMPLE + (5 * week) - 4;
	last_sell_day =  FIRST_SAMPLE + (5 * week);
	for (i = first_sell_day; i <= last_sell_day; i++)
	{
		current_min_return = (g_train_data_set[i].min - buy_price) / buy_price;
		current_max_return = (g_train_data_set[i].max - buy_price) / buy_price;
		if (current_min_return <= STOP_LOSS)
			return (current_min_return);
		else if (current_max_return >= STOP_GAIN)
			return (current_max_return);
	}
	i--;
	return ((g_train_data_set[i].avg - buy_price) / buy_price);
}

#else

float
get_return (int week)
{
	float buy_price;
	int first_buy_day, last_buy_day;
	int first_sell_day, last_sell_day;
	int i;
	float current_min_return;
	float current_max_return;
	
	buy_price = g_train_data_set[FIRST_SAMPLE + (5 * week) - 5].clo * (1.0 - STOP_GAIN);
	first_buy_day = FIRST_SAMPLE + (5 * week) - 4;
	last_buy_day =  FIRST_SAMPLE + (5 * week) - 3;
	for (i = first_buy_day; i <= last_buy_day; i++)
	{
		if (g_train_data_set[i].min < buy_price)
			break;
	}
	if (i > last_buy_day)
		return (0.0);
		
	first_sell_day = i++;
	last_sell_day =  FIRST_SAMPLE + (5 * week);
	for (i = first_sell_day; i <= last_sell_day; i++)
	{
		current_min_return = (g_train_data_set[i].min - buy_price) / buy_price;
		current_max_return = (g_train_data_set[i].max - buy_price) / buy_price;
		if (current_min_return <= STOP_LOSS)
			return (current_min_return);
		else if (current_max_return >= STOP_GAIN)
			return (current_max_return);
	}
	i--;
	return ((g_train_data_set[i].clo - buy_price) / buy_price);
}

#endif



float 
EvaluateOutputRiRj (OUTPUT_DESC *output, int *num_neurons)
{
	int week_j, week_i;
	float return_j, return_i;
	
	week_j = get_week_j (output, num_neurons);
	week_i = g_TrainWeekNumber;

	return_j = get_return (week_j);
	return_i = get_return (week_i);	

/*	if (return_i < STOP_LOSS)
		g_train_data_set[FIRST_SAMPLE + 5 * g_TrainWeekNumber].week_return = STOP_LOSS;
	else if (return_i > STOP_GAIN)
		g_train_data_set[FIRST_SAMPLE + 5 * g_TrainWeekNumber].week_return = STOP_GAIN;
	else
*/		g_train_data_set[FIRST_SAMPLE + 5 * g_TrainWeekNumber].week_return = return_i;
		
	return (return_j);
}
