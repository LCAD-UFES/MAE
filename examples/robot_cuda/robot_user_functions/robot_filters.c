#include <stdio.h>
#include <stdlib.h>

#include "mae.h"
#include "filter.h"
#include "robot_user_functions.h"

extern double  y0_val;

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

	//printf("LOG_POLAR\n");

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
	float /*dif_float,*/ dif_accumulator, a, b, denom;
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
		dif_accumulator = sqrtf (dif_accumulator);
		denom = k + sqrtf (denom);
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

void robot_mt_cell (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list;
	NEURON_LAYER *nl_complex_left, *nl_complex_right, *nl_complex_binocular;
	PARAM_LIST *p_list;
	int i/*, j*/;
	int num_neurons, numNL, nNumParam;
	float fltComplexBinocular, fltComplexLeft, fltComplexRight, k;

	for (numNL = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, numNL++)
		;

	if (numNL != 3) 
	{
		Erro ("Wrong number of neuron layers. The robot_mt_cell must be applied on three neuron layers.", "", "");
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

	// Numero de neuron layers deve ser igual a tres
	if (nNumParam != 1)
	{
		Erro ("The robot_sum_filter_normalized must have one parameter: Selectivity.", "", "");
		return;
	}

	// Seletividade
	k = filter_desc->filter_params->next->param.fval;

	nl_complex_left = filter_desc->neuron_layer_list->neuron_layer;
	nl_complex_right = filter_desc->neuron_layer_list->next->neuron_layer;
	nl_complex_binocular = filter_desc->neuron_layer_list->next->next->neuron_layer;

	num_neurons = get_num_neurons (filter_desc->output->dimentions);

	for (i = 0; i < num_neurons; i++)
	{
		fltComplexLeft = nl_complex_left->neuron_vector[i].output.fval;
		fltComplexRight = nl_complex_right->neuron_vector[i].output.fval;
		fltComplexBinocular = nl_complex_binocular->neuron_vector[i].output.fval;
		
		filter_desc->output->neuron_vector[i].output.fval = fltComplexBinocular / (fltComplexLeft + fltComplexRight + k);
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
#define NEW_GAUSSIAN_FILTER

#ifdef NEW_GAUSSIAN_FILTER
void robot_gaussian_filter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list;
	NEURON_LAYER *nl_input = NULL, *nl_output = NULL;
	NEURON *nv_input = NULL, *nv_output = NULL;
	KERNEL *gaussian_kernel;
	PARAM_LIST *p_list;
	int i, j, wo, ho, xo, yo, wi, hi, xi, yi, kernel_size, kernel_div_2, input_line, output_line;
	float sigma, *kernel_points = NULL, wi_wo_factor, *intensity = NULL, accumulator;
	static float *intensity_vector = NULL;

// 	struct timeval start;
// 	struct timeval end; 
// 	double start_time, end_time;
// 	gettimeofday(&start,NULL);

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

	kernel_size   = gaussian_kernel->kernel_size;
	kernel_div_2 = kernel_size >> 1;

	nl_input = filter_desc->neuron_layer_list->neuron_layer;
	wi = nl_input->dimentions.x;
	hi = nl_input->dimentions.y;

	nl_output = filter_desc->output;
	wo = nl_output->dimentions.x;
	ho = nl_output->dimentions.y;

	if (intensity_vector == NULL)
	{
		if ((intensity_vector = (float *) alloc_mem ((wo + kernel_size) * sizeof (float))) == NULL)
		{
			Erro ("Cannot allocate more memory (robot_gaussian_filter).", "", "");
			return;
		}

		for (i = 0; i < (wo + kernel_size); i++)
			intensity_vector[i] = .0f;
	}

	wi_wo_factor = (float) wi / (float) wo;

	nv_input  = nl_input->neuron_vector;
	nv_output = nl_output->neuron_vector;

	for (i = 0; i < wo * ho; i++)
		nv_output[i].output.fval = .0f;

	for (yo = 0; yo < ho; yo++)
	{
		yi = (int) ((float) (yo * hi) / (float) ho + 0.5);
		input_line  = yi * wi;

		intensity = &(intensity_vector[kernel_div_2]);
		for (xo = 0; xo < wo; xo++)
		{
			xi = (int) ((float) xo * wi_wo_factor + 0.5);
			intensity[xo] = nv_input[input_line + xi].output.fval;
		}

		for (j = 0; j < kernel_size; j++)
		{
			output_line = yo + kernel_div_2 - j;

			if (output_line < 0 || output_line >= ho)
				continue;

			output_line *= wo;
			kernel_points = &(gaussian_kernel->kernel_points[kernel_size * j]);
			intensity = intensity_vector;
			
			for (xo = 0; xo < wo; xo++)
			{
				accumulator = .0f;
				for (i = 0; i < kernel_size; i++)
					accumulator += kernel_points[i] * intensity[i];
		
				nv_output[output_line + xo].output.fval += accumulator;
				
				intensity++;
			}
		}
	}

// 	gettimeofday(&end,NULL);
// 	start_time = (double) start.tv_sec + (double) start.tv_usec*1e-6;
// 	end_time   = (double) end.tv_sec   + (double) end.tv_usec*1e-6;
// 	printf("Elapsed time: %2.5lfseg (robot_gaussian_filter).\n", end_time - start_time);
}
#else
void robot_gaussian_filter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list;
	NEURON_LAYER *nl;
	RECEPTIVE_FIELD_DESCRIPTION *receptive_field;
	PARAM_LIST *p_list;
	int i, j;
	int wo, ho, nKernelSize;
	float fltSigma;

// 	struct timeval start;
// 	struct timeval end; 
// 	double start_time, end_time;
// 	gettimeofday(&start,NULL);

	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++);

	if (filter_desc->private_state == NULL)
	{
		if (i != 1) 
		{
			Erro ("Wrong number of neuron layers. The robot_gaussian_filter must be applied on only one neuron layer.", "", "");
			return;
		}
	
		if (filter_desc->neuron_layer_list->neuron_layer->output_type != filter_desc->output->output_type)
		{
			Erro ("The output type of input neuron layer is different of the robot_gaussian_filter output.", "", "");
			return;
		}

		// Buscar o KernelSize e o Sigma
		for (i = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, i++);
		i--;

		if (i != 2) 
		{
			Erro ("Wrong number of parameters. The robot_gaussian_filter have 2 parameters: kernel_size and sigma respectivally.", "", "");
			return;
		}

		nKernelSize	= filter_desc->filter_params->next->param.ival;
		fltSigma	= filter_desc->filter_params->next->next->param.fval;

		receptive_field = (RECEPTIVE_FIELD_DESCRIPTION*)malloc(sizeof(RECEPTIVE_FIELD_DESCRIPTION));
		compute_gaussian_kernel(receptive_field, nKernelSize, fltSigma);

		filter_desc->private_state = (void*)receptive_field;
	}

	receptive_field = (RECEPTIVE_FIELD_DESCRIPTION*)filter_desc->private_state;
	nl = filter_desc->neuron_layer_list->neuron_layer;

	wo = filter_desc->output->dimentions.x;
	ho = filter_desc->output->dimentions.y;



	switch (filter_desc->output->output_type)
	{
		case GREYSCALE:
		{
			for (i = 0; i < wo; i++)
			{
				for (j = 0; j < ho; j++)
				{
					filter_desc->output->neuron_vector[j * wo + i].output.ival = (int)apply_gaussian_kernel(receptive_field, nl, i, j);
				}
			}
		}
		break;

		case GREYSCALE_FLOAT:
		{
			for (i = 0; i < wo; i++)
			{
				for (j = 0; j < ho; j++)
				{
					filter_desc->output->neuron_vector[j * wo + i].output.fval = apply_gaussian_kernel(receptive_field, nl, i, j);
				}
			}
		}
		break;
	}
	
// 	gettimeofday(&end,NULL);
// 	start_time = (double) start.tv_sec + (double) start.tv_usec*1e-6;
// 	end_time   = (double) end.tv_sec   + (double) end.tv_usec*1e-6;
// 	printf("Elapsed time: %2.5lfseg (robot_gaussian_filter).\n", end_time - start_time);

}
#endif


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
	float L/*, weight*/, L_up, L_down, L_left, L_right;
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
	/*static int flgInitialize = 0;*/

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
********************************************************************************
********************************* CUDA SESSION *********************************
******************* Camilo A. Carvalho & Alberto F. De Souza *******************
********************** {camilo, alberto}@lcad.inf.ufes.br **********************
*********************************** AGO/2009 ***********************************
********************************************************************************
*/

