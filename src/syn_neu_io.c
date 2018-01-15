#include "syn_neu.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

/*
 *********************************************************************************
 * Reload a synapse								*
 *********************************************************************************
 */

void
reload_synapse (NEURON *neuron)
{
	NEURON_OUTPUT sensitivity;
	NEURON_LAYER *neuron_layer_src;
	NEURON *neuron_src;
	int n_layer_src_id;
	int n_src_id;
	float weight;

	if(fread(&n_layer_src_id, sizeof (int), 1, neural_connections) != 1)
	{
		fprintf(stderr,"Erro na leitura\n");
		exit (0);
	}

	if(fread(&n_src_id, sizeof (int), 1, neural_connections) != 1)
	{
		fprintf(stderr,"Erro na leitura\n");
		exit (0);
	}

	if(fread(&sensitivity, sizeof (NEURON_OUTPUT), 1, neural_connections) != 1)
	{
		fprintf(stderr,"Erro na leitura\n");
		exit (0);
	}

	if(fread(&weight, sizeof (float), 1, neural_connections) != 1)
	{
		fprintf(stderr,"Erro na leitura\n");
		exit (0);
	}

	if (n_layer_src_id == -1) // synapse connected to the neuron_with_output_zero
	{
		reconnect_2_neurons(NULL, &(neuron_with_output_zero), neuron, sensitivity, weight);
	}
	else
	{
		neuron_layer_src = n_layer_vec[n_layer_src_id];
		neuron_src = &(neuron_layer_src->neuron_vector[n_src_id]);
		reconnect_2_neurons(neuron_layer_src, neuron_src, neuron, sensitivity, weight);
	}
	return;
}


/*
 *********************************************************************************
 * Reload neuron's memory							*
 *********************************************************************************
 */

void
reload_synapses (NEURON *neuron_dest)
{
	int num_synapses, i;

	if (fread(&num_synapses, sizeof (int), 1, neural_connections) != 1)
	{
		fprintf(stderr,"Erro na leitura\n");
		exit (0);
	}

	for (i = 0; i < num_synapses; i++)
		reload_synapse (neuron_dest);

	return;
}


/*
 *********************************************************************************
 * Reload neuron's memory							*
 *********************************************************************************
 */

void
read_pattern_string(int memory_bit_group_size, PATTERN *input_pattern, const char *bits)
{
	unsigned long long int bit_value;
	int num_synapses, i, j;

	i = -1;
	num_synapses = 0;
	for (j=0; j<memory_bit_group_size*PATTERN_UNIT_SIZE; j++)
	{
		bit_value = (int)bits[j];

		if ((num_synapses % PATTERN_UNIT_SIZE) == 0)
		{
			i++;
			input_pattern[i] = 0;
		}

		input_pattern[i] = input_pattern[i] >> 1;

		input_pattern[i] |= bit_value << (PATTERN_UNIT_SIZE - 1);

		num_synapses++;
	}

	if (num_synapses % PATTERN_UNIT_SIZE)
		input_pattern[i] = input_pattern[i] >> (PATTERN_UNIT_SIZE - (num_synapses % PATTERN_UNIT_SIZE));

}


void
read_pattern(int memory_bit_group_size, PATTERN *input_pattern)
{
	char *string_pattern = NULL;

	string_pattern = calloc(sizeof(char), memory_bit_group_size*PATTERN_UNIT_SIZE+1);

	if (fscanf(neural_connections, "%s\n", string_pattern) < 0)
	{
		fprintf(stderr,"Erro na leitura\n");
		exit (0);
	}

	read_pattern_string(memory_bit_group_size, input_pattern, string_pattern);

	free(string_pattern);
}


/*
 *********************************************************************************
 * Delete neuron hash								*
 *********************************************************************************
 */

void
delete_neuron_hash (NEURON_LAYER *neuron_layer, int n)
{
	NEURON *neuron;
	int i,j;
	long long int **wisard_memory;

	neuron = &(neuron_layer->neuron_vector[n]);

	minchinton_fat_fast_neuron_initialize_memory_parameters(neuron->n_in_connections, neuron->memory_size);

	if( 	neuron_layer->neuron_type == &minchinton_fat_fast  	||
			neuron_layer->neuron_type == &minchinton_fat_fast_rand 	)
	{
		for (i = 0; i < number_of_subtables; i++)
		{
			wisard_memory = (long long int **) neuron->hash[i];				// lli pointer vector

			if (wisard_memory)
			{
				for (j = 0; j < subtable_rows; j++)
					if(wisard_memory[j])
						free(wisard_memory[j]);

				free(wisard_memory);
			}
		}

		free(neuron->hash);
		neuron->hash = NULL;
	}
	else if( neuron_layer->neuron_type == &minchinton_fat_fast_overwrite )	//for fat fast overwrite neurons
	{
		if(neuron->hash)
			free(neuron->hash);
	}
}


void
delete_neuron_memory_fat_fast_index (NEURON_LAYER *neuron_layer, int neuron_index)
{
	NEURON *neuron;

	if(neuron_layer->neuron_type != &minchinton_fat_fast_index)
		return;

	neuron = &(neuron_layer->neuron_vector[neuron_index]);

	if ((neuron_layer->memory_t == DISTRIBUTED_MEMORY) || (!neuron_index && neuron_layer->memory_t == SHARED_MEMORY))
	{	//for every neuron on distributed memory layer or just the 1st neuron of a shared memory layer - initialize

		if (neuron->memory_index)
			free(neuron->memory_index);

		neuron->memory_index = NULL;
	}
}


