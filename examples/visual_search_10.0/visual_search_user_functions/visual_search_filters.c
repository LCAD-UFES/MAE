#include "visual_search_filters.h"



/*
*********************************************************************************
* Function: SimilarityFunction							*
* Description:                                        				*
* Inputs:									*
* Output:									*
*********************************************************************************
*/

float SimilarityFunction_test (int nVectorLength, float *pVectorA, float *pVectorB, float *pSigmaVector)
{
	int i;
	float fltSimilarity, fltDiff, fltSquareSigma, energy;
	
	for (i = 0, fltSimilarity = 1.0; i < nVectorLength; i += 2)
	{
		energy = sqrtf (pVectorA[i] * pVectorA[i] + pVectorA[i+1] * pVectorA[i+1]);

		fltSquareSigma = pSigmaVector[i] * pSigmaVector[i];
		fltDiff = (pVectorA[i] - pVectorB[i]) * (pVectorA[i] - pVectorB[i]);
		fltSimilarity *= energy * expf (-(fltDiff) / (2 * fltSquareSigma));

		fltSquareSigma = pSigmaVector[i+1] * pSigmaVector[i+1];
		fltDiff = (pVectorA[i+1] - pVectorB[i+1]) * (pVectorA[i+1] - pVectorB[i+1]);
		fltSimilarity *= energy * expf (-(fltDiff) / (2 * fltSquareSigma));
	}		
	return (fltSimilarity);
}


float norma;


float
vector_norma (int VectorLength, float *Vector)
{
	int i;
	float acumulator;
	
	for (i = 0, acumulator = 0.0; i < VectorLength; i++)
		acumulator += Vector[i] * Vector[i];

	return (sqrtf (acumulator));
}



