
#include "mae.h"

void nl_program_filter(FILTER_DESC *filter_desc) {

	NEURON_LAYER *nl_output = NULL, *nl_input = NULL;

	int xi, yi, wi, hi, xo, yo, wo, ho;

	// Gets the Input Neuron Layer
	nl_input = filter_desc->neuron_layer_list->neuron_layer;

	// Gets the Filter Output
	nl_output = filter_desc->output;

	wi = nl_input->dimentions.x;
	hi = nl_input->dimentions.y;

	wo = nl_output->dimentions.x;
	ho = nl_output->dimentions.y;


	for (yo = 0; yo < ho; yo++)
	{
		yi = yo;

		for (xo = 0; xo < wo; xo++)
		{
			xi = xo;
			if ((xi >= 0) && (xi < wi) && (yi >= 0) && (yi < hi))
				nl_output->neuron_vector[xo + yo * wo].output = nl_input->neuron_vector[xi + yi * wi].output;
			else
				nl_output->neuron_vector[xo + yo * wo].output.ival = 0;
		}
	}
}


void nl_function_filter(FILTER_DESC *filter_desc) {

}


void nl_command_filter(FILTER_DESC *filter_desc) {

}
