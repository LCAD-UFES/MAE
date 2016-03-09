#include "filter.h"



/*
*********************************************************************************
* Function: apply_gaussian_kernel						*
* Description:                                        				*
* Inputs:									*
* Output:									*
*********************************************************************************
*/

float
apply_gaussian_kernel_on_activation_map (RECEPTIVE_FIELD_DESCRIPTION *gaussian_kernel, float *activation_map, int wo, int ho, int x0, int y0)
{
	int i;
	int xr, yr;
	float fltWeight, fltResult;

	fltResult = 0.0;
	fltWeight = 0.0;
	for (i = 0; i < gaussian_kernel->num_points; i++)
	{
		xr = x0 + gaussian_kernel->points[i].x;
		yr = y0 + gaussian_kernel->points[i].y;

		if ((xr < 0) || (xr >= wo) || (yr < 0) || (yr >= ho))
			continue;

		fltResult += gaussian_kernel->points[i].gaussian * activation_map[2 * (yr * wo + xr)];
		fltWeight += gaussian_kernel->points[i].gaussian;
	}

	fltResult /= fltWeight;

	return (fltResult);
}



/*
*********************************************************************************
* Function: activation_map_filter                 				*
* Description: 			                        			*
* Inputs: 				                  			*
* Output: 				                			*
*********************************************************************************
*/

void
activation_map_filter (FILTER_DESC *filter_desc)
{
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list;
	NEURON_LAYER *nl_input, *nl_output, *nl_features;
	int feature, samples_number, features_number, kernel_size, i, j, u, v, ho, wo, nl_number, p_number;
	float value, feature_value, sigma;
	float *auxiliary_map = NULL;
	static RECEPTIVE_FIELD_DESCRIPTION *gaussian_kernel = NULL; // Needs change
	
	// Checks the Neuron Layers Number
	for (nl_number = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, nl_number++)
            	;

	// Checks the Parameters Number
	for (p_number = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, p_number++)
            	;

	if (p_number != 3)
	{
		Erro ("Error: The activation map filter need two parameters. The gaussian <sigma> and the <kernel_size>", "", "");
		return;
	}
	
	// Gets the Filter Parameters
	sigma       = filter_desc->filter_params->next->param.fval;
	kernel_size = filter_desc->filter_params->next->next->param.ival;
	
	// Gets the Filter Output 
	nl_output = filter_desc->output;

	// Gets the Image Dimentions
	wo = nl_output->dimentions.x;
	ho = nl_output->dimentions.y;
	
	// Creates the Auxiliary Map and the Gaussian Kernel
	if (filter_desc->private_state == NULL)
	{
		auxiliary_map = (float *) alloc_mem (2 * wo * ho * sizeof (float));
		filter_desc->private_state = (void *) auxiliary_map;
		gaussian_kernel = (RECEPTIVE_FIELD_DESCRIPTION *) alloc_mem (sizeof (RECEPTIVE_FIELD_DESCRIPTION));
		compute_gaussian_kernel (gaussian_kernel, kernel_size, sigma);
	}
	else
		auxiliary_map   = (float *) filter_desc->private_state;
	
	// Gets the Features Neuron Layer
	nl_features = filter_desc->neuron_layer_list->neuron_layer;
	
	// Gets the Features Neuron Layer Dimentions
	features_number = nl_features->dimentions.x;
	samples_number  = nl_features->dimentions.y;
	
	// Initializes Filter Output
	for (i = 0; i < wo * ho; i++)
		nl_output->neuron_vector[i].output.fval = 0.0;
	
	/************************************************************************/
	/* Given A = {A1, A2, ..., An} and B = {B1, B2, ..., Bn}, where n is    */
	/* the features number, the normalized distance D between A and B is	*/
	/*  									*/
	/* D = ((A1 - B1)^2 + ...+ (An - Bn)^2) / (1 + |A|^2 + |B|^2) 		*/
	/*									*/  
	/* where |V| denotes the norm of array V				*/
	/************************************************************************/
	
	for (j = 0; j < samples_number; j++)
	{
		// Initializes the Auxiliary Map
		for (i = 0; i < 2 * wo * ho; i++)
			auxiliary_map[i] = 0.0; 
		
		for (feature = 0, n_list = filter_desc->neuron_layer_list->next; n_list != NULL; feature++, n_list = n_list->next)
		{	
			nl_input = n_list->neuron_layer;
			feature_value = nl_features->neuron_vector[feature + j * features_number].output.fval;
	
			for (i = 0; i < wo * ho; i++)
			{
				value = nl_input->neuron_vector[i].output.fval;
				auxiliary_map[2 * i + 0] += (feature_value - value) * (feature_value - value); // (Ai - Bi)^2
				auxiliary_map[2 * i + 1] += (value * value + feature_value * feature_value); // Ai^2 + Bi^2
			}
		}

		// @ O melhor seria computar o inverso e somar 1.0 ao denominador. Para termos um maximo e nao
		//   um minimo podemos subtrair o resultado de um valor grande; 1000.0, por exemplo.
		// Normalizes the Auxiliary Map
		for (i = 0; i < wo * ho; i++)
			auxiliary_map[2 * i + 0] = auxiliary_map[2 * i + 0] / (1.0 + auxiliary_map[2 * i + 1]); // D 
		
		//for (i = 0; i < wo * ho; i++)
		//	auxiliary_map[2 * i + 0] = auxiliary_map[2 * i + 1] / auxiliary_map[2 * i + 0]; // 1 / D 
		
		// Applies the Gaussian Filter on the Auxiliary Map
		for (u = 0; u < wo; u++)
		{
			for (v = 0; v < ho; v++)
			{
				// @ E' preciso testar se o passa baixa abaixo ajuda ou atrapalha.
				//   Tambem e' preciso verificar se ele esta fazendo a operaccao observando corretamente
				//   o mapeamento feito em auxiliary_map.
				value = apply_gaussian_kernel_on_activation_map(gaussian_kernel, auxiliary_map, wo, ho, u, v);
				
				// Chooses the Greater Value 
				// @ Podemos, no lugar de pegar o maior, somar todos eles. Assim, muitos juntos ganham um sozinho.
				// @ E' importante checar como este mapeamento e' usado para obter as coordenadas alvo.
				//   O ideal seria pegar o maximo (winner takes it all).
				// @ Testar inicialmente apenas com as pessoas 1 e 2.
				//if (value > nl_output->neuron_vector[u + v * wo].output.fval)
				//	nl_output->neuron_vector[u + v * wo].output.fval = value;
					
				nl_output->neuron_vector[u + v * wo].output.fval += value;
			}
		}
	}
}
