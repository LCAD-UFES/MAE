#include "syn_neu.h"
#include <mae.h>


int number_of_synapses;
int number_of_subtables;
int subtable_size;
int subtable_rows;

void
minchinton_fat_fast_neuron_initialize_memory_parameters(int connections, int num_training_samples)
{
	number_of_synapses = connections;

#ifndef FAT_FAST_HASH_SIZE_AUTO
	number_of_subtables = number_of_synapses / SUB_PATTERN_UNIT_SIZE;
	if ((number_of_synapses % number_of_subtables) != 0)
		Erro("Invalid number for SUB_PATTERN_UNIT_SIZE", "", "");
#else
	SUB_PATTERN_UNIT_SIZE = (int) (log2(num_training_samples) + 1.0);
	if (SUB_PATTERN_UNIT_SIZE > 16)
		SUB_PATTERN_UNIT_SIZE = 16;

	number_of_subtables = (int)((float) number_of_synapses / (float) SUB_PATTERN_UNIT_SIZE); // the number of synapses must be divisible by log2_table_size; otherwise, some synapses will be ignored
#endif

	subtable_rows = (int) pow(2.0, SUB_PATTERN_UNIT_SIZE);

	subtable_size = subtable_rows * NEURON_MEMORY_INDEX_SIZE;
}


/*
*********************************************************************************
* Allocs a new RAM/hash entry							*
*********************************************************************************
*/

long long int*
minchinton_fat_fast_neuron_alloc_new_hash_entry(long long int vg_ram_entry_id)
{
	long long int	*new_entry;

	new_entry = alloc_mem ((size_t) 2 * sizeof (long long int));

	new_entry[0] = 1ll;					// A single entry alloc'ed
	new_entry[1] = vg_ram_entry_id;				// Sets up the new entry

	return(new_entry);
}


/*
*********************************************************************************
* Reallocs a new RAM/hash entry							*
*********************************************************************************
*/

long long int*
minchinton_fat_fast_neuron_realloc_hash_entry(long long int vg_ram_entry_id, long long int *entry)
{
	long long int	*realloced_entry;
	int size;

	size = entry[0];	//gets the previous entry
	size++ ;

	realloced_entry = realloc_mem ( (size_t) ((size + 1) * sizeof (long long int)) ,entry);	// Realloc'ed size

	realloced_entry[0] = size;			// A single entry alloc'ed
	realloced_entry[size] = vg_ram_entry_id;	// Sets up the new entry

	return(realloced_entry);
}


/*
*********************************************************************************
* Count how many collisions happened in a minchinton fat-fast neuron hash 	*
*********************************************************************************
*/

int
count_neuron_hash_collisons (NEURON *neuron)
{
	int collisions;
	int i,j;
	long long int **hash_table;

	if(!neuron->hash)	//if not available hash in neuron
		return(0);

	collisions = 0;

	for (i = 0; i < number_of_subtables; i++)
	{
		hash_table = (long long int **) neuron->hash[i];

		for(j = 0; j < subtable_rows; j++)
		{
			if(hash_table[j] && hash_table[j][0] > 1)
				collisions += hash_table[j][0] - 1;
		}
	}

	return(collisions);
}


/*
*********************************************************************************
* Count how many collisions happened in a minchinton fat-fast 	neuron layer 	*
*********************************************************************************
*/

float
count_neuron_layer_average_hash_collisions (char *neuron_layer_name)
{
	NEURON_LAYER *neuron_layer;
	NEURON *neuron_vector;
	int num_collisions;
	int num_neurons;
	int i;

	if ((neuron_layer = get_neuron_layer_by_name (neuron_layer_name)) == NULL)
	{
		show_message ("Unknown neuron layer:", neuron_layer_name, "");
		return 0;
	}

	num_neurons = get_num_neurons (neuron_layer->dimentions);
	neuron_vector = neuron_layer->neuron_vector;

	num_collisions = 0;
	for (i = 0; i < num_neurons; i++)
	{
		//printf("Collisions for Neuron %ld Hash Table = %d\n",i, count_neuron_hash_collisons(&neuron_vector[i]));
		num_collisions += count_neuron_hash_collisons(&neuron_vector[i]);
	}

	return (float) num_collisions / (float) num_neurons ;
}

