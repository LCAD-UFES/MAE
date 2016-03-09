#include "syn_neu.h"
#include <mae.h>



/*
Para rodar no cluster deve-se descomentar estas funcoes
*/
/*
unsigned int popcount64(unsigned long long x)
{
    x = (x & 0x5555555555555555ULL) + ((x >> 1) & 0x5555555555555555ULL);
    x = (x & 0x3333333333333333ULL) + ((x >> 2) & 0x3333333333333333ULL);
    x = (x & 0x0F0F0F0F0F0F0F0FULL) + ((x >> 4) & 0x0F0F0F0F0F0F0F0FULL);
    return (x * 0x0101010101010101ULL) >> 56;
}


inline int
hamming_distance (PATTERN *pattern1, PATTERN *pattern2, int connections)
{
        int i, distance;

        distance = 0;
        for (i = 0; i < N_PATTERNS; i++)
	{
                #if defined (__SSE4_2__) || defined (__ABM__)
                unsigned long long a = pattern1[i] ^ pattern2[i];
                if (a != 0) {
                        //distance += __builtin_popcountll(a);
                        distance += popcount64(a);
                 }
                 #else
                 distance += pattern_xor_table[pattern1[i] ^ pattern2[i]];
                 #endif
        }
        return (distance);
                        
}

*/





/*
*********************************************************************************
* Calculate the Hamming distance between two bit patterns			*
*********************************************************************************
*/

inline int
hamming_distance (PATTERN *pattern1, PATTERN *pattern2, int connections)
{
	int i, distance;

	distance = 0;
	for (i = 0; i < N_PATTERNS; i++)
		#if defined (__SSE4_2__) || defined (__ABM__)
		distance += __builtin_popcountll(pattern1[i] ^ pattern2[i]);
		#else
		distance += pattern_xor_table[pattern1[i] ^ pattern2[i]];
		#endif

	return (distance);
}


/*
*********************************************************************************
* Compute the input pattern seen by the default neuron				*
*********************************************************************************
*/