float
average_norma_pTargetFeatureColumnsArray (int nVectorsNumber, int nVectorLength, float *pVectorsArray)
{
	int i;
	float acumulator;
	
	for (i = 0, acumulator = 0.0; i < nVectorsNumber; i++)
		acumulator += vector_norma (nVectorLength, &(pVectorsArray[i * nVectorLength]));
		
	return (acumulator / (float) nVectorsNumber);
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
	
	/*int i, j;
	float fltSimilarity,fltAux, fltDiff, fltSquareSigma;
	float pWeights[3] = {1.0, 7.0, 7.0};
	float fltWeightsSum = 15.0;
	
	for (j = 0,fltSimilarity = 0.0; j < 3; j++, fltSimilarity += pWeights[j] * fltAux / fltWeightsSum)		
		for (i = j * nVectorLength / 3, fltAux = 1.0; i < (j + 1) * nVectorLength / 3; i++)
		{
			fltSquareSigma = pSigmaVector[i] * pSigmaVector[i];
			fltDiff = pVectorA[i] - pVectorB[i];
			fltAux *= expf (-(fltDiff * fltDiff) / (2 * fltSquareSigma));
		}*/
	
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



void
normalize_vector (int VectorLength, float *Vector)
{
	int i;
	float acumulator;
	
	for (i = 0, acumulator = 0.0; i < VectorLength; i++)
		acumulator += Vector[i] * Vector[i];

	if (acumulator > 0.0)
		acumulator = (float) VectorLength / (6.0 * sqrt (acumulator));
	
	for (i = 0; i < VectorLength; i++)
		Vector[i] *= acumulator;
}



void
normalize_pTargetFeatureColumnsArray (int nVectorsNumber, int nVectorLength, float *pVectorsArray)
{
	int i;
	
	for (i = 0; i < nVectorsNumber; i++)
		normalize_vector (nVectorLength, &(pVectorsArray[i * nVectorLength]));
}



extern float number_of_features;


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
	//float fltValue, fltMean, fltSquareValueSum;
	
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
			if (number_of_features != 0.0) 
				pTargetFeatureColumnsArray[nFeature + nColumn * nFeaturesPerColumn] = 
					nlFeatures->neuron_vector[i + j * w].output.fval / number_of_features;
		}
	}
	
	// normalize_pTargetFeatureColumnsArray (nColumnsNumber, nFeaturesPerColumn, pTargetFeatureColumnsArray);
	// norma = average_norma_pTargetFeatureColumnsArray (nColumnsNumber, nFeaturesPerColumn, pTargetFeatureColumnsArray);
	
	// Calculates the Sigma Vector (Standard Deviation)
	for (nFeature = 0; nFeature < nFeaturesPerColumn; nFeature++)
	{
		/*for (nColumn = 0, fltMean = 0.0, fltSquareValueSum = 0.0; nColumn < nColumnsNumber; nColumn++)
		{
			fltValue = pTargetFeatureColumnsArray[nFeature + nColumn * nFeaturesPerColumn];
			fltMean += fltValue;
			fltSquareValueSum += fltValue * fltValue;
		}
		fltMean /= (float) nColumnsNumber;
		pColumnSigmaVector[nFeature] = 2.0 * (fltSquareValueSum - (float) nColumnsNumber * (fltMean * fltMean)) / (float) (nColumnsNumber - 1);*/
		pColumnSigmaVector[nFeature] = 1.0;

	}
	
	// Calculates the Filter Output	 
	for (i = 0; i < wo * ho; i++)
	{			
		// Gets the Point Feature Column
		for (nFeature = 0, n_list = filter_desc->neuron_layer_list->next; n_list != NULL; nFeature++, n_list = n_list->next)
			pPointFeatureColumn[nFeature] = n_list->neuron_layer->neuron_vector[i].output.fval;

//		if (vector_norma (nFeaturesPerColumn, pPointFeatureColumn) > 0.5 * norma)
			nlOutput->neuron_vector[i].output.fval = ComputeMaxSimilarity (nColumnsNumber, nFeaturesPerColumn, pTargetFeatureColumnsArray, pPointFeatureColumn, pColumnSigmaVector);
//		else
//			nlOutput->neuron_vector[i].output.fval = 0.0;
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
	int /*index,*/ u, v, x, w, h, wo, ho;
	int num_params;
	int feature, features;
	float band_width;
	
	// Checks Parameters
	for (num_params = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, num_params++)
		;

	if (num_params != 2)
	{
		Erro ("Error: Wrong number of parameters. The features_gather_filter must have 1 parameter.", "", "");
		return;
	}

	// Gets Parameters
	band_width = filter_desc->filter_params->next->param.fval;

	if ((band_width < 0.0) && (band_width > 1.0))
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
	for (features = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, features++)
		;

	if (wo != features * (int) (band_width * (float) w))
	{
		Erro ("Error: The output width must be equal to the number of input neural layers * (int) (band_width * (float) input width).", "", "");
		return;
	}

	for (n_list = filter_desc->neuron_layer_list, feature = 0; feature < features; n_list = n_list->next, feature++)
	{
		// Gets the Current Input Neuron Layer
		nl_input = n_list->neuron_layer;
		
		for (v = 0; v < h; v++)
		{
			for (x = 0, u = (int) ((0.5 - 0.5 * band_width) * (float) w); u < (int) ((0.5 + 0.5 * band_width) * (float) w); x++, u++)
				nl_output->neuron_vector[x + feature * (int) (band_width * (float) w) + v * wo].output = nl_input->neuron_vector[u + v * w].output;
		}
	}
}



/*!
*********************************************************************************
* Function: bidimentional_convolution						*
* Description: 									*
* Inputs: 									*
* Output: 						 			*
*********************************************************************************
*/

