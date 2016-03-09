#include "visual_search_filters.h"



/*
*********************************************************************************
* Function: initialise_memory          						*
* Description:									*
* Inputs: 				                  			*
* Output: 				                			*
*********************************************************************************
*/

/*void initialise_memory (void)
{
	NEURON_LAYER_LIST *nl_list;
	NEURON *neuron;
	long num_neurons;
	long i, j;
	ASSOCIATION *memory = NULL;
	int memory_type;
	
	for (nl_list = global_neuron_layer_list; nl_list != (NEURON_LAYER_LIST *) NULL;
		nl_list = nl_list->next)
	{
		num_neurons = get_num_neurons (nl_list->neuron_layer->dimentions);
		//memory_type = nl_list->neuron_layer->memory_type; -> a new field in the neuron layer struct
		memory_type = SHARED;
		
		switch (memory_type)
			case DISTRIBUTED:
				for (i = 0; i < num_neurons; i++)
				{
					neuron = &(nl_list->neuron_layer->neuron_vector[i]);
					neuron->memory = (ASSOCIATION *) alloc_mem (sizeof (ASSOCIATION) * NEURON_MEMORY_SIZE);
					for (j = 0; j < NEURON_MEMORY_SIZE; j++)
						neuron->memory[j].pattern = NULL;
				
					if ((nl_list->neuron_layer->neuron_type != NULL) && 
			    			(nl_list->neuron_layer->neuron_type->initialise_neuron_memory  != NULL))
						(*(nl_list->neuron_layer->neuron_type->initialise_neuron_memory)) (nl_list->neuron_layer, i);
				}
				break;
			case SHARED:
				memory = (ASSOCIATION *) alloc_mem (sizeof (ASSOCIATION) * NEURON_MEMORY_SIZE);
				for (j = 0; j < NEURON_MEMORY_SIZE; j++)
					memory[j].pattern = NULL;
		
				for (i = 0; i < num_neurons; i++)
				{
					neuron = &(nl_list->neuron_layer->neuron_vector[i]);
					neuron->memory = memory;
				}
		
				if ((nl_list->neuron_layer->neuron_type != NULL) && 
		  		    (nl_list->neuron_layer->neuron_type->initialise_neuron_memory  != NULL))
		   		    (*(nl_list->neuron_layer->neuron_type->initialise_neuron_memory)) (nl_list->neuron_layer, 0);
				break;
		}
	}
}*/



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
		synapse->source = &(nl_source->neuron_vector[x + y * nl_source->dimentions.x]);
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
* Function: connect_neuron_layers_gaussianly_with_uniform_connection_pattern	*
* Description: 			                        			*
* Inputs: 				                  			*
* Output: 				                			*
*********************************************************************************
*/

