#include "mae.h"
#include "filter.h"
#include "robot_user_functions.h"
#include "segment.hpp"
#include "common.h"
#include <sys/time.h>
#include <time.h>



/*!
*********************************************************************************
* Function: bidimentional_convolution_nl						*
* Description: 									*
* Inputs: 									*
* Output: 						 			*
*********************************************************************************
*/

NEURON_OUTPUT bidimentional_convolution_nl (RECEPTIVE_FIELD_DESCRIPTION *receptive_field_descriptor, NEURON *nl, int x_center, int y_center, int w, int h)
{
	int x_current, y_current, i, kernel_size;
	float intensity, delta_area;
	NEURON_OUTPUT accumulator;

	// Gets the kernel size
	kernel_size = (int) sqrt ((double) receptive_field_descriptor->num_points);

	// Calculates the normalized delta area
	delta_area = receptive_field_descriptor->area / (float) ((kernel_size - 1) * (kernel_size - 1));

	// Initializes the accumulator variable
	accumulator.fval = 0.0;

        for (i = 0; i < receptive_field_descriptor->num_points; i++)
	{
		// Calculates the current point
		x_current = x_center + receptive_field_descriptor->points[i].x;
		y_current = y_center + receptive_field_descriptor->points[i].y;

		// Verifies if the point is inside of the neuron layer bounds
		if ((x_current < 0) || (x_current >= w) || (y_current < 0) || (y_current >= h))
		{
			accumulator.fval = 0.0 ;
			return (accumulator);
			//continue;
		}

		// Gets the output value
		intensity = nl[y_current * w + x_current].output.ival;

		// Accumulates the weighed intensity. The weight function depends of the position inside the kernel
		accumulator.fval += receptive_field_descriptor->points[i].weight * receptive_field_descriptor->points[i].gabor * intensity;
	}

	// Normalizes the result
	accumulator.fval *= delta_area / 6.0;

	return (accumulator);
}

/*!
*********************************************************************************
* \brief The gabor filter.
* \param[in] filter_desc The filter descriptor structure.
* \pre The filter descriptor created and initialized.
* \post The updated output neuron layer.
* \return Nothing.
*********************************************************************************
*/

void gabor_filter_nl (FILTER_DESC *filter_desc)
{
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nl = NULL;
	RECEPTIVE_FIELD_DESCRIPTION *receptive_field;
	int i, ho, wo, hi, wi, xi, yi, xo, yo, u, v;
	float harmonic, width, aspect_ratio, orientation, phase, log_factor;
	float frequency;
	double correction;
	int mapping, shift, kernel_type;
	int x_center, y_center;
	INPUT_DESC *input;
	NEURON_OUTPUT previous_output;
	int previous_xi, previous_yi, index;

	// Checks the Parameters
	for (i = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, i++)
		;

	if (i != 10)
	{
		Erro ("Wrong number of parameters for gabor_filter.", "", "");
		return;
	}

	// Gets the Parameters
	harmonic	= filter_desc->filter_params->next->param.fval;
	width		= filter_desc->filter_params->next->next->param.fval;
	aspect_ratio	= filter_desc->filter_params->next->next->next->param.fval;
	orientation	= filter_desc->filter_params->next->next->next->next->param.fval;
	phase		= filter_desc->filter_params->next->next->next->next->next->param.fval;
	shift		= filter_desc->filter_params->next->next->next->next->next->next->param.ival;
	mapping		= filter_desc->filter_params->next->next->next->next->next->next->next->param.ival;
	log_factor	= filter_desc->filter_params->next->next->next->next->next->next->next->next->param.fval;
	kernel_type     = filter_desc->filter_params->next->next->next->next->next->next->next->next->next->param.ival;

        // Checks the Input Neuron Layers
	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
		;

	if (i != 1)
	{
		Erro ("Wrong number of neuron layers. gabor_filter must be applied on only one neuron layer.", "", "");
		return;
	}

	// Gets the Input Neuron Layers
	nl = filter_desc->neuron_layer_list->neuron_layer;

	// Gets Input Neuron Layer Dimentions
	wi = nl->dimentions.x;
	hi = nl->dimentions.y;

	// Gets Output Neuron Layer Dimentions
	wo = filter_desc->output->dimentions.x;
	ho = filter_desc->output->dimentions.y;

	correction = (double) ho / (double) (ho - 1);

	// Computes Kernels
	if (filter_desc->private_state == NULL)
	{
		receptive_field = (RECEPTIVE_FIELD_DESCRIPTION *) malloc (sizeof(RECEPTIVE_FIELD_DESCRIPTION));

		// Calculates the sinusoidal frequency (cycles per pixel)
		frequency = (1.0f / (float) wi) * harmonic;

		receptive_field->points	= NULL;
		compute_v1_spatial_receptive_field (receptive_field, frequency, width, aspect_ratio, orientation, phase, kernel_type);
		filter_desc->private_state = (void *) receptive_field;

		// Calculates the minimum value of the result of a gabor filter
		bidimentional_convolution_test_gabor (receptive_field);
	}
	else
		receptive_field = (RECEPTIVE_FIELD_DESCRIPTION *)filter_desc->private_state;

	input = get_input_by_neural_layer (nl);

	if (input == NULL)
	{
		x_center = 0;
		y_center = 0;
	}
	else
	{
		if (TYPE_MOVING_FRAME == STOP)
		{
			x_center = input->wxd - input->wx;
			y_center = input->wyd - input->wy;
		}
		else
		{
			x_center = wi/2;
			y_center = hi/2;
		}
	}

	switch (mapping)
	{
		case MAP_PLAN:
		{
			for (xo = 0; xo < wo; xo++)
			{
				xi = (int) ((float) (xo * wi) / (float) wo + 0.5);

				for (yo = 0; yo < ho; yo++)
				{
					yi = (int) ((float) (yo * hi) / (float) ho + 0.5);

					previous_output = filter_desc->output->neuron_vector[(yo * wo) + xo].output = bidimentional_convolution_nl (receptive_field, nl->neuron_vector, xi, yi, wi, hi);

					if (((previous_output.fval > 0.0) && (previous_output.fval <= receptive_field->min_dog)) ||
						((previous_output.fval < 0.0) && (previous_output.fval >= receptive_field->min_dog)))
						filter_desc->output->neuron_vector[(yo * wo) + xo].output.fval = 0.0;
				}
			}
		}
		break;

		case MAP_LOG_POLAR:
		{
			previous_xi = -1;
			previous_yi = -1;

			for (u = 0; u < wo; u++)
			{
				for (v = 0; v < ho; v++)
				{
					map_v1_to_image (&xi, &yi, wi, hi, u, v, wo, ho, x_center, y_center, correction, log_factor);
					xi += shift;

					index = (u < (wo / 2)) ? ((wo / 2) - 1 - u) : (u - (wo / 2));
					if ((xi == previous_xi) && (yi == previous_yi))
						filter_desc->output->neuron_vector[(v * wo) + u].output = previous_output;
					else if (xi >= wi || xi < 0 || yi >= hi || yi < 0)
						previous_output.fval = filter_desc->output->neuron_vector[(v * wo) + u].output.fval = 0.0;
					else
					{
						previous_output = filter_desc->output->neuron_vector[(v * wo) + u].output = bidimentional_convolution (receptive_field, nl->neuron_vector, xi, yi, wi, hi);

						if (((previous_output.fval > 0.0) && (previous_output.fval <= receptive_field->min_dog)) ||
							((previous_output.fval < 0.0) && (previous_output.fval >= receptive_field->min_dog)))
							previous_output.fval = filter_desc->output->neuron_vector[(v * wo) + u].output.fval = 0.0;
					}
					previous_xi = xi;
					previous_yi = yi;
				}
			}
		}
		break;
	}
}


