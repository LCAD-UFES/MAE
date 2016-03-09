#ifndef _IO_MEMORY_H
#define _IO_MEMORY_H

#include "mae.h"

/* defines */
#define TRAINING_PHASE		1
#define RECALL_PHASE		2

#define WNN_MEMORY_PATH		"TAGGER_MEMORIES/"

/* structs */

/* global variables */

/* prototypes */

int reload_neural_memory (char *memory_name);
int save_neural_memory (char *memory_name);

NEURON_OUTPUT ResetStatistics (PARAM_LIST *pParamList);
NEURON_OUTPUT SetNetworkStatus (PARAM_LIST *pParamList);
void destroy_network (void);
void reload_network (char *file_name);
void unload_network (char *file_name);
#endif