/*!
*********************************************************************************
* Function: cuda_map_v1_to_image & cuda_biological_gabor_filter
* Description: map_v1_to_image & biological_gabor_filter implemented in CUDA
* Inputs: stereo images
* Output: monocular_simple_cells
*********************************************************************************
*/

// Provavelmente seja melhor setar estas variaveis localmente, para
// evitar que alterações em outras aplicacoes do ROBOT afetem esta.
extern float global_factor;
extern float teste;

void
cuda_biological_gabor_filter (FILTER_DESC *filter_desc)
{
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nl_image = NULL;
	RECEPTIVE_FIELD_DESCRIPTION *receptive_fields;
	int i, *xi = NULL, *yi = NULL, hi, wi, ho, wo, x_center, y_center;
	float camera_opening, cycles_per_degree, frequency, width, aspect_ratio, orientation, phase, log_factor;
	double correction;
	int mapping, shift, kernel_type;
	INPUT_DESC *input;
	NEURON_OUTPUT previous_output;
	int pyramid_height, pyramid_level;
	RECEPTIVE_FIELD_DESCRIPTION *receptive_field_descriptor;
	float *kernel_points;
	float delta_area;

	// Checks the Parameters
	for (i = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, i++)
		;

	if (i != 11)
	{
		Erro ("Wrong number of parameters for biological_gabor_filter.", "", "");
		return;
	}

	// Gets the Parameters
	camera_opening		= filter_desc->filter_params->next->param.fval;
	cycles_per_degree	= filter_desc->filter_params->next->next->param.fval;
	width			= filter_desc->filter_params->next->next->next->param.fval;
	aspect_ratio		= filter_desc->filter_params->next->next->next->next->param.fval;
	orientation		= filter_desc->filter_params->next->next->next->next->next->param.fval;
	phase			= filter_desc->filter_params->next->next->next->next->next->next->param.fval;
	shift			= filter_desc->filter_params->next->next->next->next->next->next->next->param.ival;
	mapping			= filter_desc->filter_params->next->next->next->next->next->next->next->next->param.ival;
	log_factor		= filter_desc->filter_params->next->next->next->next->next->next->next->next->next->param.fval;
	kernel_type             = filter_desc->filter_params->next->next->next->next->next->next->next->next->next->next->param.ival;

        // Checks the Input Neuron Layers
	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
		;

	if (i != 1 && i != 2)
	{
		Erro ("Wrong number of neuron layers. biological_gabor_filter must be applied on one or two neuron layers.", "", "");
		return;
	}

	// Gets the Input Neuron Layers
	nl_image = filter_desc->neuron_layer_list->neuron_layer;

	// Gets Input Neuron Layer Dimentions
	wi = nl_image->dimentions.x;
	hi = nl_image->dimentions.y;

	// Gets Output Neuron Layer Dimentions
	wo = filter_desc->output->dimentions.x;
	ho = filter_desc->output->dimentions.y;

	correction = (double) ho / (double) (ho - 1);

	input = get_input_by_neural_layer (nl_image);

	pyramid_height = input->pyramid_height;

	// Computes Kernels
	if (filter_desc->private_state == NULL)
	{
		receptive_fields = (RECEPTIVE_FIELD_DESCRIPTION *) alloc_mem ((pyramid_height + 1) * sizeof(RECEPTIVE_FIELD_DESCRIPTION));

		for (pyramid_level = 0; pyramid_level <= pyramid_height; pyramid_level++)
		{
			// Calculates the spatial frequency (cycles per pixel)
			frequency = (camera_opening / (float) wi) * cycles_per_degree * pow (2.0f, (float) pyramid_level);

			receptive_fields[pyramid_level].points = NULL;
			compute_v1_spatial_receptive_field (&(receptive_fields[pyramid_level]), frequency, width, aspect_ratio, orientation, phase, kernel_type);

			// Calculates the minimum value of the result of a gabor filter
			bidimentional_convolution_test_gabor (&(receptive_fields[pyramid_level]));
		}
		filter_desc->private_state = (void *) receptive_fields;
	}
	else
		receptive_fields = (RECEPTIVE_FIELD_DESCRIPTION *) filter_desc->private_state;

	if (input == NULL)
	{
		x_center = 0;
		y_center = 0;
	}
	else
	{
		if (TYPE_MOVING_FRAME == STOP)
		{
			x_center = (input->wxd - input->wx) >> PYRAMID_LEVEL;
			y_center = (input->wyd - input->wy) >> PYRAMID_LEVEL;
		}
		else
		{
			x_center = wi/2;
			y_center = hi/2;
		}
	}

	if (mapping == MAP_PLAN)
	{
		receptive_field_descriptor = &(receptive_fields[0]);
		kernel_points = (float *) alloc_mem (receptive_field_descriptor->num_points * sizeof (float));
		for (i = 0; i < receptive_field_descriptor->num_points; i++)
			kernel_points[i] = receptive_field_descriptor->points[i].weight * receptive_field_descriptor->points[i].gabor;
		delta_area = (receptive_field_descriptor->area / (float) receptive_field_descriptor->num_points) / 6.0;

		optimized_2d_point_convolution (filter_desc->output, filter_desc->neuron_layer_list->neuron_layer, 
						kernel_points, delta_area, receptive_field_descriptor->num_points, kernel_type, COLOR);

		free (kernel_points);
	}
	else
	{
		xi = (int *) malloc (wo*ho * sizeof (int));
		yi = (int *) malloc (wo*ho * sizeof (int));

		cuda_map_v1_to_image (xi, yi, wi, hi, wo, ho, x_center, y_center, correction, log_factor, shift);

		int rf_num_points = receptive_fields[0].num_points;

		// Hackeado violentamente!
		// Motivo: (receptive_field_descriptor->area) nao funciona caso passado, mesmo tendo valor fixo,
		//		atribuído na chamada de COMPUTE_V1_SPATIAL_RECEPTIVE_FIELD
		// Solucao: atribuir manualmente.
		float c_delta_area = 1.0f / (float) (rf_num_points);

		// Alocacao dos vetores de dados que serao manipulados pela funcao em CUDA.
		float *s_m = (float *) malloc ((wo*ho) * sizeof (float));	// Saida do filtro (celulas simples monoculares).
		int *image_vector = (int *) malloc ((wi*hi) * sizeof (int));	// Vetor contendo os valores do neuronio da imagem.
		float *rf_points_w = (float *) malloc ((rf_num_points)* sizeof (float));
		float *rf_points_g = (float *) malloc ((rf_num_points)* sizeof (float));
		int *rf_points_x = (int *) malloc ((rf_num_points)* sizeof (int));
		int *rf_points_y = (int *) malloc ((rf_num_points)* sizeof (int));

		// Copies the receptive_fields values to simple vectors (that will be send to CUDA).
		for (i = 0; i < rf_num_points; i++)
		{
			rf_points_w[i] = receptive_fields[0].points[i].weight;
			rf_points_g[i] = receptive_fields[0].points[i].gabor;

			rf_points_x[i] = receptive_fields[0].points[i].x;
			rf_points_y[i] = receptive_fields[0].points[i].y;
		}

		int dimension = wi*hi;	// Dimension of inputs.
		int size = wo*ho;	// Dimension of outputs.

		// Copies the input neuron to a simple vector (that will be send to CUDA).
		for (i = 0; i < dimension; i++)
		{
			image_vector[i] = nl_image->neuron_vector[i].output.ival;
		}

		// Sending the simple vectors to CUDA_BIOLOGICAL_GABOR_NLS, implemented in CUDA
		cuda_biological_gabor_nls_old (s_m, xi, yi, wi, hi, size, rf_num_points, rf_points_x, rf_points_y,
						image_vector, rf_points_w, rf_points_g, global_factor, c_delta_area, teste);

		for (i = 0; i < wo*ho; i++)
		{
				previous_output.fval = filter_desc->output->neuron_vector[i].output.fval = s_m[i];
		}

		// Cleaning
		free (s_m);
		free (image_vector);
		free (xi);
		free (yi);
		free (rf_points_x);
		free (rf_points_y);
		free (rf_points_w);
		free (rf_points_g);
	}
}