void
reload_neuron_memory_pattern_only (NEURON_LAYER *neuron_layer, NEURON *neuron)
{
	int i, memory_used;
	int connections;
	int pattern_flag;

	connections = neuron->n_in_connections;

	if(fscanf(neural_connections, "%d\n", &memory_used) < 0)
	{
		fprintf(stderr,"Erro na leitura\n");
		exit (0);
	}

	/* TODO: All neurons Must have the same Memory Used value, otherwise the Hash memory reload/read would fail */
	NEURON_MEMORY_SIZE = memory_used;
	neuron->memory_size = memory_used;

	if(neuron->memory)
	{
		for (i = 0; i < memory_used; i++)
		{
			if (neuron->memory[i].pattern != NULL)
				free(neuron->memory[i].pattern);

			if (neuron->memory[i].valid_bits != NULL)
				free(neuron->memory[i].valid_bits);
		}

		free(neuron->memory);
		neuron->memory = NULL;
	}
	// THE NEURON HASHES MUST ALSO BE FREED !! (OTHERWISE, IT MAY CAUSE POTENTIAL LEAKS).
	delete_neuron_hash(neuron_layer, neuron->id);
	delete_neuron_memory_fat_fast_index(neuron_layer, neuron->id);
	initialise_neuron_memory(neuron_layer, neuron->id);

	for (i = 0; i < memory_used; i++)
	{
		if(fscanf(neural_connections, "%d\n", &pattern_flag) < 0)
		{
			fprintf(stderr,"Erro na leitura\n");
			exit (0);
		}

		if(pattern_flag)
		{
			(*(neuron_layer->neuron_type->initialize_input_pattern)) (&neuron->memory[i].pattern, connections);

			read_pattern(N_PATTERNS, neuron->memory[i].pattern);
		}

		if (neuron_layer->output_type == GREYSCALE_FLOAT)
		{
			if(fscanf(neural_connections, "%f\n", &neuron->memory[i].associated_value.fval) < 0)
			{
				fprintf(stderr,"Erro na leitura\n");
				exit(0);
			}
		}
		else
		{
			if(fscanf(neural_connections, "%d\n", &neuron->memory[i].associated_value.ival) < 0)
			{
				fprintf(stderr,"Erro na leitura\n");
				exit(0);
			}
		}
	}

	return;
}


void
reload_neuron_hash_pattern_only (NEURON_LAYER *neuron_layer, NEURON *neuron, int n) //Neuron id and number of connections must be a parameter also
{
	int i,j,k;
	long long int **wisard_memory;
	PATTERN *input_pattern;
	SUB_PATTERN subtable_row;

	/* Get hash parameters */
	minchinton_fat_fast_neuron_initialize_memory_parameters(neuron->n_in_connections, neuron->memory_size);

	/* Reintialize neuron hash (or skip if this is not the first shared memory neuron in the list)*/
	if ((neuron_layer->memory_t == DISTRIBUTED_MEMORY) || (!n && neuron_layer->memory_t == SHARED_MEMORY))
	{
		//for every neuron on distributed memory layer or just the 1st neuron of a shared memory layer - initialize
		neuron->hash = alloc_mem ((size_t) (number_of_subtables * sizeof (void *)));

		for (i = 0; i < number_of_subtables; i++)
		{
			neuron->hash[i] = alloc_mem ((size_t) (subtable_rows * sizeof (long long int *)));	// lli pointer vector
			wisard_memory = (long long int **) neuron->hash[i];				// lli pointer vector
			for (j = 0; j < subtable_rows; j++)
				wisard_memory[j] = NULL;					//null pointers empty memory
		}
	}
	else if (neuron_layer->memory_t == SHARED_MEMORY) //If a shared memory has has already been made
	{	//every neuron on a shared memory layer must point to the same hash location
		neuron->hash = neuron_layer->neuron_vector[0].hash;
		return;	//Neuron Hash doesn't need to be rewritten
	}

	/* Hash entry reload */

	//j-1 keeps the latest VG-RAM used memory position
	j = get_neuron_memory_size(neuron);

	//For each sample already stored in neuron memory, read and write on the hash
	for(k = 0; k < j; k++)
	{
		// Reads the bit pattern from neuron memory
		input_pattern = neuron->memory[k].pattern;

		// For Each neuron table
		for (i = number_of_subtables-1; i >= 0; i--)
		{
#ifndef FAT_FAST_HASH_SIZE_AUTO
			subtable_row = ((SUB_PATTERN *)input_pattern)[i];
#else
			subtable_row = get_bits(SUB_PATTERN_UNIT_SIZE, i * SUB_PATTERN_UNIT_SIZE, input_pattern, number_of_synapses);
#endif
			wisard_memory = (long long int **) neuron->hash[i];

			if (!wisard_memory[subtable_row])		//if null RAM/hash entry, must alloc a new one
				wisard_memory[subtable_row] = minchinton_fat_fast_neuron_alloc_new_hash_entry(k);
			else						//else, we have a collision
				wisard_memory[subtable_row] = minchinton_fat_fast_neuron_realloc_hash_entry(k, wisard_memory[subtable_row]);
		}
	}

}

