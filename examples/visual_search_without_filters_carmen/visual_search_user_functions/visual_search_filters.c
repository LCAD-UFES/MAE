#include "visual_search_filters.h"

/* sigle Neural Layer translation function, obtained in face_recog_planar application */

/*
*********************************************************************************
* Function: translate_nl_filter	             	 				*
* Description: 			                        			*
* Inputs: 				                  			*
* Output: 				                			*
*********************************************************************************
*/

void translate_nl_filter (FILTER_DESC *filter_desc)
{
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nl_output = NULL, *nl_input = NULL;
	int nl_number, p_number;
	float x_offset, y_offset;
	int xi, yi, wi, hi, xo, yo, wo, ho;

	// Checks the Neuron Layers Number
	for (nl_number = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, nl_number++)
            	;

	// Checks the Parameters Number
	for (p_number = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, p_number++)
            	;

	if (p_number != 3)
	{
		Erro ("Error: Wrong number of parameters. The rotate_nl_filter must have two parameters <x_offset> <y_offset>.", "", "");
		return;
	}
	
	// Gets the Filter Parameters - The Pointers And The Values - Void pointers must be casted
	x_offset = *((float*) (filter_desc->filter_params->next->param.pval));
	y_offset = *((float*) (filter_desc->filter_params->next->next->param.pval));

	// Gets the Input Neuron Layer
	nl_input = filter_desc->neuron_layer_list->neuron_layer;

	// Gets the Filter Output 
	nl_output = filter_desc->output;
	
	wi = nl_input->dimentions.x;
	hi = nl_input->dimentions.y;

	wo = nl_output->dimentions.x;
	ho = nl_output->dimentions.y;

	// Parallel translation filter capabilities where OpenMP available
#ifdef	_OPENMP
	#pragma omp parallel for private(yo,yi,xo,xi)
#endif	
	for (yo = 0; yo < ho; yo++)
	{			
		yi = (int) ((float) yo + y_offset + .5f);
		
		for (xo = 0; xo < wo; xo++)
		{
			xi = (int) ((float) xo + x_offset + .5f);
			
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

