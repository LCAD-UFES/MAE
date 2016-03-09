/*
This file is part of MAE.

History:

[2006.02.13 - Helio Perroni Filho] Created.

[2006.02.16 - Helio Perroni Filho] Added method to update all output layers.
*/

#ifndef __MAE_NEURON_NETWORK_H
#define __MAE_NEURON_NETWORK_H

#ifdef __cplusplus
extern "C" {
#endif

/*
Function Section
*/

/**
Starts the underlying MAE framework.
*/
void mae_neuron_network_start();

/**
Updates the state of all output layers.
*/
void mae_neuron_network_update_all_outputs();

/*
Input / Output Section
*/

void mae_neuron_network_load(const char* name, const char* path);

void mae_neuron_network_save(const char* name, const char* path);

#ifdef __cplusplus
}
#endif


#endif
