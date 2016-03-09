#include "stereo_filters.h"



/*! 
*********************************************************************************
* \brief The log polar mapping.
* \param[in] filter_desc The filter descriptor structure.		
* \pre The filter descriptor created and initialized.
* \post The updated output neuron layer.
* \return Nothing.
*********************************************************************************
*/

void log_polar_filter (FILTER_DESC *filter_desc)
{
	NEURON_LAYER_LIST *n_list;
	NEURON_LAYER *n_l;
	PARAM_LIST *p_list;
	int i, u, v, h, w, hi, wi, xi, yi, previous_xi, previous_yi, index;
	int x_center, y_center;
	INPUT_DESC *input;
	NEURON_OUTPUT previous_output;
	float fltLogFactor;
	float hAux;
	int nNumParam;

	previous_output.ival = 0;

	// Check Neuron Layers
	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++);

	if (i != 1)
	{
		Erro ("Wrong number of neuron layers. The log_polar_filter filter must be applied on only one neuron layer.", "", "");
		return;
	}
	n_l = filter_desc->neuron_layer_list->neuron_layer;

	// Achar o numero de parametros
	for (nNumParam = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, nNumParam++);
	nNumParam--;

	// Numero de neuron layers deve ser igual a tres
	if (nNumParam != 1)
	{
		Erro ("The log_polar_filter filter must have one parameter: log_factor.", "", "");
		return;
	}
	fltLogFactor = filter_desc->filter_params->next->param.fval;

	wi = n_l->dimentions.x;
	hi = n_l->dimentions.y;
	w = filter_desc->output->dimentions.x;
	h = filter_desc->output->dimentions.y;

	input = get_input_by_neural_layer (n_l);

	previous_xi = -1;
	previous_yi = -1;

	if (input == NULL)
	{
		x_center = 0;
		y_center = 0;
	}
	else
	{
		if (TYPE_MOVING_FRAME == STOP)
		{
			x_center = input->wxd - input->wx;
			y_center = input->wyd - input->wy;
		}
		else
		{
			x_center = wi/2;
			y_center = hi/2;
		}
	}

	hAux = (float) h / (float) (h - 1.0);
	for (u = 0; u < w; u++)
	{
		for (v = 0; v < h; v++)
		{
			map_v1_to_image (&xi, &yi, wi, hi, u, v, w, h, x_center, y_center, hAux, fltLogFactor);

			index = (u < (w / 2)) ? ((w / 2) - 1 - u) : (u - (w / 2));
			if ((xi == previous_xi) && (yi == previous_yi))
				filter_desc->output->neuron_vector[(v * w) + u].output = previous_output;
			else
				if (xi >= wi || xi < 0 || yi >= hi || yi < 0)
					previous_output.ival = filter_desc->output->neuron_vector[(v * w) + u].output.ival = 0;
				else
					previous_output = filter_desc->output->neuron_vector[(v * w) + u].output = input->neuron_layer->neuron_vector[yi * wi + xi].output;
			previous_xi = xi;
			previous_yi = yi;
		}
	}
}



/*! 
*********************************************************************************
* \brief The magnitude filter.
* \param[in] filter_desc The filter descriptor structure.		
* \pre The filter descriptor created and initialized.
* \post The updated output neuron layer.
* \return Nothing.
*********************************************************************************
*/

void mag_filter (FILTER_DESC *filter_desc)
{
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nl_real = NULL, *nl_imag = NULL;
	int i;
        float fltReal, fltImag, fltMaxValue = FLT_MIN, fltMinValue = FLT_MAX, fltValue, k1, k2;

	// Checks the filter parameters
	for (i = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, i++)
		;

	if (i != 1)
	{
		Erro ("The mag_filter don´t need parameters.", "", "");
		return;
	}
		
        // Checks the input neuron layers
	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
		;

	if (i != 2)
	{
		Erro ("Wrong number of neuron layers. mag_filter must be applied on two neuron layers.", "", "");
		return;
	}

	// Gets the input neuron layers
	nl_real = filter_desc->neuron_layer_list->neuron_layer;
       	nl_imag = filter_desc->neuron_layer_list->next->neuron_layer;
	
	for (i = 0; i < filter_desc->output->num_neurons; i++)
        {
                fltReal = nl_real->neuron_vector[i].output.fval;
                fltImag = nl_imag->neuron_vector[i].output.fval;

                filter_desc->output->neuron_vector[i].output.fval = fltValue = MAG(fltReal,fltImag);

                if (fltValue > fltMaxValue)
                        fltMaxValue = fltValue;

                if (fltValue < fltMinValue)
                        fltMinValue = fltValue;
        }

/*       k1 = 1.0f / (fltMaxValue - fltMinValue);

        for (i = 0; i < filter_desc->output->num_neurons; i++)
		filter_desc->output->neuron_vector[i].output.fval = k1 * (filter_desc->output->neuron_vector[i].output.fval - fltMinValue);
*/
 	k1 = 1.0f / log ((fltMaxValue + 1.0f) / (fltMinValue + 1.0f));
        k2 = log ((fltMinValue + 1.0f));

        for (i = 0; i < filter_desc->output->num_neurons; i++)
		filter_desc->output->neuron_vector[i].output.fval = k1 * (log (filter_desc->output->neuron_vector[i].output.fval + 1.0f) - k2);

        for (i = 0; i < filter_desc->output->num_neurons; i++)
	{
		fltValue = filter_desc->output->neuron_vector[i].output.fval;
		filter_desc->output->neuron_vector[i].output.fval = (fltValue >= 0.1) ? fltValue : .0f;
        }
}



