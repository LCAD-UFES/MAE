#include "filter.h"



#define VECTOR_SUM		0
#define EUCLIDIAN_DISTANCE	1
#define DOT_PRODUCT		2
#define GAUSSIAN_DISTRIBUTION	3


/*
*********************************************************************************
* Function: similarity_function							*
* Description:                                        				*
* Inputs:									*
* Output:									*
*********************************************************************************
*/

float
similarity_function (int functionType, int vectorLength, float *vectorA, float *vectorB)
{
	int i;
	float result, norm, value, vectorNormA = 0.0, vectorNormB = 0.0, sigma = 0.1;
	
	switch (functionType)
	{
		case VECTOR_SUM:
			// Computes the Norm of Vectors A and B
			for (i = 0; i < vectorLength; i++)
			{
				vectorNormA += vectorA[i] * vectorA[i];
				vectorNormB += vectorB[i] * vectorB[i];
			}
	
			vectorNormA = sqrt (vectorNormA);
			vectorNormB = sqrt (vectorNormB);
	
			// Computes the Sum of Vectors A and B
			for (i = 0, result = 0.0; i < vectorLength; i++)
			{
				value = vectorA[i] / (1.0 + vectorNormA) + vectorB[i] / (1.0 + vectorNormB);
				result += value * value;
			}
	
			result = sqrt(result);
			break;
		case EUCLIDIAN_DISTANCE:
			// Computes the Euclidian Distance between Vectors A and B
			for (i = 0, result = 0.0, norm = 0.0; i < vectorLength; i++)
			{
				value = vectorA[i] - vectorB[i];
				result += value * value;
				//norm += vectorA[i] * vectorA[i] + vectorB[i] * vectorB[i];
			}
	
			//result = norm / result;
			result = 1.0/sqrt(result);
			break;
		case DOT_PRODUCT:
			// Computes the Dot Product between Vectors A and B
			for (i = 0, result = 0.0; i < vectorLength; i++)
				result += vectorA[i] * vectorB[i];		
			break;
		case GAUSSIAN_DISTRIBUTION:
			for (i = 0, result = 1.0; i < vectorLength; i++)
				result *= exp (- (vectorA[i] - vectorB[i]) * (vectorA[i] - vectorB[i]) / (sigma * sigma));;		
			break;
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
	int feature, samples_number, features_number, kernel_size, i, j, u, v, ho, wo, nl_number, p_number;
	float value, feature_value, sigma;
	float *point_features_vector  = NULL; 
	float *target_features_vector = NULL;
	
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
	samples_number  = nl_features->dimentions.y;
	
	// Creates the Target and the Point Vectors
	if (filter_desc->private_state == NULL)
	{
		target_features_vector = (float *) alloc_mem (2 * features_number * sizeof (float));
		point_features_vector = &(target_features_vector[features_number]);
		filter_desc->private_state = (void *) target_features_vector;
	}
	else
	{
		target_features_vector = (float *) filter_desc->private_state;
		point_features_vector = &(target_features_vector[features_number]);
	}
	
	// Initializes the Filter Output
	for (i = 0; i < wo * ho; i++)
		nl_output->neuron_vector[i].output.fval = 0.0;
		
	for (j = 0; j < samples_number; j++)
	{
		for (i = 0; i < wo * ho; i++)
		{
			// Gets the Target and the Point Vectors
			for (feature = 0, n_list = filter_desc->neuron_layer_list->next; n_list != NULL; feature++, n_list = n_list->next)
			{	
				point_features_vector[feature] = n_list->neuron_layer->neuron_vector[i].output.fval;
				target_features_vector[feature] = nl_features->neuron_vector[feature + j * features_number].output.fval;
			}
			
			nl_output->neuron_vector[i].output.fval += similarity_function (GAUSSIAN_DISTRIBUTION, features_number, target_features_vector, point_features_vector);
		}
	}
}
