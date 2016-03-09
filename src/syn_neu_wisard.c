#include "syn_neu.h"
#include <mae.h>
#include <sys/mman.h>


/*
*********************************************************************************
* Find the nearest pattern in memory for wisard neuron				*
*********************************************************************************
*/
//#if 0
int
find_nearest_pattern_wisard(NEURON *neuron, PATTERN *input_pattern, int connections, int *distance)
{
	int i,j;
	int log2_table_size, number_of_tables;
	int max_votes, most_voted_candidate;
	int candidate_votes[8 * sizeof(unsigned long long int) - 1];
	int hash_address;
	long long int *wisard_memory;
	long long int memory_value;

	get_wisard_table_size_and_log2_table_size_and_number_of_tables(&log2_table_size, &number_of_tables, neuron->memory_size, neuron);

	for (i = 0; i < (8 * sizeof(unsigned long long int) - 1); i++)
		candidate_votes[i] = 0;

	for (i = 0; i < number_of_tables; i++)
	{
		//wisard_memory = (long long int *) &(neuron->memory[i].pattern);
		wisard_memory = (long long int *) neuron->hash[i];
		hash_address = get_bits(log2_table_size, i * log2_table_size, input_pattern, connections);
		memory_value = wisard_memory[hash_address];

		if (memory_value != -1ll)
		{
			for (j = 0; j < (8 * sizeof(unsigned long long int) - 1); j++)
			{
				if ((memory_value >> j) & 0x1ll)
				{
					candidate_votes[j]++;
				}
			}
		}
	}

	max_votes = 0;
	most_voted_candidate = -1;
	for (i = 0; i < (8 * sizeof(unsigned long long int) - 1); i++)
	{
		if (candidate_votes[i] > max_votes)
		{
			max_votes = candidate_votes[i];
			most_voted_candidate = i;
		}
	}

	neuron->last_num_candidates = 1;

	return (most_voted_candidate);
}
//#endif
/*
*********************************************************************************
* Find the nearest pattern in memory for wisard neuron				*
*********************************************************************************
*/

#if 0	// TODO: THIS FIND NEAREST PATTERN FUNCTION IS STILL UNDER TESTING
int
find_nearest_pattern_wisard(NEURON *neuron, PATTERN *input_pattern, int connections, int *distance)
{
	int i,j;
	int log2_table_size, number_of_tables;
	int most_voted_candidate;

	int total_hash_candidates;
	int hash_candidate_votes[8 * sizeof(unsigned long long int) - 1];
	float neuron_candidate_votes[8 * sizeof(unsigned long long int) - 1];
	float max_votes,total_hash_candidates_reciprocal ;

	int hash_address;
	long long int *wisard_memory;
	long long int memory_value;

	get_wisard_table_size_and_log2_table_size_and_number_of_tables(&log2_table_size, &number_of_tables, neuron->memory_size, neuron);

	for (i = 0; i < (8 * sizeof(unsigned long long int) - 1); i++)
		neuron_candidate_votes[i] = 0.0;

	for (i = 0; i < number_of_tables; i++)
	{
		//wisard_memory = (long long int *) &(neuron->memory[i].pattern);
		wisard_memory = (long long int *) neuron->hash[i];
		hash_address = get_bits(log2_table_size, i * log2_table_size, input_pattern, connections);
		memory_value = wisard_memory[hash_address];

		if (memory_value != -1ll)
		{
			total_hash_candidates = 0;

			for (j = 0; j < (8 * sizeof(unsigned long long int) - 1); j++)
			{
				if ((memory_value >> j) & 0x1ll)
				{
					hash_candidate_votes[j] = 1;
					total_hash_candidates++;
				}
				else
					hash_candidate_votes[j] = 0;
			}

			total_hash_candidates_reciprocal = 1.0 / (float) total_hash_candidates;

			for (j = 0; j < (8 * sizeof(unsigned long long int) - 1); j++)
				if(hash_candidate_votes[j])
					neuron_candidate_votes[j] += total_hash_candidates_reciprocal;
		}
	}

	max_votes = 0;
	most_voted_candidate = -1;
	for (i = 0; i < (8 * sizeof(unsigned long long int) - 1); i++)
	{
		if (neuron_candidate_votes[i] > max_votes)
		{
			max_votes = neuron_candidate_votes[i];
			most_voted_candidate = i;
		}
	}

	neuron->last_num_candidates = 1;

	return (most_voted_candidate);
}
#endif
/*
*********************************************************************************
* Read wisard neuron memory							*
*********************************************************************************
*/