NEURON_OUTPUT bidimentional_cortex_convolution (RECEPTIVE_FIELD_DESCRIPTION *receptive_field_descriptor, NEURON_LAYER *image_n_l, int u_center, int v_center, int w, int h)
{
	int u_current, v_current, i, kernel_size;
	float intensity, delta_area, angle, x_ker, y_ker, x_rot, y_rot;
	NEURON_OUTPUT accumulator;
	
	// Gets the kernel size
	kernel_size = (int) sqrt ((double) receptive_field_descriptor->num_points);
	
	// Calculates the normalized delta area
	delta_area = receptive_field_descriptor->area / (float) ((kernel_size - 1) * (kernel_size - 1));
	
	// Initializes the accumulator variable
	accumulator.fval = 0.0;
	
	angle = (u_center < w/2)? pi * (((float) (h-1)/2.0 - (float) v_center) / (float) (h-1)): -pi * (((float) (h-1)/2.0 - (float) v_center) / (float) (h-1));
	for (i = 0; i < receptive_field_descriptor->num_points; i++)
	{
		// Rotate Kernal
		x_ker = (float) receptive_field_descriptor->points[i].x;
		y_ker = (float) receptive_field_descriptor->points[i].y;
		x_rot = x_ker * cos (angle) + y_ker * sin (angle);
		y_rot = y_ker * cos (angle) - x_ker * sin (angle);

		// Calculates the current point
		u_current = u_center + (int) (x_rot + 0.5);
		v_current = v_center + (int) (y_rot + 0.5);

		// Verifies if the point is inside of the neuron layer bounds 
		
		if ((u_current < 0) || (u_current >= w))
		{
			accumulator.fval = 0.0 ;
			break;
		}
					
		if (v_current < 0)
		{
			v_current = -v_current - 1;
			u_current = (w-1)/2 - (u_current - (w-1)/2);
		}
		if (v_current >= h)
		{
			v_current = v_current - (2 * (v_current - h) + 1);
			u_current = (w-1)/2 - (u_current - (w-1)/2);
		}
		
//		if (((u_center % 32) == 0) && (((v_center+1) % 8) == 0) && (u_center <= w/2))
//		{
//			if (fabs(image_n_l->neuron_vector[v_current * w + u_current].output.fval) < fabs(receptive_field_descriptor->points[i].weight * receptive_field_descriptor->points[i].gabor))
//				image_n_l->neuron_vector[v_current * w + u_current].output.fval = receptive_field_descriptor->points[i].weight * receptive_field_descriptor->points[i].gabor;
//		}		
		// Gets the input value 
		intensity = image_n_l->neuron_vector[v_current * w + u_current].output.fval;

		// Accumulates the weighed intensity. The weight function depends of the position inside the kernel
		accumulator.fval += receptive_field_descriptor->points[i].weight * receptive_field_descriptor->points[i].gabor * intensity;
	}
	
	// Normalizes the result
	accumulator.fval *= delta_area / 6.0;
	
	return (accumulator);
}



/*!
*********************************************************************************
* Function: activation_map_filter						*
* Description: 									*
* Inputs: 									*
* Output: 						 			*
*********************************************************************************
*/

void activation_map_filter (FILTER_DESC *filter_desc)
{	
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *image_n_l = NULL;
	RECEPTIVE_FIELD_DESCRIPTION *receptive_field;
	int i, ho, wo, hi, wi/*, xi, yi*/, xo, yo/*, u, v*/;
	float camera_opening, cycles_per_degree, width, aspect_ratio, orientation, phase, log_factor;
	float frequency;
	//double correction;
	int mapping, shift;
	//int x_center, y_center;
	NEURON_OUTPUT previous_output;
	//int previous_xi, previous_yi, index;
	
	// Checks the Parameters
	for (i = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, i++)
		;

	if (i != 10)
	{
		Erro ("Wrong number of parameters for activation_map_filter.", "", "");
		return;
	}

	// Gets the Parameters
	camera_opening		= filter_desc->filter_params->next->param.fval;
	cycles_per_degree	= filter_desc->filter_params->next->next->param.fval;
	width			= filter_desc->filter_params->next->next->next->param.fval;
	aspect_ratio		= filter_desc->filter_params->next->next->next->next->param.fval;
	orientation		= filter_desc->filter_params->next->next->next->next->next->param.fval;
	phase			= filter_desc->filter_params->next->next->next->next->next->next->param.fval;
	shift			= filter_desc->filter_params->next->next->next->next->next->next->next->param.ival;
	mapping			= filter_desc->filter_params->next->next->next->next->next->next->next->next->param.ival;
	log_factor		= filter_desc->filter_params->next->next->next->next->next->next->next->next->next->param.fval;
	
	// Checks the Input Neuron Layers
	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
		;

	if (i != 1)
	{
		Erro ("Wrong number of neuron layers. activation_map_filter must be applied on only one neuron layer.", "", "");
		return;
	}

	// Gets the Input Neuron Layers
	image_n_l = filter_desc->neuron_layer_list->neuron_layer;

	// Gets Input Neuron Layer Dimentions
	wi = image_n_l->dimentions.x;
	hi = image_n_l->dimentions.y;
	
	// Gets Output Neuron Layer Dimentions
	wo = filter_desc->output->dimentions.x;
	ho = filter_desc->output->dimentions.y;
	
	// Computes Kernels
	if (filter_desc->private_state == NULL)
	{
		receptive_field = (RECEPTIVE_FIELD_DESCRIPTION *) malloc (sizeof(RECEPTIVE_FIELD_DESCRIPTION));

		// Calculates the spatial frequency (cycles per pixel)
		frequency = (1.0 / (float) wi) * camera_opening * cycles_per_degree;

		compute_v1_spatial_receptive_field (receptive_field, frequency, width, aspect_ratio, orientation, phase, KERNEL_2D);
		filter_desc->private_state = (void *) receptive_field;

		// Calculates the minimum value of the result of a gabor filter
		bidimentional_convolution_test_gabor (receptive_field);
	}
	else
		receptive_field = (RECEPTIVE_FIELD_DESCRIPTION *)filter_desc->private_state;

	for (xo = 0; xo < wo; xo++)
	{
		for (yo = 0; yo < ho; yo++)
		{
//			bidimentional_cortex_convolution (receptive_field, filter_desc->output, xo, yo, wo, ho);
			previous_output = filter_desc->output->neuron_vector[(yo * wo) + xo].output = bidimentional_cortex_convolution (receptive_field, image_n_l, xo, yo, wo, ho);
			if (((previous_output.fval > 0.0) && (previous_output.fval <= receptive_field->min_dog)) ||
			    ((previous_output.fval < 0.0) && (previous_output.fval >= receptive_field->min_dog)))
				filter_desc->output->neuron_vector[(yo * wo) + xo].output.fval = 0.0;
		}
	}
}



