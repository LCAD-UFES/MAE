#include "mae.h"
#include "filter.h"
#include "robot_user_functions.h"
#include "robot_filters.h"

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


/*
*********************************************************************************
* Function: initialise_synapse2		          				*
* Description: 			                        			*
* Inputs: 				                  			*
* Output: 				                			*
*********************************************************************************
*/

void initialise_synapse2 (SYNAPSE *synapse, NEURON_LAYER *nl_source, int x, int y, NEURON_OUTPUT sensitivity)
{
     	int w, h;
	
	w = nl_source->dimentions.x;
	h = nl_source->dimentions.y;
	
	if((x >= 0) && (x < w) && (y >= 0) && (y < h))
		synapse->source = &(nl_source->neuron_vector[x + y * w]);
	else
		synapse->source = &(neuron_with_output_zero);
		
	synapse->sensitivity = sensitivity;
	synapse->nl_source = nl_source;
     	synapse->x = x;
     	synapse->y = y;
}



/*
*********************************************************************************
* Function: connect_2_neurons2		          				*
* Description: 			                        			*
* Inputs: 				                  			*
* Output: 				                			*
*********************************************************************************
*/

void connect_2_neurons2 (NEURON_LAYER *nl_source, int x, int y, NEURON *ndest, NEURON_OUTPUT synapse_sensitivity)
{
	SYNAPSE *synapse;
	
	synapse = (SYNAPSE *) alloc_mem ((size_t) sizeof (SYNAPSE));
	/* The synapse is initialized */
	initialise_synapse2 (synapse, nl_source, x, y, synapse_sensitivity);
	add_synapse (ndest, synapse);
	ndest->n_in_connections++;
}



/*
*********************************************************************************
* Function: connect_gaussrand2_neurons2		          			*
* Description: Gaussian random distribution generator         			*
* Inputs: 				                  			*
* Output: 				                			*
*********************************************************************************
*/
 
double gaussrand2(void)
{
        static double V1, V2, S;
        static int phase = 0;
        double X;

        if(phase == 0) {
                do {
                        double U1 = (double) rand() / (double) LRAND48_MAX;
                        double U2 = (double) rand() / (double) LRAND48_MAX;

                        V1 = 2 * U1 - 1;
                        V2 = 2 * U2 - 1;
                        S = V1 * V1 + V2 * V2;
                        } while(S >= 1 || S == 0);

                X = V1 * sqrt(-2 * log(S) / S);
        } else
                X = V2 * sqrt(-2 * log(S) / S);

        phase = 1 - phase;

        return X;
}



/*
*********************************************************************************
* Function: connect_neuron_layers_gaussianly2					*
* Description: 			                        			*
* Inputs: 				                  			*
* Output: 				                			*
*********************************************************************************
*/

void connect_neuron_layers_gaussianly2 (NEURON_LAYER *neuron_layer_src, NEURON_LAYER *neuron_layer_dst,
				  					int nun_inputs_per_neuron, double gaussian_radius) 
{
	int i, j, gx, gy, num_neurons;
	NEURON_OUTPUT synapse_sensitivity;
	SENSITIVITY_TYPE neuron_layer_sensitivity;

	if (neuron_layer_dst->sensitivity == NOT_SPECIFIED)
		neuron_layer_sensitivity = neuron_layer_src->output_type;
	else
		neuron_layer_sensitivity = neuron_layer_dst->sensitivity;

	num_neurons = get_num_neurons(neuron_layer_dst->dimentions);
	
	for (j = 0; j < num_neurons; j++)
	{
		for (i = 0; i < nun_inputs_per_neuron; i++)
		{
			gx = (int)(gaussrand2 () * gaussian_radius + 0.5);
			gy = (int)(gaussrand2 () * gaussian_radius + 0.5);

			switch(neuron_layer_sensitivity)
			{
				case GREYSCALE:
					synapse_sensitivity.ival = rand () % NUM_GREYLEVELS;
					break;
				case BLACK_WHITE:
					synapse_sensitivity.ival = NUM_GREYLEVELS - 1;
					break;
				case GREYSCALE_FLOAT: // Range = [-1.0 : 1.0]
					synapse_sensitivity.fval = ((float) (rand () % 100)) / 50.0 - 1.0;
					break;
				case COLOR:
					synapse_sensitivity.ival = rand () % NUM_COLORS;
					break;	
			}
			
			connect_2_neurons2 (neuron_layer_src, gx, gy, &(neuron_layer_dst->neuron_vector[j]), synapse_sensitivity);
		}
	}
}