// ----------------------------------------------------------------------------
// log_polar_filter -
//
// Entrada: filter_desc - Descritor do filtro
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void log_polar_filter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list;
	NEURON_LAYER *n_l;
	PARAM_LIST *p_list;
	int i, u, v, h, w, hi, wi, xi, yi, previous_xi, previous_yi, index;
	int x_center, y_center;
	INPUT_DESC *input;
	NEURON_OUTPUT previous_output;
	float fltLogFactor;
	float hAux;
	int nNumParam;

	previous_output.ival = 0;

	// Check Neuron Layers
	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++);

	if (i != 1)
	{
		Erro ("Wrong number of neuron layers. The log_polar_filter filter must be applied on only one neuron layer.", "", "");
		return;
	}
	n_l = filter_desc->neuron_layer_list->neuron_layer;

	// Achar o numero de parametros
	for (nNumParam = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, nNumParam++);
	nNumParam--;

	// Numero de neuron layers deve ser igual a tres
	if (nNumParam != 1)
	{
		Erro ("The log_polar_filter filter must have one parameter: log_factor.", "", "");
		return;
	}
	fltLogFactor = filter_desc->filter_params->next->param.fval;
// 	printf("fltLogFactor: %f\n", fltLogFactor);

	wi = n_l->dimentions.x;
	hi = n_l->dimentions.y;
	w = filter_desc->output->dimentions.x;
	h = filter_desc->output->dimentions.y;

	input = get_input_by_neural_layer (n_l);

	previous_xi = -1;
	previous_yi = -1;

	if (input == NULL)
	{
		x_center = 0;
		y_center = 0;
	}
	else
	{
		if (TYPE_MOVING_FRAME == STOP)
		{
			x_center = input->wxd - input->wx;
			y_center = input->wyd - input->wy;
		}
		else
		{
			x_center = wi/2;
			y_center = hi/2;
		}
	}

	hAux = (float) h / (float) (h - 1.0);
	for (u = 0; u < w; u++)
	{
		for (v = 0; v < h; v++)
		{
			map_v1_to_image (&xi, &yi, wi, hi, u, v, w, h, x_center, y_center, hAux, fltLogFactor);

			index = (u < (w / 2)) ? ((w / 2) - 1 - u) : (u - (w / 2));
			if ((xi == previous_xi) && (yi == previous_yi))
				filter_desc->output->neuron_vector[(v * w) + u].output = previous_output;
			else
				if (xi >= wi || xi < 0 || yi >= hi || yi < 0)
					previous_output.ival = filter_desc->output->neuron_vector[(v * w) + u].output.ival = 0;
				else
					previous_output = filter_desc->output->neuron_vector[(v * w) + u].output = input->neuron_layer->neuron_vector[yi * wi + xi].output;
			previous_xi = xi;
			previous_yi = yi;
		}
	}
}


/*!
*********************************************************************************
* Function: robot_sum_filter_normalized
* Description: Computes de vector difference between the n input neuron layers.
* dif = sqrt(SUM((nl[i] - nl[i + (n / 2)])^2))
* Inputs:
* Output:
*********************************************************************************
*/