/*
*********************************************************************************
* Delete minchinton fat fast neuron memory										*
*********************************************************************************
*/

void
neuron_hash_entry_delete_minchinton_fat_fast(NEURON_LAYER *neuron_layer, int n, PATTERN *input_pattern, int connections, int mem_line_to_delete)
{
	int i, j, k;
	int hash_limit;
	NEURON *neuron;
	long long int **wisard_memory;
	SUB_PATTERN subtable_row;

	neuron = &(neuron_layer->neuron_vector[n]);

	/* WISARD (RAM) Memory de */
	for (i = number_of_subtables-1; i >= 0; i--)
	{
		wisard_memory = (long long int **) neuron->hash[i];
#ifndef FAT_FAST_HASH_SIZE_AUTO
		subtable_row = ((SUB_PATTERN *)input_pattern)[i];
#else
		subtable_row = get_bits(SUB_PATTERN_UNIT_SIZE, i * SUB_PATTERN_UNIT_SIZE, input_pattern, connections);
#endif

		if (wisard_memory[subtable_row])		//if null RAM/hash entry, must alloc a new one
		{
			hash_limit = (wisard_memory[subtable_row])[0];

			for(j = 1; j<= hash_limit ; j++)
				if((wisard_memory[subtable_row])[j] == mem_line_to_delete)
					break;

			for(k = j; k< hash_limit ; k++)
				(wisard_memory[subtable_row])[k] = (wisard_memory[subtable_row])[k+1];

			(wisard_memory[subtable_row])[0]--;

			//The Wisard memory entry will now be realloc'ed
			wisard_memory[subtable_row] = realloc_mem ( (size_t) (((wisard_memory[subtable_row])[0] + 1) * sizeof (long long int)) ,wisard_memory[subtable_row]);	// Realloc'ed size for subtable
		}
		else
			return;
	}
}

/*
*********************************************************************************
* Find the nearest pattern in memory for minchinton fat fast neuron		*
*********************************************************************************
*/

int
find_nearest_pattern_minchinton_fat_fast(NEURON *neuron, PATTERN *input_pattern, int connections, int *distance)
{
	int i,j;
	int hash_limit;

	long long int **wisard_memory;
	long long int *memory_value;

	register int candidate_location;

	int cur_ham_dist, new_ham_dist;
	int nearest_pattern;
	int candidates[NEURON_MEMORY_SIZE];
	int num_candidates = 0;
	ASSOCIATION *memory;
	SUB_PATTERN subtable_row;

	memory = neuron->memory;

	cur_ham_dist = MAX_CONNECTIONS - 1;

	if (memory[0].pattern == NULL)
	{
		*distance = cur_ham_dist;
		neuron->last_num_candidates = 0;
		return 0;
	}

	for (i = number_of_subtables-1; i >= 0; i--)
	{
		wisard_memory = (long long int **) neuron->hash[i];
#ifndef FAT_FAST_HASH_SIZE_AUTO
		subtable_row = ((SUB_PATTERN *)input_pattern)[i];
#else
		subtable_row = get_bits(SUB_PATTERN_UNIT_SIZE, i * SUB_PATTERN_UNIT_SIZE, input_pattern, connections);
#endif
		memory_value = wisard_memory[subtable_row];

		if (memory_value)			//if we have a non-null point value (the memory value exists)
		{
			hash_limit = memory_value[0];
			neuron->hash_collisions++;	//Accumulate the number of hash test collisions for this neuron

			for (j = 0; j < hash_limit ; j++)
			{
				candidate_location = memory_value[j+1];		//Obtains the candidate location

				new_ham_dist = hamming_distance (memory[candidate_location].pattern, input_pattern, connections);
				if (new_ham_dist < cur_ham_dist)
				{
					candidates[0] = candidate_location;
					num_candidates = 1;
					cur_ham_dist = new_ham_dist;
				}
				else if (new_ham_dist == cur_ham_dist)
				{
					candidates[num_candidates] = candidate_location;
					num_candidates++;
				}
			}
		}
	}

	if (num_candidates > 0)
		nearest_pattern = candidates[mae_fastrand() % num_candidates];
	else
		nearest_pattern = mae_fastrand() % get_neuron_memory_size(neuron);

	neuron->last_num_candidates = num_candidates;

	*distance = cur_ham_dist;
	return (nearest_pattern);
}