void
reload_neuron_hash_pattern_only_overwrite (NEURON_LAYER *neuron_layer, NEURON *neuron, int n) //Neuron id and number of connections must be a parameter also
{
	int i,j,k;
	PATTERN *input_pattern;
	SUB_PATTERN subtable_row;

	/* Get hash parameters */
	minchinton_fat_fast_neuron_initialize_memory_parameters(neuron->n_in_connections, neuron->memory_size);

	/* Reintialize neuron hash (or skip if this is not the first shared memory neuron in the list)*/
	if ((neuron_layer->memory_t == DISTRIBUTED_MEMORY) || (!n && neuron_layer->memory_t == SHARED_MEMORY))
	{	
		//for every neuron on distributed memory layer or just the 1st neuron of a shared memory layer - initialize 
		neuron->hash = alloc_mem ((size_t) (subtable_rows * sizeof (int)));

		for (i = 0; i < subtable_rows; i++)
			((int*)neuron->hash)[i] = -1;		//null pointers empty memory

	}
	else if (neuron_layer->memory_t == SHARED_MEMORY) //If a shared memory has has already been made
	{	//every neuron on a shared memory layer must point to the same hash location
		neuron->hash = neuron_layer->neuron_vector[0].hash;
		return;	//Neuron Hash doesn't need to be rewritten
	}

	/* Hash entry reload */

	//j-1 keeps the latest VG-RAM used memory position
	j = get_neuron_memory_size(neuron);

	//For each sample already stored in neuron memory, read and write on the hash
	for(k = 0; k < j; k++)	
	{
		// Reads the bit pattern from neuron memory
		input_pattern = neuron->memory[k].pattern;

		// For Each neuron table 
		for (i = number_of_subtables-1; i >= 0; i--)
		{
#ifndef FAT_FAST_HASH_SIZE_AUTO
			subtable_row = ((SUB_PATTERN *)input_pattern)[i];
#else
			subtable_row = get_bits(SUB_PATTERN_UNIT_SIZE, i * SUB_PATTERN_UNIT_SIZE, input_pattern, number_of_synapses);
#endif

			((int*)neuron->hash)[i*subtable_rows + subtable_row] = (int) k;		//Overwrites the line entry
		}
	}

}

void
reload_neuron_memory_pattern_only_fat_fat_index (NEURON_LAYER *neuron_layer, NEURON *neuron, int n) //Neuron id and number of connections must be a parameter also
{
	int i, j, k;
	int neuron_memory_size;
	PATTERN *input_pattern;
	SUB_PATTERN subtable_row;

	minchinton_fat_fast_neuron_initialize_memory_parameters(neuron->n_in_connections, neuron->memory_size);

	initialize_neuron_memory_minchinton_fat_fast_index_only(neuron_layer, n);

	//j-1 keeps the latest VG-RAM used memory position
	neuron_memory_size = get_neuron_memory_size(neuron);

	//For each sample already stored in neuron memory, read and write on the hash
	for(k = 0; k < neuron_memory_size; k++)
	{
		// Reads the bit pattern from neuron memory
		input_pattern = neuron->memory[k].pattern;

		/* Minchinton (Index) Memory write */
		for (i = number_of_subtables-1; i >= 0; i--)
		{
#ifndef FAT_FAST_HASH_SIZE_AUTO
			subtable_row = ((SUB_PATTERN *)input_pattern)[i];
#else
			subtable_row = get_bits(SUB_PATTERN_UNIT_SIZE, i * SUB_PATTERN_UNIT_SIZE, input_pattern, number_of_synapses);
#endif

			for(j = 0; j < NEURON_MEMORY_INDEX_SIZE; j++)
			{
				if (neuron->memory_index[i * subtable_size + subtable_row * NEURON_MEMORY_INDEX_SIZE + j] == -1)
					break;
			}
			if ( j == NEURON_MEMORY_INDEX_SIZE )
				j = rand() % NEURON_MEMORY_INDEX_SIZE;

			neuron->memory_index[i * subtable_size + subtable_row * NEURON_MEMORY_INDEX_SIZE + j] = k;
		}
	}

}

void
reload_neuron_hash_pattern_only_wisard (NEURON_LAYER *neuron_layer, NEURON *neuron, int n) //Neuron id and number of connections must be a parameter also
{
	int i,j,k;
	long long int *wisard_memory;
	PATTERN *input_pattern;
	SUB_PATTERN subtable_row;

	/* Get hash parameters */
	minchinton_fat_fast_neuron_initialize_memory_parameters(neuron->n_in_connections, get_neuron_memory_size(neuron));

	/* Reintialize neuron hash (or skip if this is not the first shared memory neuron in the list)*/
	if ((neuron_layer->memory_t == DISTRIBUTED_MEMORY) || (!n && neuron_layer->memory_t == SHARED_MEMORY))

	{	//for every neuron on distributed memory layer or just the 1st neuron of a shared memory layer - initialize 
		neuron->hash = alloc_mem ((size_t) (number_of_subtables * sizeof (void *)));

		for (i = 0; i < number_of_subtables; i++)
		{
			neuron->hash[i] = alloc_mem ((size_t) (subtable_rows * sizeof (long long int)));	//integers
			madvise(neuron->hash[i], subtable_rows * sizeof (long long int), MADV_RANDOM);

			wisard_memory = (long long int *) neuron->hash[i];
			//wisard_memory = (long long int *) neuron->memory[i].pattern;
			for (j = 0; j < subtable_rows; j++)
				wisard_memory[j] = -1ll;
		}
	}
	else if (neuron_layer->memory_t == SHARED_MEMORY)
	{	//every neuron on a shared memory layer must point to the same memory location
		neuron->hash = neuron_layer->neuron_vector[0].hash;
	}

	/* Hash entry reload */

	//j-1 keeps the latest VG-RAM used memory position
	j = get_neuron_memory_size(neuron);

	//For each sample already stored in neuron memory, read and write on the hash
	for(k = 0; k < j; k++)	
	{
		// Reads the bit pattern from neuron memory
		input_pattern = neuron->memory[k].pattern;

		for (i = number_of_subtables-1; i >= 0; i--)
		{
#ifndef FAT_FAST_HASH_SIZE_AUTO
			subtable_row = ((SUB_PATTERN *)input_pattern)[i];
#else
			subtable_row = get_bits(SUB_PATTERN_UNIT_SIZE, i * SUB_PATTERN_UNIT_SIZE, input_pattern, number_of_synapses);
#endif
			wisard_memory = (long long int *) neuron->hash[i];

			if (wisard_memory[subtable_row] == -1ll )
				wisard_memory[subtable_row] = 0x1ll << neuron->memory[k].associated_value.ival;
			else
				wisard_memory[subtable_row] |= 0x1ll << neuron->memory[k].associated_value.ival;
		}
	}

	//WiSARD neuron Memory must be freed after the hash realloc
	if(neuron->memory)
	{
		for (i = 0; i < j; i++)
		{
			if (neuron->memory[i].pattern != NULL)
				free(neuron->memory[i].pattern);

			if (neuron->memory[i].valid_bits != NULL)
				free(neuron->memory[i].valid_bits);
		}

		free(neuron->memory);
		neuron->memory = NULL;
	}
}

