#include "filter.h"



/*
*********************************************************************************
* Function: similarity_function							*
* Description:                                        				*
* Inputs:									*
* Output:									*
*********************************************************************************
*/

float
similarity_function (int vectorLength, float *vectorA, float *vectorB, float *sigma_vector)
{
	int i;
	float result, diff, sigmaXsigma, normA, normB;
	
	/*for (i = 0, normA = normB = 0.0; i < vectorLength; i++)
	{
		normA += vectorA[i] * vectorA[i];
		normB += vectorB[i] * vectorB[i];
	}
		
	for (i = 0, normA = sqrt (normA), normB = sqrt (normB); i < vectorLength; i++)
	{
		if (normA != 0.0)
			vectorA[i] /= normA;
			
		if (normB != 0.0)
			vectorB[i] /= normB;
	}*/
			
	for (i = 0, result = 1.0; i < vectorLength; i++)
	{
		sigmaXsigma = sigma_vector[i];
		diff = vectorA[i] - vectorB[i];
		result *= exp (- diff * diff / sigmaXsigma);
	}		
	
	return (result);
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
	NEURON_LAYER *nl_output, *nl_features;
	int feature, features_number, kernel_size, i, u, v, ho, wo, nl_number, p_number;
	float value, feature_value, sigma;
	float *point_features_vector  = NULL; 
	float *target_features_vector = NULL;
	float *sigma_vector = NULL;

	// Checks the Neuron Layers Number
	for (nl_number = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, nl_number++)
            	;

	// Checks the Parameters Number
	for (p_number = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, p_number++)
            	;

	if (p_number != 1)
	{
		Erro ("Error: The activation map dont't need any parameters.", "", "");
		return;
	}
		
	// Gets the Filter Output 
	nl_output = filter_desc->output;

	// Gets the Image Dimentions
	wo = nl_output->dimentions.x;
	ho = nl_output->dimentions.y;
	
	// Gets the Features Neuron Layer
	nl_features = filter_desc->neuron_layer_list->neuron_layer;
	
	// Gets the Features Neuron Layer Dimentions
	features_number = nl_features->dimentions.x;
	
	// Creates the Target and the Point Vectors
	if (filter_desc->private_state == NULL)
	{
		target_features_vector = (float *) alloc_mem (3 * features_number * sizeof (float));
		point_features_vector = &(target_features_vector[features_number]);
		sigma_vector = &(target_features_vector[2 * features_number]);
		filter_desc->private_state = (void *) target_features_vector;
	}
	else
	{
		target_features_vector = (float *) filter_desc->private_state;
		point_features_vector = &(target_features_vector[features_number]);
		sigma_vector = &(target_features_vector[2 * features_number]);
	}
	
	for (i = 0; i < wo * ho; i++)
	{
		// Gets the Target and the Point Vectors
		for (feature = 0, n_list = filter_desc->neuron_layer_list->next; n_list != NULL; feature++, n_list = n_list->next)
		{	
			point_features_vector[feature] = n_list->neuron_layer->neuron_vector[i].output.fval;
			target_features_vector[feature] = nl_features->neuron_vector[feature].output.fval;
			sigma_vector[feature] = 10.0 * nl_features->neuron_vector[feature + features_number].output.fval;

		}
			
		nl_output->neuron_vector[i].output.fval = similarity_function (features_number / 3, &(target_features_vector[0]), &(point_features_vector[0]), &(sigma_vector[0]))
							+ similarity_function (features_number / 3, &(target_features_vector[4]), &(point_features_vector[4]), &(sigma_vector[4]))
							+ similarity_function (features_number / 3, &(target_features_vector[8]), &(point_features_vector[8]), &(sigma_vector[8]));
		
		//nl_output->neuron_vector[i].output.fval = similarity_function (features_number, target_features_vector, point_features_vector, sigma_vector);

	}
}
