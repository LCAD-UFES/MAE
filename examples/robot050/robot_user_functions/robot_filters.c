#include "mae.h"
#include "filter.h"
#include "robot_user_functions.h"

extern int g_Arrows, g_Arrow_Grid;
extern float g_Threshold;




/*!
*********************************************************************************
* Function: new_map_v1_to_image							*
* Description: Compute image's xi and yi using v1 v and u			*
* Inputs:									*
* Output: 						 			*
*********************************************************************************
*/

void new_map_v1_to_image (int *xi, int *yi, int wi, int hi, int u, int v, int w, int h, int x_center, int y_center, double log_factor)
{
	static int previous_u = -1;
	static double previous_d;
	double d, theta;

	if (u < w/2)
	{
		if (u == previous_u)
			d = previous_d;
		else
			d = LOG_POLAR_SCALE_FACTOR * distance_from_image_center (wi, hi, w, h, (w-1)/2 - u, log_factor);

		theta = pi * (((double) h * (1.5) - ((double) v + 0.5)) / (double) h) + LOG_POLAR_THETA_CORRECTION;

	}
	else
	{
		if (u == previous_u)
			d = previous_d;
		else
			d = LOG_POLAR_SCALE_FACTOR * distance_from_image_center (wi, hi, w, h, u - w/2, log_factor);

		theta = pi * (((double) h * (1.5) + ((double) v + 0.5)) / (double) h) + LOG_POLAR_THETA_CORRECTION;

	}

	*xi = (int) (d * cos(theta) + 0.5) + x_center;
	*yi = (int) (d * sin(theta) + 0.5) + y_center;

	previous_u = u;
	previous_d = d;
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
			new_map_v1_to_image (&xi, &yi, wi, hi, u, v, w, h, x_center, y_center, fltLogFactor);

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
	float dif_accumulator, a, b, denom;
	float k = 0.0;

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
* Function: robot_minus_filter
* Description: 
* Inputs:
* Output:
*********************************************************************************
*/

void robot_minus_filter (FILTER_DESC *filter_desc)
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
			Erro ("Wrong number of neuron layers. The robot_minus_filter must be applied on a even number of neuron layers.", "", "");
			return;
		}
	
		for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
		{
			if (n_list->next != NULL)
			{
				if (n_list->neuron_layer->output_type != n_list->next->neuron_layer->output_type)
				{
					Erro ("The robot_minus_filter must be applied on neuron layers with the same output_type.", "", "");
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
			dif_accumulator += (a - b);
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
	int i;
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
* Function: robot_gaussian_filter
* Description: Applies a gaussian kernel on a neuron layer
* Inputs:
* Output:
*********************************************************************************
*/

void robot_gaussian_filter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list;
	NEURON_LAYER *nl;
	RECEPTIVE_FIELD_DESCRIPTION *receptive_field;
	PARAM_LIST *p_list;
	int i, j;
	int wo, ho, nKernelSize;
	float fltSigma;

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
}


/*!
***********************************************************************************
* Function: robot_map_filter
* Description: The purpose of this filter declaration is only to relate 
*	       a map neuron layer to its preceeding neuron layers
* Inputs:
* Output:
***********************************************************************************
*/

void robot_map_filter (FILTER_DESC *filter_desc)
{

}


/*!
*********************************************************************************
* Function: new_map_image_to_v1									
* Description: Compute v1's u and v using image's xi and yi				
* Inputs:												
* Output: 						 			
*********************************************************************************
*/

void new_map_image_to_v1 (int xi, int yi, int wi, int hi, int *u, int *v, int w, int h, int x_center, int y_center, double log_factor)
{
	double d, theta;

	d = sqrt((double) (xi - x_center)*(xi - x_center) + (yi - y_center)*(yi - y_center));

	if (xi == x_center)
		theta = ((yi > y_center) ? (0.499999 * pi) : (yi == y_center) ? 0.0 : (- 0.499999 * pi));
	else
		theta = atan((double) (yi - y_center) / (double) (xi - x_center));

	*u = (int) ( (double) (w/2) *  ( 1.0 + log(((d / LOG_POLAR_SCALE_FACTOR) / (double) (wi/2) * (log_factor - 1.0) / log_factor ) + (1.0 / log_factor) ) / log(log_factor) ) + 0.5);
	
	*v = (int) ( (double) h * ((theta - LOG_POLAR_THETA_CORRECTION) / pi + 0.5) );

	if (xi < x_center)
	{
		*u = (w-1)/2 - *u;
		*v = (h-1) - *v;
	}
	else
		*u = *u + w/2;

}



/*!
*********************************************************************************
* Function: robot_motion_filter
* Description: Generates a motion image by the original image and the motion maps
* Inputs:
* Output:
*********************************************************************************
*/

void robot_motion_filter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list;
	NEURON_LAYER *nl_out, *nl_img, *nl_mtmap, *nl_disp, *nl_vdisp;
	PARAM_LIST *p_list;
	int i, u, v, h, w, hi, wi, xi, yi;
	int x_center, y_center, x_motion, y_motion;
	INPUT_DESC *input;
	float fltLogFactor, fltThreshold;
	float hAux;

	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++);

	if (i != 4) 
		Erro ("Wrong number of neuron layers. The robot_motion_filter must be applied on an input image "
			"and three motion map neuron layers.", "", "");
	
	if (filter_desc->neuron_layer_list->neuron_layer->output_type != filter_desc->output->output_type)
		Erro ("The output type of input image is different of the robot_motion_filter output.", "", "");

	// Buscar os parametros
	for (i = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, i++);
	i--;

	if (i != 2) 
		Erro ("Wrong number of parameters. The robot_motion_filter has two parameters: log_factor and threshold.", "", "");

	fltLogFactor = filter_desc->filter_params->next->param.fval;
	fltThreshold = filter_desc->filter_params->next->next->param.fval;

	nl_out   = filter_desc->output;
	nl_img   = filter_desc->neuron_layer_list->neuron_layer;
	nl_mtmap = filter_desc->neuron_layer_list->next->neuron_layer;
	nl_disp  = filter_desc->neuron_layer_list->next->next->neuron_layer;
	nl_vdisp = filter_desc->neuron_layer_list->next->next->next->neuron_layer;

	wi = nl_out->dimentions.x;
	hi = nl_out->dimentions.y;
	w  = nl_mtmap->dimentions.x;
	h  = nl_mtmap->dimentions.y;
	hAux = (float) h / (float) (h - 1.0);

	input = get_input_by_neural_layer (nl_img);

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

	for (xi = 0; xi < wi; xi++)
	{
		for (yi = 0; yi < hi; yi++)
		{
			nl_out->neuron_vector[(yi * wi) + xi].output.ival = PIXEL(0, 0, 63);
		}
	}
	
	for (u = 0; u < w; u++)
	{
		for (v = 0; v < h; v++)
		{
			new_map_v1_to_image (&xi, &yi, wi, hi, u, v, w, h, x_center, y_center, fltLogFactor);

			if (xi >= wi || xi < 0 || yi >= hi || yi < 0)
				continue;

			x_motion = xi + (int) nl_disp->neuron_vector[(v * w) + u].output.fval;
			y_motion = yi + (int) nl_vdisp->neuron_vector[(v * w) + u].output.fval;

			if ((x_motion < 0) || (x_motion >= wi) || (y_motion < 0) || (y_motion >= hi) ||
			    (nl_mtmap->neuron_vector[(v * w) + u].output.fval > fltThreshold))
			    	continue;

			if (nl_out->neuron_vector[(yi * wi) + xi].output.ival == PIXEL(0, 0, 63))
			{
				if (nl_disp->neuron_vector[(v * w) + u].output.fval < 0.0)
				{
					nl_out->neuron_vector[(yi * wi) + xi].output.ival = PIXEL(255, 0, 0);
					if (nl_out->neuron_vector[(y_motion * wi) + x_motion].output.ival == PIXEL(0, 0, 63))
						nl_out->neuron_vector[(y_motion * wi) + x_motion].output.ival = PIXEL(0, 0, 255);
				}
				else if (nl_disp->neuron_vector[(v * w) + u].output.fval > 0.0)
				{
					nl_out->neuron_vector[(yi * wi) + xi].output.ival = PIXEL(0, 255, 0);
					if (nl_out->neuron_vector[(y_motion * wi) + x_motion].output.ival == PIXEL(0, 0, 63))
						nl_out->neuron_vector[(y_motion * wi) + x_motion].output.ival = PIXEL(0, 0, 255);
				}
				else
					nl_out->neuron_vector[(yi * wi) + xi].output.ival = PIXEL(127, 127, 127);
			}
		}
	}
}


