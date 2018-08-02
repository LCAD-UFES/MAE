/*
*********************************************************************************
* Module : Synapse and neuron functions						*
* version: 1.0									*
*    date: 01/08/1997								*
*      By: Alberto Ferreira de Souza						*
********************************************************************************* 
*/

#include "syn_neu.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mae.h>

// Initialized as NULL the pointer vector
unsigned int	*random_neuron_index_vector = NULL;
int		random_neuron_index_vector_size;


/*
*********************************************************************************
* A fast random integer generating function for MAE				*
*********************************************************************************
*/

// Obtained from : http://software.intel.com/en-us/articles/fast-random-number-generator-on-the-intel-pentiumr-4-processor
void 
mae_fastrand_srand(int seed)
{
	g_seed = seed;
}


int 
mae_fastrand()
{
	g_seed = (214013*g_seed+2531011); 
	return (g_seed>>16)&0x7FFF; 
} 

/*
*********************************************************************************
* A MAE cache long prefetch function 						*
*********************************************************************************
*/

// Obtained from http://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html
void 
mae_long_prefetch_high_priority(void *addr, size_t len) // len is the lenght in bytes
{
	char *cp;
	char *end = addr + len;

	for (cp = addr; cp < end;cp += PREFETCH_STRIDE)
		__builtin_prefetch(cp,0,3);	//Read High Priority
}

void 
mae_long_prefetch_low_priority(void *addr, size_t len)
{
	char *cp;
	char *end = addr + len;

	for (cp = addr; cp < end;cp += PREFETCH_STRIDE)
		__builtin_prefetch(cp,0,0);	//Read Low Priority (no temporal locality)
}

int
get_neuron_memory_size(NEURON *neuron)
{
	int j = 0;

	if (neuron)
	{
		if (neuron->memory[neuron->memory_size-1].pattern == NULL)
		{
			for (j = 0; j < neuron->memory_size; j++)
				if (neuron->memory[j].pattern == NULL)
					break;
		}
		else
			j = neuron->memory_size;
	}

	return j;
}

/*
*********************************************************************************
* Set neurons with a value							*
*********************************************************************************
*/

void
set_neurons (NEURON *n, int n0, int num_neurons, int value)
{
	int i;
	
	for (i = 0; i < num_neurons; i++)
	{
		n[i+n0].output.ival = value;
	}
}


/*
*********************************************************************************
* Set a region x of neurons with a value					*
*********************************************************************************
*/

void
setx_neurons (NEURON *n, int right, int nx, int ny, int num_neurons, int value)
{
	int i, x, y;
	
	if (nx % 2)
	{
		y = ny / 2;
		x = nx / 2;
	}
	else
	{
		y = 0;
		if (right)
			x = nx / 2;
		else
			x = (nx / 2) - 1;
	}
	for (i = 0; i < num_neurons; i++)
	{
		n[x + y*nx].output.ival = value;
		y++;
		if (y == ny)
		{
			y = 0;
			if (right)
				x++;
			else
				x--;
		}
	}
}


/*
*********************************************************************************
* Set a region y of neurons with a value					*
*********************************************************************************
*/

void
sety_neurons (NEURON *n, int up, int ny, int nx, int num_neurons, int value)
{
	int i, x, y;
	
	if (ny % 2)
	{
		x = nx / 2;
		y = ny / 2;
	}
	else
	{
		x = 0;
		if (up)
			y = ny / 2;
		else
			y = (ny / 2) - 1;
	}
	for (i = 0; i < num_neurons; i++)
	{
		n[x + y*nx].output.ival = value;
		x++;
		if (x == nx)
		{
			x = 0;
			if (up)
				y++;
			else
				y--;
		}
	}
}


/*
*********************************************************************************
* Gets one bit from input_pattern						*
*********************************************************************************
*/

int
get_one_bit(int start_bit, const PATTERN *input_pattern)
{
	int desired_bit;
	int pattern_skip, bit_skip;
	int num_bits_per_PATTERN;
	PATTERN	local_pattern;
	
	num_bits_per_PATTERN = 8 * sizeof(PATTERN);
	pattern_skip = start_bit / num_bits_per_PATTERN;
	bit_skip = start_bit % num_bits_per_PATTERN;
	
	local_pattern = input_pattern[pattern_skip];
		
	desired_bit = (local_pattern >> bit_skip) & 1;
	
	return (desired_bit);
}


/*
*********************************************************************************
* Gets the number of desired bits at a max of 64				*
*********************************************************************************
*/

int
get_bits(int num_bits, int start_bit, PATTERN *input_pattern, int input_pattern_size)
{
	int desired_bits;
	int i;
	
	if ((num_bits < 0) || (num_bits > 16) || (start_bit < 0) || ((start_bit + num_bits) > input_pattern_size))
		Erro("Invalid number of bits in get_bits()", "", "");
	
	desired_bits = 0;
	for (i = 0; i < num_bits; i++)
		desired_bits |= get_one_bit(start_bit + i, input_pattern) << i;
	
	return (desired_bits);
}


/*
*********************************************************************************
* Check how many average patterns exist for the neuron layer 			*
*********************************************************************************
*/

float
count_neuron_layer_average_candidates (char *name)
{
	NEURON_LAYER *neuron_layer;
	NEURON *neuron_vector;
	long num_neurons, avg_patterns;
	long i;
	
	if ((neuron_layer = get_neuron_layer_by_name (name)) == NULL)
	{
		show_message ("Unknown neuron layer:", name, "");
		return 0.0;
	}
	
	num_neurons = get_num_neurons (neuron_layer->dimentions);
	neuron_vector = neuron_layer->neuron_vector;

	avg_patterns = 0;
	for (i = 0; i < num_neurons; i++)
		avg_patterns += neuron_vector[i].last_hamming_distance;

	return (float) avg_patterns / (float) num_neurons;
}


/*
*********************************************************************************
* Obtains the composed color selective synapse input value			*
*********************************************************************************
*/