void robot_sum_filter_normalized (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list;
	NEURON_LAYER **nl;
	PARAM_LIST *p_list;
	int i, j;
	int num_neurons, numNL, nNumParam;
	//float dif_float;
        float dif_accumulator, a, b, denom;
	float k = .0f;

	for (numNL = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, numNL++)
		;

	if (filter_desc->private_state == NULL)
	{
		if ((numNL % 2) != 0)
		{
			Erro ("Wrong number of neuron layers. The robot_sum_filter_normalized must be applied on a even number of neuron layers.", "", "");
			return;
		}

		for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
		{
			if (n_list->next != NULL)
			{
				if (n_list->neuron_layer->output_type != n_list->next->neuron_layer->output_type)
				{
					Erro ("The robot_sum_filter_normalized must be applied on neuron layers with the same output_type.", "", "");
					return;
				}
			}
		}

		nl = (NEURON_LAYER**)malloc(sizeof(NEURON_LAYER*)*numNL);
		for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
			nl[i] = n_list->neuron_layer;

		filter_desc->private_state = (void*)nl;
	}
	else
	{
		nl = (NEURON_LAYER**)filter_desc->private_state;

		// Achar o numero de parametros
		for (nNumParam = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, nNumParam++);
		nNumParam--;

		// Numero de neuron layers deve ser igual a tres
		if (nNumParam != 1)
		{
			Erro ("The robot_sum_filter_normalized must have one parameter: Selectivity.", "", "");
			return;
		}
		k = filter_desc->filter_params->next->param.fval;
	}

	num_neurons = get_num_neurons (filter_desc->output->dimentions);

	for (i = 0; i < num_neurons; i++)
	{
		dif_accumulator = denom = 0.0;
		for (j = 0; j < (numNL / 2); j++)
		{
			a = nl[j]->neuron_vector[i].output.fval;
			b = nl[j + (numNL / 2)]->neuron_vector[i].output.fval;
			dif_accumulator += (a + b)*(a + b);
			denom += a*a + b*b;
//			denom += (a + b)*(a + b);
		}
		dif_accumulator = SQRT (dif_accumulator);
		denom = k + SQRT (denom);
		filter_desc->output->neuron_vector[i].output.fval = dif_accumulator/denom;
	}
}

/*!
*********************************************************************************
* Function: robot_sum_filter
* Description:
* Inputs:
* Output:
*********************************************************************************
*/

void robot_sum_filter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list;
	NEURON_LAYER **nl;
	int i, j;
	int num_neurons, numNL;
	double dif_accumulator, a, b;

	for (numNL = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, numNL++)
		;

	if (filter_desc->private_state == NULL)
	{
		if ((numNL % 2) != 0)
		{
			Erro ("Wrong number of neuron layers. The robot_simple_cell must be applied on a even number of neuron layers.", "", "");
			return;
		}

		for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
		{
			if (n_list->next != NULL)
			{
				if (n_list->neuron_layer->output_type != n_list->next->neuron_layer->output_type)
				{
					Erro ("The robot_simple_cell must be applied on neuron layers with the same output_type.", "", "");
					return;
				}
			}
		}

		nl = (NEURON_LAYER**)malloc(sizeof(NEURON_LAYER*)*numNL);
		for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
			nl[i] = n_list->neuron_layer;

		filter_desc->private_state = (void*)nl;
	}
	else
	{
		nl = (NEURON_LAYER**)filter_desc->private_state;
	}

	num_neurons = get_num_neurons (filter_desc->output->dimentions);

	for (i = 0; i < num_neurons; i++)
	{
		dif_accumulator = 0.0;
		for (j = 0; j < (numNL / 2); j++)
		{
			a = nl[j]->neuron_vector[i].output.fval;
			b = nl[j + (numNL / 2)]->neuron_vector[i].output.fval;
			dif_accumulator += (a + b);
		}
		filter_desc->output->neuron_vector[i].output.fval = dif_accumulator;
	}
}

/*!
*********************************************************************************
* Function: robot_complex_cell
* Description:
* Inputs:
* Output:
*********************************************************************************
*/

void robot_complex_cell (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list;
	NEURON_LAYER **nl;
	int i, j;
	int num_neurons, numNL;
	double dif_accumulator, a, b;

	for (numNL = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, numNL++)
		;

	if (filter_desc->private_state == NULL)
	{
		if ((numNL % 2) != 0)
		{
			Erro ("Wrong number of neuron layers. The robot_complex_cell must be applied on a even number of neuron layers.", "", "");
			return;
		}

		for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
		{
			if (n_list->next != NULL)
			{
				if (n_list->neuron_layer->output_type != n_list->next->neuron_layer->output_type)
				{
					Erro ("The robot_complex_cell must be applied on neuron layers with the same output_type.", "", "");
					return;
				}
			}
		}

		nl = (NEURON_LAYER**)malloc(sizeof(NEURON_LAYER*)*numNL);
		for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
			nl[i] = n_list->neuron_layer;

		filter_desc->private_state = (void*)nl;
	}
	else
	{
		nl = (NEURON_LAYER**)filter_desc->private_state;
	}

	num_neurons = get_num_neurons (filter_desc->output->dimentions);

	for (i = 0; i < num_neurons; i++)
	{
		dif_accumulator = 0.0;
		for (j = 0; j < (numNL / 2); j++)
		{
			a = nl[j]->neuron_vector[i].output.fval;
			b = nl[j + (numNL / 2)]->neuron_vector[i].output.fval;
			dif_accumulator += (a*a + b*b);
		}
		filter_desc->output->neuron_vector[i].output.fval = dif_accumulator;
	}
}



/*!
*********************************************************************************
* Function: robot_mt_cell
* Description:
* Inputs:
* Output:
*********************************************************************************
*/

