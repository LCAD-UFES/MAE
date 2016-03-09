#include "mae.h"
#include "filter.h"
#include "../robot.h"
#include "robot_user_functions.h"
#include "common.h"

/*! 
*********************************************************************************
* \brief Maps the Log-Polar coordinates to the Cartesian coordinates.
* \param[out] xi The input image coordinate.		
* \param[out] yi The input image coordinate.  	  
* \param[in] wi The input image width.  	  
* \param[in] hi The input image height.  	  
* \param[in] xo The output image coordinate.  	  
* \param[in] yo The output image coordinate.  	  
* \param[in] wo The output image width.  	  
* \param[in] ho The output image height.
* \param[in] xi_center The input image center.
* \param[in] yi_center The input image center.
* \param[in] log_factor The log factor.
* \pre .
* \post .
* \return Nothing.
*********************************************************************************
*/

void log_polar_mapping (int *xi, int *yi, int wi, int hi, int u, int v, int wo, int ho, int x_center, int y_center, float log_factor)
{
	int ring, wedge;
	int /*index,*/ rings_number, wedges_number;
	float radius, radius_min, radius_max, theta, k1, k2, k3;
		
	//rings_number = wo;
	//wedges_number = ho;
	
	rings_number = (wo >> 1); // rings_number / 2
	wedges_number = (ho << 1); // 2 * rings_number
	
	radius_max = .5f * (float) wi;
	radius_min = radius_max * exp (-2.0f * M_PI * (float) (rings_number -  1) / (float) wedges_number);

	if (u < wo / 2)
	{
		ring  = (wo >> 1) - u - 1;
		wedge = (ho << 1) - v - 1;
	}
	else
	{
		ring  = u - (wo >> 1);
		wedge = v;
	}
	
	k1 = 2.0f * M_PI / (float) wedges_number;
	k2 = log (log_factor * radius_max / radius_min) / (float) (rings_number - 1);
	k3 = radius_min / log_factor;
	
	theta = k1 * (float) wedge - .5f * M_PI;
	radius = k3 * exp (k2 * (float) ring) - k3;
			
//	*xi = (int) (radius * cos (theta) + 0.5) + x_center;
//	*yi = (int) (radius * sin (theta) + 0.5) + y_center;
	*xi = (int) (radius * cos (theta)) + x_center;
	*yi = (int) (radius * sin (theta)) + y_center;
			
	return;
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

void robot_mt_cell (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list;
	NEURON_LAYER *nl_complex_left, *nl_complex_right, *nl_complex_binocular;
	PARAM_LIST *p_list;
	int i;
        //int j;
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
* \brief The gabor filter with biological parameter specification.
* \param[in] filter_desc The filter descriptor structure.		
* \pre The filter descriptor created and initialized.
* \post The updated output neuron layer.
* \return Nothing.
*********************************************************************************
*/

void biological_gabor_filter_epipolar (FILTER_DESC *filter_desc)
{	
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nl_image = NULL;
	RECEPTIVE_FIELD_DESCRIPTION *receptive_field;
	int i, xi, yi, hi, wi, xo, yo, ho, wo, x_center_right, y_center_right, x_center_left, y_center_left;
	float camera_opening, cycles_per_degree, frequency, width, aspect_ratio, orientation, phase, log_factor;
	double correction;
	int image_side, shift, kernel_type;
	INPUT_DESC *input_right = NULL, *input_left = NULL;
	NEURON_OUTPUT previous_output;
	int previous_xi, previous_yi;
	double p_dblRightPoint[2], p_dblLeftPoint[2], *p_dblAux = NULL, *p_dblWorldPoint = NULL, dblZPlane = 0.0;
	static int flag = 0;
	
	if (flag++ < 4)
		return;
		
	// Checks the Parameters
	for (i = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, i++)
		;

	if (i != 11)
	{
		Erro ("Wrong number of parameters for biological_gabor_filter.", "", "");
		return;
	}

	// Gets the Parameters
	camera_opening	  = filter_desc->filter_params->next->param.fval;
	cycles_per_degree = filter_desc->filter_params->next->next->param.fval;
	width		  = filter_desc->filter_params->next->next->next->param.fval;
	aspect_ratio	  = filter_desc->filter_params->next->next->next->next->param.fval;
	orientation	  = filter_desc->filter_params->next->next->next->next->next->param.fval;
	phase		  = filter_desc->filter_params->next->next->next->next->next->next->param.fval;
	shift		  = filter_desc->filter_params->next->next->next->next->next->next->next->param.ival;
	image_side	  = filter_desc->filter_params->next->next->next->next->next->next->next->next->param.ival;
	log_factor	  = filter_desc->filter_params->next->next->next->next->next->next->next->next->next->param.fval;
	kernel_type       = filter_desc->filter_params->next->next->next->next->next->next->next->next->next->next->param.ival;
	
        // Checks the Input Neuron Layers
	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
		;

	if (i != 1)
	{
		Erro ("Wrong number of neuron layers. biological_gabor_filter must be applied on only one neuron layer.", "", "");
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
	        
	// Computes Kernels
	if (filter_desc->private_state == NULL)
	{
                receptive_field = (RECEPTIVE_FIELD_DESCRIPTION *) alloc_mem (sizeof(RECEPTIVE_FIELD_DESCRIPTION));
                // Calculates the spatial frequency (cycles per pixel)
		frequency = (camera_opening / (float) wi) * cycles_per_degree;
                     
		receptive_field->points = NULL;
		compute_v1_spatial_receptive_field (receptive_field, frequency, width, aspect_ratio, orientation, phase, kernel_type);
		      
		// Calculates the minimum value of the result of a gabor filter
		bidimentional_convolution_test_gabor (receptive_field);
                
                filter_desc->private_state = (void *) receptive_field;
	}
	else
		receptive_field = (RECEPTIVE_FIELD_DESCRIPTION *) filter_desc->private_state;

	previous_xi = -1;
	previous_yi = -1;
	
	correction = (double) ho / (double) (ho - 1);

	if ((input_right = get_input_by_name ("image_right")) == NULL)
	{
		Erro ("Cannot get 'image_right' input","","");
		return;
	}
        
	if ((input_left  = get_input_by_name ("image_left")) == NULL)
	{
		Erro ("Cannot get 'image_left' input","","");
		return;
	}
        
	x_center_right = (input_right->wxd - input_right->wx);
	y_center_right = (input_right->wyd - input_right->wy);

	x_center_left = (input_left->wxd - input_left->wx);
	y_center_left = (input_left->wyd - input_left->wy);
				
	switch (image_side)
	{
		case RIGHT_CAMERA:
		{
			for (i = 0; i < wi * hi; i++)
				nl_sample_right.neuron_vector[i].output.ival = 0;
				//nl_sample_right.neuron_vector[i].output.ival = nl_image->neuron_vector[i].output.ival;

			for (yo = 0; yo < ho; yo++)
			{
				for (xo = 0; xo < wo; xo++)
				{
					//map_v1_to_image (&xi, &yi, wi, hi, xo, yo, wo, ho, x_center_right, y_center_right, correction, log_factor);
					log_polar_mapping (&xi, &yi, wi, hi, xo, yo, wo, ho, x_center_right, y_center_right, log_factor);
					xi += shift;
										
					if ((xi == previous_xi) && (yi == previous_yi))
						filter_desc->output->neuron_vector[yo * wo + xo].output = previous_output;
					else if (xi >= wi || xi < 0 || yi >= hi || yi < 0)
						previous_output.fval = filter_desc->output->neuron_vector[yo * wo + xo].output.fval = .0f;
					else
					{
       						nl_sample_right.neuron_vector[xi + wi * yi].output.ival = 0x0000FF00L;

						previous_output = filter_desc->output->neuron_vector[yo * wo + xo].output = bidimentional_convolution (receptive_field, nl_image->neuron_vector, xi, yi, wi, hi);

						if (((previous_output.fval > .0f) && (previous_output.fval <= receptive_field->min_dog)) ||
							((previous_output.fval < .0f) && (previous_output.fval >= receptive_field->min_dog)))
							previous_output.fval = filter_desc->output->neuron_vector[yo * wo + xo].output.fval = .0f;
                                        }
					
					previous_xi = xi;
					previous_yi = yi;
				}
			}
		}
		break;
		case LEFT_CAMERA:
		{
			for (i = 0; i < wi * hi; i++)
				nl_sample_left.neuron_vector[i].output.ival = 0;
				//nl_sample_left.neuron_vector[i].output.ival = nl_image->neuron_vector[i].output.ival;

			p_dblRightPoint[0] = (double) (image_right.ww - x_center_right - 1);
			p_dblRightPoint[1] = (double) y_center_right;
			
			p_dblLeftPoint[0] = (double) (image_left.ww - x_center_left - 1);
			p_dblLeftPoint[1] = (double) y_center_left;
				
			p_dblWorldPoint = StereoImage2WorldPoint (p_dblRightPoint, p_dblLeftPoint);
			
			dblZPlane = POINT_DISTANCE(p_dblWorldPoint);
			
			for (yo = 0; yo < ho; yo++)
			{
				for (xo = 0; xo < wo; xo++)
				{
					//map_v1_to_image (&xi, &yi, wi, hi, xo, yo, wo, ho, x_center_right, y_center_right, correction, log_factor);
					log_polar_mapping (&xi, &yi, wi, hi, xo, yo, wo, ho, x_center_right, y_center_right, log_factor);
					xi += shift;
					
					p_dblRightPoint[0] = (double) (wi - xi - 1);
					p_dblRightPoint[1] = (double) yi;
					
					p_dblWorldPoint = StereoGetWorldPointAtDistance (RIGHT_CAMERA, p_dblRightPoint, dblZPlane);
					p_dblAux= StereoProjectWorldPoint (LEFT_CAMERA, p_dblWorldPoint);

					xi = wi - (int) (p_dblAux[0] + 0.5) - 1;
					yi = (int) (p_dblAux[1] + 0.5);
								
					if ((xi == previous_xi) && (yi == previous_yi))
						filter_desc->output->neuron_vector[yo * wo + xo].output = previous_output;
					else if (xi >= wi || xi < 0 || yi >= hi || yi < 0)
						previous_output.fval = filter_desc->output->neuron_vector[yo * wo + xo].output.fval = .0f;
					else
					{
						nl_sample_left.neuron_vector[xi + wi * yi].output.ival = 0x0000FF00L;
					
						previous_output = filter_desc->output->neuron_vector[yo * wo + xo].output = bidimentional_convolution (receptive_field, nl_image->neuron_vector, xi, yi, wi, hi);

						if (((previous_output.fval > .0f) && (previous_output.fval <= receptive_field->min_dog)) ||
							((previous_output.fval < .0f) && (previous_output.fval >= receptive_field->min_dog)))
							previous_output.fval = filter_desc->output->neuron_vector[yo * wo + xo].output.fval = .0f;
                                        }
					
					previous_xi = xi;
					previous_yi = yi;
				}
			}
		}
		break;
	}
}