/*
*********************************************************************************
* Find the nearest pattern in memory for minchinton fat fast overwrite neuron	*
*********************************************************************************
*/

int
find_nearest_pattern_minchinton_fat_fast_overwrite(NEURON *neuron, PATTERN *input_pattern, int connections, int *distance)
{
	int i;

	register int candidate_location;

	int cur_ham_dist, new_ham_dist;
	int nearest_pattern;
	int candidates[NEURON_MEMORY_SIZE];
	int num_candidates = 0;
	ASSOCIATION *memory;
	SUB_PATTERN subtable_row;

	memory = neuron->memory;

	cur_ham_dist = MAX_CONNECTIONS - 1;

	if (memory[0].pattern == NULL)
	{
		*distance = cur_ham_dist;
		neuron->last_num_candidates = 0;
		return 0;
	}

	for (i = number_of_subtables-1; i >= 0; i--)
	{
#ifndef FAT_FAST_HASH_SIZE_AUTO
		subtable_row = ((SUB_PATTERN *)input_pattern)[i];
#else
		subtable_row = get_bits(SUB_PATTERN_UNIT_SIZE, i * SUB_PATTERN_UNIT_SIZE, input_pattern, connections);
#endif

		candidate_location = ((int*) neuron->hash)[i * subtable_rows + subtable_row];		//load the candidate location from hash

		if(candidate_location != -1)				//if the candidate location is valid
		{
			new_ham_dist = hamming_distance (memory[candidate_location].pattern, input_pattern, connections);
			if (new_ham_dist < cur_ham_dist)
			{
				candidates[0] = candidate_location;
				num_candidates = 1;
				cur_ham_dist = new_ham_dist;
			}
			else if (new_ham_dist == cur_ham_dist)
			{
				candidates[num_candidates] = candidate_location;
				num_candidates++;
			}
		}
	}

	if (num_candidates > 0)
		nearest_pattern = candidates[mae_fastrand() % num_candidates];
	else
		nearest_pattern = mae_fastrand() % get_neuron_memory_size(neuron);

	neuron->last_num_candidates = num_candidates;

	*distance = cur_ham_dist;
	return (nearest_pattern);
}


/*
*********************************************************************************
* Find the nearest pattern in memory for minchinton fat fast neuron		*
*********************************************************************************
*/

