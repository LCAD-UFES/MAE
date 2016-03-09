#include "visual_search_filters.h"



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
		fltSimilarity *= expf (-(fltDiff * fltDiff) / (2.0f * M_PI * fltSquareSigma));
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

	// Mounts the Target Feature Column Array
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
		pColumnSigmaVector[nFeature] = (fltSquareValueSum - (float) nColumnsNumber * (fltMean * fltMean)) / (float) (nColumnsNumber - 1);
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
	int feature, features_number, features_per_channel;
	float band_width;
	
	// Checks Parameters
	for (num_params = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, num_params++)
		;

	if (num_params != 4)
	{
		Erro ("Error: Wrong number of parameters. The features_gather_filter must have tree parameters.", "", "");
		return;
	}

	// Gets Parameters
	band_width = filter_desc->filter_params->next->param.fval;
        features_number = filter_desc->filter_params->next->next->param.ival;
        features_per_channel = filter_desc->filter_params->next->next->next->param.ival;
        
	if ((band_width < .0f) && (band_width > 1.0f))
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

	if (wo != features_number * (int) (band_width * (float) w))
	{
		Erro ("Error: The output width must be equal to the number of input neural layers * (int) (band_width * (float) input width).", "", "");
		return;
	}

	for (n_list = filter_desc->neuron_layer_list, feature = PYRAMID_LEVEL * features_per_channel; feature < (PYRAMID_LEVEL + 1) * features_per_channel; n_list = n_list->next, feature++)
	{
		// Gets the Current Input Neuron Layer
		nl_input = n_list->neuron_layer;

		for (v = 0; v < h; v++)
		{
			for (x = 0, u = (int) ((.5f - .5f * band_width) * (float) w); u < (int) ((.5f + .5f * band_width) * (float) w); x++, u++)
				nl_output->neuron_vector[x + feature * (int) (band_width * (float) w) + v * wo].output = nl_input->neuron_vector[u + v * w].output;
		}
	}
}