void robot_mt_cell_ref_right (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list;
	NEURON_LAYER *nl_simple_mono_right, *nl_simple_mono_right_q, *nl_simple_mono_left, * nl_simple_mono_left_q;
	PARAM_LIST *p_list;
	int i, yo, xo, xir, xil, d, wi, hi, yi, ho, wo;
	int num_neurons, numNL, nNumParam;
	float k;
  double sumSimpleBinocularCell, sumSimpleBinocularCellQ, r, l, rq, lq, sumComplexMonoRightCell, sumComplexMonoLeftCell, sumComplexBinocular;

	for (numNL = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, numNL++)
		;

	if (numNL != 4)
	{
		Erro ("Wrong number of neuron layers. The robot_mt_cell must be applied on four neuron layers.", "", "");
		return;
	}

	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
	{
		if (n_list->next != NULL)
		{
			if (n_list->neuron_layer->output_type != n_list->next->neuron_layer->output_type)
			{
				Erro ("The robot_mt_cell must be applied on neuron layers with the same output_type.", "", "");
				return;
			}
		}
	}

	// Achar o numero de parametros
	for (nNumParam = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, nNumParam++);
	nNumParam--;

	// Numero de neuron layers deve ser igual a um
	if (nNumParam != 1)
	{
		Erro ("The robot_mt_cell must have one parameters: Selectivity.", "", "");
		return;
	}

	// Seletividade
	k = filter_desc->filter_params->next->param.fval;

	nl_simple_mono_right = filter_desc->neuron_layer_list->neuron_layer;
	nl_simple_mono_right_q = filter_desc->neuron_layer_list->next->neuron_layer;
	nl_simple_mono_left = filter_desc->neuron_layer_list->next->next->neuron_layer;
	nl_simple_mono_left_q = filter_desc->neuron_layer_list->next->next->next->neuron_layer;

	num_neurons = get_num_neurons (nl_simple_mono_left_q->dimentions);

  d = g_nCurrDisparity;
  hi = nl_simple_mono_right->dimentions.y;
  wi = nl_simple_mono_right->dimentions.x;
  ho = filter_desc->output->dimentions.y;
  wo = filter_desc->output->dimentions.x;

  //compute each mt cell
  for (yo = 0; yo < ho; yo++)
  {
    for (xo = 0; xo < wo; xo++)
    {
      yi = (int) ((float) (yo * hi) / (float) ho + 0.5);
			xir = (int) ((float) (xo * wi) / (float) wo + 0.5);
			xil = xir + d;

      //Check bounds
      if (xil < 0 || xil >= wi) continue;

      //CELULA SIMPLES BINOCULAR
      r = nl_simple_mono_right->neuron_vector[(yi*wi) + xir].output.fval;
      l = nl_simple_mono_left->neuron_vector[(yi*wi) + xil].output.fval;
      sumSimpleBinocularCell = (r + l);

      //CELULA SIMPLES BINOCULAR
      rq = nl_simple_mono_right_q->neuron_vector[(yi*wi) + xir].output.fval;
      lq = nl_simple_mono_left_q->neuron_vector[(yi*wi) + xil].output.fval;
      sumSimpleBinocularCellQ = (rq +lq);

      //CELULA COMPLEXA MONOCULAR
      sumComplexMonoRightCell = (r*r + rq*rq);
      sumComplexMonoLeftCell = (l*l + lq*lq);

      //CELULA COMPLEXA BINOCULAR
      sumComplexBinocular = (sumSimpleBinocularCell * sumSimpleBinocularCell + sumSimpleBinocularCellQ * sumSimpleBinocularCellQ);

      //CELULA MT
      filter_desc->output->neuron_vector[(yo*wo) + xo].output.fval = sumComplexBinocular / (sumComplexMonoLeftCell + sumComplexMonoRightCell + k);
    }
  }

}



/*!
*********************************************************************************
* Function: robot_mt_cell
* Description:
* Inputs:
* Output:
*********************************************************************************
*/

void robot_mt_cell(FILTER_DESC *filter_desc)
{
	int i;
	NEURON_LAYER_LIST *n_list;
	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
	{
		if (n_list->next != NULL)
		{
			if (n_list->neuron_layer->output_type != n_list->next->neuron_layer->output_type)
			{
				Erro ("The robot_mt_cell must be applied on neuron layers with the same output_type.", "", "");
				return;
			}
		}
	}

	if (i != 4)
	{
		Erro ("Wrong number of neuron layers. The robot_mt_cell must be applied on four neuron layers.", "", "");
		return;
	}

	// Achar o numero de parametros
	PARAM_LIST *p_list;
	for (i = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, i++)
		;
	i--;

	// Numero de neuron layers deve ser igual a um
	if (i != 1 && i != 3)
	{
		Erro ("use: robot_mt_cell(float Selectivity, [int X Offset, int Y Offset])", "", "");
		return;
	}

	int scale = (i == 1); // Flag de escalamento / translação
	float k   = filter_desc->filter_params->next->param.fval; // Seletividade
	int x_offset = (scale ? 0 : filter_desc->filter_params->next->next->param.ival);
	int y_offset = (scale ? 0 : filter_desc->filter_params->next->next->next->param.ival);

	NEURON_LAYER *nl_simple_mono_right   = filter_desc->neuron_layer_list->neuron_layer;
	NEURON_LAYER *nl_simple_mono_right_q = filter_desc->neuron_layer_list->next->neuron_layer;
	NEURON_LAYER *nl_simple_mono_left    = filter_desc->neuron_layer_list->next->next->neuron_layer;
	NEURON_LAYER *nl_simple_mono_left_q  = filter_desc->neuron_layer_list->next->next->next->neuron_layer;

	int hi = nl_simple_mono_right->dimentions.y;
	int wi = nl_simple_mono_right->dimentions.x;
	int ho = filter_desc->output->dimentions.y;
	int wo = filter_desc->output->dimentions.x;

	//compute each mt cell
	int yo, xo;
	for (yo = 0; yo < ho; yo++)
	{
		for (xo = 0; xo < wo; xo++)
		{
			int yi  = (scale ? (int) ((float) (yo * hi) / (float) ho + 0.5) : yo + y_offset);
			int xir = (scale ? (int) ((float) (xo * wi) / (float) wo + 0.5) : xo + x_offset);
			int xil = xir + g_nCurrDisparity;

			//Check bounds
			if (xil < 0 || xil >= wi)
				continue;

			//CELULA SIMPLES BINOCULAR
			double r = nl_simple_mono_right->neuron_vector[(yi*wi) + xir].output.fval;
			double l = nl_simple_mono_left->neuron_vector[(yi*wi) + xil].output.fval;
			double sumSimpleBinocularCell = (r + l);

			//CELULA SIMPLES BINOCULAR
			double rq = nl_simple_mono_right_q->neuron_vector[(yi*wi) + xir].output.fval;
			double lq = nl_simple_mono_left_q->neuron_vector[(yi*wi) + xil].output.fval;
			double sumSimpleBinocularCellQ = (rq + lq);

			//CELULA COMPLEXA MONOCULAR
			double sumComplexMonoRightCell = POW2(r) + POW2(rq);
			double sumComplexMonoLeftCell  = POW2(l) + POW2(lq);

			//CELULA COMPLEXA BINOCULAR
			double sumComplexBinocular = POW2(sumSimpleBinocularCell) + POW2(sumSimpleBinocularCellQ);

			//CELULA MT
			double mt = sumComplexBinocular / (sumComplexMonoLeftCell + sumComplexMonoRightCell + k);
			filter_desc->output->neuron_vector[(yo*wo) + xo].output.fval = mt;
		}
	}

}