/*! 
*********************************************************************************
* \brief The phase filter.
* \param[in] filter_desc The filter descriptor structure.		
* \pre The filter descriptor created and initialized.
* \post The updated output neuron layer.
* \return Nothing.
*********************************************************************************
*/

void phase_filter (FILTER_DESC *filter_desc)
{
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nl_real = NULL, *nl_imag = NULL;
	int i;
	float fltReal, fltImag;

	// Checks the filter parameters
	for (i = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, i++)
		;

	if (i != 1)
	{
		Erro ("The phase_filter don´t need parameters.", "", "");
		return;
	}
		
        // Checks the input neuron layers
	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
		;

	if (i != 2)
	{
		Erro ("Wrong number of neuron layers. phase_filter must be applied on two neuron layers.", "", "");
		return;
	}

	// Gets the input neuron layers
	nl_real = filter_desc->neuron_layer_list->neuron_layer;
       	nl_imag = filter_desc->neuron_layer_list->next->neuron_layer;
	
        for (i = 0; i < filter_desc->output->num_neurons; i++)
	{
                fltReal = nl_real->neuron_vector[i].output.fval;
                fltImag = nl_imag->neuron_vector[i].output.fval;

		filter_desc->output->neuron_vector[i].output.fval = PHASE(fltReal,fltImag);
        }
}



/*! 
*********************************************************************************
* \brief The phase difference filter.
* \param[in] filter_desc The filter descriptor structure.		
* \pre The filter descriptor created and initialized.
* \post The updated output neuron layer.
* \return Nothing.
*********************************************************************************
*/

void phase_diff_filter (FILTER_DESC *filter_desc)
{
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nl_right = NULL, *nl_left = NULL;
	int i;
	float fltPhaseDiff;

	// Checks the filter parameters
	for (i = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, i++)
		;

	if (i != 1)
	{
		Erro ("The phase_diff_filter don´t need parameters.", "", "");
		return;
	}
		
        // Checks the input neuron layers
	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
		;

	if (i != 2)
	{
		Erro ("Wrong number of neuron layers. phase_diff_filter must be applied on two neuron layers.", "", "");
		return;
	}

	// Gets the input neuron layers
	nl_right = filter_desc->neuron_layer_list->neuron_layer;
       	nl_left = filter_desc->neuron_layer_list->next->neuron_layer;
	
	for (i = 0; i < filter_desc->output->num_neurons; i++)
	{
        	fltPhaseDiff = nl_right->neuron_vector[i].output.fval - nl_left->neuron_vector[i].output.fval;

        	if (fltPhaseDiff > M_PI)
        		fltPhaseDiff -= 2.0f * M_PI;

        	if (fltPhaseDiff < -M_PI)
                	fltPhaseDiff += 2.0f * M_PI;

        	filter_desc->output->neuron_vector[i].output.fval = fltPhaseDiff;
        }
}



/*! 
*********************************************************************************
* \brief The mean magnitude filter.
* \param[in] filter_desc The filter descriptor structure.		
* \pre The filter descriptor created and initialized.
* \post The updated output neuron layer.
* \return Nothing.
*********************************************************************************
*/

