#include "visual_search.h"


// Global Variables

// Target Coordinates
int g_nTargetX = 0, g_nTargetY = 0;
int samples_memorized = 0;
float global_max_value = .0f;

/*
********************************************************
* Function: output_handler_max_value_position	       *
* Description:  				       *
* Inputs:					       *
* Output:					       *
********************************************************
*/

void output_handler_max_value_position (OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)
{
	int u, v, u_max = 0, v_max = 0, w, h, xi, yi;
	float current_value, max_value = FLT_MIN, log_factor;
		
	// Gets the output handler parameters
	log_factor = output->output_handler_params->next->param.fval;
	
	// Gets the Neuron Layer Dimentions
	w = output->neuron_layer->dimentions.x;
	h = output->neuron_layer->dimentions.y;
	
	// Finds the max value position
	for (v = 0; v < h; v++)
	{
		for (u = 0; u < w; u++)
		{
			current_value = output->neuron_layer->neuron_vector[v * w + u].output.fval;
			
			if (max_value < current_value)
			{
				max_value = current_value;
				u_max = u;
				v_max = v;
			}
		}	
	}
	
	// Saves the max value
	global_max_value = max_value;
	
	// Map the max value coordinates to image
	map_v1_to_image (&xi, &yi, INPUT_WIDTH, INPUT_HEIGHT, u_max, v_max, w, h, 0, 0, (double) h / (double) (h - 1), log_factor);

	// Saves the max value position
	nl_target_coordinates.neuron_vector[0].output.fval = (float) xi;
	nl_target_coordinates.neuron_vector[1].output.fval = (float) yi;
}



/*
********************************************************
* Function: output_handler_min_value_position	       *
* Description:  				       *
* Inputs:					       *
* Output:					       *
********************************************************
*/

void output_handler_min_value_position (OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)
{
	int u, v, u_min = 0, v_min = 0, w, h, xi, yi;
	float current_value, min_value = FLT_MAX, log_factor;
		
	// Gets the output handler parameters
	log_factor = output->output_handler_params->next->param.fval;
	
	// Gets the Neuron Layer Dimentions
	w = output->neuron_layer->dimentions.x;
	h = output->neuron_layer->dimentions.y;
	
	// Finds the min value position
	for (v = 0; v < h; v++)
	{
		for (u = 0; u < w; u++)
		{
			current_value = output->neuron_layer->neuron_vector[v * w + u].output.fval;
			
			if (min_value > current_value)
			{
				min_value = current_value;
				u_min = u;
				v_min = v;
			}
		}	
	}
	
	// Map the max value coordinates to image
	map_v1_to_image (&xi, &yi, INPUT_WIDTH, INPUT_HEIGHT, u_min, v_min, w, h, 0, 0, (double) h / (double) (h - 1), log_factor);

	// Saves the max value position
	nl_target_coordinates.neuron_vector[0].output.fval = (float) xi;
	nl_target_coordinates.neuron_vector[1].output.fval = (float) yi;
}


float confidence;

/*
*********************************************************************************
* Function: output_handler_mean_position					*
* Description:                                        				*
* Inputs:									*
* Output:									*
*********************************************************************************
*/

void output_handler_mean_position (OUTPUT_DESC *output, int type_call, int mouse_button, int mouse_state)
{
	int u, v, w, h, xi, yi;
	float x_mean, y_mean, weight, accumulator = .0f, log_factor, cut_point;
		
	// Gets the output handler parameters
	log_factor = output->output_handler_params->next->param.fval;
	cut_point = output->output_handler_params->next->next->param.fval;
	
	// Gets the neuron layer dimentions
	w = output->neuron_layer->dimentions.x;
	h = output->neuron_layer->dimentions.y;
	
	// Initialize the mean coordinates 
	x_mean = .0f;
 	y_mean = .0f;

	//cut_point = cut_point * (output->neuron_layer->max_neuron_output.fval - output->neuron_layer->min_neuron_output.fval) + output->neuron_layer->min_neuron_output.fval;
	
	for (v = 0; v < h; v++)
	{
		for (u = 0; u < w; u++)
		{
			// Gets the weight value
			weight = output->neuron_layer->neuron_vector[v * w + u].output.fval;
			
			if (weight < cut_point)
			        continue;

			// Map the Mean Coordinates to Image
			map_v1_to_image (&xi, &yi, INPUT_WIDTH, INPUT_HEIGHT, u, v, w, h, 0, 0, (double) h / (double) (h - 1), log_factor);

			x_mean += (float) xi * weight;
			y_mean += (float) yi * weight;
			accumulator += weight;
		}
	}
	
	// Normalize the mean coordinates 
	x_mean /= accumulator;
	y_mean /= accumulator;
	
	// Saves the mean coordinates
	nl_target_coordinates.neuron_vector[0].output.fval = x_mean;
	nl_target_coordinates.neuron_vector[1].output.fval = y_mean;
}



/*
********************************************************
* Function: jump				       *
* Description:   				       *
* Inputs: input			 		       *
* Output: none  				       *
********************************************************
*/

