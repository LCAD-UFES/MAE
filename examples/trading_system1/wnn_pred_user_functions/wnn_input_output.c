#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <sys/types.h>
#include <dirent.h>
#include "mae.h"
#include "../wnn_pred.h"
#include "wnn_pred_user_functions.h"



extern DATA_SET *g_data_set;
extern int g_current_sample;
extern int LAST_SAMPLE;


double
compute_sample_return_new (int now, int sample)
{
	double sample_return;
	int reference;
	
	reference = now - (NETWORK_INPUT_SIZE * SAMPLE_GROUP_SIZE);
	if (reference < 0)
		reference = 0;
		
	if ((sample >= 0) && (sample < LAST_SAMPLE))
		sample_return = (g_data_set[sample].sample_price - g_data_set[reference].sample_price) / g_data_set[reference].sample_price;
	else
		sample_return = 0.0;

	return (sample_return);
}


double
compute_sample_return (int now, int sample)
{
	double sample_return;
	
	if ((sample >= 0) && (sample < LAST_SAMPLE))
	{
		sample_return = g_data_set[sample].sample_return;
#ifdef	VERBOSE
		printf ("%04d(%s)  ", sample, g_data_set[sample].minute);
#endif
	}
	else
	{
		sample_return = 0.0;
#ifdef	VERBOSE
		printf ("%04d(%s)  ", sample, "     ");
#endif
	}

	return (sample_return);
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
ReadSampleInput(INPUT_DESC *input, int input_sample)
{
	int x, y, x_w, y_h, stride, now, sample;
	NEURON *neuron;
	double sample_return, normalized_sample_return;
	int position;

	neuron = input->neuron_layer->neuron_vector;
	x_w = input->neuron_layer->dimentions.x;
	y_h = input->neuron_layer->dimentions.y;
	
	stride = x_w / NETWORK_INPUT_SIZE;
	now = LAST_SAMPLE_IN_THE_GROUP(input_sample);
	sample = now - (NETWORK_INPUT_SIZE - 1) * NETWORK_INPUT_STRIDE;
#ifdef	VERBOSE
	printf("\n");
#endif
	for (x = 0; x < x_w; x++)
	{
		if ((((x - stride/2) % stride) == 0) && (sample <= now))
		{
			sample_return = compute_sample_return (now, sample);
		    	normalized_sample_return = (sample_return - MIN_RETURN) / (MAX_RETURN - MIN_RETURN);
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
			sample += NETWORK_INPUT_STRIDE;
		}
		else
		{
			for (y = 0; y < y_h; y++)
				neuron[y * x_w + x].output.fval = (float) 0.0;
		}
	}
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
ReadSampleInput(INPUT_DESC *input, int input_sample)
{
	int x, y, x_w, y_h, stride, now, sample;
	NEURON *neuron;
	double sample_return, normalized_sample_return;
	int position;

	neuron = input->neuron_layer->neuron_vector;
	x_w = input->neuron_layer->dimentions.x;
	y_h = input->neuron_layer->dimentions.y;
	
	stride = x_w / NETWORK_INPUT_SIZE;
	now = LAST_SAMPLE_IN_THE_GROUP(input_sample);
	sample = now - (NETWORK_INPUT_SIZE - 1) * NETWORK_INPUT_STRIDE;
#ifdef	VERBOSE
	printf("\n");
#endif
	for (x = 0; x < x_w; x++)
	{
		if ((((x - stride/2) % stride) == 0) && (sample <= now))
		{
			sample_return = compute_sample_return (now, sample);
		    	normalized_sample_return = (sample_return - MIN_RETURN) / (MAX_RETURN - MIN_RETURN);
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
			sample += NETWORK_INPUT_STRIDE;
		}
		else
		{
			for (y = 0; y < y_h; y++)
				neuron[y * x_w + x].output.fval = (float) 0.0;
		}
	}
	update_input_image (input);
	check_input_bounds (input, input->wx + input->ww/2, input->wy + input->wh/2);	

	return (0);
}

#endif



/*
*********************************************************************************
* Function: next sample filter	             	 				*
* Description: 			                        			*
* Inputs: 				                  			*
* Output: 				                			*
*********************************************************************************
*/

void next_sample_filter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER *filter_output;
	INPUT_DESC *input;
	int input_sample_size;

	// Get the Filter Parameter
	input_sample_size = filter_desc->filter_params->next->param.ival;

	// Gets the filter_output Neuron Layer
	filter_output = filter_desc->output;

	input = get_input_by_neural_layer(filter_output);

	ReadSampleInput(input, g_current_sample + input_sample_size);
}



/*
*********************************************************************************
* Function: predicted sample filter	             	 			*
* Description: 			                        			*
* Inputs: 				                  			*
* Output: 				                			*
*********************************************************************************
*/

void predicted_sample_filter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER *filter_output;
	NEURON_LAYER *nl_input;
	INPUT_DESC *input;
	OUTPUT_DESC *output;
	int sample_to_display, num_neurons;

	// Gets the output Neuron Layer
	filter_output = filter_desc->output;

	input = get_input_by_neural_layer(filter_output);
	
	// Gets the Input Neuron Layer
	nl_input = filter_desc->neuron_layer_list->neuron_layer;
	output = get_output_by_neural_layer(nl_input);

	sample_to_display = EvaluateOutput(output, &num_neurons);
	ReadSampleInput(input, sample_to_display);
}