void mean_mag_filter (FILTER_DESC *filter_desc)
{
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nl_right = NULL, *nl_left = NULL;
	int i;
//	float fltPhaseDiff;

	// Checks the filter parameters
	for (i = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, i++)
		;

	if (i != 1)
	{
		Erro ("The mean_mag_filter don´t need parameters.", "", "");
		return;
	}
		
        // Checks the input neuron layers
	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
		;

	if (i != 2)
	{
		Erro ("Wrong number of neuron layers. mean_mag_filter must be applied on two neuron layers.", "", "");
		return;
	}

	// Gets the input neuron layers
	nl_right = filter_desc->neuron_layer_list->neuron_layer;
       	nl_left = filter_desc->neuron_layer_list->next->neuron_layer;
	
	for (i = 0; i < filter_desc->output->num_neurons; i++)
        	filter_desc->output->neuron_vector[i].output.fval = sqrt (nl_right->neuron_vector[i].output.fval * nl_left->neuron_vector[i].output.fval);
}



/*! 
*********************************************************************************
* \brief Computes the V1 spatial receptive field modeled by a Gabor kernel.
* \param[in] receptive_field The empty receptive field descriptor structure.		
* \param[in] frequency The kernel preferred spacial frequency.
* \param[in] width The Gabor band width.
* \param[in] aspect_ratio The Gabor aspect ratio (width/height).
* \param[in] orientation The Gabor orientation.
* \param[in] phase The Gabor phase.
* \param[in] kernel_type The kernel type dimentions (KERNEL_1D, KERNEL_2D).
* \pre The receptive field descriptor created.
* \post The V1 receptive field points initialized.
* \return Nothing.
*********************************************************************************
*/

void compute_gaussian_kernel2 (RECEPTIVE_FIELD_DESCRIPTION *receptive_field, float frequency, float width, float aspect_ratio, float orientation)
{
	int x, y, kernel_radius, kernel_size, num_points = 0;
	float k, u0, v0, a, b, xr, yr;
	POINT *kernel_points;

	receptive_field->frequency = (double) frequency;
	
	// Calculates the gaussian's sigma
	a = frequency * width;
	b = a / aspect_ratio;
	
	// Calculates the kernel size
	kernel_size = (1.0 / a) * 2.0 * (1.5);
	kernel_size = (kernel_size % 2) ? kernel_size : (kernel_size + 1);

	// Allocates memory for the receptive field points
	kernel_points = (POINT *) alloc_mem (sizeof (POINT) * kernel_size * kernel_size);
	
	// Calculates the radius size of the receptive field
	kernel_radius = (kernel_size - 1) / 2;
	
	// Scale Constant 
	k = 1.0f;
	
	// Calculates the X and Y Frequency Components
	u0 = frequency * cos (orientation);
	v0 = frequency * sin (orientation);
        
	for (y = - kernel_radius; y <= kernel_radius; y++)
	{
		for (x = - kernel_radius; x <= kernel_radius; x++)
		{
			// Saves the Point Coordinates
			kernel_points[num_points].x = x;
			kernel_points[num_points].y = y;

			// Calculates the Rotated Coordinates
			xr =   x * cos (orientation) + y * sin (orientation);
			yr = - x * sin (orientation) + y * cos (orientation);
			
			// Calculates the Gabor Function Only for Points Inside the Circle Enrolled in Kernel
			kernel_points[num_points].gaussian = k * exp (- pi * (a * a * xr * xr + b * b * yr * yr));
			// Calculates the Weight Function 
			if ((x == - kernel_radius && y == - kernel_radius) || (x == kernel_radius && y == kernel_radius))
				kernel_points[num_points].weight = 1.0f; // Points at Kernel Corner (Main Diagonal)
			else if ((x == - kernel_radius && y == kernel_radius) || (x == kernel_radius && y == - kernel_radius))
				kernel_points[num_points].weight = 2.0f; // Points at Kernel Corner (Secondary Diagonal)
			else if (x == - kernel_radius || x == kernel_radius || y == - kernel_radius || y == kernel_radius)
				kernel_points[num_points].weight = 3.0f; // Points on Kernel Edge
			else
				kernel_points[num_points].weight = 6.0f; // Points inside Kernel

			// Increments the Points Number
			num_points++;
		}
	}
	
	// Saves the Receptive Field Descriptor Attributes
	receptive_field->num_points		= num_points;
	receptive_field->points			= kernel_points;
	receptive_field->cut_orientation	= orientation;
	//receptive_field->area 		= num_points;
	receptive_field->area 			= 1.0f; // Normalized area
}



/*! 
*********************************************************************************
* \brief apply_gaussian_kernel2.
* \param[in] filter_desc The filter descriptor structure.		
* \pre The filter descriptor created and initialized.
* \post The updated output neuron layer.
* \return Nothing.
*********************************************************************************
*/

