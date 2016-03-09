/*
*********************************************************************************
* Module : Learning functions							*
* version: 1.0									*
*    date: 01/08/1997								*
*      By: Alberto Ferreira de Souza						*
********************************************************************************* 
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "libga100/ga.h"
#include "mae.h"
#include "syn_neu.h"

/* This macro has already defined in mae.h - Commented by: Jorcy Neto */
/* #define N_PATTERNS	(((connections % PATTERN_UNIT_SIZE) == 0) ? \
			 (connections / PATTERN_UNIT_SIZE): (connections / PATTERN_UNIT_SIZE) + 1) */

#define CUT_POINT	3

#define REMOVE_SYNAPSE	1
#define ADD_SYNAPSE	2


GA_Info_Ptr ga_info = NULL;
NEURON *current_neuron = NULL;


/*
*********************************************************************************
* Switch Neuron Output								*
*********************************************************************************
*/

void
switch_neuron_output (NEURON *neuron, OUTPUT_TYPE output_type)
{
	if (output_type == COLOR)
		neuron->output.ival = (neuron->output.ival + NUM_COLORS/2) % NUM_COLORS; 
	else if (output_type == GREYSCALE)	
		neuron->output.ival = (neuron->output.ival + NUM_GREYLEVELS/2) % NUM_GREYLEVELS; 
	else 	/* b&w neuron */
		neuron->output.ival = (neuron->output.ival == 0)? NUM_COLORS-1: 0;

	neuron->dendrite_state = neuron->output;
}


/*
*********************************************************************************
* Update pattern  xor table for the genetic algorithm				*
*********************************************************************************
*/

void 
update_pattern_xor_table (Chrom_Ptr chrom, int connections, int best) 
{
	int i, j, bit;
	
	if (current_neuron->pattern_xor_table == NULL)
	{
		current_neuron->pattern_xor_table = (PATTERN **) alloc_mem (N_PATTERNS * sizeof (PATTERN *));
		for (i = 0; i < N_PATTERNS; i++)
			current_neuron->pattern_xor_table[i] = (PATTERN *) alloc_mem (256 * sizeof (PATTERN));
	}
	
	for (i = 0; i < N_PATTERNS; i++)
		for (j = 0; j < 256; j++)
			current_neuron->pattern_xor_table[i][j] = pattern_xor_table[j];

	for (bit = 0; bit < chrom->length; bit++)
	{	/* each chromossome's gene corresponds to a bit of a pattern */
		for (j = 0; j < 256; j++)
		{	/* for every bit that is equal to one after a hamming distance comparison 
			   between two patterns, or a one in the binary representation of j (see 
			   hamming_distance_correlate ()), adds (or subtract) the cost 
			   of gene[bit] */
			if (j & (1 << (bit % PATTERN_UNIT_SIZE)))
			{
				if (best)
				{
					if (((int) chrom->gene[bit] > 0) && ((int) chrom->gene[bit] <= CUT_POINT))
						current_neuron->pattern_xor_table[bit / PATTERN_UNIT_SIZE][j] += (int) chrom->gene[bit] + 7 - CUT_POINT;
					else if ((int) chrom->gene[bit] > CUT_POINT)
						current_neuron->pattern_xor_table[bit / PATTERN_UNIT_SIZE][j] += 7;
				}
				else
				{
					if ((int) chrom->gene[bit] == 0)
						current_neuron->pattern_xor_table[bit / PATTERN_UNIT_SIZE][j] -= 1;
					else
						current_neuron->pattern_xor_table[bit / PATTERN_UNIT_SIZE][j] += (int) chrom->gene[bit] - 1;
				}
			}
		}
	}
}



/*
*********************************************************************************
* Genetic algorithm objective function - minimize number of synapses		*
*********************************************************************************
*/

