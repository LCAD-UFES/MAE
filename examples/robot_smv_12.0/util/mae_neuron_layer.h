/*
This file is part of MAE.

History:

[2006.02.14 - Helio Perroni Filho] Created.
*/

#ifndef __MAE_NEURON_LAYER_H
#define __MAE_NEURON_LAYER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void* mae_neuron_layer;

/*
Function Section
*/

mae_neuron_layer mae_neuron_layer_get(const char* name);

float mae_neuron_layer_output_fval(mae_neuron_layer layer, int index);

int mae_neuron_layer_output_ival(mae_neuron_layer layer, int index);

/*
Input / Output Section
*/

void mae_neuron_layer_load(mae_neuron_layer layer, const char* path);

void mae_neuron_layer_save(mae_neuron_layer layer, const char* path);

/*
Property Section
*/

int mae_neuron_layer_get_width(mae_neuron_layer layer);

int mae_neuron_layer_get_height(mae_neuron_layer layer);

void mae_neuron_layer_set_output_fval(mae_neuron_layer layer, int index, float value);

void mae_neuron_layer_set_output_ival(mae_neuron_layer layer, int index, int value);

#ifdef __cplusplus
}
#endif

#endif