/*!
*********************************************************************************
* Function: robot_compute_gaussian_kernel					*
* Description:                                        				*
* Inputs:									*
* Output:									*
*********************************************************************************
*/

KERNEL *robot_compute_gaussian_kernel (int kernel_size, float sigma)
{
	int x, y, index = 0, radius;
	float k1, k2, *kernel_points = NULL;
	KERNEL *gaussian_kernel = NULL;

	if ((kernel_size % 2) == 0)
	{
		Erro ("The kernel size must be an odd number (robot_compute_gaussian_kernel).", "", "");
		return NULL;
	}

	if ((gaussian_kernel = (KERNEL *) alloc_mem (sizeof(KERNEL))) == NULL)
	{
		Erro ("Cannot allocate more memory (robot_compute_gaussian_kernel).", "", "");
		return NULL;
	}

	gaussian_kernel->kernel_size = kernel_size;

	if ((gaussian_kernel->kernel_points = kernel_points = (float *) alloc_mem (kernel_size * kernel_size * sizeof (float))) == NULL)
	{
		Erro ("Cannot allocate more memory (robot_compute_gaussian_kernel).", "", "");
		return NULL;
	}

//	k1 = 1.0 / sqrt (2.0 * pi * fltSigma * fltSigma);
	k1 = 1.0f;
	k2 = -.5f / (sigma * sigma);
	radius = kernel_size >> 1; // kernel_size / 2

	for (y = -radius; y <= radius; y++)
	{
		for (x = -radius; x <= radius; x++)
		{
			printf("%1.03f ", kernel_points[index] = k1 * exp (k2 * (float) (x * x + y * y)));
			index++;
		}
		printf("\n");
	}

	return gaussian_kernel;
}



/*!
*********************************************************************************
* Function: robot_gaussian_filter
* Description: Applies a gaussian kernel on a neuron layer
* Inputs:
* Output:
*********************************************************************************
*/

void robot_gaussian_filter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list;
	KERNEL *gaussian_kernel;
	PARAM_LIST *p_list;
	int i, kernel_size, num_points;
	float sigma;

//	struct timeval start;
//	struct timeval end;
//	double start_time, end_time;
//	gettimeofday(&start,NULL);

	if (filter_desc->private_state == NULL)
	{
		for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
			;

		if (i != 1)
		{
			Erro ("Wrong number of neuron layers. The robot_gaussian_filter must be applied on only one neuron layer (robot_gaussian_filter).", "", "");
			return;
		}

		if (filter_desc->neuron_layer_list->neuron_layer->output_type != filter_desc->output->output_type)
		{
			Erro ("The output type of input neuron layer is different of the robot_gaussian_filter output (robot_gaussian_filter).", "", "");
			return;
		}

		for (i = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, i++)
			;

		if (--i != 2)
		{
			Erro ("Wrong number of parameters. The robot_gaussian_filter have 2 parameters: kernel_size and sigma respectivally (robot_gaussian_filter).", "", "");
			return;
		}

		kernel_size = filter_desc->filter_params->next->param.ival;
		sigma	    = filter_desc->filter_params->next->next->param.fval;

		gaussian_kernel = robot_compute_gaussian_kernel (kernel_size, sigma);

		if ((filter_desc->private_state = (void *) gaussian_kernel) == NULL)
		{
			Erro ("Cannot create gaussian kernel (robot_gaussian_filter).", "", "");
			return;
		}
	}
	else
		gaussian_kernel = (KERNEL *) filter_desc->private_state;

	num_points = gaussian_kernel->kernel_size * gaussian_kernel->kernel_size;
	optimized_2d_point_convolution (filter_desc->output, filter_desc->neuron_layer_list->neuron_layer,
					gaussian_kernel->kernel_points, 1.0 / (float) num_points, num_points, KERNEL_2D, GREYSCALE_FLOAT);

//	gettimeofday(&end,NULL);
//	start_time = (double) start.tv_sec + (double) start.tv_usec*1e-6;
//	end_time   = (double) end.tv_sec   + (double) end.tv_usec*1e-6;
//	printf("Elapsed time: %2.5lfseg (robot_gaussian_filter).\n", end_time - start_time);
}



/*!
*********************************************************************************
* Function: perception_gain_model_weight
* Description: Returns a weight used to estimate the illuminance of an image
*              pixel based on the perception gain model [Gross & Brajovic, 2003]
* Inputs:
* Output:
*********************************************************************************
*/

float perception_gain_model_weight (float Ia, float Ib, float CR)
{
	float weight, I_min, I, delta_I;

	// Weber's Contrast = abs (Ia - Ib) / min (Ia, Ib)
	//
	// The weights are modulated by the inverse of Weber's contrast:
	//		weight = min (Ia, Ib) / abs (Ia - Ib)
	//
	// The Contrast Ratio (CR) refers to the ratio of intensities between the most intense (brightest)
	// and the least intense (darkest) elements of an image. A ratio of 10:1 is typical of a CRT.

	// The intensity levels should be normalized to a minimum of (1.0 / CR) and a maxinum of 1.0
	I_min = (1.0 / CR);
	I = (Ia < Ib ? Ia : Ib) / 255.0;
	I = (I > I_min ? I : I_min);

	// To avoid division by zero, let's assume abs (Ia - Ib) >= (1.0 / CR)
	delta_I = (Ia < Ib ? (Ib - Ia) : (Ia - Ib)) / 255.0;
	delta_I = (delta_I > I_min ? delta_I : I_min);

	weight = I / delta_I;

	return weight;
}


