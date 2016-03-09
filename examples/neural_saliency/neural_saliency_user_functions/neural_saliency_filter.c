#include "neural_saliency_filter.h"

void
translate_nl_filter(FILTER_DESC *filter_desc)
{
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nl_output = NULL, *nl_input = NULL;
	int nl_number, p_number;
	float xi_target_center, yi_target_center;
	float scale_factor;
	int xi, yi, wi, hi, xo, yo, wo, ho;

	// Checks the Neuron Layers Number
	for (nl_number = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, nl_number++)
            	;

	// Checks the Parameters Number
	for (p_number = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, p_number++)
            	;

	if (p_number != 3)
	{
		Erro ("Error: Wrong number of parameters. The rotate_nl_filter must have two parameters <xi_target_center> <yi_target_center>.", "", "");
		return;
	}

	// Gets the Filter Parameters - The Pointers And The Values - Void pointers must be casted
	xi_target_center = *((float *) (filter_desc->filter_params->next->param.pval));
	yi_target_center = *((float *) (filter_desc->filter_params->next->next->param.pval));

	// Gets the Input Neuron Layer
	nl_input = filter_desc->neuron_layer_list->neuron_layer;

	// Gets the Filter Output
	nl_output = filter_desc->output;

	wi = nl_input->dimentions.x;
	hi = nl_input->dimentions.y;

	wo = nl_output->dimentions.x;
	ho = nl_output->dimentions.y;

	scale_factor = 1.0;

	// Parallel translation filter capabilities where OpenMP available
#ifdef	_OPENMP
	#pragma omp parallel for private(yo,yi,xo,xi)
#endif
	for (yo = 0; yo < ho; yo++)
	{
		yi = (int) (scale_factor * ((float) yo - (float) ho / 2.0) + yi_target_center + .5f);

		for (xo = 0; xo < wo; xo++)
		{
			xi = (int) (scale_factor * ((float) xo - (float) wo / 2.0) + xi_target_center + .5f);

			if ((xi >= 0) && (xi < wi) && (yi >= 0) && (yi < hi))
				#ifdef	CUDA_COMPILED
					nl_output->host_neuron_vector[xo + yo * wo].output = nl_input->host_neuron_vector[xi + yi * wi].output;
				#else
					nl_output->neuron_vector[xo + yo * wo].output = nl_input->neuron_vector[xi + yi * wi].output;
				#endif
			else
				#ifdef	CUDA_COMPILED
					nl_output->host_neuron_vector[xo + yo * wo].output.ival = 0;
				#else
					nl_output->neuron_vector[xo + yo * wo].output.ival = 0;
				#endif
		}
	}
}

void scale_nl_filter (FILTER_DESC *filter_desc)
{
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nl_output = NULL, *nl_input = NULL;
	int nl_number, p_number;
	float scale_factor, k;
	int xi, yi, wi, hi, xo, yo, wo, ho;

	// Checks the Neuron Layers Number
	for (nl_number = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, nl_number++)
            	;

	// Checks the Parameters Number
	for (p_number = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, p_number++)
            	;

	if (p_number != 2)
	{
		Erro ("Error: Wrong number of parameters. The scale_nl_filter must have only one parameter <scale_factor>.", "", "");
		return;
	}

	// Gets the Filter Parameters
	scale_factor = filter_desc->filter_params->next->param.fval;

	// Gets the Input Neuron Layer
	nl_input = filter_desc->neuron_layer_list->neuron_layer;

	// Gets the Filter Output
	nl_output = filter_desc->output;

	wi = nl_input->dimentions.x;
	hi = nl_input->dimentions.y;

	wo = nl_output->dimentions.x;
	ho = nl_output->dimentions.y;

	k = 1.0/scale_factor;

	for (yo = 0; yo < ho; yo++)
	{
		yi = (int) (k * (float) yo + .5f);

		for (xo = 0; xo < wo; xo++)
		{
			xi = (int) (k * (float) xo + .5f);

			if ((xi >= 0) && (xi < wi) && (yi >= 0) && (yi < hi))
				nl_output->neuron_vector[xo + yo * wo].output = nl_input->neuron_vector[xi + yi * wi].output;
			else
				nl_output->neuron_vector[xo + yo * wo].output.ival = 0;
		}
	}
}