int
obtain_composed_color_synapse_value(SYNAPSE *synapse)
{
	int	synapse_input;
	int	composed_color_output;

	composed_color_output = 0;
	if(synapse)	//test whether the synapse exists
	{
		synapse_input = synapse->source->output.ival;
		
		switch(synapse->sensitivity.ival)
		{
			case 0:	// RED
				composed_color_output = RED(synapse_input);
				break;
			case 1:	// GREEN
				composed_color_output = GREEN(synapse_input);
				break;
			case 2:	// BLUE
				composed_color_output = BLUE(synapse_input);
				break;
			case 3:	// RED + GREEN
				//composed_color_output = (RED(synapse_input) + GREEN(synapse_input))/2;
				composed_color_output = (RED(synapse_input) + GREEN(synapse_input));
				break;
			case 4:	// RED + BLUE
				//composed_color_output = (RED(synapse_input) + BLUE(synapse_input))/2;
				composed_color_output = (RED(synapse_input) + BLUE(synapse_input));
				break;
			case 5:	// BLUE + GREEN
				//composed_color_output = (BLUE(synapse_input) + GREEN(synapse_input))/2;
				composed_color_output = (BLUE(synapse_input) + GREEN(synapse_input));
				break;
			case 6:	// RED + GREEN + BLUE
				//composed_color_output = (RED(synapse_input) + GREEN(synapse_input) + BLUE(synapse_input))/3;
				composed_color_output = (RED(synapse_input) + GREEN(synapse_input) + BLUE(synapse_input));
				break;
			case 7: // (RED + GREEN)/2
				composed_color_output = (RED(synapse_input) + GREEN(synapse_input))/2;
				break;
			case 8: // (RED + BLUE)/2
				composed_color_output = (RED(synapse_input) + BLUE(synapse_input))/2;
				break;
			case 9: // (BLUE + GREEN)/2
				composed_color_output = (BLUE(synapse_input) + GREEN(synapse_input))/2;
				break;
			case 10:// (RED + GREEN + BLUE)/3
				composed_color_output = (RED(synapse_input) + GREEN(synapse_input) + BLUE(synapse_input))/3;
				break;
			default:// RED + GREEN + BLUE
				//composed_color_output = (RED(synapse_input) + GREEN(synapse_input) + BLUE(synapse_input))/3;
				composed_color_output = (RED(synapse_input) + GREEN(synapse_input) + BLUE(synapse_input));
				break;
		}
	}
	else
		composed_color_output = -1;	// Error condition
		
	return(composed_color_output);
}

/*
*********************************************************************************
* Obtains the composed color selective weighted synapse input value		*
*********************************************************************************
*/

float
obtain_composed_color_synapse_weighted_value(SYNAPSE *synapse)
{
	int	synapse_input;
	int	red_level;
	int	green_level;
	int	blue_level;
	
	float	composed_color_output;

	synapse_input = synapse->source->output.ival;
		
	red_level = RED(synapse->sensitivity.ival);
	green_level = GREEN(synapse->sensitivity.ival);
	blue_level = BLUE(synapse->sensitivity.ival);
		

/*	composed_color_output = 0.3 * (( ( (float) RED(synapse_input) * (float) red_level ) / (float) (255) ) +
					0.59 * ( ( (float) GREEN(synapse_input) * (float) green_level ) / (float) (255) ) +
					0.11 * ( ( (float) BLUE(synapse_input) * (float) blue_level ) / (float) (255) ));	*/

	composed_color_output = (( ( (float) RED(synapse_input) * (float) red_level ) / (float) (255) ) +
				( ( (float) GREEN(synapse_input) * (float) green_level ) / (float) (255) ) +
				( ( (float) BLUE(synapse_input) * (float) blue_level ) / (float) (255) ));
	
	return(composed_color_output);
}


/*
*********************************************************************************
* Obtains the composed color selective weighted synapse input value		*
*********************************************************************************
*/

float
obtain_composed_color_synapse_weighted_value_color_2_grayscale(SYNAPSE *synapse)
{
	/*	Obtained from : "Color-to-Grayscale: Does the Method Matter in Image Recognition ?"
		http://www.plosone.org/article/fetchObject.action?uri=info%3Adoi%2F10.1371%2Fjournal.pone.0029740&representation=PDF
	*/

	int	synapse_input;

	synapse_input = synapse->source->output.ival;

	// Intensity
	/* return(NEURON_MASK_COLOR_RED(synapse_input) + NEURON_MASK_COLOR_GREEN(synapse_input) + NEURON_MASK_COLOR_BLUE(synapse_input)); */
	/* return(RED(synapse_input) + GREEN(synapse_input) + BLUE(synapse_input)); */

	// Glam - Yields better results according to the paper
	/* return(	pow((float)NEURON_MASK_COLOR_RED(synapse_input),GAMMA_CORRECTION_CONST) +
		pow((float)NEURON_MASK_COLOR_GREEN(synapse_input),GAMMA_CORRECTION_CONST) +
		pow((float)NEURON_MASK_COLOR_BLUE(synapse_input),GAMMA_CORRECTION_CONST) ); */
	/* return(	pow((float)RED(synapse_input),GAMMA_CORRECTION_CONST) +
		pow((float)GREEN(synapse_input),GAMMA_CORRECTION_CONST) +
		pow((float)BLUE(synapse_input),GAMMA_CORRECTION_CONST) ); */

	//Luminance
	/* return(	0.3 * (float)NEURON_MASK_COLOR_RED(synapse_input) +
		0.59 * (float)NEURON_MASK_COLOR_GREEN(synapse_input) +
		0.11 * (float)NEURON_MASK_COLOR_BLUE(synapse_input) ); */
	/* return(	0.3 * (float)RED(synapse_input) +
		0.59 * (float)GREEN(synapse_input) +
		0.11 * (float)BLUE(synapse_input) ); */
	
	//Luma
	/* return(	0.2126 * (float)NEURON_MASK_COLOR_RED(synapse_input) +
		0.7152 * (float)NEURON_MASK_COLOR_GREEN(synapse_input) +
		0.0722 * (float)NEURON_MASK_COLOR_BLUE(synapse_input) ); */
	/* return(	0.2126 * (float)RED(synapse_input) +
		0.7152 * (float)GREEN(synapse_input) +
		0.0722 * (float)BLUE(synapse_input) ); */

	//Value
	return(max(NEURON_MASK_COLOR_RED(synapse_input),max(NEURON_MASK_COLOR_GREEN(synapse_input),NEURON_MASK_COLOR_BLUE(synapse_input))));
	/* return(max(RED(synapse_input),max(GREEN(synapse_input),BLUE(synapse_input)))); */

	//Luster
	/* return(	max(NEURON_MASK_COLOR_RED(synapse_input),max(NEURON_MASK_COLOR_GREEN(synapse_input),NEURON_MASK_COLOR_BLUE(synapse_input))) +
		min(NEURON_MASK_COLOR_RED(synapse_input),min(NEURON_MASK_COLOR_GREEN(synapse_input),NEURON_MASK_COLOR_BLUE(synapse_input))) ); */
	/* return(	max(RED(synapse_input),max(GREEN(synapse_input),BLUE(synapse_input))) +
		min(RED(synapse_input),min(GREEN(synapse_input),BLUE(synapse_input))) ); */

}