void jump (INPUT_DESC *input)
{
	float x, y;

        x = nl_target_coordinates.neuron_vector[0].output.fval;
	y = nl_target_coordinates.neuron_vector[1].output.fval;

	input->wxd += (x > .0f)? (int) (x + .5f) : (int) (x - .5f);
	input->wyd += (y > .0f)? (int) (y + .5f) : (int) (y - .5f);

	move_input_window (input->name, input->wxd, input->wyd);

	return;
}



/*
********************************************************
* Function: saccade				       *
* Description:   				       *
* Inputs: input			 		       *
* Output: none  				       *
********************************************************
*/

void saccade (INPUT_DESC *input)
{
	float x, y;
	int count = 0;

	// Saccade until reach the target
	do
	{
		x = nl_target_coordinates.neuron_vector[0].output.fval;
	        y = nl_target_coordinates.neuron_vector[1].output.fval;
	        
	        input->wxd += (x > .0f)? (int) (x + .5f) : (int) (x - .5f);
	        input->wyd += (y > .0f)? (int) (y + .5f) : (int) (y - .5f);
	        
	        move_input_window (input->name, input->wxd, input->wyd);
	} while (((fabs (x) > .5f) || (fabs (y) > .5f)) && (count++ < 7));

	return;
}



/*
*********************************************************************************
* Function: SimilarityFunction							*
* Description:                                        				*
* Inputs:									*
* Output:									*
*********************************************************************************
*/

float SimilarityFunction (int nVectorLength, float *pVectorA, float *pVectorB, float *pSigmaVector)
{
	int i;
	float fltSimilarity, fltDiff, fltSquareSigma;
	
	for (i = 0, fltSimilarity = 1.0; i < nVectorLength; i++)
	{
		fltSquareSigma = pSigmaVector[i] * pSigmaVector[i];
		fltDiff = pVectorA[i] - pVectorB[i];
		fltSimilarity *= expf (-(fltDiff * fltDiff) / (2 * fltSquareSigma));
	}		
		
	return (fltSimilarity);
}



/*
*********************************************************************************
* Function: ComputeMaxSimilarity						*
* Description:                                        				*
* Inputs:									*
* Output:									*
*********************************************************************************
*/

float ComputeMaxSimilarity (int nVectorsNumber, int nVectorLength, float *pVectorsArray, float *pVector, float *pSigmaVector)
{
	int i;
	float fltSimilarity, fltMaxSimilarity;
	
	for (i = 0, fltMaxSimilarity = FLT_MIN; i < nVectorsNumber; i++)
	{	
		fltSimilarity = SimilarityFunction (nVectorLength, &(pVectorsArray[i * nVectorLength]), pVector, pSigmaVector);
		
		if (fltSimilarity > fltMaxSimilarity)
			fltMaxSimilarity = fltSimilarity;
	}		
	
	return (fltMaxSimilarity);
}



/*
*********************************************************************************
* Function: activation_map_filter                 				*
* Description: 			                        			*
* Inputs: 				                  			*
* Output: 				                			*
*********************************************************************************
*/

