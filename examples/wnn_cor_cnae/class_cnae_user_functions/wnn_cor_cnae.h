#ifndef _WNN_COR_CNAE_H
#define _WNN_COR_CNAE_H

/* defines */
// Definitions used in data correlation
#define NUM_CLASSES		      1183
#define MAX_NUM_CLASSES_PER_DOC       200
#define MAX_SIZE_NEURONS_OUTPUT_TABLE 4000

/* structs */
typedef struct
{
	int nClasses;
	int classes[MAX_NUM_CLASSES_PER_DOC];
} neuronsOutputEntryType;

typedef struct
{
	neuronsOutputEntryType entry[MAX_SIZE_NEURONS_OUTPUT_TABLE];
	int last;
}neuronsOutputTableType;

/* global variables */
extern neuronsOutputTableType g_neuronsOutputTable;

/* prototypes */
int search_neurons_output_table(neuronsOutputEntryType *entry);
int insert_neurons_output_table (neuronsOutputEntryType *entry);
void read_cnae_classes(FILE *pFile, neuronsOutputEntryType *entry);
int save_neurons_output_table(char *wnn_memory_name);
int reload_neurons_output_table(char *wnn_memory_name);
#endif