/*
*********************************************************************************
* Obtains the composed color selective weighted synapse input value (HSV values)*
*********************************************************************************
*/

float
obtain_composed_color_synapse_weighted_value_hsv(SYNAPSE *synapse)
{
	int	synapse_input;
	int	red_level;
	int	green_level;
	int	blue_level;
	int	max_color, min_color, delta_color;
	
	float	hue;

	synapse_input = synapse->source->output.ival;
	red_level = RED(synapse_input);
	green_level = GREEN(synapse_input);
	blue_level = BLUE(synapse_input);

	max_color = max(red_level,max(green_level,blue_level));
	min_color = min(red_level,min(green_level,blue_level));
	delta_color = max_color - min_color;
	
	if(!delta_color)	// If the delta color is zero (we'd have an undefined hue value)
		return(0.0);	// Hue = 0.0 as a convenience value
	else
	{
		if(max_color == red_level)
			hue = fmodf( (float) ( green_level - blue_level ) / (float) delta_color, 6.0);
		else if(max_color == green_level)
			hue = (float) ( blue_level - red_level ) / (float) delta_color + 2.0;
		else// max_color == blue_level	
			hue = (float) ( red_level - green_level ) / (float) delta_color + 4.0;
	}

	return	(hue);
}
/*
*********************************************************************************
* Obtains the non-linear synapse with log value	output				*
*********************************************************************************
*/

float
obtain_non_linear_synapse_log_value(SYNAPSE *synapse)
{
	int	synapse_input;
	int	red_level;
	int	green_level;
	int	blue_level;
	
	float	mean_level;
	float	synapse_output;
	float	k1;
	float	k2;
	float	k3;
	
	if(synapse)	//test whether the synapse exists
	{
		synapse_input = synapse->source->output.ival;
		
		red_level = RED(synapse_input);
		green_level = GREEN(synapse_input);
		blue_level = BLUE(synapse_input);
		
		mean_level = (float) (red_level + green_level + blue_level)/3.0;
	
		k1 = ((float) RED(synapse->sensitivity.ival)  / (float) (255));
		k2 = (float) GREEN(synapse->sensitivity.ival);
		k3 = ((float) BLUE(synapse->sensitivity.ival)  / (float) (255));
		
		synapse_output = k1*log2(k2 + k3*mean_level);
	}
	else
		synapse_output = 0.0;	// Error condition
		
	return(synapse_output);
}


/*
*********************************************************************************
* Obtains the non-linear synapse with rational value output			*
*********************************************************************************
*/

float
obtain_non_linear_synapse_rational_value(SYNAPSE *synapse)
{
	int	synapse_input;
	int	red_level;
	int	green_level;
	int	blue_level;
	
	float	mean_level;
	float	synapse_output;
	float	k1;
	float	k2;
	
	if(synapse)	//test whether the synapse exists
	{
		synapse_input = synapse->source->output.ival;
		
		red_level = RED(synapse_input);
		green_level = GREEN(synapse_input);
		blue_level = BLUE(synapse_input);
		
		mean_level = (float) (red_level + green_level + blue_level)/3.0;
	
		k1 = (float) RED(synapse->sensitivity.ival);
		k2 = (float) GREEN(synapse->sensitivity.ival);
		
		synapse_output = k1*mean_level/(k2 + mean_level);
	}
	else
		synapse_output = 0.0;	// Error condition
		
	return(synapse_output);
}


/*
*********************************************************************************
* Obtains the non-linear synapse with exponential value output			*
*********************************************************************************
*/

float
obtain_non_linear_synapse_exponential_value(SYNAPSE *synapse)
{
	int	synapse_input;
	int	red_level;
	int	green_level;
	int	blue_level;
	
	float	mean_level;
	float	synapse_output;
	float	k1;
	
	if(synapse)	//test whether the synapse exists
	{
		synapse_input = synapse->source->output.ival;
		
		red_level = RED(synapse_input);
		green_level = GREEN(synapse_input);
		blue_level = BLUE(synapse_input);
		
		mean_level = (float) (red_level + green_level + blue_level)/3.0;
	
		k1 = ((float) RED(synapse->sensitivity.ival)  / (float) (255));
		
		synapse_output = pow(mean_level,k1);
	}
	else
		synapse_output = 0.0;	// Error condition
		
	return(synapse_output);
}


/*
*********************************************************************************
* Evaluate dendrite								*
*********************************************************************************
*/