extern int setverg_cont;

void
cuda_calls (V1_MT_PRIVATE_STATE *v1_mt_private_state, int wi, int hi, int wo, int ho, int right_x_center, int right_y_center,
		int left_x_center, int left_y_center, double correction, float log_factor/*, NEURON_LAYER *nl_shift_map*/,
		int *shift, int num_neurons, int rf_num_points, float c_delta_area, float k)
{
	// IMAGE_RIGHT
	if (setverg_cont == 0)
	{
		cuda_map_v1_to_image (v1_mt_private_state->d_xi, v1_mt_private_state->d_yi, wi, hi, wo, ho, right_x_center, right_y_center,
				correction, log_factor, shift[0]);

		cuda_biological_gabor_nls (v1_mt_private_state->d_s_r, v1_mt_private_state->d_xi, v1_mt_private_state->d_yi,
						v1_mt_private_state->d_image_vector, v1_mt_private_state->xi, v1_mt_private_state->yi, wi, hi, num_neurons,
						rf_num_points, v1_mt_private_state->rf_points_x[0], v1_mt_private_state->rf_points_y[0],
						v1_mt_private_state->image_vector_right, v1_mt_private_state->rf_points_w[0],
						v1_mt_private_state->rf_points_gabor[0], global_factor, c_delta_area, teste);

		cuda_biological_gabor_nls (v1_mt_private_state->d_s_r_q, v1_mt_private_state->d_xi, v1_mt_private_state->d_yi,
						v1_mt_private_state->d_image_vector, v1_mt_private_state->xi, v1_mt_private_state->yi, wi, hi, num_neurons,
						rf_num_points, v1_mt_private_state->rf_points_x[1], v1_mt_private_state->rf_points_y[1],
						v1_mt_private_state->image_vector_right, v1_mt_private_state->rf_points_w[1],
						v1_mt_private_state->rf_points_gabor[1], global_factor, c_delta_area, teste);
	}

	// IMAGE_LEFT
	cuda_map_v1_to_image (v1_mt_private_state->d_xi, v1_mt_private_state->d_yi, wi, hi, wo, ho, left_x_center, left_y_center,
			      correction, log_factor, shift[2]);

	cuda_biological_gabor_nls (v1_mt_private_state->d_s_l, v1_mt_private_state->d_xi, v1_mt_private_state->d_yi,
					v1_mt_private_state->d_image_vector, v1_mt_private_state->xi, v1_mt_private_state->yi, wi, hi, num_neurons,
					rf_num_points, v1_mt_private_state->rf_points_x[2], v1_mt_private_state->rf_points_y[2],
					v1_mt_private_state->image_vector_left, v1_mt_private_state->rf_points_w[2],
					v1_mt_private_state->rf_points_gabor[2], global_factor, c_delta_area, teste);

	cuda_biological_gabor_nls (v1_mt_private_state->d_s_l_q, v1_mt_private_state->d_xi, v1_mt_private_state->d_yi,
					v1_mt_private_state->d_image_vector, v1_mt_private_state->xi, v1_mt_private_state->yi, wi, hi, num_neurons,
					rf_num_points, v1_mt_private_state->rf_points_x[3], v1_mt_private_state->rf_points_y[3],
					v1_mt_private_state->image_vector_left, v1_mt_private_state->rf_points_w[3],
					v1_mt_private_state->rf_points_gabor[3], global_factor, c_delta_area, teste);

	// CUDA_BIGFILTER
	cuda_bigfilter_nls (v1_mt_private_state->d_mt, 
			    v1_mt_private_state->d_s_r, v1_mt_private_state->d_s_r_q, 
			    v1_mt_private_state->d_s_l, v1_mt_private_state->d_s_l_q, 
			    num_neurons, k);

	// CUDA_GAUSSIAN
	cuda_gaussian_nls (v1_mt_private_state->mt_gaussian, v1_mt_private_state->d_mt_gaussian, v1_mt_private_state->d_mt, 
				v1_mt_private_state->rf_points_x[4], v1_mt_private_state->rf_points_y[4],
				v1_mt_private_state->rf_points_gauss[4], rf_num_points, wo, ho, wi, hi);
}

