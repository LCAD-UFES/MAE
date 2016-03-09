#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <sys/types.h>
#include <dirent.h>
#include "mae.h"
#include "../wnn_pred.h"
#include "wnn_pred_user_functions.h"



extern TRAIN_DATA_SET *g_train_data_set;
extern int FIRST_SAMPLE;
extern int g_TrainWeekNumber;
extern int g_train_num_samples;


float
compute_sample_return (int sample)
{
	float sample_return;
	
	sample_return = g_train_data_set[FIRST_SAMPLE + sample].week_return;

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
	sample = - SAMPLE_SIZE + g_TrainWeekNumber;
	// printf ("\n%03d: %s %s ; ", 
	//	g_TrainWeekNumber, 
	//	long2isodate (timePlusDays(g_train_data_set[FIRST_SAMPLE + (g_TrainWeekNumber-1)].date_l, 5)),
	//	long2dows (timePlusDays(g_train_data_set[FIRST_SAMPLE + (g_TrainWeekNumber-1)].date_l, 5)));
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
                printf("%d\n",position);
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
	// printf ("-> % .6f\n", g_train_data_set[FIRST_SAMPLE + g_TrainWeekNumber].week_return);
	check_input_bounds (input, input->wx + input->ww/2, input->wy + input->wh/2);	
	update_input_image (input);

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

    //printf("-------------------------\n");

	neuron = input->neuron_layer->neuron_vector;
	x_w = input->neuron_layer->dimentions.x;
	y_h = input->neuron_layer->dimentions.y;
	stride = x_w / SAMPLE_SIZE;
	sample = - SAMPLE_SIZE + g_TrainWeekNumber;
	// printf ("\n%03d: %s %s ; ", 
	//	g_TrainWeekNumber, 
	//	long2isodate (timePlusDays(g_train_data_set[FIRST_SAMPLE + (g_TrainWeekNumber-1)].date_l, 5)),
	//	long2dows (timePlusDays(g_train_data_set[FIRST_SAMPLE + (g_TrainWeekNumber-1)].date_l, 5)));
	for (x = 0; x < x_w; x++)
	{
		if (((x - stride/2) % stride) == 0)
		{
			sample_return = compute_sample_return (sample);
		    normalized_sample_return = (sample_return - MIN_RETURN) / (MAX_RETURN - MIN_RETURN);
            //printf("%lf ==== %lf\n", sample_return, normalized_sample_return);
			for (y = 0; y < y_h; y++)
			{
				position = (int) (normalized_sample_return * (float) y_h);
				if (position >= y_h)
					position = y_h - 1;
				else if (position < 0)
					position = 0;

				if ((y >= y_h/2) && (y <= position && position != y_h/2)){
					neuron[y * x_w + x].output.fval = (float) 1.0;
                    //printf("%lf ---- 1 - %d\n",normalized_sample_return * (float) y_h, position);              
                }
				else if ((y < y_h/2) && (y >= position)) {
					neuron[y * x_w + x].output.fval = (float) 1.0;
                    //printf("%lf ---- 1 - %d\n",normalized_sample_return * (float) y_h, position);
                }
				else {
					neuron[y * x_w + x].output.fval = (float) 0.0;
                    //printf("%lf ---- 0 - %d\n",normalized_sample_return * (float) y_h, position);
                }
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
	// printf ("-> % .6f\n", g_train_data_set[FIRST_SAMPLE + g_TrainWeekNumber].week_return);
	update_input_image (input);
	check_input_bounds (input, input->wx + input->ww/2, input->wy + input->wh/2);	

	return (0);
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


int compare_float(float f1, float f2)
 {
  float precision = 0.00001;
  if (((f1 - precision) < f2) && 
      ((f1 + precision) > f2))
   {
    return 1;
   }
  else
   {
    return 0;
   }
 }