NEURON_OUTPUT 
evaluate_dendrite (NEURON_LAYER *neuron_layer, int n)
{
	PATTERN *input_pattern = NULL;
	NEURON_OUTPUT d_state;
	int connections;
	OUTPUT_TYPE output_type;
	NEURON *neuron;
	SENSITIVITY_TYPE neuron_layer_sensitivity;
	
	neuron = &(neuron_layer->neuron_vector[n]);
	
	connections = neuron->n_in_connections;
	output_type = neuron_layer->output_type;
	
	if (((SYNAPSE_LIST *) (neuron->synapses)) == NULL)
	{
		d_state.ival = NEVER_COMPUTED;
		return d_state;
	}
	
	//If this neuron layer is directly connected
	if (((SYNAPSE_LIST *) (neuron->synapses))->synapse->sensitivity.ival == DIRECT_CONNECTION)
	{
		//The dendrite state is obtained directly from the output
		d_state = ((SYNAPSE_LIST *) (neuron->synapses))->synapse->source->output;
		
		// Switching for setting the inputs (Note that they are thresholded to maximum values)
		switch(output_type)
		{
			case GREYSCALE:
				d_state.ival = ((d_state.ival >= NUM_GREYLEVELS) ? (NUM_GREYLEVELS - 1) : d_state.ival);
				return d_state;
			case BLACK_WHITE:
				d_state.ival = ((d_state.ival >= NUM_GREYLEVELS / 2) ? (NUM_GREYLEVELS - 1) : 0);
				return d_state;
			case GREYSCALE_FLOAT: 
				d_state.fval = ((d_state.fval >= (float) NUM_GREYLEVELS) ? (float) (NUM_GREYLEVELS - 1): d_state.fval);
				return d_state;
			case COLOR:
				d_state.ival = ((d_state.ival >= NUM_COLORS) ? (NUM_COLORS - 1): d_state.ival);
				return d_state;
			case COLOR_SELECTIVE:	//same as the color sensitive neuron
				d_state.ival = ((d_state.ival >= NUM_COLORS) ? (NUM_COLORS - 1): d_state.ival);
				return d_state;
		}
	}
	
	neuron_layer_sensitivity = neuron_layer->sensitivity;
	
	(*(neuron_layer->neuron_type->initialize_input_pattern)) (&input_pattern, connections);
	(*(neuron_layer->neuron_type->compute_input_pattern)) (neuron, input_pattern, neuron_layer_sensitivity, output_type);
	d_state = (*(neuron_layer->neuron_type->neuron_memory_read)) (neuron_layer, n, input_pattern, connections, output_type);

	free(input_pattern);
	input_pattern = NULL;

	return d_state;
} 


/*
*********************************************************************************
* Train a single neuron								*
*********************************************************************************
*/

void
train_neuron (NEURON_LAYER *neuron_layer, int n)
{
	PATTERN *input_pattern = NULL;
	int connections;
	SENSITIVITY_TYPE sensitivity;
	OUTPUT_TYPE output_type;
	NEURON *neuron;
	
	neuron = &(neuron_layer->neuron_vector[n]);
	connections = neuron->n_in_connections;
	
	sensitivity = neuron_layer->sensitivity;
	output_type = neuron_layer->output_type;

	(*(neuron_layer->neuron_type->initialize_input_pattern)) (&input_pattern, connections);
	(*(neuron_layer->neuron_type->compute_input_pattern)) (neuron, input_pattern, sensitivity, output_type);
	(*(neuron_layer->neuron_type->neuron_memory_write)) (neuron_layer, n, input_pattern, connections, output_type);

	free(input_pattern);
	input_pattern = NULL;
}


/*
*********************************************************************************
* Generates the random neuron index vector for sparse neuron layer training	*
*********************************************************************************
*/

void
generate_random_neuron_index_vector (float percentage,int number_of_neurons)
{
	int already_exists;
	int random_neuron_index;
	int i,j;
	
	//if(percentage < 0.0 || percentage > 100.0)
	//{
	//	printf("Percentage of sparse neurons, must be over 0.0 and below 100.0\n");
	//	return;
	//}
	
	random_neuron_index_vector_size = (int) ((percentage*number_of_neurons)/100.0);
	
	if(random_neuron_index_vector)
		free(random_neuron_index_vector);
	
	random_neuron_index_vector = (unsigned int *) alloc_mem (random_neuron_index_vector_size * sizeof (int));
	
	for(i=0;i<random_neuron_index_vector_size;i++)
	{
		already_exists = 1;
		
		while(already_exists)
		{
			random_neuron_index = (int) number_of_neurons*((double) rand() / (double) LRAND48_MAX);
			for(j=0;j<i;j++)
			{
				if(random_neuron_index_vector[j] == random_neuron_index)
				{
					already_exists = 1;
					break;
				}
			}
			if(j == i)
			{
				already_exists = 0;
				random_neuron_index_vector[i] = random_neuron_index;
			}
		}
	}
	return;
}


/*
*********************************************************************************
* Train a neural layer								*
*********************************************************************************
*/

void 
train_neuron_layer(char *name)
{
	NEURON_LAYER *neuron_layer;
	NEURON *neuron_vector;
	long num_neurons;
	long i;

	if ((neuron_layer = get_neuron_layer_by_name (name)) == NULL)
	{
		show_message ("Unknown neuron layer:", name, "");
		return;
	}
	num_neurons = get_num_neurons (neuron_layer->dimentions);
	neuron_vector = neuron_layer->neuron_vector;

	if (neuron_vector[0].synapses == NULL)
	{	
		show_message ("Neuron layer", name, "has no connections.");
		return;
	}
	
	if ((((SYNAPSE_LIST *) (neuron_vector[0].synapses))->synapse->sensitivity.ival != DIRECT_CONNECTION) &&
	    (neuron_layer->associated_neuron_vector != NULL))
	{
		/* TODO: Shared memory neuron layers trainning cannot be paralelized on OpenMP with the current neuron training implementation */
		if (neuron_layer->memory_t == DISTRIBUTED_MEMORY)
		{
			#ifdef _OPENMP
				#pragma omp parallel for private(i)	// parallel OpenMP training available for distributed memory model
			#endif
			for (i = 0; i < num_neurons; i++)
				train_neuron(neuron_layer, i);
		}
		else							// serial training for shared memory model
		{
			for (i = 0; i < num_neurons; i++)
				train_neuron(neuron_layer, i);
		}
	}
	else
	{
		show_message ("This neuron layer cannot be trained.", "It either has not an associated neuron layer", 
				 "or it is directly connected to another neural layer.");
		return;
	}
}


