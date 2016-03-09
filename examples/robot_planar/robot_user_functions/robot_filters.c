#include "mae.h"
#include "filter.h"
#include "robot_user_functions.h"

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
* Function: translate_filter
* Description:
* Inputs:
* Output:
*********************************************************************************
*/
extern int g_nCurrDisparity;

void translate_filter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *nl_list;
	NEURON_LAYER *input_nl;
	int numNL;
	int yo, xo, ho, wo;

	for (numNL = 0, nl_list = filter_desc->neuron_layer_list; nl_list != NULL; nl_list = nl_list->next, numNL++);

	if (numNL != 1)
	{
		Erro ("Wrong number of neuron layers. The translate_filter must be applied on a single neuron layer.", "", "");
		return;
	}

	input_nl = filter_desc->neuron_layer_list->neuron_layer;

	ho = filter_desc->output->dimentions.y;
	wo = filter_desc->output->dimentions.x;

	for (yo = 0; yo < ho; yo++)
	{
		for (xo = 0; xo < wo; xo++)
		{
			int yi = yo;
			int xi = xo + g_nCurrDisparity;
			if (xi < 0 || xi >= wo)
				filter_desc->output->neuron_vector[(yo*wo) + xo].output.ival = 0;
			else
				filter_desc->output->neuron_vector[(yo*wo) + xo].output = input_nl->neuron_vector[(yi*wo) + xi].output;
		}
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


extern int g_nCurrDisparity;

/*!
*********************************************************************************
* Function: robot_mt_cell
* Description:
* Inputs:
* Output:
*********************************************************************************
*/

void robot_mt_cell_planar(FILTER_DESC *filter_desc)
{
	int i;
	NEURON_LAYER_LIST *n_list;
	#define POW2(x)(x*x)

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

	int scale = (i == 1); // Flag de escalamento / transla��o
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

	struct timeval start;
	struct timeval end;
	double start_time, end_time;
	gettimeofday(&start,NULL);

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

	printf("%d\n",kernel_size);
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

	gettimeofday(&end,NULL);
	start_time = (double) start.tv_sec + (double) start.tv_usec*1e-6;
	end_time   = (double) end.tv_sec   + (double) end.tv_usec*1e-6;
	printf("Elapsed time: %2.5lfseg (robot_gaussian_filter).\n", end_time - start_time);
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

	struct timeval start;
	struct timeval end;
	double start_time, end_time;
	gettimeofday(&start,NULL);

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

	gettimeofday(&end,NULL);
	start_time = (double) start.tv_sec + (double) start.tv_usec*1e-6;
	end_time   = (double) end.tv_sec   + (double) end.tv_usec*1e-6;
	printf("Elapsed time: %2.5lfseg (robot_gaussian_filter).\n", end_time - start_time);

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