void 
genetic_algorithm_objective_function (Chrom_Ptr chrom) 
{
	int i, distance, nearest_pattern, sum_weights, errors;
	ASSOCIATION *memory;
	
	memory = current_neuron->memory;

	update_pattern_xor_table (chrom, chrom->length, 0);
	
	for (i = 0, errors = 0; (i < NEURON_MEMORY_SIZE) && (memory[i].pattern != NULL); i++)
	{
		nearest_pattern = find_nearest_pattern_correlate (current_neuron, memory[i].pattern, chrom->length, &distance);
		if ((memory[nearest_pattern].associated_value.ival != memory[i].associated_value.ival) || (distance != 0))
			errors += 1;
	}
	
	for (i = 0, sum_weights = 0; i < chrom->length; i++)
		sum_weights += chrom->gene[i];
	

/*
	if (sum_weights == 0)
		chrom->fitness = (float) (1 + 7 * errors);
	else
		chrom->fitness = (float) sum_weights / (float) chrom->length + 7.0 * (float) errors;
*/
/*
	
	if (errors == 0)
		chrom->fitness = 7.0 * (float) chrom->length / (float) sum_weights + (float) sum_weights;
	else
		chrom->fitness = (float) sum_weights *  errors ;
*/

	if (sum_weights == 0)
		chrom->fitness = 1.0 + (float) chrom->length * errors;
	else
		chrom->fitness = (float) sum_weights + (float) chrom->length * (float) errors;
/*

	if ((1 - sum_weights) > 0)
		chrom->fitness = (float) (1 - sum_weights + errors);
	else
		chrom->fitness = (float) (sum_weights - 1 + errors);
*/
	
/*	
	{
		float a, b, c;
		
		a = 1.0;
		b = - ((float) chrom->length * 2.0 * a);
		c = (b * b) / (4.0 * a);
		chrom->fitness = a * (float) sum_weights * (float) sum_weights + b * (float) sum_weights + c;
		chrom->fitness += (float) chrom->length * (float) errors;
	}
*/
 
}


/*
*********************************************************************************
* Initialize a pool of synapses							*
*********************************************************************************
*/


void
init_pool_of_synapses ( )
{
	int i, j;
	FILE *arq;
	
	/*---Open file---*/	
	if ((arq = fopen (ga_info->ip_data, "w")) == NULL)
	{
		printf ("Could not open file %s\n", ga_info->ip_data);
		exit (-1);
	}
	
	fprintf(arq,"# Chromosome length\n\n %d\n\n", ga_info->chrom_len);
	fprintf(arq,"# Pool size %d\n\n", ga_info->pool_size);
	fprintf(arq,"# Initial pool\n\n");

	SEED_RAND(ga_info->rand_seed);
	
	/*---First chromossome---*/
	for (i = 0; i < ga_info->chrom_len; i++)
		fprintf(arq,"1 ");
	fprintf(arq,"\n");
	
	/*---Others chromossomes---*/
	for (j = 1; j < ga_info->pool_size; j++)
	{
		for (i = 0; i < ga_info->chrom_len; i++)
			fprintf(arq,"%d ", RAND_DOM(0, ga_info->max_gene_val));
		fprintf(arq,"\n");
	}
	fclose(arq);
}




/*
*********************************************************************************
* Correlate neuron knowledge							*
*********************************************************************************
*/

void
correlate_neuron_knowledge (NEURON_LAYER *neuron_layer, int n)
{
	/*int i, j;*/
	int connections;
	
	
	/*---Set the current neuron---*/
	current_neuron = &(neuron_layer->neuron_vector[n]);
	connections = current_neuron->n_in_connections;
	
	
	/*--- Initialize the genetic algorithm ---*/
	if (ga_info == NULL)
		ga_info = GA_config ("ga-correlate.cfg", genetic_algorithm_objective_function);
	else
		GA_reset (ga_info, "ga-correlate.cfg");
	
	ga_info->chrom_len = connections;
	
	if (strlen (ga_info->ip_data) == 0)
		strcpy (ga_info->ip_data, "initpool.dat");

	/*---Initialize a poll of synapses---*/
	if (ga_info->ip_flag == IP_RANDOM)
		init_pool_of_synapses ();
	
		
	/*--- Run the GA ---*/
	GA_run (ga_info);
	
	update_pattern_xor_table (ga_info->best, connections, 1);


/*	for (i = 0; i < N_PATTERNS; i++)
	{
		for (j = 0; j < 256; j++)
		{
			printf ("%d - %x -> %x, %x\n", i, j, (int) current_neuron->pattern_xor_table[i][j], (int) pattern_xor_table[j]);
		}
		printf ("\n\n");
	}
*/
}



/*
*********************************************************************************
* Correlate whole network							*
*********************************************************************************
*/

