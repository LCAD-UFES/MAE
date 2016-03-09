#include "syn_neu.h"
#include <mae.h>

/*
*********************************************************************************
* Calculate the Hamming distance between two bit patterns			*
*********************************************************************************
*/

inline int 
hamming_distance_correlate (NEURON *neuron, PATTERN *pattern1, PATTERN *pattern2, int connections)
{
	int i, distance;
	PATTERN **neuron_pxt;
	
	distance = 0;
	if (neuron->pattern_xor_table != NULL)
	{
		neuron_pxt = neuron->pattern_xor_table;
		for (i = 0; i < N_PATTERNS; i++)
			distance += neuron_pxt[i][pattern1[i] ^ pattern2[i]];
	}
	else
	{
		for (i = 0; i < N_PATTERNS; i++)
			distance += pattern_xor_table[pattern1[i] ^ pattern2[i]];
	}
		
	return (distance);
}


/*
*********************************************************************************
* Find the nearest pattern in memory						*
*********************************************************************************
*/

int
find_nearest_pattern_correlate (NEURON *neuron, PATTERN *input_pattern, int connections, int *distance)
{
	int cur_ham_dist, new_ham_dist;
	int nearest_pattern;
	int i;
	int *candidates;
	int num_candidates = 1;
	ASSOCIATION *memory;
	
	candidates = (int *) alloc_mem(neuron->memory_size * sizeof(int));

	memory = neuron->memory;
	
	cur_ham_dist = MAX_CONNECTIONS - 1;
	for (i = 0; (i < neuron->memory_size) && (memory[i].pattern != NULL); i++)
	{
		new_ham_dist = hamming_distance_correlate (neuron, memory[i].pattern, input_pattern, connections);
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
	free(candidates);
	return (nearest_pattern);
}


/* Implemented Neuron Types */

NEURON_TYPE correlate = 			{compute_input_pattern,
					 	 	 	 	 neuron_memory_read,
					 	 	 	 	 neuron_memory_write,
					 	 	 	 	 find_nearest_pattern_correlate,
					 	 	 	 	 initialise_neuron_memory,
					 	 	 	 	 initialize_input_pattern};

NEURON_TYPE minchinton_kcorrelate = {compute_input_pattern_minchinton,
					 	 	 	 	 neuron_memory_read,
					 	 	 	 	 neuron_memory_write,
					 	 	 	 	 find_nearest_pattern_correlate,
					 	 	 	 	 initialise_neuron_memory,
					 	 	 	 	 initialize_input_pattern};