/*
*********************************************************************************
* Selective neuron layer trainning (non-training for dummy values)		*
*********************************************************************************
*/
void 
train_neuron_layer_selective(char *name)
{
	NEURON_LAYER *neuron_layer;
	NEURON *neuron_vector;
	long num_neurons;
	long i;

	if ((neuron_layer = get_neuron_layer_by_name (name)) == NULL)
	{
		show_message ("Unknown neuron layer:", name, "");
		return;
	}
	num_neurons = get_num_neurons (neuron_layer->dimentions);
	neuron_vector = neuron_layer->neuron_vector;

	if (neuron_vector[0].synapses == NULL)
	{	
		show_message ("Neuron layer", name, "has no connections.");
		return;
	}
	
	if ((((SYNAPSE_LIST *) (neuron_vector[0].synapses))->synapse->sensitivity.ival != DIRECT_CONNECTION) &&
	    (neuron_layer->associated_neuron_vector != NULL))
	{
		/* TODO: Shared memory neuron layers trainning cannot be paralelized on OpenMP with the current neuron training implementation */
		if(neuron_layer->memory_t == DISTRIBUTED_MEMORY)
		{
			#ifdef _OPENMP
				#pragma omp parallel for private(i)	// parallel OpenMP training available
			#endif
			for (i = 0; i < num_neurons; i++)
				if(neuron_vector[i].output.ival != DUMMY_OUTPUT_ACTIVATION_VALUE)	// if its not the dummy value
					train_neuron (neuron_layer, i);
		}
		else							// serial training
		{
			for (i = 0; i < num_neurons; i++)
				if(neuron_vector[i].output.ival != DUMMY_OUTPUT_ACTIVATION_VALUE)	// if its not the dummy value
					train_neuron (neuron_layer, i);
		}
	}
	else
	{
		show_message ("This neuron layer cannot be trained.", "It either has not an associated neuron layer", 
				 "or it is directly connected to another neural layer.");
		return;
	}
}


/*
*********************************************************************************
* Sparse Train a neural layer							*
*********************************************************************************
*/

void 
sparse_train_neuron_layer(char *name, float percentage)
{
	NEURON_LAYER *neuron_layer;
	NEURON *neuron_vector;
	long num_neurons;
	long i;

	if ((neuron_layer = get_neuron_layer_by_name (name)) == NULL)
	{
		show_message ("Unknown neuron layer:", name, "");
		return;
	}
	num_neurons = get_num_neurons (neuron_layer->dimentions);
	neuron_vector = neuron_layer->neuron_vector;

	if (neuron_vector[0].synapses == NULL)
	{	
		show_message ("Neuron layer", name, "has no connections.");
		return;
	}
	
	if ((((SYNAPSE_LIST *) (neuron_vector[0].synapses))->synapse->sensitivity.ival != DIRECT_CONNECTION) &&
	    (neuron_layer->associated_neuron_vector != NULL))
	{
		/* Valid percentage values must be guaranteed */
		if (percentage < 0.0 || percentage > 100.0)
		{
			printf("Percentage of sparse neurons, must be over 0.0 and below 100.0\n");
			return;
		}
		
		generate_random_neuron_index_vector (percentage, num_neurons);
		
		/* TODO: Shared memory neuron layers trainning cannot be paralelized on OpenMP with the current neuron training implementation */
		if (neuron_layer->memory_t == DISTRIBUTED_MEMORY)
		{
			#ifdef _OPENMP
				#pragma omp parallel for private(i)	// parallel OpenMP training available
			#endif
			for (i = 0; i < random_neuron_index_vector_size; i++)
				train_neuron(neuron_layer, random_neuron_index_vector[i]);
		}
		else							// serial training
		{
			for (i = 0; i < random_neuron_index_vector_size; i++)
				train_neuron(neuron_layer, random_neuron_index_vector[i]);
		}
	}
	else
	{
		show_message ("This neuron layer cannot be trained.", "It either has not an associated neuron layer", 
				 "or it is directly connected to another neural layer.");
		return;
	}
}


/*
*********************************************************************************
* Train the whole network							*
*********************************************************************************
*/

void 
train_network (void)
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
		if (neuron_vector[0].synapses != NULL)
			if ((((SYNAPSE_LIST *) (neuron_vector[0].synapses))->synapse->sensitivity.ival != DIRECT_CONNECTION) &&
			    (nl_list->neuron_layer->associated_neuron_vector != NULL))
			#ifdef _OPENMP
				#pragma omp parallel for private(i)
			#endif
				for (i = 0; i < num_neurons; i++)
					train_neuron (nl_list->neuron_layer, i);
	}
}


/*
*********************************************************************************
* Sparse train the whole network						*
*********************************************************************************
*/

void 
sparse_train_network (float percentage)
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
		if (neuron_vector[0].synapses != NULL)
			if ((((SYNAPSE_LIST *) (neuron_vector[0].synapses))->synapse->sensitivity.ival != DIRECT_CONNECTION) &&
			    (nl_list->neuron_layer->associated_neuron_vector != NULL))
			{
			
			
			if(percentage < 0.0 || percentage > 100.0)
			{
				printf("Percentage of sparse neurons, must be over 0.0 and below 100.0\n");
				return;
			}
			
			generate_random_neuron_index_vector (percentage,num_neurons);	
			#ifdef _OPENMP
				#pragma omp parallel for private(i)
			#endif
				for (i = 0; i < num_neurons; i++)
					train_neuron (nl_list->neuron_layer, random_neuron_index_vector[i]);
			}
	}
}


/*
*********************************************************************************
* All neurons update								*
*********************************************************************************
*/

void
all_neurons_update (void)
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
#ifdef _OPENMP
		#pragma omp parallel for private(i)
#endif
		for (i = 0; i < num_neurons; i++)
		{
			//neuron_vector[i].backpropagation_error = 0;
			if (neuron_vector[i].dendrite_state.ival != NEVER_COMPUTED)
				neuron_vector[i].output = neuron_vector[i].dendrite_state;
		}
	}
}


/*
*********************************************************************************
* All dendrites update								*
*********************************************************************************
*/

void
all_dendrites_update (void)
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
#ifdef _OPENMP
		#pragma omp parallel for private(i)