void 
correlate_network ()
{
	NEURON_LAYER_LIST *nl_list;
	NEURON *neuron_vector;
	long num_neurons;
	long i;

	for (nl_list = global_neuron_layer_list; nl_list != (NEURON_LAYER_LIST *) NULL;
		nl_list = nl_list->next)
	{
		num_neurons = get_num_neurons (nl_list->neuron_layer->dimentions);
		neuron_vector = nl_list->neuron_layer->neuron_vector;
		if (((SYNAPSE_LIST *) (neuron_vector[0].synapses))->synapse->sensitivity.ival != DIRECT_CONNECTION)
			for (i = 0; i < num_neurons; i++)
				correlate_neuron_knowledge (nl_list->neuron_layer, i);
	}
}


/*
*********************************************************************************
* Correlate a list of neural layers						*
*********************************************************************************
*/

void
correlate_neural_layers (char *neural_layers)
{
	NEURON *neuron_vector;
	long num_neurons;
	long i;
	char *next_name;
	char name[SYMBOL_SIZE];
	NEURON_LAYER *neuron_layer;
	
	next_name = neural_layers;
	while ((next_name = get_name (next_name, name)) != NULL)
	{
		if (get_object_by_name (name, (void **) &neuron_layer) == NEURON_LAYER_OBJECT)
		{
			num_neurons = get_num_neurons (neuron_layer->dimentions);
			neuron_vector = neuron_layer->neuron_vector;
			if (((SYNAPSE_LIST *) (neuron_vector[0].synapses))->synapse->sensitivity.ival != DIRECT_CONNECTION)
				for (i = 0; i < num_neurons; i++)
					correlate_neuron_knowledge (neuron_layer, i);
		}
		else
			Erro ("Attempt to backpropagate object different of neural_layer named:", name, "");
	}
}


/*
*********************************************************************************
* Update pattern xor table for prunning						*
*********************************************************************************
*/

void 
update_pattern_xor_table_prunning (NEURON *neuron, int num_synapses, int synapse, int action) 
{
	int i, j, bit;
	int connections;
	
	/* N_PATTERNS is a macro which needs a variable named connections */
	connections = num_synapses;
	
	if (neuron->pattern_xor_table == NULL)
	{
		neuron->pattern_xor_table = (PATTERN **) alloc_mem (N_PATTERNS * sizeof (PATTERN *));
		for (i = 0; i < N_PATTERNS; i++)
			neuron->pattern_xor_table[i] = (PATTERN *) alloc_mem (256 * sizeof (PATTERN));
	}
	
	for (i = 0; i < N_PATTERNS; i++)
		for (j = 0; j < 256; j++)
			neuron->pattern_xor_table[i][j] = pattern_xor_table[j];

	for (bit = 0; bit < num_synapses; bit++)
	{	/* each chromossome's gene corresponds to a bit of a pattern */
		if (bit == synapse)
		{
			for (j = 0; j < 256; j++)
			{	/* for every bit that is equal to one after a hamming distance comparison 
				   between two patterns, or a one in the binary representation of j (see 
				   hamming_distance_correlate ()), adds (or subtract) the influence 
				   of the synapse */
				if (j & (1 << (bit % PATTERN_UNIT_SIZE)))
				{
					if (action == REMOVE_SYNAPSE)
						neuron->pattern_xor_table[bit / PATTERN_UNIT_SIZE][j] -= 1;
					else
						neuron->pattern_xor_table[bit / PATTERN_UNIT_SIZE][j] += 1;
				}
			}
		}
	}
}



int
count_errors_in_knowledge (NEURON *neuron, int num_synapses)
{
	int i, distance, nearest_pattern, errors;
	ASSOCIATION *memory;
	
	memory = neuron->memory;
	for (i = 0, errors = 0; (i < neuron->memory_size) && (memory[i].pattern != NULL); i++)
	{
		nearest_pattern = find_nearest_pattern_correlate (neuron, memory[i].pattern, num_synapses, &distance);
		if ((memory[nearest_pattern].associated_value.ival != memory[i].associated_value.ival) || (distance != 0))
			errors += 1;
	}
	return (errors);
}



