/*
This file is part of MAE.

History:

[2006.02.14 - Helio Perroni Filho] Created.
*/

#include "mae_neuron_layer.h"

#include <string.h>

#include "mae.h"

/*
Function Section
*/

mae_neuron_layer mae_neuron_layer_get(const char* name)
{
	char neuron_layer_name[256];
	strcpy(neuron_layer_name, name);
	
	return (mae_neuron_layer) get_neuron_layer_by_name(neuron_layer_name);
}

float mae_neuron_layer_output_fval(mae_neuron_layer layer, int index)
{
	NEURON_LAYER* nn = (NEURON_LAYER*) layer;
	return nn->neuron_vector[index].output.fval;
}

int mae_neuron_layer_output_ival(mae_neuron_layer layer, int index)
{
	NEURON_LAYER* nn = (NEURON_LAYER*) layer;
	return nn->neuron_vector[index].output.ival;
}

/*
Input / Output Section
*/

void mae_neuron_layer_load(mae_neuron_layer layer, const char* path)
{
	char file_path[256];
	strcpy(file_path, path);
	
	NEURON_LAYER* nn = (NEURON_LAYER*) layer;
	load_image_to_object(nn->name, file_path);
}

void mae_neuron_layer_save(mae_neuron_layer layer, const char* path)
{
	char file_path[256];
	strcpy(file_path, path);

	NEURON_LAYER* nn = (NEURON_LAYER*) layer;
	save_object_image(file_path, nn->name);
}

/*
Property Section
*/

int mae_neuron_layer_get_width(mae_neuron_layer layer)
{
	NEURON_LAYER* nn = (NEURON_LAYER*) layer;
	return nn->dimentions.x;
}

int mae_neuron_layer_get_height(mae_neuron_layer layer)
{
	NEURON_LAYER* nn = (NEURON_LAYER*) layer;
	return nn->dimentions.y;
}

void mae_neuron_layer_set_output_fval(mae_neuron_layer layer, int index, float value)
{
	NEURON_LAYER* nn = (NEURON_LAYER*) layer;
	nn->neuron_vector[index].output.fval = value;
}

void mae_neuron_layer_set_output_ival(mae_neuron_layer layer, int index, int value)
{
	NEURON_LAYER* nn = (NEURON_LAYER*) layer;
	nn->neuron_vector[index].output.ival = value;
}