int
find_nearest_pattern_minchinton_fat_fast_random(NEURON *neuron, PATTERN *input_pattern, int connections, int *distance)
{
	int i,j;
	long long int **wisard_memory;
	long long int *memory_value;
	int hash_limit;

	register int candidate_location;

	int cur_ham_dist, new_ham_dist;
	int nearest_pattern;
	int candidates[NEURON_MEMORY_SIZE];
	int num_candidates = 0;
	ASSOCIATION *memory;
	SUB_PATTERN subtable_row;

	memory = neuron->memory;
	cur_ham_dist = MAX_CONNECTIONS - 1;

	if (memory[0].pattern == NULL)
	{
		*distance = cur_ham_dist;
		neuron->last_num_candidates = 0;
		return 0;
	}

	for (i = number_of_subtables-1; i >= 0; i--)
	{
		wisard_memory = (long long int **) neuron->hash[i];
#ifndef FAT_FAST_HASH_SIZE_AUTO
		subtable_row = ((SUB_PATTERN *)input_pattern)[i];
#else
		subtable_row = get_bits(SUB_PATTERN_UNIT_SIZE, i * SUB_PATTERN_UNIT_SIZE, input_pattern, connections);
#endif
		memory_value = wisard_memory[subtable_row];

		if (memory_value)		//if we have a non-null point value
		{
			hash_limit = memory_value[0];

			if(hash_limit <= NEURON_MEMORY_INDEX_SIZE)
			{
				// Here we have an upper limit on the max number of samples per entry

				for (j = 0; j < hash_limit ; j++)
				{
					candidate_location = memory_value[j+1];		//Obtains the candidate location

					new_ham_dist = hamming_distance (memory[candidate_location].pattern, input_pattern, connections);
					if (new_ham_dist < cur_ham_dist)
					{
						candidates[0] = candidate_location;
						num_candidates = 1;
						cur_ham_dist = new_ham_dist;
					}
					else if (new_ham_dist == cur_ham_dist)
					{
						candidates[num_candidates] = candidate_location;
						num_candidates++;
					}
				}
			}
			else		//Over the maximum collisions, we obtain some random samples from the hash position vector
			{
				neuron->hash_collisions++;	//Accumulate the number of test collisions for this neuron

				for (j = 0; j <  NEURON_MEMORY_INDEX_SIZE; j++)
				{
					candidate_location = memory_value[ mae_fastrand() % hash_limit + 1];	//MUST USE mae_fastrand()

					new_ham_dist = hamming_distance (memory[candidate_location].pattern, input_pattern, connections);
					if (new_ham_dist < cur_ham_dist)
					{
						candidates[0] = candidate_location;
						num_candidates = 1;
						cur_ham_dist = new_ham_dist;
					}
					else if (new_ham_dist == cur_ham_dist)
					{
						candidates[num_candidates] = candidate_location;
						num_candidates++;
					}
				}
			}
		}
	}

	if (num_candidates > 0)
		nearest_pattern = candidates[mae_fastrand() % num_candidates];
	else
		nearest_pattern = mae_fastrand() % get_neuron_memory_size(neuron);

	neuron->last_num_candidates = num_candidates;

	*distance = cur_ham_dist;
	return (nearest_pattern);
}


/*
*********************************************************************************
* Write minchinton fat fast neuron memory					*
*********************************************************************************
*/

int
neuron_memory_write_minchinton_fat_fast(NEURON_LAYER *neuron_layer, int n, PATTERN *input_pattern, int connections, OUTPUT_TYPE output_type)
{
	int i, j;
	int mem_line_to_delete; //, distance;
	NEURON *neuron;
	long long int **wisard_memory;
	SUB_PATTERN subtable_row;
	NEURON *associated_neuron;

	neuron = &(neuron_layer->neuron_vector[n]);
	associated_neuron = &(neuron_layer->associated_neuron_vector[n]);

	/* Minchinton (VG-RAM) Memory write/overwire */
	if (neuron->memory[neuron->memory_size-1].pattern == NULL)		//if there is still space in memory
	{
		j = neuron_memory_write(neuron_layer, n, input_pattern, connections, output_type);
	}
	else
	{
	//	if ((rand () % 100) < 5)
	//	{	/* 5% of probability of changing the nearest pattern */
			/* Overwrites the nearest pattern entry */

			// Deletes the hash entry
	//		mem_line_to_delete = find_nearest_pattern_minchinton_fat_fast (neuron, input_pattern, connections, &distance);
	//		neuron_hash_entry_delete_minchinton_fat_fast(neuron_layer,n,input_pattern,connections,mem_line_to_delete);

	//		for (i = 0; i < N_PATTERNS; i++)
	//			neuron->memory[mem_line_to_delete].pattern[i] = input_pattern[i];

	//		neuron->memory[mem_line_to_delete].associated_value = associated_neuron->output;
	//		j = mem_line_to_delete;
	//	}
	//	else
	//	{	/* Else, chooses another pattern at random */

			// Deletes the hash entry
			mem_line_to_delete = rand () % neuron->memory_size;
			neuron_hash_entry_delete_minchinton_fat_fast(neuron_layer,n,input_pattern,connections,mem_line_to_delete);

			for (i = 0; i < N_PATTERNS; i++)
				neuron->memory[mem_line_to_delete].pattern[i] = input_pattern[i];

			neuron->memory[mem_line_to_delete].associated_value = associated_neuron->output;
			j = mem_line_to_delete;
	//	}
	}

	/* Minchinton (VG-RAM) Memory write/overwire */
	//j = neuron_memory_write(neuron_layer, n, input_pattern, connections, output_type);

	/* WISARD (RAM) Memory write */
	for (i = number_of_subtables-1; i >= 0; i--)
	{
		wisard_memory = (long long int **) neuron->hash[i];
#ifndef FAT_FAST_HASH_SIZE_AUTO
		subtable_row = ((SUB_PATTERN *)input_pattern)[i];
#else
		subtable_row = get_bits(SUB_PATTERN_UNIT_SIZE, i * SUB_PATTERN_UNIT_SIZE, input_pattern, connections);
#endif

		if (!wisard_memory[subtable_row])		//if null RAM/hash entry, must alloc a new one
			wisard_memory[subtable_row] = minchinton_fat_fast_neuron_alloc_new_hash_entry(j);
		else						//else, we have a collision
			wisard_memory[subtable_row] = minchinton_fat_fast_neuron_realloc_hash_entry(j, wisard_memory[subtable_row]);
	}

	return (j);
}


