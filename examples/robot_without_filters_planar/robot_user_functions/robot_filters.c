#include "mae.h"
#include "filter.h"
#include "robot_user_functions.h"
#include "robot_filters.h"





/*
*********************************************************************************
* Function: initialise_synapse2		          				*
* Description: 			                        			*
* Inputs: 				                  			*
* Output: 				                			*
*********************************************************************************
*/

void initialise_synapse2 (SYNAPSE *synapse, NEURON_LAYER *nl_source, int x, int y, NEURON_OUTPUT sensitivity)
{
     	int w, h;
	
	w = nl_source->dimentions.x;
	h = nl_source->dimentions.y;
	
	if((x >= 0) && (x < w) && (y >= 0) && (y < h))
		synapse->source = &(nl_source->neuron_vector[x + y * w]);
	else
		synapse->source = &(neuron_with_output_zero);
		
	synapse->sensitivity = sensitivity;
	synapse->nl_source = nl_source;
     	synapse->x = x;
     	synapse->y = y;
}



/*
*********************************************************************************
* Function: connect_2_neurons2		          				*
* Description: 			                        			*
* Inputs: 				                  			*
* Output: 				                			*
*********************************************************************************
*/

void connect_2_neurons2 (NEURON_LAYER *nl_source, int x, int y, NEURON *ndest, NEURON_OUTPUT synapse_sensitivity)
{
	SYNAPSE *synapse;
	
	synapse = (SYNAPSE *) alloc_mem ((size_t) sizeof (SYNAPSE));
	/* The synapse is initialized */
	initialise_synapse2 (synapse, nl_source, x, y, synapse_sensitivity);
	add_synapse (ndest, synapse);
	ndest->n_in_connections++;
}



/*
*********************************************************************************
* Function: connect_gaussrand2_neurons2		          			*
* Description: Gaussian random distribution generator         			*
* Inputs: 				                  			*
* Output: 				                			*
*********************************************************************************
*/
 
double gaussrand2(void)
{
        static double V1, V2, S;
        static int phase = 0;
        double X;

        if(phase == 0) {
                do {
                        double U1 = (double) rand() / (double) LRAND48_MAX;
                        double U2 = (double) rand() / (double) LRAND48_MAX;

                        V1 = 2 * U1 - 1;
                        V2 = 2 * U2 - 1;
                        S = V1 * V1 + V2 * V2;
                        } while(S >= 1 || S == 0);

                X = V1 * sqrt(-2 * log(S) / S);
        } else
                X = V2 * sqrt(-2 * log(S) / S);

        phase = 1 - phase;

        return X;
}



/*
*********************************************************************************
* Function: connect_neuron_layers_gaussianly2					*
* Description: 			                        			*
* Inputs: 				                  			*
* Output: 				                			*
*********************************************************************************
*/

void connect_neuron_layers_gaussianly2 (NEURON_LAYER *neuron_layer_src, NEURON_LAYER *neuron_layer_dst,
				  					int nun_inputs_per_neuron, double gaussian_radius) 
{
	int i, j, gx, gy, num_neurons;
	NEURON_OUTPUT synapse_sensitivity;
	SENSITIVITY_TYPE neuron_layer_sensitivity;

	if (neuron_layer_dst->sensitivity == NOT_SPECIFIED)
		neuron_layer_sensitivity = neuron_layer_src->output_type;
	else
		neuron_layer_sensitivity = neuron_layer_dst->sensitivity;

	num_neurons = get_num_neurons(neuron_layer_dst->dimentions);
	
	for (j = 0; j < num_neurons; j++)
	{
		for (i = 0; i < nun_inputs_per_neuron; i++)
		{
			gx = (int)(gaussrand2 () * gaussian_radius + 0.5);
			gy = (int)(gaussrand2 () * gaussian_radius + 0.5);

			switch(neuron_layer_sensitivity)
			{
				case GREYSCALE:
					synapse_sensitivity.ival = rand () % NUM_GREYLEVELS;
					break;
				case BLACK_WHITE:
					synapse_sensitivity.ival = NUM_GREYLEVELS - 1;
					break;
				case GREYSCALE_FLOAT: // Range = [-1.0 : 1.0]
					synapse_sensitivity.fval = ((float) (rand () % 100)) / 50.0 - 1.0;
					break;
				case COLOR:
					synapse_sensitivity.ival = rand () % NUM_COLORS;
					break;	
			}
			
			connect_2_neurons2 (neuron_layer_src, gx, gy, &(neuron_layer_dst->neuron_vector[j]), synapse_sensitivity);
		}
	}
}

