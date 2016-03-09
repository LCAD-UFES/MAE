#ifndef _ROBOT_FILTERS_H
#define _ROBOT_FILTERS_H

// Includes
#include "filter.h"
#include "../robot.h"

// Definitions

// Macros

// Types

// Prototypes
void mt_filter (FILTER_DESC *filter_desc);
NEURON_OUTPUT 
evaluate_dendrite (NEURON_LAYER *neuron_layer, int n);
void
train_neuron (NEURON_LAYER *neuron_layer, int n);
// Global Variables
int g_nNetworkStatus = RUNNING;

#endif