float apply_gaussian_kernel2 (RECEPTIVE_FIELD_DESCRIPTION *receptive_field_descriptor, NEURON *nl_weight, NEURON *nl_image, int x_center, int y_center, int w, int h)
{
	int i, x_current, y_current;
	float fltWeight, fltWeightSum = .0f, fltResult = .0f;
		
        for (i = 0; i < receptive_field_descriptor->num_points; i++)
	{
		// Calculates the current point
		x_current = x_center + receptive_field_descriptor->points[i].x;
		y_current = y_center + receptive_field_descriptor->points[i].y;
		
		// Verifies if the point is inside of the neuron layer bounds 
		if ((x_current < 0) || (x_current >= w) || (y_current < 0) || (y_current >= h))
			continue;
					
		fltWeightSum += fltWeight = receptive_field_descriptor->points[i].weight * receptive_field_descriptor->points[i].gaussian * nl_weight[x_current + w * y_current].output.fval;
		fltResult += fltWeight * nl_image[x_current + w * y_current].output.fval;
	}
	
	fltResult = (fltWeightSum != .0f) ? (fltResult / fltWeightSum) : .0f;
	
	return (fltResult);
}



/*! 
*********************************************************************************
* \brief The spatial pooling filter.
* \param[in] filter_desc The filter descriptor structure.		
* \pre The filter descriptor created and initialized.
* \post The updated output neuron layer.
* \return Nothing.
*********************************************************************************
*/

void spatial_pooling_filter (FILTER_DESC *filter_desc)
{	
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nl_phase_diff = NULL, *nl_mean_mag = NULL;
	RECEPTIVE_FIELD_DESCRIPTION *receptive_fields = NULL;
	int i, xi, yi, hi, wi, xo, yo, ho, wo;
	float camera_opening, cycles_per_degree, frequency, width, aspect_ratio, orientation;
	int pyramid_height, pyramid_level;
	
	// Checks the Parameters
	for (i = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, i++)
		;

	if (i != 6)
	{
		Erro ("Wrong number of parameters for spatial_pooling_filter.", "", "");
		return;
	}

	// Gets the Parameters
	camera_opening		= filter_desc->filter_params->next->param.fval;
	cycles_per_degree	= filter_desc->filter_params->next->next->param.fval;
	width			= filter_desc->filter_params->next->next->next->param.fval;
	aspect_ratio		= filter_desc->filter_params->next->next->next->next->param.fval;
	orientation		= filter_desc->filter_params->next->next->next->next->next->param.fval;
	
        // Checks the input neuron layers
	for (i = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, i++)
		;

	if (i != 2)
	{
		Erro ("Wrong number of neuron layers. spatial_pooling_filter must be applied on two neuron layers.", "", "");
		return;
	}

	// Gets the input neuron layers
	nl_mean_mag = filter_desc->neuron_layer_list->neuron_layer;
       	nl_phase_diff = filter_desc->neuron_layer_list->next->neuron_layer;

	// Gets Input Neuron Layer Dimentions
	wi = nl_phase_diff->dimentions.x;
	hi = nl_phase_diff->dimentions.y;
	
	// Gets Output Neuron Layer Dimentions
	wo = filter_desc->output->dimentions.x;
	ho = filter_desc->output->dimentions.y;
		
	// Computes Kernels
	if (filter_desc->private_state == NULL)
	{
		pyramid_height = (get_input_by_name ("image_left"))->pyramid_height;
	
                receptive_fields = (RECEPTIVE_FIELD_DESCRIPTION *) alloc_mem ((pyramid_height + 1) * sizeof(RECEPTIVE_FIELD_DESCRIPTION));
                
                for (pyramid_level = 0; pyramid_level <= pyramid_height; pyramid_level++)
                {
		      // Calculates the spatial frequency (cycles per pixel)
		      frequency = (1.0f / (float) wi) * camera_opening * cycles_per_degree / pow (2.0f, (float) pyramid_level);

		      compute_gaussian_kernel2 (&(receptive_fields[pyramid_level]), frequency, width, aspect_ratio, orientation);
                }
		
                filter_desc->private_state = (void *) receptive_fields;
	}
	else
		receptive_fields = (RECEPTIVE_FIELD_DESCRIPTION *) filter_desc->private_state;

	for (yo = yi = 0; yo < ho; yo++, yi = (int) ((float) (yo * hi) / (float) ho + 0.5))
		for (xo = xi = 0; xo < wo; xo++, xi = (int) ((float) (xo * wi) / (float) wo + 0.5))
			filter_desc->output->neuron_vector[yo * wo + xo].output.fval = apply_gaussian_kernel2 (&(receptive_fields[PYRAMID_LEVEL]), nl_mean_mag->neuron_vector, nl_phase_diff->neuron_vector, xi, yi, wi, hi);
}