/*
*********************************************************************************
* Function: connect_neuron_layers_gaussianly3					*
* Description: 			                        			*
* Inputs: 				                  			*
* Output: 				                			*
*********************************************************************************
*/

void connect_neuron_layers_gaussianly3 (NEURON_LAYER *neuron_layer_src, NEURON_LAYER *neuron_layer_dst,
				  					int nun_inputs_per_neuron, double gaussian_radius, 
									double log_factor) 
{
	int u, v, w, h, xi, yi, wi, hi, x_center, y_center, i, gx, gy, num_neurons;
	NEURON_OUTPUT synapse_sensitivity;
	SENSITIVITY_TYPE neuron_layer_sensitivity;
	double correction;
	
	if (neuron_layer_dst->sensitivity == NOT_SPECIFIED)
		neuron_layer_sensitivity = neuron_layer_src->output_type;
	else
		neuron_layer_sensitivity = neuron_layer_dst->sensitivity;

	num_neurons = get_num_neurons(neuron_layer_dst->dimentions);
	
	// Gets the destination neuron layer dimentions
	w = neuron_layer_dst->dimentions.x;
	h = neuron_layer_dst->dimentions.y;
	
	// Gets the source neuron layer dimentions
	wi = neuron_layer_src->dimentions.x;
	hi = neuron_layer_src->dimentions.y;

	// Calculates the correction factor
	correction = (double) h / (double) (h - 1);
	
	// Calculates the source neuron layer center
	x_center = wi >> 1;
	y_center = hi >> 1;
	
	for (v = 0; v < h; v++)
	{
		for (u = 0; u < w; u++)
		{
			map_v1_to_image (&xi, &yi, wi, hi, u, v, w, h, x_center, y_center, correction, log_factor);

			for (i = 0; i < nun_inputs_per_neuron; i++)
			{
				gx = (int)(gaussrand2 () * gaussian_radius + 0.5) + xi;
				gy = (int)(gaussrand2 () * gaussian_radius + 0.5) + yi;

				switch(neuron_layer_sensitivity)
				{
					case GREYSCALE:
						synapse_sensitivity.ival = rand () % NUM_GREYLEVELS;
						break;
					case BLACK_WHITE:
						synapse_sensitivity.ival = NUM_GREYLEVELS - 1;
						break;
					case GREYSCALE_FLOAT: // Range = [-1.0 : 1.0]
						synapse_sensitivity.fval = ((float) (rand () % 100)) / 50.0 - 1.0;
						break;
					case COLOR:
						synapse_sensitivity.ival = rand () % NUM_COLORS;
						break;	
				}
			
				connect_2_neurons2 (neuron_layer_src, gx, gy, &(neuron_layer_dst->neuron_vector[u + v * w]), synapse_sensitivity);
			}
		}
	}
}


/*
*********************************************************************************
* Function: compute_input_pattern_minchinton_black_white_with_offset  		*
* Description: Compute the input pattern seen by minchinton_black_white neuron	*
* Inputs: 				                  			*
* Output: 				                			*
*********************************************************************************
*/

