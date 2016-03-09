#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "io_memory.h"

int 
reload_neural_memory (char *memory_name)
{
	NEURON_OUTPUT output;
	char strCommand[128];
	PARAM_LIST *p_list, *pval;

	pval = (PARAM_LIST *) malloc (sizeof(PARAM_LIST));
	pval->next = NULL;
	pval->param.ival = 0;

	p_list = (PARAM_LIST *) malloc (sizeof(PARAM_LIST));
	p_list->next = pval;
	p_list->param.ival = 1;

	output = ResetStatistics (p_list);

	pval->param.ival = RECALL_PHASE;
	output = SetNetworkStatus (p_list);

	destroy_network ();

	sprintf (strCommand, "\"%s%s.bin\"", WNN_MEMORY_PATH, memory_name);
	reload_network (strCommand);

	free (pval);
	free (p_list);
	
	return 0;
}


int
save_neural_memory (char *memory_name)
{
	NEURON_OUTPUT output;
	char strCommand[128];
	PARAM_LIST *p_list, *pval;

	pval = (PARAM_LIST *) malloc (sizeof(PARAM_LIST));
	pval->next = NULL;
	pval->param.ival = 0;

	p_list = (PARAM_LIST *) malloc (sizeof(PARAM_LIST));
	p_list->next = pval;
	p_list->param.ival = 1;

	output = ResetStatistics (p_list);

	pval->param.ival = TRAINING_PHASE;
	output = SetNetworkStatus (p_list);

	sprintf (strCommand, "\"%s%s.bin\"", WNN_MEMORY_PATH, memory_name);
	unload_network (strCommand);

	free (pval);
	free (p_list);
	
	return 0;
}