void
v1_mt (FILTER_DESC *filter_desc)
{
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nl_image_right = NULL, *nl_image_left = NULL;
	int i, j, hi, wi, ho, wo, left_x_center, left_y_center, right_x_center, right_y_center;
	float camera_opening, cycles_per_degree, frequency, width, aspect_ratio, orientation[4], phase[4], log_factor;
	int mapping, shift[4], kernel_type;
	double correction;
	INPUT_DESC *input_left, *input_right;
	float *mt_gaussian;
	float c_delta_area;
	int rf_num_points, num_neurons;
	int dimension;
	int nKernelSize;
	float fltSigma, k;

	V1_MT_PRIVATE_STATE *v1_mt_private_state;

	// Checks the Parameters
	for (i = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, i++)
		;

	if (--i != 22)
	{
		Erro ("Wrong number of parameters for biological_gabor_filter.", "", "");
		return;
	}

	// Gets the Parameters
	camera_opening		= filter_desc->filter_params->next->param.fval;
	cycles_per_degree	= filter_desc->filter_params->next->next->param.fval;
	width			= filter_desc->filter_params->next->next->next->param.fval;
	aspect_ratio		= filter_desc->filter_params->next->next->next->next->param.fval;
	orientation[0]		= filter_desc->filter_params->next->next->next->next->next->param.fval;
	phase[0]		= filter_desc->filter_params->next->next->next->next->next->next->param.fval;
	shift[0]		= filter_desc->filter_params->next->next->next->next->next->next->next->param.ival;
	orientation[1]		= filter_desc->filter_params->next->next->next->next->next->next->next->next->param.fval;
	phase[1]		= filter_desc->filter_params->next->next->next->next->next->next->next->next->next->param.fval;
	shift[1]		= filter_desc->filter_params->next->next->next->next->next->next->next->next->next->next->param.ival;
	orientation[2]		= filter_desc->filter_params->next->next->next->next->next->next->next->next->next->next->next->param.fval;
	phase[2]		= filter_desc->filter_params->next->next->next->next->next->next->next->next->next->next->next->next->param.fval;
	shift[2]		= filter_desc->filter_params->next->next->next->next->next->next->next->next->next->next->next->next->next->param.ival;
	orientation[3]		= filter_desc->filter_params->next->next->next->next->next->next->next->next->next->next->next->next->next->next->param.fval;
	phase[3]		= filter_desc->filter_params->next->next->next->next->next->next->next->next->next->next->next->next->next->next->next->param.fval;
	shift[3]		= filter_desc->filter_params->next->next->next->next->next->next->next->next->next->next->next->next->next->next->next->next->param.ival;
	mapping			= filter_desc->filter_params->next->next->next->next->next->next->next->next->next->next->next->next->next->next->next->next->next->param.ival;
	log_factor		= filter_desc->filter_params->next->next->next->next->next->next->next->next->next->next->next->next->next->next->next->next->next->next->param.fval;
	kernel_type             = filter_desc->filter_params->next->next->next->next->next->next->next->next->next->next->next->next->next->next->next->next->next->next->next->param.ival;

        // Checks the Input Neuron Layers
	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
		;

	if (i != 1 && i != 2 && i !=3)
	{
		Erro ("Wrong number of neuron layers. biological_gabor_filter must be applied on one or two neuron layers.", "", "");
		return;
	}

	// Gets the Input Neuron Layers
	nl_image_right = filter_desc->neuron_layer_list->neuron_layer;
	nl_image_left = filter_desc->neuron_layer_list->next->neuron_layer;

	// Gets Input Neuron Layer Dimentions
	wi = nl_image_right->dimentions.x;
	hi = nl_image_right->dimentions.y;

	dimension = wi*hi;	// Dimension of inputs.

	// Gets Output Neuron Layer Dimentions
	wo = filter_desc->output->dimentions.x;
	ho = filter_desc->output->dimentions.y;

	num_neurons = wo*ho;	// Dimension of outputs.

	correction = (double) ho / (double) (ho - 1);

	input_right = get_input_by_neural_layer (nl_image_right);
	input_left = get_input_by_neural_layer (nl_image_left);

	// Computes Kernels
	if (filter_desc->private_state == NULL)
	{
		v1_mt_private_state = (V1_MT_PRIVATE_STATE *) alloc_mem (sizeof (V1_MT_PRIVATE_STATE));

		v1_mt_private_state->receptive_fields = (RECEPTIVE_FIELD_DESCRIPTION *) alloc_mem ((5) * sizeof (RECEPTIVE_FIELD_DESCRIPTION));

		for (i = 0; i < 4; i++)
		{
			// Calculates the spatial frequency (cycles per pixel)
			frequency = (camera_opening / (float) wi) * cycles_per_degree * pow (2.0f, (float) 0);

			v1_mt_private_state->receptive_fields[i].points = NULL;
			compute_v1_spatial_receptive_field (&(v1_mt_private_state->receptive_fields[i]), frequency, width, aspect_ratio, orientation[i], phase[i], kernel_type);

			// Calculates the minimum value of the result of a gabor filter
			bidimentional_convolution_test_gabor (&(v1_mt_private_state->receptive_fields[i]));
		}

		// Gets the kernel_size & sigma parameters that MT Gaussian Cell calculum needs
		nKernelSize	= filter_desc->filter_params->next->next->next->next->next->next->next->next->next->next->next->next->next->next->next->next->next->next->next->next->next->param.ival;
		fltSigma	= filter_desc->filter_params->next->next->next->next->next->next->next->next->next->next->next->next->next->next->next->next->next->next->next->next->next->next->param.fval;

		compute_gaussian_kernel (&(v1_mt_private_state->receptive_fields[4]), nKernelSize, fltSigma);

		cuda_alloc_device_data_structures (v1_mt_private_state, wi, hi, num_neurons);

		v1_mt_private_state->image_vector_right = (int *) malloc ((wi*hi) * sizeof (int));	// Vetor contendo os valores do neuronio da imagem.
		v1_mt_private_state->image_vector_left  = (int *) malloc ((wi*hi) * sizeof (int));

		// Alocacao dos vetores de dados que serao manipulados pela funcao em CUDA.
		rf_num_points = v1_mt_private_state->receptive_fields[0].num_points;

		v1_mt_private_state->rf_points_w = (float **) malloc (5 * sizeof (float *));
		v1_mt_private_state->rf_points_gabor = (float **) malloc (5 * sizeof (float *));
		v1_mt_private_state->rf_points_gauss = (float **) malloc (5 * sizeof (float *));
		v1_mt_private_state->rf_points_x = (int **) malloc (5 * sizeof (int *));
		v1_mt_private_state->rf_points_y = (int **) malloc (5 * sizeof (int *));

		for (j = 0; j < 5; j++)
		{
			v1_mt_private_state->rf_points_w[j] = (float *) malloc (rf_num_points * sizeof (float));
			v1_mt_private_state->rf_points_gabor[j] = (float *) malloc (rf_num_points * sizeof (float));
			v1_mt_private_state->rf_points_gauss[j] = (float *) malloc (rf_num_points * sizeof (float));
			v1_mt_private_state->rf_points_x[j] = (int *) malloc (rf_num_points * sizeof (int));
			v1_mt_private_state->rf_points_y[j] = (int *) malloc (rf_num_points * sizeof (int));

			for (i = 0; i < rf_num_points; i++)
			{
				v1_mt_private_state->rf_points_w[j][i] = v1_mt_private_state->receptive_fields[j].points[i].weight;
				v1_mt_private_state->rf_points_gabor[j][i] = v1_mt_private_state->receptive_fields[j].points[i].gabor;
				v1_mt_private_state->rf_points_gauss[j][i] = v1_mt_private_state->receptive_fields[j].points[i].gaussian;
				v1_mt_private_state->rf_points_x[j][i] = v1_mt_private_state->receptive_fields[j].points[i].x;
				v1_mt_private_state->rf_points_y[j][i] = v1_mt_private_state->receptive_fields[j].points[i].y;
			}
		}

		v1_mt_private_state->mt_gaussian = (float *) malloc (num_neurons * sizeof (float));

		v1_mt_private_state->xi = (int *) malloc (num_neurons * sizeof (int));
		v1_mt_private_state->yi = (int *) malloc (num_neurons * sizeof (int));

		filter_desc->private_state = (void *) v1_mt_private_state;
	}
	else
		v1_mt_private_state = (V1_MT_PRIVATE_STATE *) filter_desc->private_state;

	left_x_center  = input_left->wxd - input_left->wx;
	left_y_center  = input_left->wyd - input_left->wy;
	right_x_center = input_right->wxd - input_right->wx;
	right_y_center = input_right->wyd - input_right->wy;

	// Copies the input neuron to a simple vector (that will be send to CUDA).
	for (i = 0; i < dimension; i++)
	{
		v1_mt_private_state->image_vector_right[i] = nl_image_right->neuron_vector[i].output.ival;
		v1_mt_private_state->image_vector_left[i] = nl_image_left->neuron_vector[i].output.ival;
	}

	rf_num_points = v1_mt_private_state->receptive_fields[0].num_points;

	// Hackeado violentamente!
	// Motivo: (receptive_field_descriptor->area) nao funciona caso passado, mesmo tendo valor fixo,
	//		atribuído na chamada de COMPUTE_V1_SPATIAL_RECEPTIVE_FIELD
	// Solucao: atribuir manualmente.
	c_delta_area = 1.0f / (float) (rf_num_points);

	// Gets the selectivity adjust parameter that MT Cell calculum needs
	k = filter_desc->filter_params->next->next->next->next->next->next->next->next->next->next->
					next->next->next->next->next->next->next->next->next->next->param.fval;
	
	cuda_calls (v1_mt_private_state, wi, hi, wo, ho, right_x_center, right_y_center, left_x_center, left_y_center,
			correction, log_factor, shift, num_neurons, rf_num_points, c_delta_area, k);

	// Getting back the result of CUDA_V1_MT_FILTER, and converting to the neuron vector of the output neuron layer
	switch (filter_desc->output->output_type)
	{
		case GREYSCALE:
		{
			mt_gaussian = v1_mt_private_state->mt_gaussian;
			for (i = 0; i < num_neurons; i++)
			{
				filter_desc->output->neuron_vector[i].output.ival = (int) mt_gaussian[i];
			}
		}
		break;

		case GREYSCALE_FLOAT:
		{
			mt_gaussian = v1_mt_private_state->mt_gaussian;
			for (i = 0; i < num_neurons; i++)
			{
				filter_desc->output->neuron_vector[i].output.fval = mt_gaussian[i];
			}
		}
		break;
	}
}



