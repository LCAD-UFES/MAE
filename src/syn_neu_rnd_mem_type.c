#include "syn_neu.h"
#include <mae.h>


/*
*********************************************************************************
* Initialise neuron memory with random patterns					*
*********************************************************************************
*/

void
initialise_neuron_memory_rnd (NEURON_LAYER *neuron_layer, int n)
{
	int i, j;
	NEURON *neuron;
	int connections;
	OUTPUT_TYPE output_type;
	
	neuron = &(neuron_layer->neuron_vector[n]);
	connections = neuron->n_in_connections;
	output_type = neuron_layer->output_type;

	neuron->hash = NULL;						//This neuron does not uses a hash
	
	for (i = 0; i < neuron->memory_size; i++)
	{
		//neuron->memory[i].pattern = (PATTERN *) alloc_mem (N_PATTERNS * sizeof (PATTERN));
		(*(neuron_layer->neuron_type->initialize_input_pattern)) (&neuron->memory[i].pattern, connections);
		for (j = 0; j < N_PATTERNS; j++)
			neuron->memory[i].pattern[j] = rand ();

		if (output_type == COLOR)
			neuron->memory[i].associated_value.ival = rand () % NUM_COLORS;
		else if (output_type == GREYSCALE)
			neuron->memory[i].associated_value.ival = rand () % NUM_GREYLEVELS;
		else
			neuron->memory[i].associated_value.ival = (rand () & 1) ? NUM_COLORS - 1: 0;
	}
}

NEURON_TYPE rnd_mem_type = {compute_input_pattern,
					 	 	neuron_memory_read,
					 	 	neuron_memory_write,
					 	 	find_nearest_pattern,
					 	 	initialise_neuron_memory_rnd,
					 	 	initialize_input_pattern};