/*
*********************************************************************************
* Function: connect_neuron_layers_gaussianly3					*
* Description: 			                        			*
* Inputs: 				                  			*
* Output: 				                			*
*********************************************************************************
*/

void connect_neuron_layers_gaussianly3 (NEURON_LAYER *neuron_layer_src, NEURON_LAYER *neuron_layer_dst,
				  					int nun_inputs_per_neuron, double gaussian_radius, 
									double log_factor) 
{
	int u, v, w, h, xi, yi, wi, hi, x_center, y_center, i, gx, gy, num_neurons;
	NEURON_OUTPUT synapse_sensitivity;
	SENSITIVITY_TYPE neuron_layer_sensitivity;
	double correction;
	
	if (neuron_layer_dst->sensitivity == NOT_SPECIFIED)
		neuron_layer_sensitivity = neuron_layer_src->output_type;
	else
		neuron_layer_sensitivity = neuron_layer_dst->sensitivity;

	num_neurons = get_num_neurons(neuron_layer_dst->dimentions);
	
	// Gets the destination neuron layer dimentions
	w = neuron_layer_dst->dimentions.x;
	h = neuron_layer_dst->dimentions.y;
	
	// Gets the source neuron layer dimentions
	wi = neuron_layer_src->dimentions.x;
	hi = neuron_layer_src->dimentions.y;

	// Calculates the correction factor
	correction = (double) h / (double) (h - 1);
	
	// Calculates the source neuron layer center
	x_center = wi >> 1;
	y_center = hi >> 1;
	
	for (v = 0; v < h; v++)
	{
		for (u = 0; u < w; u++)
		{
			map_v1_to_image (&xi, &yi, wi, hi, u, v, w, h, x_center, y_center, correction, log_factor);

			for (i = 0; i < nun_inputs_per_neuron; i++)
			{
				gx = (int)(gaussrand2 () * gaussian_radius + 0.5) + xi;
				gy = (int)(gaussrand2 () * gaussian_radius + 0.5) + yi;

				switch(neuron_layer_sensitivity)
				{
					case GREYSCALE:
						synapse_sensitivity.ival = rand () % NUM_GREYLEVELS;
						break;
					case BLACK_WHITE:
						synapse_sensitivity.ival = NUM_GREYLEVELS - 1;
						break;
					case GREYSCALE_FLOAT: // Range = [-1.0 : 1.0]
						synapse_sensitivity.fval = ((float) (rand () % 100)) / 50.0 - 1.0;
						break;
					case COLOR:
						synapse_sensitivity.ival = rand () % NUM_COLORS;
						break;	
				}
			
				connect_2_neurons2 (neuron_layer_src, gx, gy, &(neuron_layer_dst->neuron_vector[u + v * w]), synapse_sensitivity);
			}
		}
	}
}


/*
*********************************************************************************
* Function: compute_input_pattern_minchinton_black_white_with_offset  		*
* Description: Compute the input pattern seen by minchinton_black_white neuron	*
* Inputs: 				                  			*
* Output: 				                			*
*********************************************************************************
*/