void
reload_neuron_memory (NEURON_LAYER *neuron_layer, NEURON *neuron)
{
	int i, memory_used;
	int connections;
	int pattern_flag, vb_flag;

	connections = neuron->n_in_connections;

	if(fread(&memory_used, sizeof (int), 1, neural_connections) != 1)
	{
		fprintf(stderr,"Erro na leitura\n");
		exit (0);
	}

	for (i = 0; i < memory_used; i++)
	{
		if(fread(&pattern_flag, sizeof (int), 1, neural_connections) != 1)
		{
			fprintf(stderr,"Erro na leitura\n");
			exit (0);
		}

		if(pattern_flag)
		{
			if (neuron->memory[i].pattern != NULL)
			{
				free(neuron->memory[i].pattern);
				neuron->memory[i].pattern = NULL;
			}

			(*(neuron_layer->neuron_type->initialize_input_pattern)) (&(neuron->memory[i].pattern), connections);

			if(fread(neuron->memory[i].pattern, sizeof (PATTERN), N_PATTERNS, neural_connections) != N_PATTERNS)
			{
				fprintf(stderr,"Erro na leitura\n");
				exit(0);
			}
		}

		if(fread(&vb_flag, sizeof (int), 1, neural_connections) != 1)
		{
			fprintf(stderr,"Erro na leitura\n");
			exit (0);
		}

		if(vb_flag)
		{
			if (neuron->memory[i].valid_bits != NULL)
				free(neuron->memory[i].valid_bits);

			(*(neuron_layer->neuron_type->initialize_input_pattern)) (&neuron->memory[i].valid_bits, connections);

			if(fread(neuron->memory[i].valid_bits, sizeof (PATTERN), N_PATTERNS, neural_connections) != N_PATTERNS)
			{
				fprintf(stderr,"Erro na leitura\n");
				exit(0);
			}
		}

		if(fread(&neuron->memory[i].associated_value, sizeof (NEURON_OUTPUT), 1, neural_connections) != 1)
		{
			fprintf(stderr,"Erro na leitura\n");
			exit(0);
		}

		if(fread(&neuron->memory[i].confidence, sizeof (int), 1, neural_connections) != 1)
		{
			fprintf(stderr,"Erro na leitura\n");
			exit(0);
		}

	}

	return;
}

void
reload_neuron_memory_wisard (NEURON_LAYER *neuron_layer, NEURON *neuron)
{
	int i, memory_used;
	int connections;
	int pattern_flag, vb_flag;
	NEURON_OUTPUT read_file_neuron_output;

	connections = neuron->n_in_connections;
	PATTERN	read_file_vector[N_PATTERNS];

	if(fread(&memory_used, sizeof (int), 1, neural_connections) != 1)
	{
		fprintf(stderr,"Erro na leitura\n");
		exit (0);
	}

	for (i = 0; i < memory_used; i++)
	{
		if(fread(&pattern_flag, sizeof (int), 1, neural_connections) != 1)
		{
			fprintf(stderr,"Erro na leitura\n");
			exit (0);
		}

		if(pattern_flag)
		{

			if(fread(read_file_vector, sizeof (PATTERN), N_PATTERNS, neural_connections) != N_PATTERNS)
			{
				fprintf(stderr,"Erro na leitura\n");
				exit(0);
			}
		}

		if(fread(&vb_flag, sizeof (int), 1, neural_connections) != 1)
		{
			fprintf(stderr,"Erro na leitura\n");
			exit (0);
		}

		if(vb_flag)
		{		

			if(fread(read_file_vector, sizeof (PATTERN), N_PATTERNS, neural_connections) != N_PATTERNS)
			{
				fprintf(stderr,"Erro na leitura\n");
				exit(0);
			}
		}

		if(fread(&read_file_neuron_output, sizeof (NEURON_OUTPUT), 1, neural_connections) != 1)
		{
			fprintf(stderr,"Erro na leitura\n");
			exit(0);
		}
	}

	return;
}

/*
 *********************************************************************************
 * Unload neuron's xor_table							*
 *********************************************************************************
 */

void
reload_xor_table (NEURON *neuron)
{
	int i;
	int xor_table_flag, connections;

	connections = neuron->n_in_connections;

	if(fread(&xor_table_flag, sizeof (int), 1, neural_connections) != 1)
	{
		fprintf(stderr,"Erro na escrita\n");
		exit(0);
	}

	if(xor_table_flag)
	{
		for(i = 0; i < N_PATTERNS; i++)
		{
			if(fread(neuron->pattern_xor_table[i], sizeof (PATTERN), 256, neural_connections) != 256)
			{
				fprintf(stderr,"Erro na leitura\n");
				exit(0);
			}
		}
	}

	return;
}