/*!
*********************************************************************************
* Function: cuda_sum_filter
* Description: robot_sum_filter implemented in CUDA
* Inputs: monocular_simple_cells
* Output: binocular_simple_cells
*********************************************************************************
*/

void
cuda_sum_filter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list;
	NEURON_LAYER **nl;
	int i;
	int num_neurons, numNL;

	for (numNL = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, numNL++)
		;

	if (filter_desc->private_state == NULL)
	{
		// Counting the neuron layers (must be even number - most precisely two)
		if ((numNL % 2) != 0) 
		{
			Erro ("Wrong number of neuron layers. The robot_simple_cell must be applied on a even number of neuron layers.", "", "");
			return;
		}

		// Verifying the outputs types
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

		// Allocating and setting the input neuron layers in a private area of filter descriptor
		nl = (NEURON_LAYER **) malloc (sizeof (NEURON_LAYER *) * numNL);
		for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
			nl[i] = n_list->neuron_layer;

		filter_desc->private_state = (void *) nl;
	}
	else
	{
		nl = (NEURON_LAYER **) filter_desc->private_state;
	}

	num_neurons = get_num_neurons (filter_desc->output->dimentions);

	// Sending the simple vectors to CUDA_ADD_NLS, implemented in CUDA
	cuda_add_nls ((filter_desc->output->neuron_vector), (nl[0]->neuron_vector), (nl[1]->neuron_vector), num_neurons);
}