void compute_input_pattern_minchinton_black_white_with_offset (NEURON *neuron, PATTERN *input_pattern, 
						   SENSITIVITY_TYPE sensitivity, OUTPUT_TYPE output_type,
						   int x_offset, int y_offset)
{
	SYNAPSE_LIST *s_list;
	int num_synapses, i;
	int bit_value = 0;
	NEURON_LAYER *nl_source1 = NULL, *nl_source2 = NULL;
	NEURON *nsrc1 = NULL, *nsrc2 = NULL;
	int x1, y1, w1, h1, x2, y2, w2, h2;
	
	nsrc1 = nsrc2 = &(neuron_with_output_zero);
	
	i = -1;
	num_synapses = 0;
	for (s_list = ((SYNAPSE_LIST *) (neuron->synapses)); s_list != (SYNAPSE_LIST *) NULL; s_list = s_list->next)
	{
		if ((num_synapses % PATTERN_UNIT_SIZE) == 0)
		{
			i++;
			input_pattern[i] = 0;
		}

		input_pattern[i] = input_pattern[i] >> 1;

		if ((nl_source1 = s_list->synapse->nl_source) != NULL)
		{
			w1 = nl_source1->dimentions.x;
			h1 = nl_source1->dimentions.y;
			x1 = s_list->synapse->x + x_offset;
			y1 = s_list->synapse->y + y_offset;
			if ((x1 >= 0) && (x1 < w1) && (y1 >= 0) && (y1 < h1))
				nsrc1 = &(nl_source1->neuron_vector[x1 + w1 * y1]);
		}
			
		if (s_list->next != NULL)
		{
			if ((nl_source2 = s_list->next->synapse->nl_source) != NULL)
			{
				w2 = nl_source2->dimentions.x;
				h2 = nl_source2->dimentions.y;
				x2 = s_list->next->synapse->x + x_offset;
				y2 = s_list->next->synapse->y + y_offset;
				if ((x2 >= 0) && (x2 < w2) && (y2 >= 0) && (y2 < h2))
					nsrc2 = &(nl_source2->neuron_vector[x2 + w2 * y2]);
			}
		}
		else
		{
			if((nl_source2 = ((SYNAPSE_LIST *) (neuron->synapses))->synapse->nl_source) != NULL)
			{
				w2 = nl_source2->dimentions.x;
				h2 = nl_source2->dimentions.y;
				x2 = ((SYNAPSE_LIST *) (neuron->synapses))->synapse->x + x_offset;
				y2 = ((SYNAPSE_LIST *) (neuron->synapses))->synapse->y + y_offset;
				if ((x2 >= 0) && (x2 < w2) && (y2 >= 0) && (y2 < h2))
					nsrc2 = &(nl_source2->neuron_vector[x2 + w2 * y2]);
			}
		}
			
		switch(output_type)
		{
			case GREYSCALE:
			case BLACK_WHITE:
			case COLOR:
				bit_value = (nsrc1->output.ival < nsrc2->output.ival) ? 1 : 0;
				break;
			case GREYSCALE_FLOAT:
				switch (g_nNetworkStatus)
				{
					case TRAINNING:
						bit_value = (nsrc1->output.fval + THRESHOLD < nsrc2->output.fval) ? 1 : 0;
						break;
					case RUNNING:
						bit_value = (nsrc1->output.fval + THRESHOLD < nsrc2->output.fval) ? 1 : 0;
						break;
				}
				break;
		}

		input_pattern[i] |= bit_value << (PATTERN_UNIT_SIZE - 1);
		
		s_list->synapse->source = nsrc1;

		num_synapses++;
	}
	if (num_synapses % PATTERN_UNIT_SIZE)
		input_pattern[i] = input_pattern[i] >> (PATTERN_UNIT_SIZE - (num_synapses % PATTERN_UNIT_SIZE));
}



/*
*********************************************************************************
* Function: evaluate_dendrite_with_offset  					*
* Description: Evaluate dendrite						*
* Inputs: 				                  			*
* Output: 				                			*
*********************************************************************************
*/