void
report_prunning_results (int neuron, int num_synapses, int prunned_synapses, int initial_num_errors, int current_num_errors)
{
	printf ("Prunning report: Neuron = %d\n", neuron);
	printf ("Prunning report: Synapses before = %d, Synapses after = %d\n", num_synapses, num_synapses - prunned_synapses);
	printf ("Prunning report: Number of errors before = %d, Number of errors after = %d\n\n", initial_num_errors, current_num_errors);
	fflush (stdout);
}

		

/*
*********************************************************************************
* Minimize number of synapses via prunning					*
*********************************************************************************
*/

void 
prune_neuron_sinapses (NEURON_LAYER *neuron_layer, int n)
{
	NEURON *neuron;
	int num_synapses;
	int synapse, current_num_errors, initial_num_errors, prunned_synapses;
	
	neuron = &(neuron_layer->neuron_vector[n]);
	num_synapses = neuron->n_in_connections;
	current_num_errors = 0;

	initial_num_errors = count_errors_in_knowledge (neuron, num_synapses);
	for (synapse = prunned_synapses = 0; synapse < num_synapses; synapse++)
	{
		update_pattern_xor_table_prunning (neuron, num_synapses, synapse, REMOVE_SYNAPSE);
		current_num_errors = count_errors_in_knowledge (neuron, num_synapses);
		if (current_num_errors > initial_num_errors)
			update_pattern_xor_table_prunning (neuron, num_synapses, synapse, ADD_SYNAPSE);
		else
			prunned_synapses++;
	}
	
	report_prunning_results (n, num_synapses, prunned_synapses, initial_num_errors, current_num_errors);
}


/*
*********************************************************************************
* Prune the synapses of the whole network					*
*********************************************************************************
*/

void 
prune_network ()
{
	NEURON_LAYER_LIST *nl_list;
	NEURON *neuron_vector;
	long num_neurons;
	long i;

	for (nl_list = global_neuron_layer_list; nl_list != (NEURON_LAYER_LIST *) NULL;
		nl_list = nl_list->next)
	{
		num_neurons = get_num_neurons (nl_list->neuron_layer->dimentions);
		neuron_vector = nl_list->neuron_layer->neuron_vector;
		if (((SYNAPSE_LIST *) (neuron_vector[0].synapses))->synapse->sensitivity.ival != DIRECT_CONNECTION)
			for (i = 0; i < num_neurons; i++)
				prune_neuron_sinapses (nl_list->neuron_layer, i);
	}
}


/*
*********************************************************************************
* Prune the synapses of a list of neural layers					*
*********************************************************************************
*/

void
prune_neural_layers (char *neural_layers)
{
	NEURON *neuron_vector;
	long num_neurons;
	long i;
	char *next_name;
	char name[SYMBOL_SIZE];
	NEURON_LAYER *neuron_layer;
	
	next_name = neural_layers;
	while ((next_name = get_name (next_name, name)) != NULL)
	{
		if (get_object_by_name (name, (void **) &neuron_layer) == NEURON_LAYER_OBJECT)
		{
			num_neurons = get_num_neurons (neuron_layer->dimentions);
			neuron_vector = neuron_layer->neuron_vector;
			if (((SYNAPSE_LIST *) (neuron_vector[0].synapses))->synapse->sensitivity.ival != DIRECT_CONNECTION)
				for (i = 0; i < num_neurons; i++)
					prune_neuron_sinapses (neuron_layer, i);
		}
		else
			Erro ("Attempt to backpropagate object different of neural_layer named:", name, "");
	}
}


/*
*********************************************************************************
* Clear neuron memory								*
*********************************************************************************
*/