/*!
*********************************************************************************
* Function: cuda_complex_cell
* Description: robot_complex_cell implemented in CUDA
* Inputs: monocular_simple_cells & binocular_simple_cells
* Output: complex_cells (monocular & binocular)
*********************************************************************************
*/

void
cuda_complex_cell (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list;
	NEURON_LAYER **nl;
	int i;
	int num_neurons, numNL;

	for (numNL = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, numNL++)
		;

	if (filter_desc->private_state == NULL)
	{
		// Counting the neuron layers (must be even number - most precisely two)
		if ((numNL % 2) != 0) 
		{
			Erro ("Wrong number of neuron layers. The robot_complex_cell must be applied on a even number of neuron layers.", "", "");
			return;
		}

		// Verifying the outputs types
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

		// Allocating and setting the input neuron layers in a private area of filter descriptor
		nl = (NEURON_LAYER **) malloc (sizeof (NEURON_LAYER *) * numNL);
		for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
			nl[i] = n_list->neuron_layer;

		filter_desc->private_state = (void *) nl;
	}
	else
	{
		nl = (NEURON_LAYER **) filter_desc->private_state;
	}

	num_neurons = get_num_neurons (filter_desc->output->dimentions);

	// Sending the simple vectors to CUDA_MULT_NLS, implemented in CUDA
	cuda_mult_nls ((filter_desc->output->neuron_vector), (nl[0]->neuron_vector), (nl[1]->neuron_vector), num_neurons);
}