#endif
		for (i = 0; i < num_neurons; i++)
			neuron_vector[i].dendrite_state = (evaluate_dendrite (nl_list->neuron_layer, i));
	}
}


/*
*********************************************************************************
* Get a name from a list							*
*********************************************************************************
*/

char *
get_name (char *list, char *name)
{
	int i;
	
	i = 0;
	while (isalnum (list[i]) || (list[i] == '_'))
	{
		name[i] = list[i];
		i++;
	}
		
	if (i == 0)
		return (NULL);
	
	name[i] = '\0';
	while ((!isalnum (list[i])) && (list[i] != '\0'))
		i++;
	
	return (&(list[i]));
}


/*
*********************************************************************************
* Forward (update) an object (neuron_layer, output, input, etc.)		*
*********************************************************************************
*/

void
forward_objects (char *objects)
{
	NEURON_LAYER *neuron_layer;
	long num_neurons;
	long i;
	NEURON *neuron_vector;
	void *object;
	char *next_name;
	char name[SYMBOL_SIZE];
	
	next_name = objects;
	while ((next_name = get_name (next_name, name)) != NULL)
	{
		switch (get_object_by_name (name, &object))
		{
			case NEURON_LAYER_OBJECT:
				neuron_layer = ((NEURON_LAYER *) object);
				num_neurons = get_num_neurons (neuron_layer->dimentions);
				neuron_vector = neuron_layer->neuron_vector;
				#ifdef _OPENMP
					#pragma omp parallel for private(i)
				#endif
				for (i = 0; i < num_neurons; i++)
				{
					neuron_vector[i].dendrite_state = neuron_vector[i].output = (evaluate_dendrite (neuron_layer, i));
					//neuron_vector[i].backpropagation_error = 0;
				}
				break;
			case INPUT_OBJECT:
				input_forward ((INPUT_DESC *) object);
				break;

			case OUTPUT_OBJECT:
				output_forward ((OUTPUT_DESC *) object);
				break;

			case FILTER_OBJECT:
				filter_update ((FILTER_DESC *) object);
				break;

			default:
				show_message ("Unknown object", name, "");
		}
	}
}


/*
*********************************************************************************
* Re-Initialise synapse                                                         *
*********************************************************************************
*/

void
reinitialize_synapse (SYNAPSE *synapse, NEURON_LAYER *neuron_layer_src, NEURON *source, NEURON_OUTPUT sensitivity, float weight)
{
	synapse->sensitivity = sensitivity;
	synapse->weight = weight;
	synapse->source  = source;
	synapse->nl_source = neuron_layer_src;
}


/* 
********************************************************************************* 
* Inlined gaussian function evaluation						* 
********************************************************************************* 
*/


float
evaluate_gaussian_function(float x, float a,float b,float c_square)
{
	//refer to http://en.wikipedia.org/wiki/Gaussian_curve for further details
	//we're not squaring c cause it's preferred to use for normal distribution variance
	return( a*exp(-(x-b)*(x-b)/(2*c_square)) );
}


/* 
********************************************************************************* 
* Evaluates a normal distribution funtion at some point	"x"			* 
********************************************************************************* 
*/

float
evaluate_normal_distribution(float x,float variance, float mean)
{
	//refer to http://en.wikipedia.org/wiki/Normal_distribution for further details
	float k1;

	k1 = 1/(sqrt(2*M_PI*variance));
	return(evaluate_gaussian_function(x,k1,mean,variance));
}

/*
*********************************************************************************
* Initialise synapse								*
*********************************************************************************
*/

void
initialise_synapse (SYNAPSE *synapse, NEURON_LAYER *neuron_layer_src, NEURON *source, NEURON *destination, NEURON_OUTPUT sensitivity, SENSITIVITY_TYPE neuron_layer_sensitivity,INTERCONNECTION_PATTERN_TYPE interconnection_pattern, int synapse_number)
{
	// Synapse initialize
    synapse->sensitivity = sensitivity;
	synapse->source  = source;
	synapse->nl_source = neuron_layer_src;
}


void
get_wisard_table_size_and_log2_table_size_and_number_of_tables(int *log2_table_size, int *number_of_tables, int num_traning_samples, NEURON *neuron)
{
	*log2_table_size = (int) (log2(num_traning_samples) + 1.0);
	//*log2_table_size = 16;
	if (*log2_table_size > 16)
		*log2_table_size = 16;

	*number_of_tables = (int)((float) neuron->n_in_connections / (float) *log2_table_size); // the number of synapses must be divisible by log2_table_size; otherwise, some synapses will be ignored
}


/*
*********************************************************************************
* Initialise neuron								*
*********************************************************************************
*/

void
initialise_neuron (NEURON_LAYER *neuron_layer, int n, int memory_size)
{
	NEURON *neuron;
	
	neuron = &(neuron_layer->neuron_vector[n]);
	neuron->dendrite_state.ival = NEVER_COMPUTED;
	neuron->output.ival = 0;
	neuron->n_out_connections = 0;
	neuron->n_in_connections = 0;
	neuron->synapses = (SYNAPSE_LIST *) NULL;
	neuron->memory = NULL;
	neuron->memory_size = memory_size;
	neuron->pattern_xor_table = NULL;
	neuron->id = n;
	neuron->holder_neuron_layer = neuron_layer;
	neuron->hash_collisions = 0;	// 0 test table hash collisions
}


/*
*********************************************************************************
* Initialise Host Neuron Vector							*
*********************************************************************************
*/

void initialise_neuron_vector (NEURON_LAYER *neuron_layer, int memory_size)
{ 
	int i;
	
	neuron_layer->neuron_vector = (NEURON *) alloc_mem ((size_t) (neuron_layer->num_neurons * sizeof (NEURON)));
	
	for (i = 0; i < neuron_layer->num_neurons; i++) 
		initialise_neuron (neuron_layer, i, memory_size);
}


/*
*********************************************************************************
* Add synapse in neuron output list						*
*********************************************************************************
*/

