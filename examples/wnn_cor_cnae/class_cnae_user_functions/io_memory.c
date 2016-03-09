#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "mae.h"
#include "io_memory.h"



/*
***********************************************************
* 							  *
***********************************************************
*/

int 
reload_neural_memory (char *memory_name)
{
	char strCommand[128];
		
	sprintf (strCommand, "ResetStatistics (0);");
	interpreter (strCommand);

	sprintf (strCommand, "SetNetworkStatus (RECALL_PHASE);");
	interpreter (strCommand);

	sprintf (strCommand, "destroy network;");
	interpreter (strCommand);

	sprintf (strCommand, "reload from \"%s%s.bin\";", WNN_MEMORY_PATH, memory_name);
	interpreter (strCommand);
	
	return 0;
}



/*
***********************************************************
* 
***********************************************************
*/

int 
save_neural_memory (char *memory_name)
{
	char strCommand[128];
	
	sprintf (strCommand, "ResetStatistics (0);");
	interpreter (strCommand);

	sprintf (strCommand, "SetNetworkStatus (TRAINING_PHASE);");
	interpreter (strCommand);

	sprintf (strCommand, "unload to \"%s%s.bin\";", WNN_MEMORY_PATH, memory_name);
	interpreter (strCommand);
	
	return 0;
}