/*!
*********************************************************************************
* Function: perception_gain_model
* Description: Returns an estimate of the illuminance of an image pixel based on
*              the perception gain model [Gross & Brajovic, 2003]
*              The linear partial differential equation can be solved numerically
*              using the Gauss-Seidel iterative method. This function returns
*              the result of one iteration.
* Inputs:
* Output:
*********************************************************************************
*/

float perception_gain_model (NEURON_LAYER *nl_illuminance, NEURON_LAYER *nl_image, int i, int j, float fltSmoothness, float fltContrastRatio)
{
	int   w, h;
	int   px, px_up, px_down, px_left, px_right;
	float I, I_up, I_down, I_left, I_right;
	float L, /*weight,*/ L_up, L_down, L_left, L_right;
	float weight_up, weight_down, weight_left, weight_right;
	float weighted_sum, total_weight;

	w = nl_image->dimentions.x;
	h = nl_image->dimentions.y;

	px       = nl_image->neuron_vector[j * w + i].output.ival;
	px_up    = (j ==  0      ? 0 : nl_image->neuron_vector[(j - 1) * w + i].output.ival);
	px_down  = (j == (h - 1) ? 0 : nl_image->neuron_vector[(j + 1) * w + i].output.ival);
	px_left  = (i ==  0      ? 0 : nl_image->neuron_vector[j * w + (i - 1)].output.ival);
	px_right = (i == (w - 1) ? 0 : nl_image->neuron_vector[j * w + (i + 1)].output.ival);

	I       = (float) (RED (px)       + GREEN (px)       + BLUE (px)      ) / 3.0;
	I_up    = (float) (RED (px_up)    + GREEN (px_up)    + BLUE (px_up)   ) / 3.0;
	I_down  = (float) (RED (px_down)  + GREEN (px_down)  + BLUE (px_down) ) / 3.0;
	I_left  = (float) (RED (px_left)  + GREEN (px_left)  + BLUE (px_left) ) / 3.0;
	I_right = (float) (RED (px_right) + GREEN (px_right) + BLUE (px_right)) / 3.0;

	L_up    = (j ==  0      ? 0.0 : nl_illuminance->neuron_vector[(j - 1) * w + i].output.fval);
	L_down  = (j == (h - 1) ? 0.0 : nl_illuminance->neuron_vector[(j + 1) * w + i].output.fval);
	L_left  = (i ==  0      ? 0.0 : nl_illuminance->neuron_vector[j * w + (i - 1)].output.fval);
	L_right = (i == (w - 1) ? 0.0 : nl_illuminance->neuron_vector[j * w + (i + 1)].output.fval);

	weight_up    = (j ==  0      ? 0.0 : perception_gain_model_weight (I_up,    I, fltContrastRatio));
	weight_down  = (j == (h - 1) ? 0.0 : perception_gain_model_weight (I_down,  I, fltContrastRatio));
	weight_left  = (i ==  0      ? 0.0 : perception_gain_model_weight (I_left,  I, fltContrastRatio));
	weight_right = (i == (w - 1) ? 0.0 : perception_gain_model_weight (I_right, I, fltContrastRatio));

	weighted_sum = (I + fltSmoothness * (L_up * weight_up + L_down * weight_down + L_left * weight_left + L_right * weight_right));
	total_weight = (1.0 + fltSmoothness * (weight_up + weight_down + weight_left + weight_right));
	L = weighted_sum / total_weight;

	return L;
}


/*!
*********************************************************************************
* Function: robot_illuminance_filter
* Description: Applies a preprocessing algorithm for illuminance estimate
*              based on neighbor input stimuli and contrast  [Gross & Brajovic, 2003]
* Inputs:
* Output:
*********************************************************************************
*/

void robot_illuminance_filter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list;
	NEURON_LAYER *nl_image, *nl_illuminance;
	PARAM_LIST *p_list;
	int i, j;
	int w, h;
	float fltSmoothness, fltContrastRatio, fltOutput;
	float *avg_val;
	//static int flgInitialize = 0;

	if (filter_desc->private_state == NULL)
	{
		for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++);

		if (i != 1)
		{
			Erro ("Wrong number of neuron layers. The robot_illuminance_filter must be applied on only one neuron layer.", "", "");
			return;
		}

		if (filter_desc->output->output_type != GREYSCALE_FLOAT)
		{
			Erro ("The output type of the robot_illuminance_filter output must be greyscale_float.", "", "");
			return;
		}

		// Get the smoothness constant and the Weber's contrast ratio
		for (i = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, i++);
		i--;

		if (i != 2)
		{
			Erro ("Wrong number of parameters. The robot_illuminance_filter has two parameters: smoothness and contrast ratio.", "", "");
			return;
		}

		filter_desc->private_state = (void *) alloc_mem (sizeof(float));

		nl_image = filter_desc->neuron_layer_list->neuron_layer;
		nl_illuminance = filter_desc->output;
		copy_neuron_outputs (nl_illuminance, nl_image);
	}

	avg_val = (float *) filter_desc->private_state;
	*avg_val = 0.0;

	nl_image = filter_desc->neuron_layer_list->neuron_layer;
	nl_illuminance = filter_desc->output;

	fltSmoothness     = filter_desc->filter_params->next->param.fval;
	fltContrastRatio  = filter_desc->filter_params->next->next->param.fval;

	w = filter_desc->output->dimentions.x;
	h = filter_desc->output->dimentions.y;

	for (i = 0; i < w; i++)
	{
		for (j = 0; j < h; j++)
		{
			fltOutput = perception_gain_model (nl_illuminance, nl_image, i, j, fltSmoothness, fltContrastRatio);
			filter_desc->output->neuron_vector[j * w + i].output.fval = fltOutput;
			(*avg_val) += fltOutput;
		}
	}

	(*avg_val) /= (w * h);
}