void compute_input_pattern_minchinton_black_white_with_offset (NEURON *neuron, PATTERN *input_pattern, 
						   SENSITIVITY_TYPE sensitivity, OUTPUT_TYPE output_type,
						   int x_offset, int y_offset)
{
	SYNAPSE_LIST *s_list;
	int num_synapses, i;
	int bit_value = 0;
	NEURON_LAYER *nl_source1 = NULL, *nl_source2 = NULL;
	NEURON *nsrc1 = NULL, *nsrc2 = NULL;
	int x1, y1, w1, h1, x2, y2, w2, h2;
	
	nsrc1 = nsrc2 = &(neuron_with_output_zero);
	
	i = -1;
	num_synapses = 0;
	for (s_list = ((SYNAPSE_LIST *) (neuron->synapses)); s_list != (SYNAPSE_LIST *) NULL; s_list = s_list->next)
	{
		if ((num_synapses % PATTERN_UNIT_SIZE) == 0)
		{
			i++;
			input_pattern[i] = 0;
		}

		input_pattern[i] = input_pattern[i] >> 1;

		if ((nl_source1 = s_list->synapse->nl_source) != NULL)
		{
			w1 = nl_source1->dimentions.x;
			h1 = nl_source1->dimentions.y;
			x1 = s_list->synapse->x + x_offset;
			y1 = s_list->synapse->y + y_offset;
			if ((x1 >= 0) && (x1 < w1) && (y1 >= 0) && (y1 < h1))
				nsrc1 = &(nl_source1->neuron_vector[x1 + w1 * y1]);
		}
			
		if (s_list->next != NULL)
		{
			if ((nl_source2 = s_list->next->synapse->nl_source) != NULL)
			{
				w2 = nl_source2->dimentions.x;
				h2 = nl_source2->dimentions.y;
				x2 = s_list->next->synapse->x + x_offset;
				y2 = s_list->next->synapse->y + y_offset;
				if ((x2 >= 0) && (x2 < w2) && (y2 >= 0) && (y2 < h2))
					nsrc2 = &(nl_source2->neuron_vector[x2 + w2 * y2]);
			}
		}
		else
		{
			if((nl_source2 = ((SYNAPSE_LIST *) (neuron->synapses))->synapse->nl_source) != NULL)
			{
				w2 = nl_source2->dimentions.x;
				h2 = nl_source2->dimentions.y;
				x2 = ((SYNAPSE_LIST *) (neuron->synapses))->synapse->x + x_offset;
				y2 = ((SYNAPSE_LIST *) (neuron->synapses))->synapse->y + y_offset;
				if ((x2 >= 0) && (x2 < w2) && (y2 >= 0) && (y2 < h2))
					nsrc2 = &(nl_source2->neuron_vector[x2 + w2 * y2]);
			}
		}
			
		switch(output_type)
		{
			case GREYSCALE:
			case BLACK_WHITE:
			case COLOR:
				bit_value = (nsrc1->output.ival < nsrc2->output.ival) ? 1 : 0;
				break;
			case GREYSCALE_FLOAT:
				switch (g_nNetworkStatus)
				{
					case TRAINNING:
						bit_value = (nsrc1->output.fval + THRESHOLD < nsrc2->output.fval) ? 1 : 0;
						break;
					case RUNNING:
						bit_value = (nsrc1->output.fval + THRESHOLD < nsrc2->output.fval) ? 1 : 0;
						break;
				}
				break;
		}

		input_pattern[i] |= bit_value << (PATTERN_UNIT_SIZE - 1);
		
		s_list->synapse->source = nsrc1;

		num_synapses++;
	}
	if (num_synapses % PATTERN_UNIT_SIZE)
		input_pattern[i] = input_pattern[i] >> (PATTERN_UNIT_SIZE - (num_synapses % PATTERN_UNIT_SIZE));
}



/*
*********************************************************************************
* Function: evaluate_dendrite_with_offset  					*
* Description: Evaluate dendrite						*
* Inputs: 				                  			*
* Output: 				                			*
*********************************************************************************
*/

