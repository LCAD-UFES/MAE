#include "syn_neu.h"
#include <mae.h>


/*
*********************************************************************************
* Calculate the Hamming distance between two labels			*
*********************************************************************************
*/

inline int
hamming_distance_label (PATTERN *pattern1, PATTERN *pattern2, int connections)
{
	int i, distance;

	distance = 0;
	if (pattern1 != NULL && pattern2 != NULL)
		for (i = 0; i < connections; i++)
			if (pattern1[i] != pattern2[i])
				distance++;

	return (distance);
}


/*
*********************************************************************************
* Compute the input pattern seen by the label neuron				*
*********************************************************************************
*/

void
compute_input_pattern_label (NEURON *neuron, PATTERN *input_pattern, SENSITIVITY_TYPE sensitivity, OUTPUT_TYPE output_type)
{
	SYNAPSE_LIST *s_list;
	int i;
	PATTERN byte_value;
	SENSITIVITY_TYPE neuron_layer_sensitivity;

	if (sensitivity == NOT_SPECIFIED)
		neuron_layer_sensitivity = output_type;
	else
		neuron_layer_sensitivity = sensitivity;

	for (s_list = ((SYNAPSE_LIST *) (neuron->synapses)), i = 0; s_list != NULL; s_list = s_list->next, i++)
	{

		switch (neuron_layer_sensitivity)
		{
			case BLACK_WHITE:
			case GREYSCALE:
				byte_value = (unsigned char)s_list->synapse->source->output.ival;
				break;
			default:
				byte_value = 0;
				Erro ("output_type not supported by compute_input_pattern_label", "", "");
				break;
		}

		input_pattern[i] = byte_value;

	}

}


/*
*********************************************************************************
* Find the nearest pattern in memory						*
*********************************************************************************
*/

int
find_nearest_pattern_label (NEURON *neuron, PATTERN *input_pattern, int connections, int *distance)
{
	int cur_ham_dist, new_ham_dist;
	int nearest_pattern;
	int i;
	int candidates[NEURON_MEMORY_SIZE];
	//int *candidates;
	int num_candidates = 1;
	ASSOCIATION *memory;

	//candidates = (int *) alloc_mem(neuron->memory_size * sizeof(int));

	memory = neuron->memory;

	cur_ham_dist = MAX_CONNECTIONS - 1;
	for (i = 0; (i < neuron->memory_size) && (memory[i].pattern != NULL); i++)
	{
		new_ham_dist = hamming_distance_label (memory[i].pattern, input_pattern, connections);
		if (new_ham_dist < cur_ham_dist)
		{
			candidates[0] = i;
			num_candidates = 1;
			cur_ham_dist = new_ham_dist;
		}
		else if (new_ham_dist == cur_ham_dist)
		{
			candidates[num_candidates] = i;
			num_candidates++;
		}
	}

	if (memory[0].pattern == NULL)
		nearest_pattern = 0;
	else
		nearest_pattern = candidates[rand() % num_candidates];

	neuron->last_num_candidates = num_candidates;

	*distance = cur_ham_dist;
	//free(candidates);
	return (nearest_pattern);
}


/*
*********************************************************************************
* Write neuron memory label							*
*********************************************************************************
*/

int
neuron_memory_write_label (NEURON_LAYER *neuron_layer, int n, PATTERN *input_pattern, int connections, OUTPUT_TYPE output_type)
{
	int i, j = -1;
	int nearest_pattern;
	int distance;
	NEURON *neuron;
	NEURON *associated_neuron;

	neuron = &(neuron_layer->neuron_vector[n]);
	associated_neuron = &(neuron_layer->associated_neuron_vector[n]);

	if (neuron->memory[neuron->memory_size-1].pattern == NULL)
	{
		j = get_neuron_memory_size(neuron);

		//neuron->memory[j].pattern = (PATTERN *) alloc_mem (connections * sizeof (PATTERN));
		(*(neuron_layer->neuron_type->initialize_input_pattern)) (&neuron->memory[j].pattern, connections);

		for (i = 0; i < connections; i++)
			neuron->memory[j].pattern[i] = input_pattern[i];
		neuron->memory[j].associated_value = associated_neuron->output;
	}
	else
	{
		if ((rand () % 100) < 5)
		{	/* 5% of probability of changing the nearest pattern */
			nearest_pattern = find_nearest_pattern_label (neuron, input_pattern, connections, &distance);
			for (i = 0; i < connections; i++)
				neuron->memory[nearest_pattern].pattern[i] = input_pattern[i];
			neuron->memory[nearest_pattern].associated_value = associated_neuron->output;
		}
		else
		{
			j = rand () % neuron->memory_size;
			for (i = 0; i < connections; i++)
				neuron->memory[j].pattern[i] = input_pattern[i];
			neuron->memory[j].associated_value = associated_neuron->output;
		}
	}
	return (j);
}


void
initialize_input_pattern_label (PATTERN **input_pattern, int connections)
{
	if (connections > MAX_CONNECTIONS)
		Erro ("max number of connections exceeded", "", "");

	if ((input_pattern != NULL) && ((*input_pattern) == NULL))
		(*input_pattern) = (PATTERN *) alloc_mem (connections * sizeof (PATTERN));
}


/* Implemented Neuron Types */

NEURON_TYPE label = {compute_input_pattern_label,
					 neuron_memory_read,
					 neuron_memory_write_label,
					 find_nearest_pattern_label,
					 initialise_neuron_memory,
					 initialize_input_pattern_label};