void
clear_neuron_memory (NEURON_LAYER *neuron_layer, int n)
{
	int i;
	NEURON *neuron;
	
	long long int *wisard_memory;
	long long int **neuron_hash_memory;
	int number_of_tables;
	int table_size;
	int log2_table_size;
	int j;

	neuron = &(neuron_layer->neuron_vector[n]);
	
	if(neuron_layer->neuron_type == &wisard)	//for a wisard type neuron layer we must perform the same process as the memory initialise
	{
		get_wisard_table_size_and_log2_table_size_and_number_of_tables(&log2_table_size, &number_of_tables, neuron->memory_size, neuron);
		table_size = (int) pow(2.0, (double) log2_table_size);
			
		for (i = 0; i < number_of_tables; i++)
		{
			wisard_memory = (long long int *) neuron->hash[i];
			for (j = 0; j < table_size; j++)
				wisard_memory[j] = -1;
		}
	}
	else if ((neuron_layer->neuron_type == &minchinton_fat_fast) ||
			(neuron_layer->neuron_type == &minchinton_fat_fast_rand))	//For minchinton-ff a very similar procedure must be applied
	{
		if (neuron->memory != NULL)
		{
			for (i = 0; (i < NEURON_MEMORY_SIZE) && (neuron->memory[i].pattern != NULL); i++)
			{
				free (neuron->memory[i].pattern);		//Alloced patterns must be freed
				neuron->memory[i].pattern = NULL;		//Must be reset as NULL
			}

			get_wisard_table_size_and_log2_table_size_and_number_of_tables(&log2_table_size, &number_of_tables, NEURON_MEMORY_SIZE, neuron);
			table_size = (int) pow(2.0, (double) log2_table_size);

			//for (i = 0; i < number_of_tables; i++)		// TODO: Changed to number_of_subtable
			for (i = 0; i < number_of_subtables; i++)
			{
				neuron_hash_memory = (long long int **) neuron->hash[i];
				//for (j = 0; j < table_size; j++)		// TODO: Changed to subtable_rows
				for (j = 0; j < subtable_rows; j++)
					if(neuron_hash_memory[j])		//Alloced hash entries must be freed also
					{
						free(neuron_hash_memory[j]);
						neuron_hash_memory[j] = NULL;	//Must be reset as NULL
					}
			}
		}
	}
	else
	{
		if (neuron->memory != NULL)
		{
			for (i = 0; (i < NEURON_MEMORY_SIZE) && (neuron->memory[i].pattern != NULL); i++)
			{
				free (neuron->memory[i].pattern);
			}

			if (neuron_layer->neuron_type != NULL)
				(*(neuron_layer->neuron_type->initialise_neuron_memory)) (neuron_layer, n);
		}
	}
	neuron->dendrite_state.ival = 0;
}


/*
*********************************************************************************
* Clear whole network memory							*
*********************************************************************************
*/


void
clear_network_memory (void)
{
	NEURON_LAYER_LIST *nl_list;
	long num_neurons;
	long i;

	for (nl_list = global_neuron_layer_list; nl_list != (NEURON_LAYER_LIST *) NULL;
		nl_list = nl_list->next)
	{
		num_neurons = get_num_neurons (nl_list->neuron_layer->dimentions);
		if(nl_list->neuron_layer->memory_t == DISTRIBUTED_MEMORY)
		{
			for (i = 0; i < num_neurons; i++)
				clear_neuron_memory (nl_list->neuron_layer, i);
		}
		else
			clear_neuron_memory (nl_list->neuron_layer, 0);	//Makes the memory clearing much more effective
	}
}



/*
*********************************************************************************
* Clear the memory of a list of neural layers					*
*********************************************************************************
*/

void
clear_neural_layers_memory (char *neural_layers)
{
	long	num_neurons;
	char	*next_name;
	char	name[SYMBOL_SIZE];
	NEURON_LAYER	*neuron_layer;
	long 	i;
	NEURON	*neuron_vector;
	
	next_name = neural_layers;
	while ((next_name = get_name (next_name, name)) != NULL)
	{
		if (get_object_by_name (name, (void **) &neuron_layer) == NEURON_LAYER_OBJECT)
		{
			num_neurons = get_num_neurons (neuron_layer->dimentions);
			//num_neurons = get_num_neurons (neuron_layer->dimentions);
			neuron_vector = neuron_layer->neuron_vector;
			if (((SYNAPSE_LIST *) (neuron_vector[0].synapses))->synapse->sensitivity.ival != DIRECT_CONNECTION)
			{
				if(neuron_layer->memory_t == DISTRIBUTED_MEMORY)
				{
					for (i = 0; i < num_neurons; i++)
						clear_neuron_memory (neuron_layer, i);
				}
				else
					clear_neuron_memory (neuron_layer, 0);	//Makes the memory clearing much more effective
			}
		}
		else
			Erro ("Attempt to clear memory of object different of neural_layer named:", name, "");
	}
}