/*
 *********************************************************************************
 * Reload a neuron								*
 *********************************************************************************
 */

void
reload_neuron (void)
{
	int n_l_dest_id, n_dest_id;
	NEURON_LAYER *neuron_layer_dest;
	NEURON *neuron_dest;
	NEURON_OUTPUT dendrite_state;
	NEURON_OUTPUT output;

	if(fread(&n_l_dest_id, sizeof (int), 1, neural_connections) != 1)
	{
		fprintf(stderr,"Erro na leitura\n");
		exit (0);
	}

	if(fread(&n_dest_id, sizeof (int), 1, neural_connections) != 1)
	{
		fprintf(stderr,"Erro na leitura\n");
		exit (0);
	}

	if(fread(&dendrite_state, sizeof (NEURON_OUTPUT), 1, neural_connections) != 1)
	{
		fprintf(stderr,"Erro na leitura\n");
		exit (0);
	}

	if(fread(&output, sizeof (NEURON_OUTPUT), 1, neural_connections) != 1)
	{
		fprintf(stderr,"Erro na leitura\n");
		exit (0);
	}

	neuron_layer_dest = n_layer_vec[n_l_dest_id];

	neuron_dest = &(neuron_layer_dest->neuron_vector[n_dest_id]);

	reload_synapses (neuron_dest);

	if(neuron_layer_dest->neuron_type != &wisard)		//WiSARD Neurons shouldn't be allowed to reload memory conventionally
		reload_neuron_memory (neuron_layer_dest, neuron_dest);
	else
		reload_neuron_memory_wisard (neuron_layer_dest, neuron_dest);

	//Remakes the neuron hash for Fat-Fast type neurons (used only when neural layer is reloaded)
#if 1
	if( 	neuron_layer_dest->neuron_type == &minchinton_fat_fast  	||
			neuron_layer_dest->neuron_type == &minchinton_fat_fast_rand )
	{
		reload_neuron_hash_pattern_only (neuron_layer_dest,neuron_dest,n_dest_id); //Neuron id and number of connections must be a parameter
	}
#endif

	reload_xor_table (neuron_dest);

	return;
}


void
reload_neuron_memory_only (NEURON_LAYER *neuron_layer_dest)
{
	int n_l_dest_id, n_dest_id;
	NEURON *neuron_dest;

	if(fscanf(neural_connections, "%d\n", &n_l_dest_id) < 0)
	{
		fprintf(stderr,"Erro na leitura\n");
		exit (0);
	}

	if(fscanf(neural_connections, "%d\n", &n_dest_id) < 0)
	{
		fprintf(stderr,"Erro na leitura\n");
		exit (0);
	}

	neuron_dest = &(neuron_layer_dest->neuron_vector[n_dest_id]);

	//WiSARD Neurons would reload memory and then be freed
	reload_neuron_memory_pattern_only (neuron_layer_dest, neuron_dest);

	//	Checks wheter the destination neuron hash should be remade (for Fat-FAST like or Wisard neuron types only)
	if( 	neuron_layer_dest->neuron_type == &minchinton_fat_fast  	||
			neuron_layer_dest->neuron_type == &minchinton_fat_fast_rand
	)
	{
		reload_neuron_hash_pattern_only (neuron_layer_dest,neuron_dest,n_dest_id); //Neuron id and number of connections must be a parameter		
	}
	else if( neuron_layer_dest->neuron_type == &minchinton_fat_fast_overwrite )	//for fat fast overwrite neurons
	{
		reload_neuron_hash_pattern_only_overwrite (neuron_layer_dest,neuron_dest,n_dest_id);
	}
	else if( neuron_layer_dest->neuron_type == &minchinton_fat_fast_index )	//for fat fast index neurons
	{
		reload_neuron_memory_pattern_only_fat_fat_index (neuron_layer_dest,neuron_dest,n_dest_id);
	}
	else if( neuron_layer_dest->neuron_type == &wisard )	//Reloads the WiSARD neuron type hash
	{
		reload_neuron_hash_pattern_only_wisard (neuron_layer_dest,neuron_dest,n_dest_id);
	}


	//	reload_xor_table (neuron_dest);

	return;
}


void
reload_neuron_layer_from_file (char *file_name, char *neuron_layer_name)
{
	int count_neurons;
	char *aux_str;

	aux_str = alloc_mem (strlen (file_name) + 1);
	strcpy (aux_str, file_name + 1);
	aux_str[strlen (file_name) - 2] = '\0';

	// Opens neural conncetions file
	if((neural_connections = fopen(aux_str, "r")) == NULL)
	{
		Erro ("Cannot open conncetions file: ", aux_str, "");
	}

	free(aux_str);

	aux_str = alloc_mem (strlen (neuron_layer_name) + 1);
	strcpy (aux_str, neuron_layer_name + 1);
	aux_str[strlen (neuron_layer_name) - 2] = '\0';

	NEURON_LAYER *neuron_layer = get_neuron_layer_by_name(aux_str);

	free(aux_str);

	// Checks wheter the neuron memory can be reloaded or not
	if(	neuron_layer->neuron_type != &minchinton 		&& 		//VG-RAM
			neuron_layer->neuron_type != &minchinton_fat_fast  	&&		//FAT-FAST
			neuron_layer->neuron_type != &minchinton_fat_fast_index &&		//FAT-FAST-INDEX
			neuron_layer->neuron_type != &minchinton_fat_fast_rand	&&		//FAT-FAST-RAND
			neuron_layer->neuron_type != &minchinton_fat_fast_overwrite	&&	//FAT-FAST-OVERWRITE
			neuron_layer->neuron_type != &wisard					//WISARD
	)
	{
		show_message("Neuron Layer Memory reload","Only Available For :","VG-RAM , Fat-FAST & WiSARD Neuron Type Layers");
		exit(0);
	}

	count_neurons = get_num_neurons (neuron_layer->dimentions);

	while(count_neurons)
	{
		reload_neuron_memory_only (neuron_layer);
		count_neurons--;
	}

	fclose(neural_connections);

	return;
}