/*!
*********************************************************************************
* Function: cuda_mt_cell
* Description: robot_mt_cell implemented in CUDA
* Inputs: complex_cells
* Output: mt_cell
*********************************************************************************
*/

void
cuda_mt_cell (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list;
	NEURON_LAYER *nl_complex_left, *nl_complex_right, *nl_complex_binocular;
	PARAM_LIST *p_list;
	int i;
	int num_neurons, numNL, nNumParam;
	float k;

	// Counting the neuron layers (must be even number - most precisely three)
	for (numNL = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, numNL++)
		;

	if (numNL != 3) 
	{
		Erro ("Wrong number of neuron layers. The robot_mt_cell must be applied on three neuron layers.", "", "");
		return;
	}

	// Verifying the outputs types
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

	// Getting the parameters list - must be one (Selectivity)
	for (nNumParam = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, nNumParam++);
		;

	if (--nNumParam != 1)
	{
		Erro ("The robot_sum_filter_normalized must have one parameter: Selectivity.", "", "");
		return;
	}

	// Gets the selectivity adjust parameter that MT Cell calculum needs
	k = filter_desc->filter_params->next->param.fval;

	// Setting the neuron vectors that will be used in MT Cell calculum
	nl_complex_left = filter_desc->neuron_layer_list->neuron_layer;
	nl_complex_right = filter_desc->neuron_layer_list->next->neuron_layer;
	nl_complex_binocular = filter_desc->neuron_layer_list->next->next->neuron_layer;

	num_neurons = get_num_neurons (filter_desc->output->dimentions);

	// Sending the simple vectors to CUDA_DIV_NLS, implemented in CUDA
	cuda_div_nls ((filter_desc->output->neuron_vector), (nl_complex_left->neuron_vector), (nl_complex_right->neuron_vector), (nl_complex_binocular->neuron_vector), num_neurons, k);
}



/*!
*********************************************************************************
* Function: cuda_bigfilter
* Description: robot_sum_filter, robot_complex_cell & robot_mt_cell implemented in CUDA
* Inputs: monocular_simple_cells
* Output: mt_cells
*********************************************************************************
*/

void
cuda_bigfilter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list;
	NEURON_LAYER **nl;
	PARAM_LIST *p_list;
	int num_neurons, numNL, nNumParam, i;
	float k;
	float *s_l, *s_l_q, *s_r, *s_r_q, *mt;

	// Verifying the outputs types
	for (numNL = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, numNL++)
	{
		if (n_list->next != NULL)
		{
			if (n_list->neuron_layer->output_type != n_list->next->neuron_layer->output_type)
			{
				Erro ("The cuda_bigfilter must be applied on neuron layers with the same output_type.", "", "");
				return;
			}
		}
	}

	// Counting the neuron layers (must be even number - most precisely four)
	if ((numNL % 2) != 0) 
	{
		Erro ("Wrong number of neuron layers. The cuda_bigfilter must be applied on a even number of neuron layers.", "", "");
		return;
	}

	// Allocating and setting the input neuron layers in a private area of filter descriptor
	if (filter_desc->private_state == NULL)
	{
		nl = (NEURON_LAYER **) malloc (sizeof (NEURON_LAYER *) * numNL);

		for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
			nl[i] = n_list->neuron_layer;

		filter_desc->private_state = (void *) nl;
	}
	else
	{
		nl = (NEURON_LAYER **) filter_desc->private_state;
	}

	// Getting the parameters list - must be one (Selectivity)
	for (nNumParam = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, nNumParam++)
		;

	if (--nNumParam != 1)
	{
		Erro ("The cuda_bigfilter must have one parameter: Selectivity.", "", "");
		return;
	}

	// Gets the selectivity adjust parameter that MT Cell calculum needs
	k = filter_desc->filter_params->next->param.fval;

	// Gets the dimension of the neuron layers
	num_neurons = get_num_neurons (filter_desc->output->dimentions);

	// Allocating the simple vectors were the neuron vectors will be convert
	s_r   = (float *) malloc (num_neurons * sizeof (float));
	s_r_q = (float *) malloc (num_neurons * sizeof (float));
	s_l   = (float *) malloc (num_neurons * sizeof (float));
	s_l_q = (float *) malloc (num_neurons * sizeof (float));
	mt    = (float *) malloc (num_neurons * sizeof (float));

	// Converting the neuron vectors into simple vectors
	for (i = 0; i < num_neurons; i++)
	{
		s_r[i]   = nl[0]->neuron_vector[i].output.fval;
		s_r_q[i] = nl[1]->neuron_vector[i].output.fval;
		s_l[i]   = nl[2]->neuron_vector[i].output.fval;
		s_l_q[i] = nl[3]->neuron_vector[i].output.fval;
	}

	// Sending the simple vectors to CUDA_BIGFILTER, implemented in CUDA
	cuda_bigfilter_nls (mt, s_r, s_r_q, s_l, s_l_q, num_neurons, k);

	// Getting back the result of CUDA_BIGFILTER, and converting to the neuron vector of the output neuron layer
	for (i = 0; i < num_neurons; i++)
	{
		filter_desc->output->neuron_vector[i].output.fval = mt[i];
	}

	// Cleaning
	free (s_r);
	free (s_r_q);
	free (s_l);
	free (s_l_q);
	free (mt);
}