NEURON_OUTPUT evaluate_dendrite_with_offset (NEURON_LAYER *neuron_layer, int n, int x_offset, int y_offset)
{
	PATTERN input_pattern [MAX_CONNECTIONS / PATTERN_UNIT_SIZE];
	NEURON_OUTPUT d_state;
	int connections;
	OUTPUT_TYPE output_type;
	NEURON *neuron;
	SENSITIVITY_TYPE neuron_layer_sensitivity;
	NEURON_LAYER *nl_source = NULL;
	int x, y, w, h;

	d_state = neuron_with_output_zero.output;
	
	neuron = &(neuron_layer->neuron_vector[n]);
	connections = neuron->n_in_connections;
	output_type = neuron_layer->output_type;
	
	if ((connections / PATTERN_UNIT_SIZE) > (MAX_CONNECTIONS / PATTERN_UNIT_SIZE))
		Erro ("max number of connections exceeded", "", "");
		 
	if (((SYNAPSE_LIST *) (neuron->synapses)) == NULL)
	{
		d_state.ival = NEVER_COMPUTED;
		return d_state;
	}
		
	if (((SYNAPSE_LIST *) (neuron->synapses))->synapse->sensitivity.ival == DIRECT_CONNECTION)
	{
		if ((nl_source = ((SYNAPSE_LIST *) (neuron->synapses))->synapse->nl_source) != NULL)
		{
			w = nl_source->dimentions.x;
			h = nl_source->dimentions.y;
			x = ((SYNAPSE_LIST *) (neuron->synapses))->synapse->x + x_offset;
			y = ((SYNAPSE_LIST *) (neuron->synapses))->synapse->y + y_offset;
			
			if ((x >= 0) && (x < w) && (y >= 0) && (y < h))
				d_state = nl_source->neuron_vector[x + w * y].output;
		}
		
		switch(output_type)
		{
			case GREYSCALE:
				d_state.ival = ((d_state.ival >= NUM_GREYLEVELS) ? (NUM_GREYLEVELS - 1) : d_state.ival);
				return d_state;
			case BLACK_WHITE:
				d_state.ival = ((d_state.ival >= NUM_GREYLEVELS / 2) ? (NUM_GREYLEVELS - 1) : 0);
				return d_state;
			case GREYSCALE_FLOAT: 
				d_state.fval = ((d_state.fval >= (float) NUM_GREYLEVELS) ? (float) (NUM_GREYLEVELS - 1): d_state.fval);
				return d_state;
			case COLOR:
				d_state.ival = ((d_state.ival >= NUM_COLORS) ? (NUM_COLORS - 1): d_state.ival);
				return d_state;
		}
	}
	
	neuron_layer_sensitivity = neuron_layer->sensitivity;
	
	//(*(neuron_layer->neuron_type->compute_input_pattern)) (neuron, input_pattern, neuron_layer_sensitivity, output_type);
	compute_input_pattern_minchinton_black_white_with_offset (neuron, input_pattern, neuron_layer_sensitivity, output_type, x_offset, y_offset);
	d_state = (*(neuron_layer->neuron_type->neuron_memory_read)) (neuron_layer, n, input_pattern, connections, output_type);
	return d_state;
} 



/*
*********************************************************************************
* Function: train_neuron_with_offset                				*
* Description: 			                        			*
* Inputs: 				                  			*
* Output: 				                			*
*********************************************************************************
*/

void train_neuron_with_offset (NEURON_LAYER *neuron_layer, int n, int x_offset, int y_offset)
{
	PATTERN input_pattern[MAX_CONNECTIONS/PATTERN_UNIT_SIZE];
	int connections;
	SENSITIVITY_TYPE sensitivity;
	OUTPUT_TYPE output_type;
	NEURON *neuron;
	
	neuron = &(neuron_layer->neuron_vector[n]);
	connections = neuron->n_in_connections;
	
	if ((connections/PATTERN_UNIT_SIZE) > (MAX_CONNECTIONS/PATTERN_UNIT_SIZE))
		Erro ("max number of connections exceeded", "", "");

	sensitivity = neuron_layer->sensitivity;
	output_type = neuron_layer->output_type;
	//(*(neuron_layer->neuron_type->compute_input_pattern)) (neuron, input_pattern, sensitivity, output_type);
	compute_input_pattern_minchinton_black_white_with_offset (neuron, input_pattern, sensitivity, output_type, x_offset, y_offset);
	(*(neuron_layer->neuron_type->neuron_memory_write)) (neuron_layer, n, input_pattern, connections, output_type);
}



/*
*********************************************************************************
* Function: mt_filter		                 				*
* Description: 			                        			*
* Inputs: 				                  			*
* Output: 				                			*
*********************************************************************************
*/

