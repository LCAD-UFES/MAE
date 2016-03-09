/*
This file is part of MAE.

History:

[2006.02.13 - Helio Perroni Filho] Created.
*/

#include "mae_neuron_network.h"

#include "mae.h"

/*
Function Section
*/

void mae_neuron_network_start()
{
	init_pattern_xor_table();
	init_measuraments();
	build_network();
}

void mae_neuron_network_update_all_outputs()
{
	all_outputs_update();
}

/*
Input / Output Section
*/

void mae_neuron_network_load(const char* name, const char* path)
{
	char layer_name[256];
	strcpy(layer_name, name);

	char file_path[256];
	strcpy(file_path, path);

	load_image_to_object(layer_name, file_path);
}

void mae_neuron_network_save(const char* name, const char* path)
{
	char layer_name[256];
	strcpy(layer_name, name);

	char file_path[256];
	strcpy(file_path, path);

	save_object_image(file_path, layer_name);
}