NEURON_OUTPUT
neuron_memory_read_wisard(NEURON_LAYER *neuron_layer, int n, PATTERN *input_pattern, int connections, OUTPUT_TYPE output_type)
{
	NEURON *neuron;
	NEURON_OUTPUT neuron_output;

	neuron = &(neuron_layer->neuron_vector[n]);

	//if (neuron->memory[0].pattern == NULL)
	if (neuron->hash == NULL)	//IF no hash table is ready in this neuron
	{
		switch(output_type)
		{
			case GREYSCALE:
				neuron_output.ival = rand () % NUM_GREYLEVELS;
				return neuron_output;
			case BLACK_WHITE:
				neuron_output.ival = ((rand () % NUM_GREYLEVELS) >= (NUM_GREYLEVELS / 2)) ? NUM_GREYLEVELS - 1: 0;
				return neuron_output;
			case GREYSCALE_FLOAT:
				neuron_output.fval = ((float) NUM_GREYLEVELS) * ((float) (rand () % 100) / 50.0f - 1.0f);
				return neuron_output;
			case COLOR:
				neuron_output.ival = rand () % NUM_COLORS;
				return neuron_output;
			case COLOR_SELECTIVE:	// Same as color synapses
				neuron_output.ival = rand () % NUM_COLORS;
				return neuron_output;
		}
	}

	neuron_output.ival = (*(neuron_layer->neuron_type->find_nearest_pattern)) (neuron, input_pattern, connections, NULL);
	neuron->last_hamming_distance = -1;		//non used hamming distance
	neuron->last_best_pattern = -1;

	return (neuron_output);
}


/*
*********************************************************************************
* Write wisard neuron memory							*
*********************************************************************************
*/

int
neuron_memory_write_wisard(NEURON_LAYER *neuron_layer, int n, PATTERN *input_pattern, int connections, OUTPUT_TYPE output_type)
{
	int i;
	NEURON *neuron;
	NEURON *associated_neuron;
	int log2_table_size;
	int number_of_tables;
	long long int *wisard_memory;
	int hash_address;
	int associted_value;

	// Must check for non-supported wisard neurons type
	if (!((output_type == GREYSCALE) || (output_type == BLACK_WHITE)))
		Erro("Currently, wisard neurons can only learn integer values in the 0-62 range", "", "");

	neuron = &(neuron_layer->neuron_vector[n]);
	associated_neuron = &(neuron_layer->associated_neuron_vector[n]);
	associted_value = associated_neuron->output.ival; // Currently only integer output values are possible with wisard neurons
	if (associted_value > 62 || associted_value < 0)
		Erro("Currently, wisard neurons can only learn integer values in the 0-62 range", "", "");

	get_wisard_table_size_and_log2_table_size_and_number_of_tables(&log2_table_size, &number_of_tables, neuron->memory_size, neuron);

	for (i = 0; i < number_of_tables; i++)
	{
		wisard_memory = (long long int *) neuron->hash[i];
		//wisard_memory = (long long int *) &(neuron->memory[i].pattern);
		hash_address = get_bits(log2_table_size, i * log2_table_size, input_pattern, connections);

		if (wisard_memory[hash_address] == -1ll )
			wisard_memory[hash_address] = 0x1ll << associted_value;
		else
			wisard_memory[hash_address] |= 0x1ll << associted_value;
	}
	return -1;
}


/*
*********************************************************************************
* Initialise wizard neuron memory (empty)					*
*********************************************************************************
*/

void
initialise_neuron_memory_wisard (NEURON_LAYER *neuron_layer, int n)
{
	NEURON *neuron;
	int i, j;
	int table_size;
	int log2_table_size;
	int number_of_tables;
	long long int *wisard_memory;

	neuron = &(neuron_layer->neuron_vector[n]);

	if ((neuron_layer->memory_t == DISTRIBUTED_MEMORY) || (!n && neuron_layer->memory_t == SHARED_MEMORY))
	{	//for every neuron on distributed memory layer or just the 1st neuron of a shared memory layer - initialize
		get_wisard_table_size_and_log2_table_size_and_number_of_tables(&log2_table_size, &number_of_tables, neuron->memory_size, neuron);
		table_size = (int) pow(2.0, (double) log2_table_size);

		neuron->memory = NULL;
		neuron->hash = alloc_mem ((size_t) (number_of_tables * sizeof (void *)));

		//printf("nuron i = %d, has %d tables, of %d size and uses %ld bytes of memory\n", n, number_of_tables, table_size, table_size * sizeof (long long int) * number_of_tables);
		for (i = 0; i < number_of_tables; i++)
		{
			neuron->hash[i] = alloc_mem ((size_t) (table_size * sizeof (long long int)));	//integers
			madvise(neuron->hash[i], table_size * sizeof (long long int), MADV_RANDOM);

			wisard_memory = (long long int *) neuron->hash[i];
			//wisard_memory = (long long int *) neuron->memory[i].pattern;
			for (j = 0; j < table_size; j++)
				wisard_memory[j] = -1ll;
		}
	}
	else if (neuron_layer->memory_t == SHARED_MEMORY)
	{	//every neuron on a shared memory layer must point to the same memory location
		neuron->hash = neuron_layer->neuron_vector[0].hash;
		//neuron->memory = neuron_layer->neuron_vector[0].memory;
	}
}


/* Implemented Neuron Types */

NEURON_TYPE wisard = {compute_input_pattern_minchinton,
					  neuron_memory_read_wisard,
					  neuron_memory_write_wisard,
					  find_nearest_pattern_wisard,
					  initialise_neuron_memory_wisard,
					  initialize_input_pattern};