/*!
*********************************************************************************
* Function: robot_reflectance_filter
* Description: Applies a preprocessing algorithm for reflectance estimate
*              based on neighbor input stimuli and contrast  [Gross & Brajovic, 2003]
* Inputs:
* Output:
*********************************************************************************
*/

void robot_reflectance_filter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list;
	NEURON_LAYER *nl_image, *nl_illuminance;
	PARAM_LIST *p_list;
	int i, j;
	int w, h;
	int pixel;
	float I, L, R;

	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++);

	if (i != 2)
	{
		Erro ("Wrong number of neuron layers. The robot_reflectance_filter must be applied on two neuron layers.", "", "");
		return;
	}

	if (filter_desc->neuron_layer_list->next->neuron_layer->output_type != GREYSCALE_FLOAT)
	{
		Erro ("The output type of the illuminance neuron layer must be greyscale_float.", "", "");
		return;
	}

	if (filter_desc->output->output_type != GREYSCALE_FLOAT)
	{
		Erro ("The output type of the robot_reflectance_filter output must be greyscale_float.", "", "");
		return;
	}

	for (i = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, i++);
	i--;

	if (i != 0)
	{
		Erro ("Wrong number of parameters. The robot_reflectance_filter has no parameters.", "", "");
		return;
	}

	nl_image = filter_desc->neuron_layer_list->neuron_layer;
	nl_illuminance = filter_desc->neuron_layer_list->next->neuron_layer;

	w = filter_desc->output->dimentions.x;
	h = filter_desc->output->dimentions.y;

	for (i = 0; i < w; i++)
	{
		for (j = 0; j < h; j++)
		{
			pixel = nl_image->neuron_vector[j * w + i].output.ival;
			I = (float) (RED (pixel) + GREEN (pixel) + BLUE (pixel)) / 3.0;
			L = nl_illuminance->neuron_vector[j * w + i].output.fval;
			if (I == 0.0)
				R = 0.0;
			else if (L == 0.0)
				R = 1.0;
			else
				R = I / L;
			filter_desc->output->neuron_vector[j * w + i].output.fval =  R;
		}
	}
}

