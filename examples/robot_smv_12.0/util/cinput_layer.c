/*
This file is part of MAE.

History:

[2006.02.13 - Helio Perroni Filho] Created.
*/

#include "cinput_layer.h"

#include <string.h>

#include "mae.h"

/*
Function Section
*/

mae_input_layer mae_input_layer_get(const char* name)
{
	char input_name[256];
	strcpy(input_name, name);
	
	return (mae_input_layer) get_input_by_name(input_name);
}

float mae_input_layer_output_fval(mae_input_layer layer, int index)
{
	INPUT_DESC* input = (INPUT_DESC*) layer;
	return input->neuron_layer->neuron_vector[index].output.fval;
}

int mae_input_layer_output_ival(mae_input_layer layer, int index)
{
	INPUT_DESC* input = (INPUT_DESC*) layer;
	return input->neuron_layer->neuron_vector[index].output.ival;
}

void mae_input_layer_update(mae_input_layer layer)
{
	INPUT_DESC* input = (INPUT_DESC*) layer;
	update_input_neurons(input);
	update_input_image(input);
}

/*
Input / Output Section
*/

void mae_input_network_load(mae_input_layer layer, const char* path)
{
	char file_path[256];
	strcpy(file_path, path);

	INPUT_DESC* input = (INPUT_DESC*) layer;
	load_image_to_object(input->name, file_path);
}

void mae_input_network_save(mae_input_layer layer, const char* path)
{
	char file_path[256];
	strcpy(file_path, path);

	INPUT_DESC* input = (INPUT_DESC*) layer;
	save_object_image(input->name, file_path);
}

/*
Property Section
*/

GLubyte* mae_input_layer_get_texture_frame_buffer(mae_input_layer layer)
{
	INPUT_DESC* input = (INPUT_DESC*) layer;
	return input->image;
}

int mae_input_layer_get_texture_frame_width(mae_input_layer layer)
{
	INPUT_DESC* input = (INPUT_DESC*) layer;
	return input->tfw;
}

int mae_input_layer_get_texture_frame_height(mae_input_layer layer)
{
	INPUT_DESC* input = (INPUT_DESC*) layer;
	return input->tfh;
}

int mae_input_layer_get_width(mae_input_layer layer)
{
	INPUT_DESC* input = (INPUT_DESC*) layer;
	return input->neuron_layer->dimentions.x;
}

int mae_input_layer_get_height(mae_input_layer layer)
{
	INPUT_DESC* input = (INPUT_DESC*) layer;
	return input->neuron_layer->dimentions.y;
}

int mae_input_layer_get_visible_width(mae_input_layer layer)
{
	INPUT_DESC* input = (INPUT_DESC*) layer;
	return input->vpw;
}

int mae_input_layer_get_visible_height(mae_input_layer layer)
{
	INPUT_DESC* input = (INPUT_DESC*) layer;
	return input->vph;
}