void connect_neuron_layers_gaussianly_with_uniform_connection_pattern (NEURON_LAYER *neuron_layer_src, NEURON_LAYER *neuron_layer_dst,
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
	
	for (i = 0; i < nun_inputs_per_neuron; i++)
	{
		gx = (int)(gaussrand2 () * gaussian_radius + 0.5);
		gy = (int)(gaussrand2 () * gaussian_radius + 0.5);
		//printf ("%d %d\n", gx, gy);

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
		
		for (j = 0; j < num_neurons; j++)
			connect_2_neurons2 (neuron_layer_src, gx, gy, &(neuron_layer_dst->neuron_vector[j]), synapse_sensitivity);
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
				bit_value = (nsrc1->output.fval < nsrc2->output.fval) ? 1 : 0;
				break;
		}

		input_pattern[i] |= bit_value << (PATTERN_UNIT_SIZE - 1);

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
	NEURON_LAYER *nlOutput = NULL, *nlInput = NULL, *nlInput_filtered = NULL;
	INPUT_DESC *input = NULL;
	int i, xi, yi, wi, hi, u, v, h, w, x_center, y_center, nl_number, p_number, nInputsPerNeuron, nIndex, num_neurons;
	float fltGaussianRadius, fltLogFactor;
	static int flag = 0;
	double correction;
	ASSOCIATION *memory = NULL;

	// Checks the Neuron Layers Number
	for (nl_number = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, nl_number++)
            	;

	// Checks the Parameters Number
	for (p_number = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, p_number++)
            	;

	if (p_number != 4)
	{
		Erro ("Error: Wrong number of parameters. The activation_map_filter must have only one parameter <nInputsPerNeuron> <fltGaussianRadius> <fltLogFactor>.", "", "");
		return;
	}
	
	// Gets the Filter Parameters
	nInputsPerNeuron   = filter_desc->filter_params->next->param.ival;
	fltGaussianRadius  = filter_desc->filter_params->next->next->param.fval;
	fltLogFactor  	   = filter_desc->filter_params->next->next->next->param.fval;
	
	//printf ("Inputs per neuron=%d\n", nInputsPerNeuron);
	//printf ("Gaussian radius=%f\n", fltGaussianRadius);
	//printf ("Log factor=%f\n", fltLogFactor);
	
	// Gets the Filter Output 
	nlOutput = filter_desc->output;

	// Gets the Filter Output Dimentions
	w = nlOutput->dimentions.x;
	h = nlOutput->dimentions.y;
	
	// Gets the Input Neuron Layer
	nlInput = filter_desc->neuron_layer_list->neuron_layer;
	nlInput_filtered = filter_desc->neuron_layer_list->next->neuron_layer;
	
	// Gets the Input Neuron Layer Dimentions
	wi = nlInput->dimentions.x;
	hi = nlInput->dimentions.y;
	
	correction = (double) h / (double) (h - 1);

        input = get_input_by_neural_layer (nlInput);
	
	if (input == NULL)
	{
		x_center = 0;
		y_center = 0;
	}
	else
	{
		if (TYPE_MOVING_FRAME == STOP)
		{
			x_center = (input->wxd - input->wx);
			y_center = (input->wyd - input->wy);
		}
		else
		{
			x_center = wi/2;
			y_center = hi/2;
		}
	}
	
	// Initializes the shared memory and creates the connection pattern
	if (!flag)
	{
		memory = (ASSOCIATION *) alloc_mem (sizeof (ASSOCIATION) * NEURON_MEMORY_SIZE);
		num_neurons = get_num_neurons (nlOutput->dimentions);
		
		for (i = 0; i < NEURON_MEMORY_SIZE; i++)
			memory[i].pattern = NULL;
		
		for (i = 0; i < num_neurons; i++)
			nlOutput->neuron_vector[i].memory = memory;
		
		if ((nlOutput->neuron_type != NULL) && 
		    (nlOutput->neuron_type->initialise_neuron_memory  != NULL))
		    (*(nlOutput->neuron_type->initialise_neuron_memory)) (nlOutput, 0);
		
		connect_neuron_layers_gaussianly_with_uniform_connection_pattern (nlInput_filtered, nlOutput, nInputsPerNeuron, fltGaussianRadius);
		
		flag++;
	}

	switch (g_nNetworkStatus)
	{
		case TRAINNING:
			for (v = 0; v < h; v++)
			{
				for (u = 0; u < w; u++)
				{
					map_v1_to_image (&xi, &yi, wi, hi, u, v, w, h, x_center, y_center, correction, fltLogFactor);
					nIndex = u + v * w;
					train_neuron_with_offset (nlOutput, nIndex, xi, yi);
				}
			}
			break;
		case RUNNING:
			for (v = 0; v < h; v++)
			{
				for (u = 0; u < w; u++)
				{
					map_v1_to_image (&xi, &yi, wi, hi, u, v, w, h, x_center, y_center, correction, fltLogFactor);
					nIndex = u + v * w;
					nlOutput->neuron_vector[nIndex].output = evaluate_dendrite_with_offset (nlOutput, nIndex, xi, yi);
				}
			}
			break;
			
	}
}