/*
*********************************************************************************
* Write minchinton fat fast overwrite neuron memory				*
*********************************************************************************
*/

int
neuron_memory_write_minchinton_fat_fast_overwrite(NEURON_LAYER *neuron_layer, int n, PATTERN *input_pattern, int connections, OUTPUT_TYPE output_type)
{
	int i, j;
	NEURON *neuron;
	SUB_PATTERN subtable_row;

	neuron = &(neuron_layer->neuron_vector[n]);

	/* Minchinton (VG-RAM) Memory write */
	j = neuron_memory_write(neuron_layer, n, input_pattern, connections, output_type);

	/* Hash Memory write */
	for (i = number_of_subtables-1; i >= 0; i--)
	{
#ifndef FAT_FAST_HASH_SIZE_AUTO
		subtable_row = ((SUB_PATTERN *)input_pattern)[i];
#else
		subtable_row = get_bits(SUB_PATTERN_UNIT_SIZE, i * SUB_PATTERN_UNIT_SIZE, input_pattern, connections);
#endif

		((int *)neuron->hash)[subtable_rows * i + subtable_row] = (int) j;	//Overwrites the last value (no collision handling)

	}

	return (j);
}


/*
*********************************************************************************
* Initialize minchinton fat fast neuron memory (empty)				*
*********************************************************************************
*/

void
initialize_neuron_memory_minchinton_fat_fast_only (NEURON_LAYER *neuron_layer, int n)
{
	NEURON *neuron;
	int i, j;
	long long int **wisard_memory;

	neuron = &(neuron_layer->neuron_vector[n]);

	minchinton_fat_fast_neuron_initialize_memory_parameters(neuron->n_in_connections, neuron->memory_size);

	/* WISARD (RAM) memory initialize */
	if ((neuron_layer->memory_t == DISTRIBUTED_MEMORY) || (!n && neuron_layer->memory_t == SHARED_MEMORY))
	{	//for every neuron on distributed memory layer or just the 1st neuron of a shared memory layer - initialize

		neuron->hash = alloc_mem ((size_t) (number_of_subtables * sizeof (void *)));

		for (i = 0; i < number_of_subtables; i++)
		{
			neuron->hash[i] = alloc_mem ((size_t) (subtable_rows * sizeof (long long int *)));	// lli pointer vector
			wisard_memory = (long long int **) neuron->hash[i];				// lli pointer vector

			for (j = 0; j < subtable_rows; j++)
				wisard_memory[j] = NULL;					//null pointers empty memory
		}
	}
	else if (neuron_layer->memory_t == SHARED_MEMORY)
	{	//every neuron on a shared memory layer must point to the same memory location
		neuron->hash = neuron_layer->neuron_vector[0].hash;
	}
}


void
initialize_neuron_memory_minchinton_fat_fast (NEURON_LAYER *neuron_layer, int n)
{
	initialise_neuron_memory(neuron_layer, n);

	initialize_neuron_memory_minchinton_fat_fast_only(neuron_layer, n);
}


/*
*********************************************************************************
* Initialize Minchinton Fat Fast overwrite neuron memory (empty)		*
*********************************************************************************
*/