void
add_synapse (NEURON *neuron, SYNAPSE *synapse)
{
	SYNAPSE_LIST *s_list;

	if (((SYNAPSE_LIST *) (neuron->synapses)) == (SYNAPSE_LIST *) NULL)
	{
		s_list = (SYNAPSE_LIST *) alloc_mem ((size_t) sizeof (SYNAPSE_LIST));
		s_list->next = (SYNAPSE_LIST *) NULL;
		s_list->synapse = synapse;
		neuron->synapses = s_list;
	}
	else
	{
		for (s_list = ((SYNAPSE_LIST *) (neuron->synapses)); s_list->next != (SYNAPSE_LIST *) NULL;
			s_list = s_list->next)
			;
		s_list->next = (SYNAPSE_LIST *) alloc_mem ((size_t) sizeof (SYNAPSE_LIST));
		s_list->next->next = (SYNAPSE_LIST *) NULL;
		s_list->next->synapse = synapse;
	}
}


/*
*********************************************************************************
* Add neuron layer in the neuron layer list					*
*********************************************************************************
*/

void
add_neuron_layer (NEURON_LAYER *neuron_layer)
{
	NEURON_LAYER_LIST *nl_list;
	
	n_n_layers++;

	if (global_neuron_layer_list == (NEURON_LAYER_LIST *) NULL)
	{
		nl_list = (NEURON_LAYER_LIST *) alloc_mem ((size_t) sizeof (NEURON_LAYER_LIST));
		nl_list->next = (NEURON_LAYER_LIST *) NULL;
		nl_list->neuron_layer = neuron_layer;
		global_neuron_layer_list = nl_list;
	}
	else
	{
		for (nl_list = global_neuron_layer_list; nl_list->next != (NEURON_LAYER_LIST *) NULL;
			nl_list = nl_list->next)
			;
			
		nl_list->next = (NEURON_LAYER_LIST *) alloc_mem ((size_t) sizeof (NEURON_LAYER_LIST));
		nl_list->next->next = (NEURON_LAYER_LIST *) NULL;
		nl_list->next->neuron_layer = neuron_layer;
	}
}
 
 
/* 
********************************************************************************* 
* Create neuron layer								* 
********************************************************************************* 
*/ 
 
void 
create_neuron_layer (NEURON_LAYER *neuron_layer, NEURON_TYPE *neuron_type, SENSITIVITY_TYPE sensitivity, OUTPUT_TYPE output_type, int x_size, int y_size, MEMORY_TYPE memory_type, int memory_size)
{
	if (neuron_type != NULL)		
		neuron_layer->neuron_type = neuron_type;
	else
	{
		neuron_layer->neuron_type = (NEURON_TYPE *) alloc_mem (sizeof (NEURON_TYPE));
		neuron_layer->neuron_type->compute_input_pattern = compute_input_pattern;
		neuron_layer->neuron_type->neuron_memory_read = neuron_memory_read;
		neuron_layer->neuron_type->neuron_memory_write = neuron_memory_write;
		neuron_layer->neuron_type->find_nearest_pattern = find_nearest_pattern;
		neuron_layer->neuron_type->initialise_neuron_memory = initialise_neuron_memory;
		neuron_layer->neuron_type->initialize_input_pattern = initialize_input_pattern;
	}
	neuron_layer->dimentions.x = x_size;
	neuron_layer->dimentions.y = y_size;
	neuron_layer->num_neurons = x_size * y_size;
	neuron_layer->memory_t = memory_type;
	initialise_neuron_vector (neuron_layer, memory_size);
	neuron_layer->associated_neuron_vector = NULL;
	neuron_layer->sensitivity = sensitivity;
	neuron_layer->output_type = output_type;
	add_neuron_layer (neuron_layer);
}


/*
*********************************************************************************
* Initialise memory								*
*********************************************************************************
*/

void
initialise_memory (void)
{
	NEURON_LAYER_LIST *nl_list;
	long num_neurons;
	long i;

	for (nl_list = global_neuron_layer_list; nl_list != (NEURON_LAYER_LIST *) NULL;
		nl_list = nl_list->next)
	{
		num_neurons = get_num_neurons (nl_list->neuron_layer->dimentions);

		if (nl_list->neuron_layer->neuron_vector[0].synapses == NULL)
			continue; /* This neuron layer does not need memory, since it is not connected to another neuron layer */

		if ((nl_list->neuron_layer->associated_neuron_vector == NULL) || 
		    (((SYNAPSE_LIST *) (nl_list->neuron_layer->neuron_vector[0].synapses))->synapse->sensitivity.ival == DIRECT_CONNECTION))
			continue; /* This neuron layer does not need memory. It either has not an associated neuron layer
				     or it is directly connected to another neural layer.*/
		
		/* This will be the only neuron layer with long-term memory, which neurons memory must be individually initialized */
		for (i = 0; i < num_neurons; i++)
		{
			if ((nl_list->neuron_layer->neuron_type != NULL) && 
			    (nl_list->neuron_layer->neuron_type->initialise_neuron_memory  != NULL))
				(*(nl_list->neuron_layer->neuron_type->initialise_neuron_memory)) (nl_list->neuron_layer, i);
		}
	}
}


/*
*********************************************************************************
* Get neuron layer by name							*
*********************************************************************************
*/