/*!
*********************************************************************************
* Function: cuda_gaussian_filter
* Description: robot_gaussian_filter implemented in CUDA
* Inputs: mt_cells
* Output: mt_gaussian_cells
*********************************************************************************
*/

void cuda_gaussian_filter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list;
	NEURON_LAYER *nl;
	RECEPTIVE_FIELD_DESCRIPTION *receptive_field;
	PARAM_LIST *p_list;
	int i;
	int wi, hi, wo, ho, nKernelSize;
	float fltSigma;

	float *mt, *mt_gaussian, *rf_points_g;
	int *rf_points_x, *rf_points_y;

	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++);

	if (filter_desc->private_state == NULL)
	{
		// Checking the number of neuron layers (must be only one - MT)
		if (i != 1) 
		{
			Erro ("Wrong number of neuron layers. The robot_gaussian_filter must be applied on only one neuron layer.", "", "");
			return;
		}

		// Checking the type of output
		if (filter_desc->neuron_layer_list->neuron_layer->output_type != filter_desc->output->output_type)
		{
			Erro ("The output type of input neuron layer is different of the robot_gaussian_filter output.", "", "");
			return;
		}

		// Getting the parameters list - must be two (KernelSize e o Sigma)
		for (i = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, i++);
			;

		if (--i != 2) 
		{
			Erro ("Wrong number of parameters. The robot_gaussian_filter have 2 parameters: kernel_size and sigma respectivally.", "", "");
			return;
		}

		nKernelSize	= filter_desc->filter_params->next->param.ival;
		fltSigma	= filter_desc->filter_params->next->next->param.fval;

		// Allocating and setting the RECEPTIVE_FIELD in a private area of filter descriptor
		receptive_field = (RECEPTIVE_FIELD_DESCRIPTION *) malloc (sizeof (RECEPTIVE_FIELD_DESCRIPTION));
		compute_gaussian_kernel (receptive_field, nKernelSize, fltSigma);

		filter_desc->private_state = (void *) receptive_field;
	}

	receptive_field = (RECEPTIVE_FIELD_DESCRIPTION *) filter_desc->private_state;
	nl = filter_desc->neuron_layer_list->neuron_layer;

	// Gets the input/output dimensions
	wo = filter_desc->output->dimentions.x;
	ho = filter_desc->output->dimentions.y;

	wi = nl->dimentions.x;
	hi = nl->dimentions.y;

	// Allocating the simple vectors were the neuron vectors will be convert
	mt = (float *) malloc (wi*hi * sizeof (float));
	mt_gaussian = (float *) malloc (wo*ho * sizeof (float));

	for (i = 0; i < wi*hi; i++)
	{
		mt[i] = nl->neuron_vector[i].output.fval;
	}

	// Allocating the vectors where the points of RECEPTIVE_FIELD are stored
	int num_points = receptive_field->num_points;

	rf_points_x = (int *) malloc (num_points * sizeof (int));
	rf_points_y = (int *) malloc (num_points * sizeof (int));
	rf_points_g = (float *) malloc (num_points * sizeof (float));

	for (i = 0; i < num_points; i++)
	{
		rf_points_g[i] = receptive_field->points[i].gaussian;
		rf_points_x[i] = receptive_field->points[i].x;
		rf_points_y[i] = receptive_field->points[i].y;
	}

	// Sending the simple vectors to CUDA_GAUSSIAN_FILTER, implemented in CUDA
	cuda_gaussian_nls_old (mt_gaussian, mt, rf_points_x, rf_points_y, rf_points_g, num_points, wo, ho, wi, hi);

	// Getting back the result of CUDA_BIGFILTER, and converting to the neuron vector of the output neuron layer
	int size = wo*ho;

	switch (filter_desc->output->output_type)
	{
		case GREYSCALE:
		{
			for (i = 0; i < size; i++)
			{
				filter_desc->output->neuron_vector[i].output.ival = (int) mt_gaussian[i];
			}
		}
		break;

		case GREYSCALE_FLOAT:
		{
			for (i = 0; i < size; i++)
			{
				filter_desc->output->neuron_vector[i].output.fval = mt_gaussian[i];
			}
		}
		break;
	}

	// Cleaning
	free (mt_gaussian);
	free (mt);
	free (rf_points_x);
	free (rf_points_y);
	free (rf_points_g);
}