/*
 *********************************************************************************
 * Reload the neural network							*
 *********************************************************************************
 */

void
reload_network (char *file_name)
{
	int count_neurons;
	char *aux_str;

	aux_str = alloc_mem (strlen (file_name) + 1);
	strcpy (aux_str, file_name + 1);
	aux_str[strlen (file_name) - 2] = '\0';

	// Opens neural conncetions file
	if((neural_connections = fopen(aux_str, "rb")) == NULL)
	{
		Erro ("Cannot open conncetions file: ", aux_str, "");
	}

	free(aux_str);

	count_neurons = total_n_neurons;

	while(count_neurons)
	{
		reload_neuron ();
		count_neurons--;
	}

	fclose(neural_connections);

	return;
}


/*
 *********************************************************************************
 * Unload a synapse								*
 *********************************************************************************
 */

void
unload_synapse (SYNAPSE *synapse)
{

	int n_layer_src_id;
	int n_src_id;
	NEURON_OUTPUT sensitivity;
	float weight;

	if (synapse->nl_source == NULL) // synapse connected to neuron_with_output_zero
		n_layer_src_id = -1;
	else
		n_layer_src_id = synapse->nl_source->id;
	n_src_id = synapse->source->id;
	sensitivity = synapse->sensitivity;
	weight = synapse->weight;

	if(fwrite(&n_layer_src_id, sizeof (int), 1, neural_connections) != 1)
	{
		fprintf(stderr,"Erro na escrita\n");
		exit (0);
	}

	if(fwrite(&n_src_id, sizeof (int), 1, neural_connections) != 1)
	{
		fprintf(stderr,"Erro na escrita\n");
		exit (0);
	}

	if(fwrite(&sensitivity, sizeof (NEURON_OUTPUT), 1, neural_connections) != 1)
	{
		fprintf(stderr,"Erro na escrita\n");
		exit (0);
	}

	if(fwrite(&weight, sizeof (float), 1, neural_connections) != 1)
	{
		fprintf(stderr,"Erro na escrita\n");
		exit (0);
	}

	return;
}


/*
 *********************************************************************************
 * Unload neuron's synapses							*
 *********************************************************************************
 */

void
unload_synapses (NEURON *neuron)
{
	SYNAPSE *synapse;
	SYNAPSE_LIST *s_list;
	int num_synapses;

	num_synapses = 0;
	for (s_list = ((SYNAPSE_LIST *) (neuron->synapses)); s_list != (SYNAPSE_LIST *) NULL; s_list = s_list->next)
		num_synapses++;

	if (fwrite(&num_synapses, sizeof (int), 1, neural_connections) != 1)
	{
		fprintf(stderr,"Erro na escrita\n");
		exit (0);
	}

	for (s_list = ((SYNAPSE_LIST *) (neuron->synapses)); s_list != (SYNAPSE_LIST *) NULL; s_list = s_list->next)
	{
		synapse = s_list->synapse;

		unload_synapse (synapse);
	}

	return;
}


/*
 *********************************************************************************
 * Unload neuron's memory							*
 *********************************************************************************
 */

int
print_pattern_string(int memory_bit_group_size, const PATTERN *bit_pattern, char *string_pattern)
{
	int j;
	char bit[2];

	if (string_pattern == NULL)
		return -1;

	for (j=0; j<memory_bit_group_size*PATTERN_UNIT_SIZE; j++)
	{
		sprintf(bit, "%d", get_one_bit(j, bit_pattern));
		string_pattern[j] = bit[0];
	}

	return 0;
}


void
print_pattern(int memory_bit_group_size, PATTERN *input_pattern)
{
	char *string_pattern = NULL;

	string_pattern = calloc(sizeof(char), memory_bit_group_size*PATTERN_UNIT_SIZE+1);

	if (print_pattern_string(memory_bit_group_size, input_pattern, string_pattern) < 0)
	{
		fprintf(stderr,"Erro na escrita\n");
		exit (0);
	}


	if (fprintf(neural_connections, "%s", string_pattern) < 0)
	{
		fprintf(stderr,"Erro na escrita\n");
		exit (0);
	}

	free(string_pattern);
}


