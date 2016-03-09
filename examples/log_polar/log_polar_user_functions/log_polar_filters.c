#include "filter.h"



/*! 
*********************************************************************************
* \brief Maps the Log-Polar coordinates to the Cartesian coordinates.
* \param[out] xi The input image coordinate.		
* \param[out] yi The input image coordinate.  	  
* \param[in] wi The input image width.  	  
* \param[in] hi The input image height.  	  
* \param[in] xo The output image coordinate.  	  
* \param[in] yo The output image coordinate.  	  
* \param[in] wo The output image width.  	  
* \param[in] ho The output image height.
* \param[in] xi_center The input image center.
* \param[in] yi_center The input image center.
* \param[in] log_factor The log factor.
* \pre .
* \post .
* \return Nothing.
*********************************************************************************
*/

void log_polar_mapping (int *xi, int *yi, int wi, int hi, int u, int v, int wo, int ho, int x_center, int y_center, float log_factor)
{
	int ring, wedge;
	int index, rings_number, wedges_number;
	float radius, radius_min, radius_max, theta, k1, k2, k3;
		
	//rings_number = wo;
	//wedges_number = ho;
	
	rings_number = (wo >> 1); // rings_number / 2
	wedges_number = (ho << 1); // 2 * rings_number
	
	radius_max = (float) wi / 2.0;
	radius_min = radius_max * exp (-2.0 * M_PI * (float) (rings_number -  1) / (float) wedges_number);

	if (u < wo / 2)
	{
		ring  = (wo >> 1) - u - 1;
		wedge = (ho << 1) - v - 1;
	}
	else
	{
		ring  = u - (wo >> 1);
		wedge = v;
	}
	
	k1 = 2.0 * M_PI / (float) wedges_number;
	k2 = log (log_factor * radius_max / radius_min) / (float) (rings_number - 1);
	k3 = radius_min / log_factor;
	
	theta = k1 * (float) wedge - M_PI / 2.0;
	radius = k3 * exp (k2 * (float) ring) - k3;
			
	*xi = (int) (radius * cos (theta) + 0.5) + x_center;
	*yi = (int) (radius * sin (theta) + 0.5) + y_center;
			
	return;
}



/*!
*********************************************************************************
* Function: log_polar_filter							*
* Description:									*
* Inputs:									*
* Output: 									*
* http://homepages.inf.ed.ac.uk/rbf/CVonline/LOCAL_COPIES/YOUNG2/		*
*********************************************************************************
*/

void log_polar_filter (FILTER_DESC *filter_desc)
{
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nl_input  = NULL;
	NEURON_LAYER *nl_output = NULL;
	INPUT_DESC *input = NULL;
	int num_params, num_layers;
	int xi, yi, wi, hi, u, v, wo, ho, x_center, y_center;
	int ring, wedge;
	int index, rings_number, wedges_number;
	float radius, radius_min, radius_max, theta, log_factor, k1, k2, k3;
	
	// Checks Parameters
	for (num_params = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, num_params++)
		;

	if (num_params != 3)
	{
		Erro ("Error: Wrong number of parameters. The log_polar_mapping needs two parameters: <input name>, <log_factor>", "", "");
		return;
	}
	
	// Gets Parameters
	input = get_input_by_name (filter_desc->filter_params->next->param.sval);
	log_factor = filter_desc->filter_params->next->next->param.fval;
	
	// Checks Neuron Layers
	for (num_layers = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, num_layers++)
		;
	
	if (num_layers != 1)
	{
		Erro ("Error: Wrong number of input neuron layers. The log_polar_mapping must have only one input.", "", "");
		return;
	}
	
	// Gets the Input Neuron Layer
	nl_input = filter_desc->neuron_layer_list->neuron_layer;
	
	// Gets the Input Dimentions
	wi = nl_input->dimentions.x;
	hi = nl_input->dimentions.y;
	
	// Gets the Output Neuron Layer
	nl_output = filter_desc->output;
	
	// Gets the Output Dimentions
	wo = nl_output->dimentions.x;
	ho = nl_output->dimentions.y;
	
	//rings_number = wo;
	//wedges_number = ho;
	
	rings_number = (wo >> 1);
	wedges_number = (ho << 1);
	
	radius_max = (float) wi / 2.0;
	radius_min = radius_max * exp (-2.0 * M_PI * (float) (rings_number -  1) / (float) wedges_number);

	k1 = 2.0 * M_PI / (float) wedges_number;
	k2 = log (log_factor * radius_max / radius_min) / (float) (rings_number - 1);
	k3 = radius_min / log_factor;
	
	// Gets the Center Point
	x_center = input->wxd - input->wx;
	y_center = input->wyd - input->wy;
	
	/*for (v = 0; v < wo; v++)
	{		
		for (u = 0; u < wo; u++)
		{
			log_polar_mapping (&xi, &yi, wi, hi, u, v, wo, ho, x_center, y_center, log_factor);
			
			if ((xi > 0) && (xi < wi) && (yi > 0) && (yi < hi))
				nl_output->neuron_vector[u + v * wo].output = nl_input->neuron_vector[xi + yi * wi].output;
			else
				nl_output->neuron_vector[u + v * wo].output.fval = 0.0;
		}
	}*/
	
	for (wedge = 0; wedge < wedges_number; wedge++)
	{
		v = (wedge < ho) ? wedge : (ho << 1) - wedge - 1;
		
		for (ring = 0; ring < rings_number; ring++)
		{
			u = (wedge < ho) ? (wo >> 1) + ring : (wo >> 1) - ring - 1;
			
			theta = k1 * (float) wedge - M_PI / 2.0;
			radius = k3 * exp (k2 * (float) ring) - k3;
			
			xi = (int) (radius * cos (theta) + 0.5) + x_center;
			yi = (int) (radius * sin (theta) + 0.5) + y_center;
			
			//index = ring + wedge * rings_number;
			
			index = u + v * wo;
					
			if ((xi > 0) && (xi < wi) && (yi > 0) && (yi < hi))
				nl_output->neuron_vector[index].output = nl_input->neuron_vector[xi + yi * wi].output;
			else
				nl_output->neuron_vector[index].output.fval = 0.0;
		}
	}

	return;	
}