void swap(NEURON_OUTPUT* a, NEURON_OUTPUT* b) {
  NEURON_OUTPUT tmp;
  tmp = *a;
  *a = *b;
  *b = tmp;
}

void swap_index(int* a, int* b) {
  int tmp;
  tmp = *a;
  *a = *b;
  *b = tmp;
}

int partition(NEURON* vec, int* vec_idx, int left, int right) {
  int i, j;

  i = left;
  for (j = left + 1; j <= right; ++j) {
    if (vec[j].output.ival < vec[left].output.ival) {
      ++i;
      swap(&vec[i].output, &vec[j].output);
      swap_index(&vec_idx[i], &vec_idx[j]);
    }
  }

  swap(&vec[left].output, &vec[i].output);
  swap_index(&vec_idx[left], &vec_idx[i]);

  return i;
}

void quickSort(NEURON* vec, int* vec_idx, int left, int right) {
  int r;

  if (right > left) {
    r = partition(vec, vec_idx, left, right);
    quickSort(vec, vec_idx, left, r - 1);
    quickSort(vec, vec_idx, r + 1, right);
  }
}


void hamming_nl_filter(FILTER_DESC *filter_desc)
{
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nl_output = NULL, *nl_input = NULL;
	int nl_number, p_number;
	float threshold;
	int i, wi, hi, wo, ho;

	// Checks the Neuron Layers Number
	for (nl_number = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, nl_number++)
            	;

	// Checks the Parameters Number
	for (p_number = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, p_number++)
            	;

	if (p_number != 2)
	{
		Erro ("Error: Wrong number of parameters. The scale_nl_filter must have only one parameter <scale_factor>.", "", "");
		return;
	}

	// Gets the Filter Parameters
	threshold = filter_desc->filter_params->next->param.ival;
	// Gets the Input Neuron Layer
	nl_input = filter_desc->neuron_layer_list->neuron_layer;

	// Gets the Filter Output
	nl_output = filter_desc->output;

	wi = nl_input->dimentions.x;
	hi = nl_input->dimentions.y;

	wo = nl_output->dimentions.x;
	ho = nl_output->dimentions.y;

	if(wo != wi || hi != ho)
	{
		Erro ("Error: The input and output neural layers must have the same size.", "", "");
		return;
	}

	NEURON *neuron_input = (NEURON *) malloc (wi * hi * sizeof(NEURON));
	int *ordered_neuron_index = (int *) malloc (wi * hi * sizeof(NEURON));

	for(i = 0; i < wi * hi; i++)
	{
		nl_output->neuron_vector[i].output.ival = 0;
		neuron_input[i] = nl_input->neuron_vector[i];
		ordered_neuron_index[i] = i;
	}

	quickSort(neuron_input, ordered_neuron_index, 0, (wi * hi) - 1);

	for(i = 0; i < threshold; i++)
	{
		nl_output->neuron_vector[ordered_neuron_index[(wi * hi) - 1 - i]].output.ival = 255;
	}
}

void crop_nl_filter (FILTER_DESC *filter_desc)
{
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nl_output = NULL, *nl_input = NULL;
	int nl_number, p_number, origin_x, origin_y;
	int xi, yi, wi, xo, yo, wo, ho;
	int posi, poso;

	// Checks the Neuron Layers Number
	for (nl_number = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, nl_number++)
            	;

	// Checks the Parameters Number
	for (p_number = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, p_number++)
            	;

	if (p_number > 3)
	{
		Erro ("Error: Wrong number of parameters. The crop_nl_filter must have only two parameter <scale_factor>.", "", "");
		return;
	}

	// Gets the Filter Parameters
	origin_x = filter_desc->filter_params->next->param.ival;
	origin_y = filter_desc->filter_params->next->next->param.ival;

	// Gets the Input Neuron Layer
	nl_input = filter_desc->neuron_layer_list->neuron_layer;

	// Gets the Filter Output
	nl_output = filter_desc->output;

	wi = nl_input->dimentions.x;

	wo = nl_output->dimentions.x;
	ho = nl_output->dimentions.y;

	for (yi = origin_y, yo = 0; yi < origin_y + ho; yi++, yo++)
	{
		for (xi = origin_x, xo = 0; xi < origin_x + wo; xi++, xo++)
		{
			poso = xo + yo * wo;
			posi = xi + yi * wi;

			nl_output->neuron_vector[poso].output = nl_input->neuron_vector[posi].output;
		}
	}
}