void
unload_neuron_memory_pattern_only (NEURON *neuron, OUTPUT_TYPE output_type)
{
	int i, memory_used;
	int connections;
	int pattern_flag;

	connections = neuron->n_in_connections;
	memory_used = 0;

	if (neuron->memory != NULL)
	{
		for (i = 0; (i < neuron->memory_size) && (neuron->memory[i].pattern != NULL); i++)
			memory_used++;
	}

	if(fprintf(neural_connections, "%d\n", memory_used) < 0)
	{
		fprintf(stderr,"Erro na escrita\n");
		exit (0);
	}

	for (i = 0; i < memory_used; i++)
	{
		if(neuron->memory[i].pattern == NULL)
		{
			pattern_flag = 0;

			if(fprintf(neural_connections, "%d\n", pattern_flag) < 0)
			{
				fprintf(stderr,"Erro na escrita\n");
				exit(0);
			}
		}
		else
		{
			pattern_flag = 1;

			if(fprintf(neural_connections, "%d\n", pattern_flag) < 0)
			{
				fprintf(stderr,"Erro na escrita\n");
				exit(0);
			}

			print_pattern(N_PATTERNS, neuron->memory[i].pattern);
		}

		if (output_type == GREYSCALE_FLOAT)
		{
			if(fprintf(neural_connections, "\n%f\n", neuron->memory[i].associated_value.fval) < 0)
			{
				fprintf(stderr,"Erro na leitura\n");
				exit(0);
			}
		}
		else
		{
			if(fprintf(neural_connections, "\n%d\n", neuron->memory[i].associated_value.ival) < 0)
			{
				fprintf(stderr,"Erro na leitura\n");
				exit(0);
			}
		}

	}

	return;
}


void
unload_neuron_memory (NEURON *neuron)
{
	int i, memory_used;
	int connections;
	int pattern_flag, vb_flag;

	connections = neuron->n_in_connections;
	memory_used = 0;

	if (neuron->memory != NULL)
	{
		for (i = 0; (i < neuron->memory_size) && (neuron->memory[i].pattern != NULL); i++)
			memory_used++;
	}

	if(fwrite(&memory_used, sizeof (int), 1, neural_connections) != 1)
	{
		fprintf(stderr,"Erro na escrita\n");
		exit (0);
	}

	for (i = 0; i < memory_used; i++)
	{
		if(neuron->memory[i].pattern == NULL)
		{
			pattern_flag = 0;

			if(fwrite(&pattern_flag, sizeof (int), 1, neural_connections) != 1)
			{
				fprintf(stderr,"Erro na escrita\n");
				exit(0);
			}
		}
		else
		{
			pattern_flag = 1;

			if(fwrite(&pattern_flag, sizeof (int), 1, neural_connections) != 1)
			{
				fprintf(stderr,"Erro na escrita\n");
				exit(0);
			}

			if(fwrite(neuron->memory[i].pattern, sizeof (PATTERN), N_PATTERNS, neural_connections) != N_PATTERNS)
			{
				fprintf(stderr,"Erro na escrita\n");
				exit(0);
			}
		}

		if(neuron->memory[i].valid_bits == NULL)
		{
			vb_flag = 0;

			if(fwrite(&vb_flag, sizeof (int), 1, neural_connections) != 1)
			{
				fprintf(stderr,"Erro na escrita\n");
				exit(0);
			}
		}
		else
		{
			vb_flag = 1;

			if(fwrite(&vb_flag, sizeof (int), 1, neural_connections) != 1)
			{
				fprintf(stderr,"Erro na escrita\n");
				exit(0);
			}

			if(fwrite(neuron->memory[i].valid_bits, sizeof (PATTERN), N_PATTERNS, neural_connections) != N_PATTERNS)
			{
				fprintf(stderr,"Erro na escrita\n");
				exit(0);
			}
		}

		if(fwrite(&neuron->memory[i].associated_value, sizeof (NEURON_OUTPUT), 1, neural_connections) != 1)
		{
			fprintf(stderr,"Erro na escrita\n");
			exit(0);
		}

		if(fwrite(&neuron->memory[i].confidence, sizeof (int), 1, neural_connections) != 1 )
		{
			fprintf(stderr,"Erro na escrita\n");
			exit(0);
		}
	}

	return;
}


/*
 *********************************************************************************
 * Unload neuron's xor_table							*
 *********************************************************************************
 */

void
unload_xor_table (NEURON *neuron)
{
	int i;
	int connections, xor_table_flag;

	connections = neuron->n_in_connections;

	if(neuron->pattern_xor_table == NULL)
	{
		xor_table_flag = 0;

		if(fwrite(&xor_table_flag, sizeof (int), 1, neural_connections) != 1)
		{
			fprintf(stderr,"Erro na escrita\n");
			exit(0);
		}
	}
	else
	{
		xor_table_flag = 1;

		if(fwrite(&xor_table_flag, sizeof (int), 1, neural_connections) != 1)
		{
			fprintf(stderr,"Erro na escrita\n");
			exit(0);
		}

		for(i = 0; i < N_PATTERNS; i++)
		{
			if(fwrite(neuron->pattern_xor_table[i], sizeof (PATTERN), 256, neural_connections) != 256)
			{
				fprintf(stderr,"Erro na escrita\n");
				exit(0);
			}
		}
	}

	return;
}


/*
 *********************************************************************************
 * Unload a neuron								*
 *********************************************************************************
 */

void
unload_neuron (NEURON *neuron)
{
	NEURON_OUTPUT dendrite_state;
	NEURON_OUTPUT output;
	int neuron_id;

	neuron_id = neuron->id;
	dendrite_state = neuron->dendrite_state;
	output = neuron->output;

	if(fwrite(&neuron_id, sizeof (int), 1, neural_connections) != 1)
	{
		fprintf(stderr,"Erro na escrita\n");
		exit (0);
	}

	if(fwrite(&dendrite_state, sizeof (NEURON_OUTPUT), 1, neural_connections) != 1)
	{
		fprintf(stderr,"Erro na escrita\n");
		exit (0);
	}

	if(fwrite(&output, sizeof (NEURON_OUTPUT), 1, neural_connections) != 1)
	{
		fprintf(stderr,"Erro na escrita\n");
		exit (0);
	}

	unload_synapses (neuron);

	unload_neuron_memory (neuron);

	unload_xor_table (neuron);

	return;
}


