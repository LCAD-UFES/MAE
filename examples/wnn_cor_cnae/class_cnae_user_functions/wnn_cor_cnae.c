#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "wnn_cor_cnae.h"
#include "io_memory.h"



neuronsOutputTableType g_neuronsOutputTable;



/*
*********************************************************************************
*  Brief: This routine catches errors				 	        *
*  Author: Felipe Thomaz Pedroni                                                *
*  Version: 1.0                                                                 * 
*  Input: The xml-rpc enviroment variable and the xml-rpc value variable        * 
*  Output: Nothing                                                              *
*********************************************************************************
*/
int 
search_neurons_output_table (neuronsOutputEntryType *entry)
{
  	int i, j, found;

  	i=0;
  	found=-1;
  	while( (i<g_neuronsOutputTable.last-1) && (found<0) )
    	{
      		found = i;
      		for (j=0;j<entry->nClasses;j++)
		{
	  		if (entry->classes[j] != g_neuronsOutputTable.entry[i].classes[j])
	    		{
	      			found=-1;
	      			break;
	    		}
		}
      		i++;
    	}

  	return(found);
}



/*
*********************************************************************************
*  Brief: This routine catches errors				 	        *
*  Author: Felipe Thomaz Pedroni                                                *
*  Version: 1.0                                                                 * 
*  Input: The xml-rpc enviroment variable and the xml-rpc value variable        * 
*  Output: Nothing                                                              *
*********************************************************************************
*/
int
insert_neurons_output_table (neuronsOutputEntryType *entry)
{
  	if (g_neuronsOutputTable.last > MAX_SIZE_NEURONS_OUTPUT_TABLE)
    	{	
      		fprintf(stderr, "Neurons output table is full\n");
      		exit(1);
    	}
  	else
    	{
      		g_neuronsOutputTable.entry[g_neuronsOutputTable.last-1] = *entry;
      		g_neuronsOutputTable.last++;
    	}
  	return(g_neuronsOutputTable.last-2);
}



/*
*********************************************************************************
*  Brief: This routine catches errors				 	        *
*  Author: Felipe Thomaz Pedroni                                                *
*  Version: 1.0                                                                 * 
*  Input: The xml-rpc enviroment variable and the xml-rpc value variable        * 
*  Output: Nothing                                                              *
*********************************************************************************
*/

void 
read_cnae_classes (FILE *pFile, neuronsOutputEntryType *entry)
{
	int i, j, x, min;

	// Read cnae classes associated to a document
  	entry->nClasses = 0;
  	while(!feof(pFile))
    	{
      		if (entry->nClasses==MAX_NUM_CLASSES_PER_DOC)
      		{
      			fprintf(stderr, "The number of classes associated to a train document exceeds the defined maximum number of classes per document\n");
			exit(1);
      		}		
      		fscanf (pFile, "%d", &(entry->classes[entry->nClasses]));
      		entry->nClasses++;
    	}		
  	entry->nClasses--;

  	printf ("cnae classes = ");
  	for (i=0;i<entry->nClasses;i++)
    	{
      		printf("%i ", entry->classes[i]);
    	}
  	printf ("\n");

  	// Sort the identifiers of cnae classes
  	for (i=1;i<entry->nClasses;i++)
    	{
      		min = i;
      		for (j=i+1;j<=entry->nClasses;j++)
		{
	  		if (entry->classes[j-1] < entry->classes[min-1])
	    		{
	      			min = j;
	    		}
	  		x = entry->classes[min-1];
	  		entry->classes[min-1] = entry->classes[i-1];
	  		entry->classes[i-1]= x;
		}
    	}
}



/*
*********************************************************************************
*  Brief: This routine catches errors				 	        *
*  Author: Felipe Thomaz Pedroni                                                *
*  Version: 1.0                                                                 * 
*  Input: The xml-rpc enviroment variable and the xml-rpc value variable        * 
*  Output: Nothing                                                              *
*********************************************************************************
*/
int 
save_neurons_output_table(char *wnn_memory_name)
{
  	FILE *table_file;
	int i, j;
	char buf[256];
	int class, n_classes, n_entries;
	
	sprintf(buf,"%s%s_table.bin",WNN_MEMORY_PATH, wnn_memory_name);
	
	if((table_file = fopen(buf,"w")) == NULL)
    	{
      		fprintf (stderr, "Could not write file %s\n", buf);
      		return 1;
    	}
	
	n_entries = g_neuronsOutputTable.last-1;
    
    	if(fwrite(&n_entries, sizeof(int), 1, table_file) != 1)
	{
      		fprintf (stderr, "Write error\n");
      		return 1;
    	}
	
  	for(i = 0; i < n_entries; i++)
	{	
		n_classes = g_neuronsOutputTable.entry[i].nClasses;
		
		if(fwrite(&n_classes, sizeof(int), 1, table_file) != 1)
		{
      			fprintf (stderr, "Write error\n");
			return 1;
    		}
		
		for (j = 0; j < n_classes; j++)
		{
			class = g_neuronsOutputTable.entry[i].classes[j];
			
			if(fwrite(&class, sizeof(int), 1, table_file) != 1)
			{
      				fprintf (stderr, "Write error\n");
      				return 1;
    			}
		}
	}	
	
	fclose (table_file);
	
  	return 0;
}



/*
*********************************************************************************
*  Brief: This routine catches errors				 	        *
*  Author: Felipe Thomaz Pedroni                                                *
*  Version: 1.0                                                                 * 
*  Input: The xml-rpc enviroment variable and the xml-rpc value variable        * 
*  Output: Nothing                                                              *
*********************************************************************************
*/
int 
reload_neurons_output_table(char *wnn_memory_name)
{
  	FILE *table_file;
	int i, j, n_entries;
	char buf[256];
	int n_classes, class;
	
	sprintf(buf,"%s%s_table.bin",WNN_MEMORY_PATH, wnn_memory_name);
	
	if((table_file = fopen(buf,"r")) == NULL)
    	{
      		fprintf (stderr, "Could not read file %s\n", buf);
      		return 1;
    	}
	
    	if(fread(&n_entries, sizeof(int), 1, table_file) != 1)
	{
      		fprintf (stderr, "Read error\n");
      		return 1;
    	}
	
	g_neuronsOutputTable.last = n_entries + 1;
	
  	for(i = 0; i < n_entries; i++)
	{		
		if(fread(&n_classes, sizeof(int), 1, table_file) != 1)
		{
      			fprintf (stderr, "Read error\n");
	      		return 1;
    		}
				
		g_neuronsOutputTable.entry[i].nClasses = n_classes;
		
		for (j = 0; j < n_classes; j++)
		{
			if(fread(&class, sizeof(int), 1, table_file) != 1)
			{
      				fprintf (stderr, "Read error\n");
	      	      		return 1;
    			}
			
			g_neuronsOutputTable.entry[i].classes[j] = class;
		}
	}	
	
	fclose (table_file);
	
  	return 0;
}