void draw_arrow (NEURON_LAYER *nl_out, int xi, int yi, int wi, int hi, int x_motion, int y_motion)
{
	int x_min, y_min, x_max, y_max, nPixels, i, x, y;
	float flt_x, flt_y, delta_x, delta_y;

	if ((x_motion < 0) || (x_motion >= wi) || (y_motion < 0) || (y_motion >= hi))
	    	return;

	x_min = (xi > x_motion ? x_motion : xi);
	y_min = (yi > y_motion ? y_motion : yi);
	x_max = (xi > x_motion ? xi : x_motion);
	y_max = (yi > y_motion ? yi : y_motion);
	nPixels = ((x_max - x_min) > (y_max - y_min) ? (x_max - x_min) : (y_max - y_min));
	if (nPixels)
	{
		delta_x = (float) (x_motion - xi) / (float) nPixels;
		delta_y = (float) (y_motion - yi) / (float) nPixels;
			
		for (i = 0, flt_x = (float) xi, flt_y = (float) yi; i < nPixels; i++)
		{
			flt_x += delta_x;
			flt_y += delta_y;
			x = (int) (flt_x + 0.5);
			y = (int) (flt_y + 0.5);
			nl_out->neuron_vector[y * wi + x].output.ival = PIXEL(0, 255, 0);
		}
	}
	nl_out->neuron_vector[y_motion * wi + x_motion].output.ival = PIXEL(255, 255, 0);
	if ((((xi > x_motion) && (yi == y_motion)) || ((xi == x_motion) && (yi > y_motion))) &&
	    ((x_motion + 1) < wi) && ((y_motion + 1) < hi))
		nl_out->neuron_vector[((y_motion + 1) * wi) + (x_motion + 1)].output.ival = PIXEL(255, 255, 0);
	if ((((xi < x_motion) && (yi == y_motion)) || ((xi == x_motion) && (yi > y_motion))) &&
	    ((x_motion - 1) >= 0) && ((y_motion + 1) < hi))
		nl_out->neuron_vector[((y_motion + 1) * wi) + (x_motion - 1)].output.ival = PIXEL(255, 255, 0);
	if ((((xi < x_motion) && (yi == y_motion)) || ((xi == x_motion) && (yi < y_motion))) &&
	    ((x_motion - 1) >= 0) && ((y_motion - 1) >= 0))
		nl_out->neuron_vector[((y_motion - 1) * wi) + (x_motion - 1)].output.ival = PIXEL(255, 255, 0);
	if ((((xi > x_motion) && (yi == y_motion)) || ((xi == x_motion) && (yi < y_motion))) &&
	    ((x_motion + 1) < wi) && ((y_motion - 1) >= 0))
		nl_out->neuron_vector[((y_motion - 1) * wi) + (x_motion + 1)].output.ival = PIXEL(255, 255, 0);
	if ((((xi > x_motion) && (yi > y_motion)) || ((xi > x_motion) && (yi < y_motion))) &&
	    ((x_motion + 1) < wi))
		nl_out->neuron_vector[(y_motion * wi) + (x_motion + 1)].output.ival = PIXEL(255, 255, 0);
	if ((((xi < x_motion) && (yi > y_motion)) || ((xi < x_motion) && (yi < y_motion))) &&
	    ((x_motion - 1) >= 0))
		nl_out->neuron_vector[(y_motion * wi) + (x_motion - 1)].output.ival = PIXEL(255, 255, 0);
	if ((((xi < x_motion) && (yi < y_motion)) || ((xi > x_motion) && (yi < y_motion))) &&
	    ((y_motion - 1) >= 0))
		nl_out->neuron_vector[((y_motion - 1) * wi) + x_motion].output.ival = PIXEL(255, 255, 0);
	if ((((xi < x_motion) && (yi > y_motion)) || ((xi > x_motion) && (yi > y_motion))) &&
	    ((y_motion + 1) < hi))
		nl_out->neuron_vector[((y_motion + 1) * wi) + x_motion].output.ival = PIXEL(255, 255, 0);
}


