#include "visual_search_filters.h"

extern int g_flip_horizontaly;
extern int g_flip_verticaly;

/* sigle Neural Layer translation function, obtained in face_recog_planar application */

/*
*********************************************************************************
* Function: translate_nl_filter	             	 				*
* Description: 			                        			*
* Inputs: 				                  			*
* Output: 				                			*
*********************************************************************************
*/

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
	
	scale_factor = 1.0 / g_scale_factor;

	// Parallel translation filter capabilities where OpenMP available
#ifdef	_OPENMP
	#pragma omp parallel for private(yo,yi,xo,xi)
#endif	
	for (yo = 0; yo < ho; yo++)
	{			
		if (g_flip_verticaly)
			yi = (int) (scale_factor * ((float) -yo + (float) ho / 2.0) + yi_target_center + .5f);
		else
			yi = (int) (scale_factor * ((float) yo - (float) ho / 2.0) + yi_target_center + .5f);
		
		for (xo = 0; xo < wo; xo++)
		{
			if (g_flip_horizontaly)
				xi = (int) (scale_factor * ((float) -xo + (float) wo / 2.0) + xi_target_center + .5f);
			else
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


/*
*********************************************************************************
* Function: translate_nl_filter	             	 				*
* Description: 			                        			*
* Inputs: 				                  			*
* Output: 				                			*
*********************************************************************************
*/

void 
color_filter(FILTER_DESC *filter_desc)
{
	NEURON_LAYER *nl_output = NULL, *nl_input = NULL;
	int x, y, w, h;
	int r, g, b, intensity;

	// Gets the Input Neuron Layer
	nl_input = filter_desc->neuron_layer_list->neuron_layer;

	// Gets the Filter Output 
	nl_output = filter_desc->output;
	
	w = nl_output->dimentions.x;
	h = nl_output->dimentions.y;
	
	for (y = 0; y < h; y++)
	{			
		y = y;
		
		for (x = 0; x < w; x++)
		{
			x = x;

			r = RED(nl_input->neuron_vector[x + y * w].output.ival);
			g = GREEN(nl_input->neuron_vector[x + y * w].output.ival);
			b = BLUE(nl_input->neuron_vector[x + y * w].output.ival);
			intensity = r - (g + b) / 2;
			if (intensity < 0)
				intensity = 0;
			nl_output->neuron_vector[x + y * w].output.ival = intensity;
		}
	}
}