void
initialize_neuron_memory_minchinton_fat_fast_overwrite_only (NEURON_LAYER *neuron_layer, int n)
{
	int i;
	NEURON *neuron;

	neuron = &(neuron_layer->neuron_vector[n]);

	minchinton_fat_fast_neuron_initialize_memory_parameters(neuron->n_in_connections, neuron->memory_size);

	/* Hash memory initialize */
	if ((neuron_layer->memory_t == DISTRIBUTED_MEMORY) || (!n && neuron_layer->memory_t == SHARED_MEMORY))
	{	//for every neuron on distributed memory layer or just the 1st neuron of a shared memory layer - initialize

		neuron->hash = alloc_mem ((size_t) (number_of_subtables * subtable_rows * sizeof(int)));

		for (i = 0; i < number_of_subtables * subtable_rows; i++)
		{
			((int *) neuron->hash)[i] = (int) -1;	//-1 as dummy value
		}
	}
	else if (neuron_layer->memory_t == SHARED_MEMORY)
	{	//every neuron on a shared memory layer must point to the same memory location
		neuron->hash = neuron_layer->neuron_vector[0].hash;
	}
}


void
initialize_neuron_memory_minchinton_fat_fast_overwrite (NEURON_LAYER *neuron_layer, int n)
{
	initialise_neuron_memory(neuron_layer, n);

	initialize_neuron_memory_minchinton_fat_fast_overwrite_only(neuron_layer, n);
}

/*
*********************************************************************************
* Initialize Minchinton Fat Fast index neuron memory (empty)		*
*********************************************************************************
*/

void
initialize_neuron_memory_minchinton_fat_fast_index_only (NEURON_LAYER *neuron_layer, int n)
{
	int i;
	NEURON *neuron;

	neuron = &(neuron_layer->neuron_vector[n]);

	minchinton_fat_fast_neuron_initialize_memory_parameters(neuron->n_in_connections, neuron->memory_size);
	
	if ((neuron_layer->memory_t == DISTRIBUTED_MEMORY) || (!n && neuron_layer->memory_t == SHARED_MEMORY))
	{	//for every neuron on distributed memory layer or just the 1st neuron of a shared memory layer - initialize

		/* Minchinton Memory (Index) initialize */
		neuron->memory_index = (int *) alloc_mem(number_of_subtables * subtable_size * sizeof(int));

		for (i = 0; i < number_of_subtables * subtable_size; i++)
		{
			neuron->memory_index[i] = (int) -1;	//-1 as dummy value
		}
	}
	else if (neuron_layer->memory_t == SHARED_MEMORY)
	{	//every neuron on a shared memory layer must point to the same memory location
		neuron->memory_index = neuron_layer->neuron_vector[0].memory_index;
	}
}


void
initialize_neuron_memory_minchinton_fat_fast_index (NEURON_LAYER *neuron_layer, int neuron_index)
{
	initialise_neuron_memory(neuron_layer, neuron_index);

	initialize_neuron_memory_minchinton_fat_fast_index_only(neuron_layer, neuron_index);
}


/*
*********************************************************************************
* Write minchinton fat fast index neuron memory				*
*********************************************************************************
*/

int
neuron_memory_write_minchinton_fat_fast_index(NEURON_LAYER *neuron_layer, int n, PATTERN *input_pattern, int connections, OUTPUT_TYPE output_type)
{
	int i, j;
	int last_input_pattern_index;
	NEURON *neuron;
	SUB_PATTERN subtable_row;

	neuron = &(neuron_layer->neuron_vector[n]);

	/* Minchinton (VG-RAM) Memory write */
	last_input_pattern_index = neuron_memory_write(neuron_layer, n, input_pattern, connections, output_type);

	if (last_input_pattern_index >= 0)
	{
		/* Minchinton (Index) Memory write */
		for (i = 0; i < number_of_subtables; i++)
		{
#ifndef FAT_FAST_HASH_SIZE_AUTO
		subtable_row = ((SUB_PATTERN *)input_pattern)[i];
#else
		subtable_row = get_bits(SUB_PATTERN_UNIT_SIZE, i * SUB_PATTERN_UNIT_SIZE, input_pattern, connections);
#endif

			for(j = 0; j < NEURON_MEMORY_INDEX_SIZE; j++)
			{
				if (neuron->memory_index[i * subtable_size + subtable_row * NEURON_MEMORY_INDEX_SIZE + j] == -1)
					break;
			}
			if ( j == NEURON_MEMORY_INDEX_SIZE )
				j = mae_fastrand() % NEURON_MEMORY_INDEX_SIZE;

			neuron->memory_index[i * subtable_size + subtable_row * NEURON_MEMORY_INDEX_SIZE + j] = last_input_pattern_index;
		}
	}
	return (last_input_pattern_index);
}