/*!
*********************************************************************************
* \brief Apply Segmentation Filter.
* \param[in] filter_desc The filter descriptor structure.
* \pre The filter descriptor created and initialized. The output neuron layer
* must have the same dimentions.
* \post The updated output neuron layer.
* \return Nothing.
*********************************************************************************
*/
void segment_filter_nl (FILTER_DESC *filter_desc)
{
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER *nl = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_OUTPUT value;
	int i, ho, wo, hi, wi, j;
	unsigned char *temp_img2, *temp_img1;
	float sigma, k, min, max_fval, min_fval;

	// Checks the Parameters
	for (i = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, i++)
		;

	if (i < 3)
	{
		Erro ("Wrong number of parameters for segment_filter. ", "", "");
		//printf("%i   !!!!!!!!!!!!!!!!!!!!!!! \n ", i);
		return;
	}

	// Gets the Parameters
	sigma	= filter_desc->filter_params->next->param.fval;
	k	= filter_desc->filter_params->next->next->param.fval;
	min	= filter_desc->filter_params->next->next->next->param.fval;
	//printf("sigma = %f, k = %f, min = %f   !!!!!!!!!!!!!!!!!!!!!!! \n ", sigma, k, min );

        // Checks the Input Neuron Layers
	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
		;

	if (i != 1)
	{
		Erro ("Wrong number of neuron layers. segment_filter must be applied on only one neuron layer.", "", "");
		return;
	}

	// Gets the Input Neuron Layers
	nl = filter_desc->neuron_layer_list->neuron_layer;

	// Gets Input Neuron Layer Dimentions
	wi = nl->dimentions.x;
	hi = nl->dimentions.y;

	// Gets Output Neuron Layer Dimentions
	wo = filter_desc->output->dimentions.x;
	ho = filter_desc->output->dimentions.y;
	//printf("wi = %i, hi = %i, wo= %i, ho = %i \n ", wi, hi, wo, ho);

	if ((wi != wo) || (hi != ho))
	{
		Erro ("Wrong dimentions of neuron layers. segment_filter must be applied on neuron layers of same dimention.", "", "");
		return;
	}

//	min_fval = nl->min_neuron_output.fval; // o codigo teve que ser alterado pelo abaixo, pois os valores de min e max
//	max_fval = nl->max_neuron_output.fval; // as vezes não correspondiam ao real.
//	printf("min_fval = %f e max_fval = %f \n", min_fval, max_fval);
	max_fval = min_fval = nl->neuron_vector[0].output.fval;
	for (i = 0; i < (wi * hi); i++)
	{
		value = nl->neuron_vector[i].output;

		if (value.fval > max_fval)
			max_fval = value.fval;
		else if (value.fval < min_fval)
			min_fval = value.fval;
	}

	temp_img1 = (unsigned char*) malloc (3 * wi * hi * sizeof (unsigned char));
	temp_img2 = (unsigned char*) malloc (3 * wi * hi * sizeof (unsigned char));

	// Os testes no algoritmo de segmentação foram feitos inicialmente com um arquivo contendo o mapa de disparidades.
	// Este arquivo foi gerado pela propria MAE.
	// Porém os dados da neuron_vector da MAE possuem valores negativos e o arquivo salvo, não.
	// Como os resultados em arquivo foram muito superiores ao resultado com valores negativos,
	// foi feita a transformação dos valores negativos, tal qual da gravação do arquivo.
	for (j = 0; j < hi; j++)
	{
		for (i = 0; i < wi; i++)
			filter_desc->output->neuron_vector[i + j * wi].output.fval =
                        255.0 * (nl->neuron_vector[i + j * wi].output.fval - min_fval) / (max_fval - min_fval);
	}

	// este bloco de comandos for converte os dados da neuron_vector para o formato
	// de imagem de entrada do algoritmo de segmentação.
	for (j = 0; j < hi; j++)
	{
		for (i = 0; i < wi; i++)
		{
			value = filter_desc->output->neuron_vector[i + wi * j].output;
			temp_img1[3 * (j * wi + i) + 0] = RED((int)value.fval);
			temp_img1[3 * (j * wi + i) + 1] = GREEN((int)value.fval);
			temp_img1[3 * (j * wi + i) + 2] = BLUE((int)value.fval);
		}
	}

	// O código abaixo, o comando for + chamada da dunção ApplyEdgeFilter + comando for
	// foram comentados, pois a chamda de funções do Matlab estavam um pouco instáveis em minha estação. 
	// A instabilidade de se ter segmentation fault do matlab intermitentemente. Mesmo dentro do DDD, o trace
	// não conseguia identificar onde no Matlab ocorria este erro,.
	// Para que o mesmo faça efeito, descomente este bloco de comandos e troque o nome temp_img1 para temp_img2
	// do comando for anterior à este comentário.
	// As transformações feitas pelo comando for anterior e este posterior comentado poderiam ser unificados,
	// contudo devido aos problemas citados, as conversões de imagens foram feitas separadamente.

	// O bloco de comando for abaixo transforma a imagem entendida pelo algoritmo de segmentação para 
	// o formato da imagem de entrada do Matlab. O bloco de comando for após o ApplyEdgeFilter faz o inverso.
/*	for (j = 0; j < hi; j++)
	{
		for (i = 0; i < wi; i++)
		{
			temp_img1[(hi - j - 1) + i * hi + 0 * wi * hi] = temp_img2[3 * (j * wi + i) + 0];
			temp_img1[(hi - j - 1) + i * hi + 1 * wi * hi] = temp_img2[3 * (j * wi + i) + 1];
			temp_img1[(hi - j - 1) + i * hi + 2 * wi * hi] = temp_img2[3 * (j * wi + i) + 2];
		}
	}

	ApplyEdgeFilter(temp_img2, temp_img1, wi, hi);

	for (j = 0; j < hi; j++)
	{
		for (i = 0; i < wi; i++)
		{
			temp_img1[3 * (j * wi + i) + 0] = temp_img2[(hi - j - 1) + i * hi + 0 * wi * hi];
			temp_img1[3 * (j * wi + i) + 1] = temp_img2[(hi - j - 1) + i * hi + 1 * wi * hi];
			temp_img1[3 * (j * wi + i) + 2] = temp_img2[(hi - j - 1) + i * hi + 2 * wi * hi];
		}
	}

*/
	temp_img2 = (unsigned char *)segment_image (sigma, k, min, (char *)temp_img1, wi, hi);

	// este bloco de comandos for converte os dados do algoritmo de segmentação 
	// para o formato da neuron_vector
	for (j = 0; j < ho; j++)
	{
		for (i = 0; i < wo; i++)
		{
			filter_desc->output->neuron_vector[j * wo + i].output.fval =
				PIXEL(temp_img2[3 * (j * wo + i) + 0],
				      temp_img2[3 * (j * wo + i) + 1],
				      temp_img2[3 * (j * wo + i) + 2]);
		}
	}
	free(temp_img2);
	free(temp_img1);
}

/*!
*********************************************************************************
* \brief Rotate Transform 90 degrees clockwise.
* \param[in] filter_desc The filter descriptor structure.
* \pre The filter descriptor created and initialized. The output neuron layer
* must have correct dimention.
* \post The updated output neuron layer.
* \return Nothing.
*********************************************************************************
*/
void rotate_nl (FILTER_DESC *filter_desc)
{
	PARAM_LIST *p_list = NULL;
	int i, ho, wo, hi, wi, j;
	float angle;
	NEURON_LAYER *nl = NULL;
	NEURON_LAYER_LIST *n_list = NULL;

	// Checks the Parameters
	for (i = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, i++)
		;

	if (i < 1)
	{
		Erro ("Wrong number of parameters for segment_filter. ", "", "");
		return;
	}

	// Gets the Parameters
	angle	= filter_desc->filter_params->next->param.fval;
	printf("angle = %f   !!!!!!!!!!!!!!!!!!!!!!! \n ", angle);

        // Checks the Input Neuron Layers
	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
		;

	if (i != 1)
	{
		Erro ("Wrong number of neuron layers. segment_filter must be applied on only one neuron layer.", "", "");
		return;
	}

	// Gets the Input Neuron Layers
	nl = filter_desc->neuron_layer_list->neuron_layer;

	// Gets Input Neuron Layer Dimentions
	wi = nl->dimentions.x;
	hi = nl->dimentions.y;

	// Gets Output Neuron Layer Dimentions
	wo = filter_desc->output->dimentions.x;
	ho = filter_desc->output->dimentions.y;
	printf("wi = %i, hi = %i, wo= %i, ho = %i \n ", wi, hi, wo, ho);

	// Validates the input and output dimentions
	if ((wi != ho) || (hi != wo))
	{
		Erro ("Wrong dimentions of neuron layers.", "", "");
		return;
	}

	// No futuro, incluir mais angulos de rotação.	
	if (angle == 90.0)
	{
		for (j = 0; j < ho; j++)
		{
			for (i = 0; i < wo; i++)
			{
				filter_desc->output->neuron_vector[(ho - j - 1) * wo + i].output.fval =
					nl->neuron_vector[i * wi + j].output.fval;
			}
		}
	}
}
