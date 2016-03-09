#include "syn_neu.h"
#include <mae.h>


/*
*********************************************************************************
* Compute the input pattern seen by minchinton neuron				*
*********************************************************************************
*/

void
compute_input_pattern_minchinton_center_surround(NEURON *neuron, PATTERN *input_pattern, SENSITIVITY_TYPE sensitivity, OUTPUT_TYPE output_type)
{
	SYNAPSE_LIST *s_list, *central_synapse;
	int num_synapses, i;
	//unsigned long long int bit_value;
	PATTERN bit_value;
	SENSITIVITY_TYPE neuron_layer_sensitivity;

	if (sensitivity == NOT_SPECIFIED)
		neuron_layer_sensitivity = output_type;
	else
		neuron_layer_sensitivity = sensitivity;

	i = -1;
	num_synapses = 0;

	for (central_synapse = s_list = ((SYNAPSE_LIST *) (neuron->synapses)); s_list != (SYNAPSE_LIST *) NULL; s_list = s_list->next)
	{
		if ((num_synapses % PATTERN_UNIT_SIZE) == 0)
		{
			i++;
			input_pattern[i] = 0;
		}

		input_pattern[i] = input_pattern[i] >> 1;

		switch (neuron_layer_sensitivity)
		{
			case GREYSCALE:
				bit_value = (s_list->synapse->source->output.ival < central_synapse->synapse->source->output.ival) ? 1 : 0;
				break;
			case BLACK_WHITE:
				bit_value = (s_list->synapse->source->output.ival < central_synapse->synapse->source->output.ival) ? 1 : 0;
				break;
			case GREYSCALE_FLOAT:
				bit_value = (s_list->synapse->source->output.fval < central_synapse->synapse->source->output.fval) ? 1 : 0;
				break;
			case COLOR:
				bit_value = (s_list->synapse->source->output.ival < central_synapse->synapse->source->output.ival) ? 1 : 0;
				break;
			case COLOR_SELECTIVE:
				bit_value = (obtain_composed_color_synapse_weighted_value(s_list->synapse) < obtain_composed_color_synapse_weighted_value(central_synapse->synapse)) ? 1 : 0;
				break;
			default:
				bit_value = (s_list->synapse->source->output.ival < central_synapse->synapse->source->output.ival) ? 1 : 0;
				break;
		}

		input_pattern[i] |= bit_value << (PATTERN_UNIT_SIZE - 1);

		num_synapses++;
	}
	if (num_synapses % PATTERN_UNIT_SIZE)
		input_pattern[i] = input_pattern[i] >> (PATTERN_UNIT_SIZE - (num_synapses % PATTERN_UNIT_SIZE));
}


/* Implemented Neuron Types */

NEURON_TYPE minchinton_center_surround = {	compute_input_pattern_minchinton_center_surround,
					 	 	 	 	 	 	neuron_memory_read,
					 	 	 	 	 	 	neuron_memory_write,
					 	 	 	 	 	 	find_nearest_pattern,
					 	 	 	 	 	 	initialise_neuron_memory,
					 	 	 	 	 	 	initialize_input_pattern};