/*
*********************************************************************************
* Find the nearest pattern in memory for minchinton fat fast index neuron	*
*********************************************************************************
*/

int
find_nearest_pattern_minchinton_fat_fast_index(NEURON *neuron, PATTERN *input_pattern, int connections, int *distance)
{
	int i, j;
	int *possible_candidates;
	int possible_candidates_empty = 0;
	int candidates[NEURON_MEMORY_SIZE];
	int num_candidates = 1;
	int nearest_pattern = 0;
	int cur_ham_dist = MAX_CONNECTIONS - 1;
	int new_ham_dist;
	SUB_PATTERN subtable_row;

	/* VG-RAM find nearest pattern */
	if (neuron->memory[0].pattern != NULL)
	{
		for (i = number_of_subtables-1; i >= 0; i--)
		{
#ifndef FAT_FAST_HASH_SIZE_AUTO
		subtable_row = ((SUB_PATTERN *)input_pattern)[i];
#else
		subtable_row = get_bits(SUB_PATTERN_UNIT_SIZE, i * SUB_PATTERN_UNIT_SIZE, input_pattern, connections);
#endif

			possible_candidates = &(neuron->memory_index[i * subtable_size + subtable_row * NEURON_MEMORY_INDEX_SIZE]);

			for(j = 0; j < NEURON_MEMORY_INDEX_SIZE; j++)
			{
				if( possible_candidates[j] != -1 )
				{
					new_ham_dist = hamming_distance (neuron->memory[possible_candidates[j]].pattern, input_pattern, connections);
					if (new_ham_dist < cur_ham_dist)
					{
						candidates[0] = possible_candidates[j];
						num_candidates = 1;
						cur_ham_dist = new_ham_dist;
					}
					else if (new_ham_dist == cur_ham_dist)
					{
						candidates[num_candidates] = possible_candidates[j];
						num_candidates++;
					}
				}
				else
				{
					if (j == 0)
						possible_candidates_empty++;
					break;
				}
			}
		}

		if (possible_candidates_empty == number_of_subtables)
			nearest_pattern = mae_fastrand() % neuron->memory_size;
		else
			nearest_pattern = candidates[mae_fastrand() % num_candidates];

		neuron->last_num_candidates = num_candidates;
	}

	*distance = cur_ham_dist;
	return (nearest_pattern);
}


/* Implemented Neuron Types */

NEURON_TYPE minchinton_fat_fast_index =	{compute_input_pattern_minchinton,
					 neuron_memory_read,
					 neuron_memory_write_minchinton_fat_fast_index,
					 find_nearest_pattern_minchinton_fat_fast_index,
					 initialize_neuron_memory_minchinton_fat_fast_index,
					 initialize_input_pattern};

NEURON_TYPE minchinton_fat_fast = 	{compute_input_pattern_minchinton,
					 neuron_memory_read,
					 neuron_memory_write_minchinton_fat_fast,
					 find_nearest_pattern_minchinton_fat_fast,
					 initialize_neuron_memory_minchinton_fat_fast,
					 initialize_input_pattern};

NEURON_TYPE minchinton_fat_fast_rand = 	{compute_input_pattern_minchinton,
					 neuron_memory_read,
					 neuron_memory_write_minchinton_fat_fast,
					 find_nearest_pattern_minchinton_fat_fast_random,
					 initialize_neuron_memory_minchinton_fat_fast,
					 initialize_input_pattern};

NEURON_TYPE minchinton_fat_fast_overwrite = {compute_input_pattern_minchinton,
					 neuron_memory_read,
					 neuron_memory_write_minchinton_fat_fast_overwrite,
					 find_nearest_pattern_minchinton_fat_fast_overwrite,
					 initialize_neuron_memory_minchinton_fat_fast_overwrite,
					 initialize_input_pattern};