/*!
*********************************************************************************
* Function: robot_motion_area_filter
* Description: Generates a motion area image by the original image and the motion maps
* Inputs:
* Output:
*********************************************************************************
*/

#define UV_MIN_MAX(xA,yA)	{												\
					new_map_image_to_v1 (xA, yA, wi, hi, &u, &v, w, h, x_center, y_center, fltLogFactor);	\
					u_min = (u < u_min) ? u : u_min;							\
					u_max = (u > u_max) ? u : u_max;							\
					v_min = (v < v_min) ? v : v_min;							\
					v_max = (v > v_max) ? v : v_max;							\
				}

void robot_motion_area_filter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list;
	NEURON_LAYER *nl_out, *nl_img, *nl_mtg, *nl_mtmap, *nl_disp, *nl_vdisp;
	PARAM_LIST *p_list;
	int i, u, v, h, w, hi, wi, xi, yi;
	int x_center, y_center;
	int x_motion, y_motion;
	int accum_x, accum_y, count, x_med, y_med, x_max, y_max, u_min, u_max, v_min, v_max;
	float out_min;
	INPUT_DESC *input;
	OUTPUT_DESC *output;
	float fltLogFactor, fltThreshold;

	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++);

	if (i != 5) 
		Erro ("Wrong number of neuron layers. The robot_motion_area_filter must be applied on an input image "
			"and four motion map neuron layers.", "", "");
	
	if (filter_desc->neuron_layer_list->neuron_layer->output_type != filter_desc->output->output_type)
		Erro ("The output type of input image is different of the robot_motion_area_filter output.", "", "");

	// Buscar os parametros
	for (i = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, i++);
	i--;

	if (i != 2) 
		Erro ("Wrong number of parameters. The robot_motion_area_filter has two parameters: log_factor and threshold.", "", "");

	fltLogFactor = filter_desc->filter_params->next->param.fval;
	fltThreshold = filter_desc->filter_params->next->next->param.fval;

	nl_out   = filter_desc->output;
	nl_img   = filter_desc->neuron_layer_list->neuron_layer;
	nl_mtg   = filter_desc->neuron_layer_list->next->neuron_layer;
	nl_mtmap = filter_desc->neuron_layer_list->next->next->neuron_layer;
	nl_disp  = filter_desc->neuron_layer_list->next->next->next->neuron_layer;
	nl_vdisp = filter_desc->neuron_layer_list->next->next->next->next->neuron_layer;

	wi = nl_out->dimentions.x;
	hi = nl_out->dimentions.y;
	w  = nl_mtg->dimentions.x;
	h  = nl_mtg->dimentions.y;

	input = get_input_by_neural_layer (nl_img);
	output = get_output_by_neural_layer (nl_out);

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

	if ((output) && (move_active == 1) &&
	    (output->mouse_button == GLUT_LEFT_BUTTON) &&
	    (output->mouse_state == GLUT_DOWN))
	{
		xi = output->wxd;
		yi = output->wyd;
		new_map_image_to_v1 (xi, yi, wi, hi, &u, &v, w, h, x_center, y_center, fltLogFactor);

		if (u >= w || u < 0 || v >= h || v < 0)
			return;
		if (xi >= wi || xi < 0 || yi >= hi || yi < 0 ||
		    (nl_mtmap->neuron_vector[(v * w) + u].output.fval > g_Threshold))
			return;

		x_motion = xi + (int) nl_disp->neuron_vector[(v * w) + u].output.fval;
		y_motion = yi + (int) nl_vdisp->neuron_vector[(v * w) + u].output.fval;
		draw_arrow (nl_out, xi, yi, wi, hi, x_motion, y_motion);
		printf("(%d, %d) : Vx=%d , Vy=%d\n", xi, yi, x_motion - xi, y_motion - yi);
		output->mouse_button = -1;
		return;	
	}

	for (xi = 0; xi < wi; xi++)
	{
		for (yi = 0; yi < hi; yi++)
		{
			nl_out->neuron_vector[(yi * wi) + xi].output.ival = PIXEL(0, 0, 63);

			new_map_image_to_v1 (xi, yi, wi, hi, &u, &v, w, h, x_center, y_center, fltLogFactor);

			if (u >= w || u < 0 || v >= h || v < 0 ||
			    nl_mtg->neuron_vector[(v * w) + u].output.fval < fltThreshold)
				continue;

			nl_out->neuron_vector[(yi * wi) + xi].output = input->neuron_layer->neuron_vector[(yi * wi) + xi].output;
		}
	}

	for (u = 0; u < w; u++)
	{
		for (v = 0; v < h; v++)
		{
			if (nl_mtg->neuron_vector[(v * w) + u].output.fval < fltThreshold)
				continue;

			new_map_v1_to_image (&xi, &yi, wi, hi, u, v, w, h, x_center, y_center, fltLogFactor);

			if (xi >= wi || xi < 0 || yi >= hi || yi < 0 ||
			    (nl_mtmap->neuron_vector[(v * w) + u].output.fval > g_Threshold))
				continue;

			nl_out->neuron_vector[(yi * wi) + xi].output = input->neuron_layer->neuron_vector[(yi * wi) + xi].output;

			x_motion = xi + (int) nl_disp->neuron_vector[(v * w) + u].output.fval;
			y_motion = yi + (int) nl_vdisp->neuron_vector[(v * w) + u].output.fval;

			if (g_Arrows == 1)
				draw_arrow (nl_out, xi, yi, wi, hi, x_motion, y_motion);
		}
	}
	
	if (g_Arrows == 2)
	{
		for (xi = 0; xi < wi; xi += g_Arrow_Grid)
		{
			x_med = xi + g_Arrow_Grid/2;
			x_max = xi + g_Arrow_Grid;
			if (x_med >= wi)
				break;
			for (yi = 0; yi < hi; yi += g_Arrow_Grid)
			{
				y_med = yi + g_Arrow_Grid/2;
				y_max = yi + g_Arrow_Grid;
				if (y_med >= hi)
					break;
				new_map_image_to_v1 (x_med, y_med, wi, hi, &u, &v, w, h, x_center, y_center, fltLogFactor);
				if (u >= w || u < 0 || v >= h || v < 0 ||
				    nl_mtg->neuron_vector[(v * w) + u].output.fval < fltThreshold)
					continue;
				u_min = w;
				v_min = h;
				u_max = v_max = 0;
				UV_MIN_MAX(xi, yi);
				UV_MIN_MAX(xi, y_max);
				UV_MIN_MAX(x_max, yi);
				UV_MIN_MAX(x_max, y_max);
				u_max = (u_max >= w) ? (w-1) : u_max;
				v_max = (v_max >= h) ? (h-1) : v_max;
				if ((x_center >= xi) && (x_center <= x_max))
				{
					if ((y_center >= yi) && (y_center <= y_max))
					{
						u_min = v_min = 0;
						v_max = (h-1);
					}
					else if (y_center < yi)
					{
						new_map_image_to_v1 (x_center, yi, wi, hi, &u_min, &v, w, h, x_center, y_center, fltLogFactor);
						v_max = (h-1);
					}
					else if (y_center > y_max)
					{
						new_map_image_to_v1 (x_center, y_max, wi, hi, &u_min, &v, w, h, x_center, y_center, fltLogFactor);
						v_min = 0;
					}
				}
				if ((y_center >= yi) && (y_center <= y_max))
				{
					if (x_center < xi)
					{
						new_map_image_to_v1 (xi, y_center, wi, hi, &u_min, &v, w, h, x_center, y_center, fltLogFactor);
					}
					else if (x_center > x_max)
					{
						new_map_image_to_v1 (x_max, y_center, wi, hi, &u_min, &v, w, h, x_center, y_center, fltLogFactor);
					}
				}
				
				accum_x = accum_y = count = 0;
				out_min = FLT_MAX;
				for (u = u_min; u <= u_max; u++)
				{
					for (v = v_min; v <= v_max; v++)
					{
						if (nl_mtg->neuron_vector[(v * w) + u].output.fval < fltThreshold)
							continue;

						new_map_v1_to_image (&xi, &yi, wi, hi, u, v, w, h, x_center, y_center, fltLogFactor);

						if (xi >= wi || xi < 0 || yi >= hi || yi < 0 ||
						    (nl_mtmap->neuron_vector[(v * w) + u].output.fval > g_Threshold))
							continue;

						if (ARROW_MEAN)
						{
							accum_x += (int) nl_disp->neuron_vector[(v * w) + u].output.fval;
							accum_y += (int) nl_vdisp->neuron_vector[(v * w) + u].output.fval;
							count++;
						}
						else if (nl_mtmap->neuron_vector[(v * w) + u].output.fval < out_min)
						{
							out_min = nl_mtmap->neuron_vector[(v * w) + u].output.fval;
							accum_x = (int) nl_disp->neuron_vector[(v * w) + u].output.fval;
							accum_y = (int) nl_vdisp->neuron_vector[(v * w) + u].output.fval;
							count = 1;
						}
					}
				}
				if (count)
				{
					x_motion = x_med + (int) ((float) accum_x / (float) count + 0.5);
					y_motion = y_med + (int) ((float) accum_y / (float) count + 0.5);
					draw_arrow (nl_out, x_med, y_med, wi, hi, x_motion, y_motion);
				}
			}
		}
	}	
}


// ----------------------------------------------------------------------------
// reverse_log_polar_filter -
//
// Entrada: filter_desc - Descritor do filtro
//
// Saida: Nenhuma
// ----------------------------------------------------------------------------
void reverse_log_polar_filter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list;
	NEURON_LAYER *n_l;
	PARAM_LIST *p_list;
	int i, u, v, h, w, hi, wi, xi, yi, previous_xi, previous_yi;
	int x_center, y_center;
	INPUT_DESC *input;
	NEURON_OUTPUT previous_output;
	float fltLogFactor;
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

	for (v = 0; v < h; v++)
		for (u = 0; u < w; u++)
			filter_desc->output->neuron_vector[(v * w) + u].output.ival = 0;

	for (xi = 0; xi < wi; xi++)
	{
		for (yi = 0; yi < hi; yi++)
		{
			new_map_image_to_v1 (xi, yi, wi, hi, &u, &v, w, h, x_center, y_center, fltLogFactor);

			if (u >= w || u < 0 || v >= h || v < 0)
				continue;

			filter_desc->output->neuron_vector[(v * w) + u].output = input->neuron_layer->neuron_vector[yi * wi + xi].output;
		}
	}
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
	float L, L_up, L_down, L_left, L_right;
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
