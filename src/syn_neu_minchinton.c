#include "syn_neu.h"
#include<mae.h>


/*
*********************************************************************************
* Compute the input pattern seen by minchinton neuron				*
*********************************************************************************
*/

void
compute_input_pattern_minchinton(NEURON *neuron, PATTERN *input_pattern, SENSITIVITY_TYPE sensitivity, OUTPUT_TYPE output_type)
{
	SYNAPSE_LIST *s_list;
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

	for (s_list = ((SYNAPSE_LIST *) (neuron->synapses)); s_list != (SYNAPSE_LIST *) NULL; s_list = s_list->next)
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
				if (s_list->next != NULL)
					bit_value = (s_list->synapse->source->output.ival < s_list->next->synapse->source->output.ival) ? 1 : 0;
				else
					bit_value = (s_list->synapse->source->output.ival < ((SYNAPSE_LIST *) (neuron->synapses))->synapse->source->output.ival) ? 1 : 0;
				break;
			case BLACK_WHITE:
				if (s_list->next != NULL)
					bit_value = (s_list->synapse->source->output.ival < s_list->next->synapse->source->output.ival) ? 1 : 0;
				else
					bit_value = (s_list->synapse->source->output.ival < ((SYNAPSE_LIST *) (neuron->synapses))->synapse->source->output.ival) ? 1 : 0;
				break;
			case GREYSCALE_FLOAT:
				if (s_list->next != NULL)
					bit_value = (s_list->synapse->source->output.fval < s_list->next->synapse->source->output.fval) ? 1 : 0;
				else
					bit_value = (s_list->synapse->source->output.fval < ((SYNAPSE_LIST *) (neuron->synapses))->synapse->source->output.fval) ? 1 : 0;
				break;
			case COLOR:
				if (s_list->next != NULL)
					bit_value = (s_list->synapse->source->output.ival < s_list->next->synapse->source->output.ival) ? 1 : 0;
				else
					bit_value = (s_list->synapse->source->output.ival < ((SYNAPSE_LIST *) (neuron->synapses))->synapse->source->output.ival) ? 1 : 0;
				break;
			case COLOR_SELECTIVE:
				if (s_list->next != NULL)
					bit_value = (obtain_composed_color_synapse_weighted_value(s_list->synapse) < obtain_composed_color_synapse_weighted_value(s_list->next->synapse)) ? 1 : 0;
					//bit_value = (obtain_composed_color_synapse_weighted_value_color_2_grayscale(s_list->synapse) < obtain_composed_color_synapse_weighted_value_color_2_grayscale(s_list->next->synapse)) ? 1 : 0;
				else
					bit_value = (obtain_composed_color_synapse_weighted_value(s_list->synapse) < obtain_composed_color_synapse_weighted_value(((SYNAPSE_LIST *) (neuron->synapses))->synapse) ) ? 1 : 0;
					//bit_value = (obtain_composed_color_synapse_weighted_value_color_2_grayscale(s_list->synapse) < obtain_composed_color_synapse_weighted_value_color_2_grayscale(((SYNAPSE_LIST *) (neuron->synapses))->synapse) ) ? 1 : 0;
				break;
			default:
				if (s_list->next != NULL)
					bit_value = (s_list->synapse->source->output.ival < s_list->next->synapse->source->output.ival) ? 1 : 0;
				else
					bit_value = (s_list->synapse->source->output.ival < ((SYNAPSE_LIST *) (neuron->synapses))->synapse->source->output.ival) ? 1 : 0;
				break;
		}

		input_pattern[i] |= bit_value << (PATTERN_UNIT_SIZE - 1);

		num_synapses++;
	}
	if (num_synapses % PATTERN_UNIT_SIZE)
		input_pattern[i] = input_pattern[i] >> (PATTERN_UNIT_SIZE - (num_synapses % PATTERN_UNIT_SIZE));
}



/* Implemented Neuron Types */

NEURON_TYPE minchinton = {compute_input_pattern_minchinton,
					 	  neuron_memory_read,
					 	  neuron_memory_write,
					 	  find_nearest_pattern,
					 	  initialise_neuron_memory,
					 	  initialize_input_pattern};