NEURON_OUTPUT evaluate_dendrite_with_offset (NEURON_LAYER *neuron_layer, int n, int x_offset, int y_offset)
{
	PATTERN input_pattern [MAX_CONNECTIONS / PATTERN_UNIT_SIZE];
	NEURON_OUTPUT d_state;
	int connections;
	OUTPUT_TYPE output_type;
	NEURON *neuron;
	SENSITIVITY_TYPE neuron_layer_sensitivity;
	NEURON_LAYER *nl_source = NULL;
	int x, y, w, h;

	d_state = neuron_with_output_zero.output;
	
	neuron = &(neuron_layer->neuron_vector[n]);
	connections = neuron->n_in_connections;
	output_type = neuron_layer->output_type;
	
	if ((connections / PATTERN_UNIT_SIZE) > (MAX_CONNECTIONS / PATTERN_UNIT_SIZE))
		Erro ("max number of connections exceeded", "", "");
		 
	if (((SYNAPSE_LIST *) (neuron->synapses)) == NULL)
	{
		d_state.ival = NEVER_COMPUTED;
		return d_state;
	}
		
	if (((SYNAPSE_LIST *) (neuron->synapses))->synapse->sensitivity.ival == DIRECT_CONNECTION)
	{
		if ((nl_source = ((SYNAPSE_LIST *) (neuron->synapses))->synapse->nl_source) != NULL)
		{
			w = nl_source->dimentions.x;
			h = nl_source->dimentions.y;
			x = ((SYNAPSE_LIST *) (neuron->synapses))->synapse->x + x_offset;
			y = ((SYNAPSE_LIST *) (neuron->synapses))->synapse->y + y_offset;
			
			if ((x >= 0) && (x < w) && (y >= 0) && (y < h))
				d_state = nl_source->neuron_vector[x + w * y].output;
		}
		
		switch(output_type)
		{
			case GREYSCALE:
				d_state.ival = ((d_state.ival >= NUM_GREYLEVELS) ? (NUM_GREYLEVELS - 1) : d_state.ival);
				return d_state;
			case BLACK_WHITE:
				d_state.ival = ((d_state.ival >= NUM_GREYLEVELS / 2) ? (NUM_GREYLEVELS - 1) : 0);
				return d_state;
			case GREYSCALE_FLOAT: 
				d_state.fval = ((d_state.fval >= (float) NUM_GREYLEVELS) ? (float) (NUM_GREYLEVELS - 1): d_state.fval);
				return d_state;
			case COLOR:
				d_state.ival = ((d_state.ival >= NUM_COLORS) ? (NUM_COLORS - 1): d_state.ival);
				return d_state;
		}
	}
	
	neuron_layer_sensitivity = neuron_layer->sensitivity;
	
	//(*(neuron_layer->neuron_type->compute_input_pattern)) (neuron, input_pattern, neuron_layer_sensitivity, output_type);
	compute_input_pattern_minchinton_black_white_with_offset (neuron, input_pattern, neuron_layer_sensitivity, output_type, x_offset, y_offset);
	d_state = (*(neuron_layer->neuron_type->neuron_memory_read)) (neuron_layer, n, input_pattern, connections, output_type);
	return d_state;
} 



/*
*********************************************************************************
* Function: train_neuron_with_offset                				*
* Description: 			                        			*
* Inputs: 				                  			*
* Output: 				                			*
*********************************************************************************
*/

void train_neuron_with_offset (NEURON_LAYER *neuron_layer, int n, int x_offset, int y_offset)
{
	PATTERN input_pattern[MAX_CONNECTIONS/PATTERN_UNIT_SIZE];
	int connections;
	SENSITIVITY_TYPE sensitivity;
	OUTPUT_TYPE output_type;
	NEURON *neuron;
	
	neuron = &(neuron_layer->neuron_vector[n]);
	connections = neuron->n_in_connections;
	
	if ((connections/PATTERN_UNIT_SIZE) > (MAX_CONNECTIONS/PATTERN_UNIT_SIZE))
		Erro ("max number of connections exceeded", "", "");

	sensitivity = neuron_layer->sensitivity;
	output_type = neuron_layer->output_type;
	//(*(neuron_layer->neuron_type->compute_input_pattern)) (neuron, input_pattern, sensitivity, output_type);
	compute_input_pattern_minchinton_black_white_with_offset (neuron, input_pattern, sensitivity, output_type, x_offset, y_offset);
	(*(neuron_layer->neuron_type->neuron_memory_write)) (neuron_layer, n, input_pattern, connections, output_type);
}



/*
*********************************************************************************
* Function: mt_filter		                 				*
* Description: 			                        			*
* Inputs: 				                  			*
* Output: 				                			*
*********************************************************************************
*/

