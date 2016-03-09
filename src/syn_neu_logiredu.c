#include "syn_neu.h"
#include <mae.h>


/*
*********************************************************************************
* Calculate the Hamming distance between two bit patterns			*
*********************************************************************************
*/

inline int
hamming_distance_logiredu (PATTERN *pattern1, PATTERN *pattern2, PATTERN *valid_bits, int connections)
{
	int i, distance;

	distance = 0;
	for (i = 0; i < N_PATTERNS; i++)
		distance += pattern_xor_table[(pattern1[i] ^ pattern2[i]) & valid_bits[i]];

	return (distance);
}


/*
*********************************************************************************
* Calculate the Hamming distance between two bit patterns (2)			*
*********************************************************************************
*/

inline int
hamming_distance_logiredu2 (PATTERN *pattern1, PATTERN *pattern2, PATTERN *valid_bits1, PATTERN *valid_bits2, int connections)
{
	int i, distance, valid_bits_distance;

	valid_bits_distance = distance = 0;
	for (i = 0; i < N_PATTERNS; i++)
	{
		distance += pattern_xor_table[(pattern1[i] ^ pattern2[i]) & (valid_bits1[i] & valid_bits2[i])];
		valid_bits_distance += pattern_xor_table[valid_bits1[i] ^ valid_bits2[i]];
	}
	if (valid_bits_distance > 1)
		distance += valid_bits_distance;
	return (distance);
}


/*
*********************************************************************************
* Find the nearest pattern in memory						*
*********************************************************************************
*/

int
find_nearest_pattern_logiredu (NEURON *neuron, PATTERN *input_pattern, int connections, int *distance)
{
	int cur_ham_dist, new_ham_dist;
	int nearest_pattern;
	int i;
	int *candidates;
	int num_candidates = 1;
	int flag = 0;
	ASSOCIATION *memory;

	candidates = (int *) alloc_mem(neuron->memory_size * sizeof(int));

	memory = neuron->memory;

	cur_ham_dist = MAX_CONNECTIONS - 1;
	for (i = 0; i < neuron->memory_size; i++)
	{
		if (memory[i].pattern != NULL)
		{
			new_ham_dist = hamming_distance_logiredu (memory[i].pattern, input_pattern, memory[i].valid_bits, connections);
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
			flag = 1;
		}
	}

	if (flag == 0)
	{
		nearest_pattern = 0;
		*distance = connections;
	}
	else
	{
		nearest_pattern = candidates[rand() % num_candidates];
		*distance = cur_ham_dist;
	}

	neuron->last_num_candidates = num_candidates;

	free(candidates);
	return (nearest_pattern);
}


/*
*********************************************************************************
* Find the nearest pattern in memory						*
*********************************************************************************
*/

int
find_nearest_pattern_logiredu2 (NEURON *neuron, int reference_pattern, int connections, int *distance)
{
	int cur_ham_dist, new_ham_dist;
	int nearest_pattern;
	int i;
	int *candidates;
	int num_candidates = 1;
	int flag = 0;
	ASSOCIATION *memory;

	candidates = (int *) alloc_mem(neuron->memory_size * sizeof(int));

	memory = neuron->memory;

	cur_ham_dist = MAX_CONNECTIONS - 1;
	for (i = 0; i < neuron->memory_size; i++)
	{
		if (memory[i].pattern != NULL)
		{
			new_ham_dist = hamming_distance_logiredu2 (memory[i].pattern, memory[reference_pattern].pattern, memory[i].valid_bits, memory[reference_pattern].valid_bits, connections);
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
			flag = 1;
		}
	}

	if (flag == 0)
	{
		nearest_pattern = 0;
		*distance = connections;
	}
	else
	{
		nearest_pattern = candidates[rand() % num_candidates];
		*distance = cur_ham_dist;
	}

	neuron->last_num_candidates = num_candidates;

	free(candidates);
	return (nearest_pattern);
}


/*
*********************************************************************************
* Remove redundance from neuron memory						*
*********************************************************************************
*/

void
make_bit_redundant (ASSOCIATION *memory, PATTERN *input_pattern, int nearest_pattern, int connections)
{
	int i;
	PATTERN p_xor;

	for (i = 0; i < N_PATTERNS; i++)
	{
		p_xor = (memory[nearest_pattern].pattern[i] ^ input_pattern[i]) & memory[nearest_pattern].valid_bits[i]; 	/* find the redundant bit */
		if (p_xor)
		{
			memory[nearest_pattern].valid_bits[i] &= ~p_xor;	/* make the bit redundant */
			break;
		}
	}
}


/*
*********************************************************************************
* Remove redundance from neuron memory						*
*********************************************************************************
*/

