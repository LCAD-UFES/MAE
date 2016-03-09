#include "syn_neu.h"
#include <mae.h>

/*
*********************************************************************************
* Write neuron memory								*
*********************************************************************************
*/

int
neuron_memory_write_mem_overwrite (NEURON_LAYER *neuron_layer, int n, PATTERN *input_pattern, int connections, OUTPUT_TYPE output_type)
{
	int i, j;
	NEURON *neuron;
	NEURON *associated_neuron;

	neuron = &(neuron_layer->neuron_vector[n]);
	associated_neuron = &(neuron_layer->associated_neuron_vector[n]);

	for (j = 0; j < neuron->memory_size; j++)
	{
		if (neuron->memory[j].pattern == NULL)
			break;
		if (hamming_distance (input_pattern, neuron->memory[j].pattern, connections) == 0)
		{	/* input already seen -> overwrite */
			neuron->memory[j].associated_value = associated_neuron->output;
			return (-1);
		}
	}

	if (j < neuron->memory_size)
	{
		//neuron->memory[j].pattern = (PATTERN *) alloc_mem (N_PATTERNS * sizeof (PATTERN));
		(*(neuron_layer->neuron_type->initialize_input_pattern)) (&neuron->memory[j].pattern, connections);

		for (i = 0; i < N_PATTERNS; i++)
			neuron->memory[j].pattern[i] = input_pattern[i];
		neuron->memory[j].associated_value = associated_neuron->output;
	}
	else
	{
		j = rand () % neuron->memory_size;
		for (i = 0; i < N_PATTERNS; i++)
			neuron->memory[j].pattern[i] = input_pattern[i];
		neuron->memory[j].associated_value = associated_neuron->output;
	}
	return (j);
}


/* Implemented Neuron Types */

NEURON_TYPE mem_overwrite = {compute_input_pattern,
					 	 	 neuron_memory_read,
					 	 	 neuron_memory_write_mem_overwrite,
					 	 	 find_nearest_pattern,
					 	 	 initialise_neuron_memory,
					 	 	 initialize_input_pattern};