/*!
*********************************************************************************
* Function: apply_cortex_gaussian_kernel						*
* Description:                                        				*
* Inputs:									*
* Output:									*
*********************************************************************************
*/

float apply_cortex_gaussian_kernel (RECEPTIVE_FIELD_DESCRIPTION *receptive_field_descriptor, NEURON_LAYER *image_n_l, int x0, int y0)
{
	int i;
	int xr, yr, w, h;
	float fltWeight, fltResult;

	fltResult = 0.0;
	fltWeight = 0.0;
	w = image_n_l->dimentions.x;
	h = image_n_l->dimentions.y;
	for (i = 0; i < receptive_field_descriptor->num_points; i++)
	{
		xr = x0 + receptive_field_descriptor->points[i].x;
		yr = y0 + receptive_field_descriptor->points[i].y;

		if ((xr < 0) || (xr >= w))
			continue;
					
		if (yr < 0)
		{
			yr = -yr - 1;
			xr = (w-1)/2 - (xr - (w-1)/2);
		}
		if (yr >= h)
		{
			yr = yr - (2 * (yr - h) + 1);
			xr = (w-1)/2 - (xr - (w-1)/2);
		}
		
		fltResult += receptive_field_descriptor->points[i].gaussian * image_n_l->neuron_vector[yr * image_n_l->dimentions.x + xr].output.fval;
		fltWeight += receptive_field_descriptor->points[i].gaussian;
	}

	fltResult /= fltWeight;

	return fltResult;
}



/*!
*********************************************************************************
* Function: cortex_gaussian_filter
* Description: Applies a gaussian kernel on a cortex neuron layer
* Inputs:
* Output:
*********************************************************************************
*/

void cortex_gaussian_filter (FILTER_DESC *filter_desc)
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
			Erro ("Wrong number of neuron layers. The cortex_gaussian_filter must be applied on only one neuron layer.", "", "");
			return;
		}
	
		if (filter_desc->neuron_layer_list->neuron_layer->output_type != filter_desc->output->output_type)
		{
			Erro ("The output type of input neuron layer is different of the cortex_gaussian_filter output.", "", "");
			return;
		}

		// Buscar o KernelSize e o Sigma
		for (i = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, i++)
			;
		i--;

		if (i != 2) 
		{
			Erro ("Wrong number of parameters. The cortex_gaussian_filter have 2 parameters: kernel_size and sigma, respectivally.", "", "");
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
					filter_desc->output->neuron_vector[j * wo + i].output.ival = (int)apply_cortex_gaussian_kernel(receptive_field, nl, i, j);
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
					filter_desc->output->neuron_vector[j * wo + i].output.fval = apply_cortex_gaussian_kernel(receptive_field, nl, i, j);
				}
			}
		}
		break;
	}
}