void mt_filter (FILTER_DESC *filter_desc)
{
	/*PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nlMT = NULL, *nlImgLeft = NULL, *nlImgRight = NULL, *nlImg = NULL;
	INPUT_DESC *inImgLeft = NULL, *inImgRight;
	int xi, yi, wi, hi, u, v, h, w, x_center, y_center, nl_number, p_number, nInputsPerNeuron, nIndex;
	float fltGaussianRadius, fltLogFactor;
	double correction;
	//static RECEPTIVE_FIELD_DESCRIPTION * receptive_field = NULL;
	
	// Checks the neuron layers number
	for (nl_number = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, nl_number++)
            	;

	if (nl_number != 3)
	//if (nl_number != 2)
	{
		Erro ("Error: Wrong number of input neuron layers. The mt_filter must have two neuron layers: the left and right input layers.", "", "");
		return;
	}
	
	// Checks the parameters number
	for (p_number = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, p_number++)
            	;

	if (p_number != 4)
	{
		Erro ("Error: Wrong number of parameters. The mt_filter must have tree parameter <nInputsPerNeuron> <fltGaussianRadius> <fltLogFactor>.", "", "");
		return;
	}
	
	// Gets the filter parameters
	nInputsPerNeuron  = filter_desc->filter_params->next->param.ival;
	fltGaussianRadius = filter_desc->filter_params->next->next->param.fval;
	fltLogFactor  	  = filter_desc->filter_params->next->next->next->param.fval;
	
	// Gets the filter output 
	nlMT = filter_desc->output;

	// Gets the filter output dimentions
	w = nlMT->dimentions.x;
	h = nlMT->dimentions.y;
	
	// Gets the input neuron layer
	nlImgLeft  = filter_desc->neuron_layer_list->neuron_layer;
	nlImgRight = filter_desc->neuron_layer_list->next->neuron_layer;
	nlImg      = filter_desc->neuron_layer_list->next->next->neuron_layer;

	// Gets the associated inputs
        inImgLeft  = get_input_by_neural_layer (nlImgLeft);
	inImgRight = get_input_by_neural_layer (nlImgRight);
	
	// Gets the inputs dimentions
	wi = nlImgLeft->dimentions.x;
	hi = nlImgLeft->dimentions.y;

	// Calculates the correction factor
	correction = (double) h / (double) (h - 1);
 	
	// Creates the connection pattern
	if (filter_desc->private_state == NULL)
	{	
		//nlImg = (NEURON_LAYER *) alloc_mem (sizeof (NEURON_LAYER));
		//create_neuron_layer (nlImg, NULL, NOT_SPECIFIED, GREYSCALE_FLOAT, wi, hi);
		connect_neuron_layers_gaussianly2 (nlImg, nlMT, nInputsPerNeuron, fltGaussianRadius);
		//connect_neuron_layers_gaussianly2 (nlImg, nlMT, nInputsPerNeuron, 2.0f*fltGaussianRadius);
		//connect_neuron_layers_gaussianly2 (nlImg, nlMT, nInputsPerNeuron, 4.0f*fltGaussianRadius);
		//connect_neuron_layers_gaussianly2 (nlImg, nlMT, nInputsPerNeuron, 8.0f*fltGaussianRadius);
		
		//receptive_field = (RECEPTIVE_FIELD_DESCRIPTION *) alloc_mem (sizeof(RECEPTIVE_FIELD_DESCRIPTION));
		//compute_gaussian_kernel(receptive_field, 5, 3.0f);
		
		filter_desc->private_state = (void *) nlImg;
	}
	//else
		//nlImg = (NEURON_LAYER *) filter_desc->private_state;*/

	/*switch (g_nNetworkStatus)
	{
		case TRAINNING:
			x_center = (inImgRight->wxd - inImgRight->wx);
			y_center = (inImgRight->wyd - inImgRight->wy);
			
			copy_neuron_outputs (nlImg, nlImgRight);
			
			for (v = 0; v < h; v++)
			{
				for (u = 0; u < w; u++)
				{
					map_v1_to_image (&xi, &yi, wi, hi, u, v, w, h, x_center, y_center, correction, fltLogFactor);
					nIndex = u + v * w;
					train_neuron_with_offset (nlMT, nIndex, xi, yi);
				}
			}
			break;
		case RUNNING:
			x_center = (inImgLeft->wxd - inImgLeft->wx);
			y_center = (inImgLeft->wyd - inImgLeft->wy);
			
			copy_neuron_outputs (nlImg, nlImgLeft);

			for (v = 0; v < h; v++)
			{
				for (u = 0; u < w; u++)
				{
					map_v1_to_image (&xi, &yi, wi, hi, u, v, w, h, x_center, y_center, correction, fltLogFactor);
					nIndex = u + v * w;
					nlMT->neuron_vector[nIndex].output = evaluate_dendrite_with_offset (nlMT, nIndex, xi, yi);
				}
			}
			break;
	}*/
				
	/*switch (g_nNetworkStatus)
	{
		case TRAINNING:
			x_center = (inImgLeft->wxd - inImgLeft->wx);
			y_center = (inImgLeft->wyd - inImgLeft->wy);
			
			//for (yi = 0; yi < hi; yi++)
			//	for (xi = 0; xi < wi; xi++)
			//		nlImg->neuron_vector[yi * wi + xi].output.fval = apply_gaussian_kernel(receptive_field, nlImgLeft, xi, yi);

			copy_neuron_outputs (nlImg, nlImgLeft);
			
			for (v = 0; v < h; v++)
			{
				for (u = 0; u < w; u++)
				{
					map_v1_to_image (&xi, &yi, wi, hi, u, v, w, h, x_center, y_center, correction, fltLogFactor);
					nIndex = u + v * w;
					train_neuron_with_offset (nlMT, nIndex, xi, yi);
				}
			}
			break;
		case RUNNING:
			x_center = (inImgRight->wxd - inImgRight->wx);
			y_center = (inImgRight->wyd - inImgRight->wy);
			
			//for (yi = 0; yi < hi; yi++)
			//	for (xi = 0; xi < wi; xi++)
			//		nlImg->neuron_vector[yi * wi + xi].output.fval = apply_gaussian_kernel(receptive_field, nlImgRight, xi, yi);

			copy_neuron_outputs (nlImg, nlImgRight);
			
			//x_center = (inImgLeft->wxd - inImgLeft->wx);
			//y_center = (inImgLeft->wyd - inImgLeft->wy);
			//copy_neuron_outputs (nlImg, nlImgLeft);

			for (v = 0; v < h; v++)
			{
				for (u = 0; u < w; u++)
				{
					map_v1_to_image (&xi, &yi, wi, hi, u, v, w, h, x_center, y_center, correction, fltLogFactor);
					nIndex = u + v * w;
					nlMT->neuron_vector[nIndex].output = evaluate_dendrite_with_offset (nlMT, nIndex, xi, yi);
				}
			}
			break;
	}*/
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nlMT = NULL, *nlImgLeft = NULL, *nlImgRight = NULL, *nlImg = NULL;
	INPUT_DESC *inImgLeft = NULL, *inImgRight;
	int i, num_neurons, xi, yi, wi, hi, xo, yo, wo, ho, x_center, y_center, nl_number, p_number, nInputsPerNeuron;
	float fltGaussianRadius, fltLogFactor;
	static int flag = 0;
	//RECEPTIVE_FIELD_DESCRIPTION * receptive_field = NULL;

	// Checks the neuron layers number
	for (nl_number = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, nl_number++)
            	;

	if (nl_number != 3)
	{
		Erro ("Error: Wrong number of input neuron layers. The mt_filter must have two neuron layers: the left and right input layers.", "", "");
		return;
	}
	
	// Checks the parameters number
	for (p_number = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, p_number++)
            	;

	if (p_number != 4)
	{
		Erro ("Error: Wrong number of parameters. The mt_filter must have tree parameter <nInputsPerNeuron> <fltGaussianRadius> <fltLogFactor>.", "", "");
		return;
	}
	
	// Gets the filter parameters
	nInputsPerNeuron  = filter_desc->filter_params->next->param.ival;
	fltGaussianRadius = filter_desc->filter_params->next->next->param.fval;
	fltLogFactor  	  = filter_desc->filter_params->next->next->next->param.fval;
	
	// Gets the filter output 
	nlMT = filter_desc->output;
	
	// Gets the output neurons number
	num_neurons = get_num_neurons(nlMT->dimentions);

	// Gets the input neuron layers
	nlImgLeft  = filter_desc->neuron_layer_list->neuron_layer;
	nlImgRight = filter_desc->neuron_layer_list->next->neuron_layer;
	nlImg      = filter_desc->neuron_layer_list->next->next->neuron_layer;

	// Gets the associated inputs
        inImgLeft  = get_input_by_neural_layer (nlImgLeft);
	inImgRight = get_input_by_neural_layer (nlImgRight);
	
	// Gets the inputs dimentions
	wi = nlImgLeft->dimentions.x;
	hi = nlImgLeft->dimentions.y;

	// Gets the auxiliary layer dimentions
	wo = nlImg->dimentions.x;
	ho = nlImg->dimentions.y;
	 	
	// Creates the connection pattern
	if (flag == 0)
	{	
		connect_neuron_layers_gaussianly3 (nlImg, nlMT, nInputsPerNeuron, fltGaussianRadius, fltLogFactor);
		flag++;
	}
	
	/*if (filter_desc->private_state == NULL)
	{	
		connect_neuron_layers_gaussianly3 (nlImg, nlMT, nInputsPerNeuron, fltGaussianRadius, fltLogFactor);
		receptive_field = (RECEPTIVE_FIELD_DESCRIPTION *) alloc_mem (sizeof(RECEPTIVE_FIELD_DESCRIPTION));
		compute_gaussian_kernel(receptive_field, 11, 5.0f);
		
		filter_desc->private_state = (void *) receptive_field;
	}
	else
		receptive_field = (RECEPTIVE_FIELD_DESCRIPTION *) filter_desc->private_state;*/

	switch (g_nNetworkStatus)
	{
		case TRAINNING:
			x_center = (inImgLeft->wxd - inImgLeft->wx);
			y_center = (inImgLeft->wyd - inImgLeft->wy);
			
			for (yo = 0; yo < ho; yo++)
			{
				for (xo = 0; xo < wo; xo++)
				{
					xi = xo + x_center - (wi >> 1);
					yi = yo + y_center - (hi >> 1);

					if (xi >= 0 && xi < wi && yi >= 0 && yi < hi)
						nlImg->neuron_vector[yo * wo + xo].output = nlImgLeft->neuron_vector[yi * wi + xi].output;
					else
						nlImg->neuron_vector[yo * wo + xo].output.fval = .0f;
					
					//nlImg->neuron_vector[yo * wo + xo].output.fval = apply_gaussian_kernel(receptive_field, nlImgLeft, xi, yi);
				}
			}
			
			for (i = 0; i < num_neurons; i++)
				train_neuron (nlMT, i);
			break;
		case RUNNING:
			x_center = (inImgRight->wxd - inImgRight->wx);
			y_center = (inImgRight->wyd - inImgRight->wy);
			
			for (yo = 0; yo < ho; yo++)
			{
				for (xo = 0; xo < wo; xo++)
				{
					xi = xo + x_center - (wi >> 1);
					yi = yo + y_center - (hi >> 1);

					if (xi >= 0 && xi < wi && yi >= 0 && yi < hi)
						nlImg->neuron_vector[yo * wo + xo].output = nlImgRight->neuron_vector[yi * wi + xi].output;
					else
						nlImg->neuron_vector[yo * wo + xo].output.fval = .0f;
					
					//nlImg->neuron_vector[yo * wo + xo].output.fval = apply_gaussian_kernel(receptive_field, nlImgRight, xi, yi);
				}
			}
			
			for (i = 0; i < num_neurons; i++)
				nlMT->neuron_vector[i].output = evaluate_dendrite (nlMT, i);
			break;
	}
}