void
unload_neuron_memory_only (NEURON *neuron, OUTPUT_TYPE output_type)
{
	int neuron_id;

	neuron_id = neuron->id;

	if(fprintf(neural_connections, "%d\n", neuron_id) < 0)
	{
		fprintf(stderr,"Erro na escrita\n");
		exit (0);
	}

	unload_neuron_memory_pattern_only (neuron, output_type);

	return;
}


/*
 *********************************************************************************
 * Unload a neural layer								*
 *********************************************************************************
 */

void
unload_neuron_layer (NEURON_LAYER *neuron_layer)
{
	NEURON *neuron;
	int i;
	int neuron_layer_id = neuron_layer->id;

	for (i = 0; i < neuron_layer->num_neurons; i++)
	{
		if(neuron_layer->memory_t == DISTRIBUTED_MEMORY)
			neuron = &(neuron_layer->neuron_vector[i]);
		else
			neuron = &(neuron_layer->neuron_vector[0]) + (neuron_layer->neuron_vector[0].memory_size*neuron_layer->num_neurons)*i;
//			neuron = &(neuron_layer->neuron_vector[0]) + (NEURON_MEMORY_SIZE*neuron_layer->num_neurons)*i;

		if(fwrite(&neuron_layer_id, sizeof (int), 1, neural_connections) != 1)
		{
			fprintf(stderr,"Erro na escrita\n");
			exit (0);
		}

		unload_neuron (neuron);

		fflush (neural_connections);
	}

	return;
}


void
unload_neuron_layer_memory_only (NEURON_LAYER *neuron_layer)
{
	NEURON *neuron;
	int i;
	int neuron_layer_id = neuron_layer->id;

	for (i = 0; i < neuron_layer->num_neurons; i++)
	{
		if(neuron_layer->memory_t == DISTRIBUTED_MEMORY)
			neuron = &(neuron_layer->neuron_vector[i]);
		else
			neuron = &(neuron_layer->neuron_vector[0]) + (neuron_layer->neuron_vector[0].memory_size*neuron_layer->num_neurons)*i;
//			neuron = &(neuron_layer->neuron_vector[0]) + (NEURON_MEMORY_SIZE*neuron_layer->num_neurons)*i;

		if(fprintf(neural_connections, "%d\n", neuron_layer_id) < 0)
		{
			fprintf(stderr,"Erro na escrita\n");
			exit (0);
		}

		unload_neuron_memory_only (neuron, neuron_layer->output_type);

		fflush (neural_connections);
	}

	return;
}


void
unload_neuron_layer_to_file (char *file_name, char *neuron_layer_name)
{
	char *aux_str;

	aux_str = alloc_mem (strlen (file_name) + 1);
	strcpy (aux_str, file_name + 1);
	aux_str[strlen (file_name) - 2] = '\0';

	// Creates neural conncetions file
	if((neural_connections = fopen(aux_str, "w")) == NULL)
	{
		Erro ("Cannot create connections file: ", aux_str, "");
	}

	free(aux_str);

	aux_str = alloc_mem (strlen (neuron_layer_name) + 1);
	strcpy (aux_str, neuron_layer_name + 1);
	aux_str[strlen (neuron_layer_name) - 2] = '\0';

	NEURON_LAYER *neuron_layer = get_neuron_layer_by_name(aux_str);

	free(aux_str);

	// Checks wheter the neuron memory can be unloaded or not
	if(	neuron_layer->neuron_type != &minchinton 		&&
			neuron_layer->neuron_type != &minchinton_fat_fast  	&&
			neuron_layer->neuron_type != &minchinton_fat_fast_index 	&&
			neuron_layer->neuron_type != &minchinton_fat_fast_rand
	)
	{
		show_message("Neuron Layer Memory Unload","Only Available For :","VG-RAM & Fat-FAST Neuron Type Layers");
		exit(0);
	}

	unload_neuron_layer_memory_only (neuron_layer);

	fclose(neural_connections);

	return;
}


/*
 *********************************************************************************
 * Unload the outputs								*
 *********************************************************************************
 */

void
unload_outputs (void)
{
	OUTPUT_LIST *out_list;
	NEURON_LAYER *neuron_layer;

	for (out_list = global_output_list; out_list != (OUTPUT_LIST *) NULL; out_list = out_list->next)
	{
		neuron_layer = out_list->output->neuron_layer;

		unload_neuron_layer (neuron_layer);
	}

	return;
}


/*
 *********************************************************************************
 * Unload the neural layers							*
 *********************************************************************************
 */

void
unload_neuron_layers (void)
{
	NEURON_LAYER_LIST *nl_list;
	NEURON_LAYER *neuron_layer;

	for (nl_list = global_neuron_layer_list; nl_list != (NEURON_LAYER_LIST *) NULL; nl_list = nl_list->next)
	{
		neuron_layer = nl_list->neuron_layer;

		unload_neuron_layer (neuron_layer);
	}

	return;
}


/*
 *********************************************************************************
 * Unload the neural network							*
 *********************************************************************************
 */

void
unload_network (char *file_name)
{
	char *aux_str;

	aux_str = alloc_mem (strlen (file_name) + 1);
	strcpy (aux_str, file_name + 1);
	aux_str[strlen (file_name) - 2] = '\0';

	// Creates neural conncetions file
	if((neural_connections = fopen(aux_str, "wb")) == NULL)
	{
		Erro ("Cannot create connections file: ", aux_str, "");
	}

	free(aux_str);

	unload_neuron_layers ();

	fclose(neural_connections);

	return;
}