void mt_filter (FILTER_DESC *filter_desc)
{
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nlMT = NULL, *nlImgLeft = NULL, *nlImgRight = NULL, *nlImg = NULL;
	INPUT_DESC *inImgLeft = NULL, *inImgRight;
	int i, num_neurons, xi, yi, wi, hi, xo, yo, wo, ho, x_center, y_center, nl_number, p_number, nInputsPerNeuron;
	float fltGaussianRadius, fltLogFactor;
	static int flag = 0;
	//RECEPTIVE_FIELD_DESCRIPTION * receptive_field = NULL;

	// Checks the neuron layers number
	for (nl_number = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, nl_number++)
            	;

	if (nl_number != 3)
	{
		Erro ("Error: Wrong number of input neuron layers. The mt_filter must have two neuron layers: the left and right input layers.", "", "");
		return;
	}
	
	// Checks the parameters number
	for (p_number = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, p_number++)
            	;

	if (p_number != 4)
	{
		Erro ("Error: Wrong number of parameters. The mt_filter must have tree parameter <nInputsPerNeuron> <fltGaussianRadius> <fltLogFactor>.", "", "");
		return;
	}
	
	// Gets the filter parameters
	nInputsPerNeuron  = filter_desc->filter_params->next->param.ival;
	fltGaussianRadius = filter_desc->filter_params->next->next->param.fval;
	fltLogFactor  	  = filter_desc->filter_params->next->next->next->param.fval;
	
	// Gets the filter output 
	nlMT = filter_desc->output;
	
	// Gets the output neurons number
	num_neurons = get_num_neurons(nlMT->dimentions);

	// Gets the input neuron layers
	nlImgLeft  = filter_desc->neuron_layer_list->neuron_layer;
	nlImgRight = filter_desc->neuron_layer_list->next->neuron_layer;
	nlImg      = filter_desc->neuron_layer_list->next->next->neuron_layer;

	// Gets the associated inputs
        inImgLeft  = get_input_by_neural_layer (nlImgLeft);
	inImgRight = get_input_by_neural_layer (nlImgRight);
	
	// Gets the inputs dimentions
	wi = nlImgLeft->dimentions.x;
	hi = nlImgLeft->dimentions.y;

	// Gets the auxiliary layer dimentions
	wo = nlImg->dimentions.x;
	ho = nlImg->dimentions.y;
	 	
	// Creates the connection pattern
	if (flag == 0)
	{	
		//connect_neuron_layers_gaussianly3 (nlImg, nlMT, nInputsPerNeuron, fltGaussianRadius, fltLogFactor);
		flag++;
	}
	
	switch (g_nNetworkStatus)
	{
		case TRAINNING:
			x_center = (inImgLeft->wxd - inImgLeft->wx);
			y_center = (inImgLeft->wyd - inImgLeft->wy);
			
			for (yo = 0; yo < ho; yo++)
			{
				for (xo = 0; xo < wo; xo++)
				{
					xi = xo + x_center - (wi >> 1);
					yi = yo + y_center - (hi >> 1);

					if (xi >= 0 && xi < wi && yi >= 0 && yi < hi)
						nlImg->neuron_vector[yo * wo + xo].output = nlImgLeft->neuron_vector[yi * wi + xi].output;
					else
						nlImg->neuron_vector[yo * wo + xo].output.fval = .0f;
				}
			}
			
			for (i = 0; i < num_neurons; i++)
				train_neuron (nlMT, i);
			break;
		case RUNNING:
			x_center = (inImgRight->wxd - inImgRight->wx);
			y_center = (inImgRight->wyd - inImgRight->wy);
			
			for (yo = 0; yo < ho; yo++)
			{
				for (xo = 0; xo < wo; xo++)
				{
					xi = xo + x_center - (wi >> 1);
					yi = yo + y_center - (hi >> 1);

					if (xi >= 0 && xi < wi && yi >= 0 && yi < hi)
						nlImg->neuron_vector[yo * wo + xo].output = nlImgRight->neuron_vector[yi * wi + xi].output;
					else
						nlImg->neuron_vector[yo * wo + xo].output.fval = .0f;
				}
			}
			
			for (i = 0; i < num_neurons; i++)
			{
				if (nlMT->neuron_vector[i].memory != NULL)
					nlMT->neuron_vector[i].output = evaluate_dendrite (nlMT, i);
			}
			break;
	}
}

