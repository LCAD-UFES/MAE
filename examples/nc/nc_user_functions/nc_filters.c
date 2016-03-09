#include "mae.h"
#include "nc_user_functions.h"



/*
*********************************************************************************
* Function: scale_nl_filter	             	 				*
* Description: 			                        			*
* Inputs: 				                  			*
* Output: 				                			*
*********************************************************************************
*/

void attention_filter (FILTER_DESC *filter_desc)
{
	PARAM_LIST *p_list = NULL;
	NEURON_LAYER_LIST *n_list = NULL;
	NEURON_LAYER *nl_output = NULL, *nl_input = NULL;
	int nl_number, p_number;
	int type;
	int xo, yo, wo, ho;

	// Checks the Neuron Layers Number
	for (nl_number = 0, n_list = filter_desc->neuron_layer_list; n_list != NULL; n_list = n_list->next, nl_number++)
            	;
	if (nl_number != 1)
	{
		Erro ("Error: Wrong number of inputs. The attention_filter must have only one input neural layer", "", "");
		return;
	}

	// Checks the Parameters Number
	for (p_number = 0, p_list = filter_desc->filter_params; p_list != NULL; p_list = p_list->next, p_number++)
            	;
	if (p_number != 2)
	{
		Erro ("Error: Wrong number of parameters. The attention_filter must have only one parameter <UP|DOWN>.", "", "");
		return;
	}
	
	// Gets the Filter Parameters
	type = filter_desc->filter_params->next->param.ival;

	// Gets the Input Neuron Layer
	nl_input = filter_desc->neuron_layer_list->neuron_layer;

	// Gets the Filter Output 
	nl_output = filter_desc->output;
	
	wo = nl_output->dimentions.x;
	ho = nl_output->dimentions.y;
		
	for (yo = 0; yo < ho; yo++)
	{	
		for (xo = 0; xo < wo; xo++)
		{
			if (type == UP)
			{
				if (RED(nl_input->neuron_vector[xo + yo * wo].output.ival) < 255/2)
					nl_output->neuron_vector[xo + yo * wo].output.ival = nl_input->neuron_vector[xo + yo * wo].output.ival & 0x0000ff00L;
				else
					nl_output->neuron_vector[xo + yo * wo].output.ival = 0;
			}
			else if (type == DOWN)
			{
				if (BLUE(nl_input->neuron_vector[xo + yo * wo].output.ival) < 255/2)
					nl_output->neuron_vector[xo + yo * wo].output.ival = nl_input->neuron_vector[xo + yo * wo].output.ival & 0x0000ff00L;
				else
					nl_output->neuron_vector[xo + yo * wo].output.ival = 0;
			}
		}
	}
}