NEURON_LAYER *
get_neuron_layer_by_name (char *neuron_layer_name)
{
	NEURON_LAYER_LIST *n_list;
	INPUT_LIST *i_list;
	
	if (global_neuron_layer_list == NULL)
		return (NULL);
	for (n_list = global_neuron_layer_list; n_list != (NEURON_LAYER_LIST *) NULL;
		n_list = n_list->next)
		if (strcmp (n_list->neuron_layer->name, neuron_layer_name) == 0)
			return (n_list->neuron_layer);

	if (global_input_list == NULL)
		return (NULL);
	for (i_list = global_input_list; i_list != (INPUT_LIST *) NULL; i_list = i_list->next)
		if (strcmp (i_list->input->neuron_layer->name, neuron_layer_name) == 0)
			return (i_list->input->neuron_layer);
	return (NULL); 
}

	 
 /*
*********************************************************************************
* Remap neuron layer to specific id						*
*********************************************************************************
*/
#if 0
void
remap_layers2id (void)
{
	NEURON_LAYER_LIST *nl_list;
	INPUT_LIST *in_list;
	OUTPUT_LIST *out_list;
	int n_total_layers;
	int global_layer_id = 0;

	n_total_layers = n_inputs + n_n_layers + n_outputs;
	
	n_layer_vec  = (NEURON_LAYER **) alloc_mem (n_total_layers * sizeof (* NEURON_LAYER));
	
	
	for (in_list = global_input_list; in_list != (INPUT_LIST *) NULL; in_list = in_list->next)
	{
		n_layer_vec[global_layer_id] = in_list->input->neuron_layer;
		
		global_layer_id++;
	}

	for (nl_list = global_neuron_layer_list; nl_list != (NEURON_LAYER_LIST *) NULL; nl_list = nl_list->next)
	{
		n_layer_vec[global_layer_id] = nl_list->neuron_layer;
		
		global_layer_id++;
	}
	
	for (out_list = global_output_list; out_list != (OUTPUT_LIST *) NULL; out_list = out_list->next)
	{
		n_layer_vec[global_layer_id] = out_list->output->neuron_layer;
		
		global_layer_id++;
	}
	
	return;
}
#endif


/*
* *********************************************************************************
* * Count total number of neurons	                                          *
* *********************************************************************************
* */

void
count_num_neurons (void)
{
	NEURON_LAYER_LIST *nl_list;

	for (nl_list = global_neuron_layer_list; nl_list != (NEURON_LAYER_LIST *) NULL; nl_list = nl_list->next)
		total_n_neurons += get_num_neurons (nl_list->neuron_layer->dimentions);

	return;
}
	

/*
*********************************************************************************
* Map neuron layer to specific id						*
*********************************************************************************
*/

void
map_layers2id (void)
{
	NEURON_LAYER_LIST *nl_list;
	INPUT_LIST *in_list;
	int n_total_layers;
	int global_layer_id = 0;

        n_total_layers = n_inputs + n_n_layers + n_outputs;

        n_layer_vec  = (NEURON_LAYER **) alloc_mem (n_total_layers * sizeof (NEURON_LAYER *));
	
	for (in_list = global_input_list; in_list != (INPUT_LIST *) NULL; in_list = in_list->next)
	{
		in_list->input->neuron_layer->id = global_layer_id;

		n_layer_vec[global_layer_id] = in_list->input->neuron_layer;

		global_layer_id++;
	}

	for (nl_list = global_neuron_layer_list; nl_list != (NEURON_LAYER_LIST *) NULL; nl_list = nl_list->next)
	{
		nl_list->neuron_layer->id = global_layer_id;
		
		n_layer_vec[global_layer_id] = nl_list->neuron_layer;

		global_layer_id++;
	}
#if 0	
	for (out_list = global_output_list; out_list != (OUTPUT_LIST *) NULL; out_list = out_list->next)
	{
		out_list->output->neuron_layer->id = global_layer_id;

		n_layer_vec[global_layer_id] = out_list->output->neuron_layer;		
		
		global_layer_id++;
	}
#endif
	return;
}


/* 
********************************************************************************* 
* Destroy synapse list								* 
********************************************************************************* 
*/ 

void
destroy_synapse_list (void **synapses)
{
	SYNAPSE_LIST *synapse_list, *aux;

	synapse_list = (SYNAPSE_LIST *)(*synapses);
	
	while (synapse_list != (SYNAPSE_LIST *) NULL)
	{
		aux = synapse_list->next;
		free (synapse_list->synapse);
		free (synapse_list);
		synapse_list = aux;
	}
	
	*synapses = NULL;
	
	return;
}


 /*
*********************************************************************************
* Remove a synapse								*
*********************************************************************************
*/
void
remove_synapse (NEURON *neuron_dest, SYNAPSE *synapse)
{
	NEURON *neuron_src;
	neuron_src =  synapse->source;

	neuron_src->n_out_connections--; 
	neuron_dest->n_in_connections--;
	
	return;
}


/*
*********************************************************************************
* Destroy connections to a specify neuron 					*
*********************************************************************************
*/

void
destroy_connections2neuron (NEURON *neuron)
{
	SYNAPSE *synapse;
	SYNAPSE_LIST *s_list;
        	
	for (s_list = ((SYNAPSE_LIST *) (neuron->synapses)); s_list != (SYNAPSE_LIST *) NULL; s_list = s_list->next)
	{
		synapse =  s_list->synapse;
		
		remove_synapse (neuron, synapse);
	}
	
	destroy_synapse_list (&(neuron->synapses));
	
	return;
}


 /*
*********************************************************************************
* Destroy connections to a specify neuron layer					*
*********************************************************************************
*/

void
destroy_connections2neuron_layer(NEURON_LAYER *neuron_layer)
{
	NEURON *neuron;
	int i;
	
	for (i = 0; i < neuron_layer->num_neurons; i++) 
	{
		neuron = &(neuron_layer->neuron_vector[i]);

		destroy_connections2neuron (neuron);

		//FIXME free memories?
	}

	return;
}


 /*
*********************************************************************************
* Destroy connections to a specify neuron layer					*
*********************************************************************************
*/

void
destroy_connections2neuron_layer_by_name(char *neuron_layer_name)
{
	NEURON_LAYER *neuron_layer;
	
	neuron_layer = get_neuron_layer_by_name(neuron_layer_name);
	
	destroy_connections2neuron_layer(neuron_layer);
	
	return;
}


/*
*********************************************************************************
* Destroy connections between neuron layers					*
*********************************************************************************
*/

void
destroy_inter_layers_connections (void)
{
	NEURON_LAYER_LIST *nl_list;
	NEURON_LAYER *neuron_layer;
	
	for (nl_list = global_neuron_layer_list; nl_list != (NEURON_LAYER_LIST *) NULL; nl_list = nl_list->next)
	{
		neuron_layer = nl_list->neuron_layer;

		destroy_connections2neuron_layer (neuron_layer);
	}

	return;
}


/*
*********************************************************************************
* Destroy all connections in the network					*
*********************************************************************************
*/

void
destroy_network (void)
{
	destroy_inter_layers_connections ();
	
	return;
}