void
remove_redundance (NEURON *neuron, PATTERN *input_pattern, int nearest_pattern, int connections)
{
	int distance;
	int redundant_pattern_found;
	int new_nearest_pattern;
	ASSOCIATION *memory;

	memory = neuron->memory;

	make_bit_redundant (memory, input_pattern, nearest_pattern, connections);
	do
	{
		redundant_pattern_found = 0;
		new_nearest_pattern = find_nearest_pattern_logiredu2 (neuron, nearest_pattern, connections, &distance);
		if ((distance == 1) && (memory[new_nearest_pattern].associated_value.ival == memory[nearest_pattern].associated_value.ival))
		{
			free (memory[new_nearest_pattern].pattern);
			free (memory[new_nearest_pattern].valid_bits);
			memory[new_nearest_pattern].pattern = NULL;
			memory[new_nearest_pattern].valid_bits = NULL;
			redundant_pattern_found = 1;
		}
	}while (redundant_pattern_found);
}


/*
*********************************************************************************
* Read neuron memory								*
*********************************************************************************
*/

NEURON_OUTPUT
neuron_memory_read_logiredu (NEURON_LAYER *neuron_layer, int n, PATTERN *input_pattern, int connections, OUTPUT_TYPE output_type)
{
	int nearest_pattern;
	int distance;
	NEURON *neuron;

	neuron = &(neuron_layer->neuron_vector[n]);
	nearest_pattern = find_nearest_pattern_logiredu (neuron, input_pattern, connections, &distance);
	neuron->last_hamming_distance = distance;
	neuron->last_best_pattern = nearest_pattern;

	measuraments.neuron_mem_access++;
	measuraments.neuron_mem_hits_at_distance[distance]++;

	return (neuron->memory[nearest_pattern].associated_value);
}


/*
*********************************************************************************
* Write logiredu neuron memory							*
*********************************************************************************
*/

int
neuron_memory_write_logiredu (NEURON_LAYER *neuron_layer, int n, PATTERN *input_pattern, int connections, OUTPUT_TYPE output_type)
{
	int i, j;
	int nearest_pattern;
	int distance;
	NEURON *neuron;
	NEURON *associated_neuron;

	neuron = &(neuron_layer->neuron_vector[n]);
	associated_neuron = &(neuron_layer->associated_neuron_vector[n]);

	nearest_pattern = find_nearest_pattern_logiredu (neuron, input_pattern, connections, &distance);

	if ((distance > 1) || (neuron->memory[nearest_pattern].associated_value.ival != associated_neuron->output.ival))
	{ /* new independent pattern -> store */
		j = get_neuron_memory_size(neuron);

		if (j != neuron->memory_size)
		{
			//neuron->memory[j].pattern = (PATTERN *) alloc_mem (N_PATTERNS * sizeof (PATTERN));
			(*(neuron_layer->neuron_type->initialize_input_pattern)) (&neuron->memory[j].pattern, connections);
			//neuron->memory[j].valid_bits = (PATTERN *) alloc_mem (N_PATTERNS * sizeof (PATTERN));
			(*(neuron_layer->neuron_type->initialize_input_pattern)) (&neuron->memory[j].valid_bits, connections);

			for (i = 0; i < N_PATTERNS; i++)
			{
				neuron->memory[j].pattern[i] = input_pattern[i];
				//neuron->memory[j].valid_bits[i] = (PATTERN) 0xffffffffffffffff;
				neuron->memory[j].valid_bits[i] = (PATTERN) 0xff;
			}
			neuron->memory[j].associated_value = associated_neuron->output;
		}
		else
		{
			j = rand () % neuron->memory_size;
			for (i = 0; i < N_PATTERNS; i++)
			{
				neuron->memory[j].pattern[i] = input_pattern[i];
				//neuron->memory[j].valid_bits[i] = (PATTERN) 0xffffffffffffffff;
				neuron->memory[j].valid_bits[i] = (PATTERN) 0xff;
			}
			neuron->memory[j].associated_value = associated_neuron->output;
		}
		return (j);
	}
	else if (distance == 1)
	{ /* same outputs and redundant bit in pattern -> do not store and remove redundancy from memory */
		remove_redundance (neuron, input_pattern, nearest_pattern, connections);
		return -1;
	}
	else
	{ /* same outputs and distance == 0, pattern already memorized -> do nothing */
		return -1;
	}
}


/* Implemented Neuron Types */

NEURON_TYPE logiredu = 	{compute_input_pattern,
					 	 neuron_memory_read_logiredu,
					 	 neuron_memory_write_logiredu,
					 	 find_nearest_pattern_logiredu,
					 	 initialise_neuron_memory,
					 	 initialize_input_pattern};