void
compute_input_pattern (NEURON *neuron, PATTERN *input_pattern, SENSITIVITY_TYPE sensitivity, OUTPUT_TYPE output_type)
{
	SYNAPSE_LIST *s_list;
	int num_synapses, i;
	unsigned long long int bit_value;
	//unsigned int bit_value;
	SENSITIVITY_TYPE neuron_layer_sensitivity;

	if (sensitivity == NOT_SPECIFIED)
		neuron_layer_sensitivity = output_type;
	else
		neuron_layer_sensitivity = sensitivity;
	i = -1;
	num_synapses = 0;
	for (s_list = ((SYNAPSE_LIST *) (neuron->synapses)); s_list != NULL; s_list = s_list->next)
	{
		if ((num_synapses % PATTERN_UNIT_SIZE) == 0)
		{
			i++;
			input_pattern[i] = 0;
		}

		input_pattern[i] = input_pattern[i] >> 1;
		
		switch (neuron_layer_sensitivity)
		{
			/* Convetional neuron input patterns just check wheter the masked output value is equal
			to the masked synapse sensitivity value. 1 if equal, 0 otherwise. Float values, just use
			the threshold. */
			case GREYSCALE:
				bit_value = (NEURON_MASK_GREYSCALE(s_list->synapse->source->output.ival) == NEURON_MASK_GREYSCALE(s_list->synapse->sensitivity.ival)) ? 1 : 0;
				break;
			case BLACK_WHITE:
				bit_value = (NEURON_MASK_BLACK_WHITE(s_list->synapse->source->output.ival) == NEURON_MASK_BLACK_WHITE(s_list->synapse->sensitivity.ival)) ? 1 : 0;
				break;
			case GREYSCALE_FLOAT:
				bit_value = (s_list->synapse->source->output.fval > s_list->synapse->sensitivity.fval) ? 1 : 0;
				break;
			case COLOR:
				bit_value = (NEURON_MASK_COLOR(s_list->synapse->source->output.ival) == NEURON_MASK_COLOR(s_list->synapse->sensitivity.ival)) ? 1 : 0;
				break;
			case COLOR_SELECTIVE:	//Color selective neurons should behave in traditional input pattern build as just as 
				bit_value = (NEURON_MASK_COLOR(s_list->synapse->source->output.ival) == NEURON_MASK_COLOR(s_list->synapse->sensitivity.ival)) ? 1 : 0;
				break;
			default:
				bit_value = (NEURON_MASK_GREYSCALE(s_list->synapse->source->output.ival) == NEURON_MASK_GREYSCALE(s_list->synapse->sensitivity.ival)) ? 1 : 0;
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
* Read neuron memory								*
*********************************************************************************
*/

NEURON_OUTPUT
neuron_memory_read (NEURON_LAYER *neuron_layer, int n, PATTERN *input_pattern, int connections, OUTPUT_TYPE output_type)
{
	int nearest_pattern;
	int distance;
	NEURON *neuron;
	NEURON_OUTPUT neuron_output;

	neuron = &(neuron_layer->neuron_vector[n]);

	if (neuron->memory[0].pattern == NULL)
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

	neuron_layer->neuron_vector[n].time_metrics = get_time();
	nearest_pattern = (*(neuron_layer->neuron_type->find_nearest_pattern)) (neuron, input_pattern, connections, &distance);
	neuron_layer->neuron_vector[n].time_metrics = get_time() - neuron_layer->neuron_vector[n].time_metrics;

	neuron->last_hamming_distance = distance;
	neuron->last_best_pattern = nearest_pattern;

	if(neuron_layer->memory_t == SHARED_MEMORY)
		measuraments.neuron_mem_access+= neuron_layer->num_neurons;	//all neurons access
	else
		measuraments.neuron_mem_access++;

	measuraments.neuron_mem_hits_at_distance[distance]++;

	if(nearest_pattern != -1)
		return (neuron->memory[nearest_pattern].associated_value);
	else if(output_type == GREYSCALE_FLOAT)
		neuron_output.fval = 0.0f;	//inhibited
	else
		neuron_output.ival = -1;	//inhibited

	return(neuron_output);

}


/*
*********************************************************************************
* Write neuron memory								*
*********************************************************************************
*/

int
neuron_memory_write (NEURON_LAYER *neuron_layer, int n, PATTERN *input_pattern, int connections, OUTPUT_TYPE output_type)
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

		//neuron->memory[j].pattern = (PATTERN *) alloc_mem (N_PATTERNS * sizeof (PATTERN));
		(*(neuron_layer->neuron_type->initialize_input_pattern)) (&neuron->memory[j].pattern, connections);

		for (i = 0; i < N_PATTERNS; i++)
			neuron->memory[j].pattern[i] = input_pattern[i];
		neuron->memory[j].associated_value = associated_neuron->output;
	}
	else
	{
		if ((rand () % 100) < 5)
		{	/* 5% of probability of changing the nearest pattern */
			nearest_pattern = find_nearest_pattern (neuron, input_pattern, connections, &distance);
			for (i = 0; i < N_PATTERNS; i++)
				neuron->memory[nearest_pattern].pattern[i] = input_pattern[i];
			neuron->memory[nearest_pattern].associated_value = associated_neuron->output;
		}
		else
		{
			j = rand () % neuron->memory_size;
			for (i = 0; i < N_PATTERNS; i++)
				neuron->memory[j].pattern[i] = input_pattern[i];
			neuron->memory[j].associated_value = associated_neuron->output;
		}
	}
	return (j);
}


/*
*********************************************************************************
* Find the nearest pattern in memory						*
*********************************************************************************
*/

int
find_nearest_pattern (NEURON *neuron, PATTERN *input_pattern, int connections, int *distance)
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

#define PREFETCH_DISTANCE 5
	cur_ham_dist = MAX_CONNECTIONS - 1;
	for (i = 0; (i < neuron->memory_size) && (memory[i].pattern != NULL); i++)
	{
/*		if(i + PREFETCH_DISTANCE < neuron->memory_size)
			if(memory[i + PREFETCH_DISTANCE].pattern != NULL)
				mae_long_prefetch_high_priority(memory[i + PREFETCH_DISTANCE].pattern,16);	*/

		new_ham_dist = hamming_distance (memory[i].pattern, input_pattern, connections);
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
* Initialise neuron memory (empty)						*
*********************************************************************************
*/

void
initialise_neuron_memory (NEURON_LAYER *neuron_layer, int n)
{
	int i;
	NEURON *neuron;

	neuron = &(neuron_layer->neuron_vector[n]);			//obtains the desired neuron

	neuron->hash = NULL;						//This neuron does not uses a hash

	if ((neuron_layer->memory_t == DISTRIBUTED_MEMORY) || ((n == 0) && (neuron_layer->memory_t == SHARED_MEMORY)))
	{	//for every neuron on distributed memory layer or just the 1st neuron of a shared memory layer - initialize
		if (neuron->memory == NULL)
			neuron->memory = (ASSOCIATION *) alloc_mem (sizeof (ASSOCIATION) * neuron->memory_size);

		for (i = 0; i < neuron->memory_size; i++)
		{
			neuron->memory[i].confidence = 0;
			neuron->memory[i].pattern = NULL;
			neuron->memory[i].valid_bits = NULL;
		}
	}
	else if (neuron_layer->memory_t == SHARED_MEMORY)
	{	//every neuron on a shared memory layer must point to the same memory location
		neuron->memory = neuron_layer->neuron_vector[0].memory;
	}


}


/*
*********************************************************************************
* Initialize input pattern						*
*********************************************************************************
*/

void
initialize_input_pattern (PATTERN **input_pattern, int connections)
{
	if ((connections/PATTERN_UNIT_SIZE) > (MAX_CONNECTIONS/PATTERN_UNIT_SIZE))
		Erro ("max number of connections exceeded", "", "");

	if ((input_pattern != NULL) && ((*input_pattern) == NULL))
		(*input_pattern) = (PATTERN *) alloc_mem (N_PATTERNS * sizeof (PATTERN));
}


/* Implemented Neuron Types */

NEURON_TYPE default_type = 	{compute_input_pattern,
					 	 	 neuron_memory_read,
					 	 	 neuron_memory_write,
					 	 	 find_nearest_pattern,
					 	 	 initialise_neuron_memory,
					 	 	 initialize_input_pattern};