void v1_activation_map_filter (FILTER_DESC *filter_desc)
{
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nlOutput = NULL, *nlFeatures = NULL;
	int i, j, w, h, ho, wo, nl_number, p_number;
	int nFeature, nFeaturesPerColumn;
	int nSubLayerWidth, nSubLayerHight;
	int nColumn, nColumnsNumber;
	float *pPointFeatureColumn  = NULL; 
	float *pColumnSigmaVector = NULL;
	float *pTargetFeatureColumnsArray = NULL;
	float fltValue, fltMean, fltSquareValueSum;
	
	// Checks the Neuron Layers Number
	for (nl_number = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, nl_number++)
            	;

	// Checks the Parameters Number
	for (p_number = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, p_number++)
            	;

	if (p_number != 2)
	{
		Erro ("Error: Wrong number of parameters. The activation_map_filter must have only one parameter <nFeaturesPerColumn>.", "", "");
		return;
	}

	// Gets the Filter Parameters
	nFeaturesPerColumn = filter_desc->filter_params->next->param.ival;

	// Gets the Filter Output 
	nlOutput = filter_desc->output;

	// Gets the Filter Output Dimentions
	wo = nlOutput->dimentions.x;
	ho = nlOutput->dimentions.y;
	
	// Gets the Features Neuron Layer
	nlFeatures = filter_desc->neuron_layer_list->neuron_layer;
	
	// Gets the Features Neuron Layer Dimentions
	w = nlFeatures->dimentions.x;
	h = nlFeatures->dimentions.y;
	
	// Calculates the Features Sub-Layers Dimentions
	nSubLayerWidth = w  / nFeaturesPerColumn;
	nSubLayerHight = h;
	
	// Calculates the Columns Number
	nColumnsNumber = nSubLayerWidth * nSubLayerHight;
	
	// Creates the Target Feature Columns Array and the Point Feature Column
	if (filter_desc->private_state == NULL)
	{
		pPointFeatureColumn = (float *) alloc_mem ((2 * nFeaturesPerColumn + w * h) * sizeof (float));
		pColumnSigmaVector = &(pPointFeatureColumn[nFeaturesPerColumn]);
		pTargetFeatureColumnsArray = &(pPointFeatureColumn[2 * nFeaturesPerColumn]);
		filter_desc->private_state = (void *) pPointFeatureColumn;
	}
	else
	{
		pPointFeatureColumn = (float *) filter_desc->private_state;
		pColumnSigmaVector = &(pPointFeatureColumn[nFeaturesPerColumn]);
		pTargetFeatureColumnsArray = &(pPointFeatureColumn[2 * nFeaturesPerColumn]);
	}
	
	// Mounts the Feature Columns Array
	for (j = 0; j < h; j++)
	{
		for (i = 0; i < w; i++)
		{
			nColumn  = i % nSubLayerWidth + j * nSubLayerWidth;
			nFeature = i / nSubLayerWidth;
			pTargetFeatureColumnsArray[nFeature + nColumn * nFeaturesPerColumn] = nlFeatures->neuron_vector[i + j * w].output.fval;
		}
	}
		
	// Calculates the Sigma Vector (Standard Deviation)
	for (nFeature = 0; nFeature < nFeaturesPerColumn; nFeature++)
	{
		for (nColumn = 0, fltMean = .0f, fltSquareValueSum = .0f; nColumn < nColumnsNumber; nColumn++)
		{
			fltValue = pTargetFeatureColumnsArray[nFeature + nColumn * nFeaturesPerColumn];
			fltMean += fltValue;
			fltSquareValueSum += fltValue * fltValue;
		}
		fltMean /= (float) nColumnsNumber;
		pColumnSigmaVector[nFeature] = 2.0f * (fltSquareValueSum - (float) nColumnsNumber * (fltMean * fltMean)) / (float) (nColumnsNumber - 1);
		//pColumnSigmaVector[nFeature] = 1.0;
	}
	
	// Calculates the Filter Output	 
	for (i = 0; i < wo * ho; i++)
	{			
		// Gets the Point Feature Column
		for (nFeature = 0, n_list = filter_desc->neuron_layer_list->next; n_list != NULL; nFeature++, n_list = n_list->next)
			pPointFeatureColumn[nFeature] = n_list->neuron_layer->neuron_vector[i].output.fval;

		nlOutput->neuron_vector[i].output.fval = ComputeMaxSimilarity (nColumnsNumber, nFeaturesPerColumn, pTargetFeatureColumnsArray, pPointFeatureColumn, pColumnSigmaVector);
	}
}



/*
*********************************************************************************
* Function: features_gather_filter                         			*
* Description:                         						*
* Inputs: filter_desc - Filter description                  			*
* Output: 				                			*
*********************************************************************************
*/

void features_gather_filter (FILTER_DESC *filter_desc)
{
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nl_input  = NULL;
	NEURON_LAYER *nl_output = NULL;
	int u, v, x, w, h, wo, ho;
	int num_params;
	int nFeature, nFeaturesNumber;
	float fltBandWidth;
	
	// Checks Parameters
	for (num_params = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, num_params++)
		;

	if (num_params != 3)
	{
		Erro ("Error: Wrong number of parameters. The features_gather_filter must have two parameters.", "", "");
		return;
	}

	// Gets Parameters
	fltBandWidth = filter_desc->filter_params->next->param.fval;
	nFeaturesNumber = filter_desc->filter_params->next->next->param.ival;

	if ((fltBandWidth < .0f) && (fltBandWidth > 1.0f))
	{
		Erro ("Error: The band width parameter must be in the interval [0.0, 1.0].", "", "");
		return;
	}
	
	// Gets the Output Neuron Layer
	nl_output = filter_desc->output;
	
	// Gets Output Dimentions
	wo = nl_output->dimentions.x;
	ho = nl_output->dimentions.y;
			
	// Gets the Input Neuron Layer Dimentions
	w = filter_desc->neuron_layer_list->neuron_layer->dimentions.x;
	h = filter_desc->neuron_layer_list->neuron_layer->dimentions.y;
		
	// Checks Neuron Layers
	for (nFeature = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, nFeature++)
		;

	if (wo != nFeature * (int) (fltBandWidth * (float) w))
	{
		Erro ("Error: The output width must be equal to the number of input neural layers * (int) (fltBandWidth * (float) input width).", "", "");
		return;
	}

	for (n_list = filter_desc->neuron_layer_list, nFeature = 0; nFeature < nFeaturesNumber; n_list = n_list->next, nFeature++)
	{
		// Gets the Current Input Neuron Layer
		nl_input = n_list->neuron_layer;
		
		for (v = 0; v < h; v++)
		{
			for (x = 0, u = (int) ((.5f - .5f * fltBandWidth) * (float) w); u < (int) ((.5 + .5f * fltBandWidth) * (float) w); x++, u++)
				nl_output->neuron_vector[x + nFeature * (int) (fltBandWidth * (float) w) + v * wo].output = nl_input->neuron_vector[u + v * w].output;
		}
	}
}
