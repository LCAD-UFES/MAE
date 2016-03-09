#ifndef VGRAM_FILES_H_
#define VGRAM_FILES_H_

#include "vgram.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

void
Error(char *message1, char *message2, char *message3);

FILE *
OpenCsvFile(char *file_name, char *csv_head);

void *
ReadCsvFileLine(FILE *csv_file, char *line_format);

void
LoadNetworkConfiguration(char *file_name, VG_RAM_WNN *host_vg_ram_wnn);

char *
BuildSynapticInterconectionPatternCsvFileHeader(int num_synapses);

char *
BuildSynapticInterconectionPatternCsvLineFormat(int num_synapses);

void
LoadSynapticInterconnectionPattern(char *file_name, VG_RAM_WNN *host_vg_ram_wnn);

char *
BuildDataSetCsvFileHeader(int network_input_size);

char *
BuildDataSetCsvLineFormat(int network_input_size);

int
CountNumberOfSamplesInDataSet(char *file_name,
		char *data_set_csv_file_header,
		char *data_set_csv_line_format,
		int num_inputs);

void
AllocDataSet(DATA_SET *data_set, int num_samples, int num_inputs, char *file_name);

void
DeallocateDataSet(DATA_SET *data_set);

int
LoadDataSet(char *file_name, DATA_SET *data_set, int num_inputs);

void
Save(VG_RAM_WNN *host_vg_ram_wnn, DATA_SET *host_training_set, const char *filename);

float
EvaluateNetworkOutput(int *network_output_class, int *network_output,
		int number_of_neurons, int expected_sample_class);

void
EvaluateNetworkOutputs(const VG_RAM_WNN *vg_ram_wnn, const DATA_SET *testing_set);

#ifdef __cplusplus
}
#endif

#endif /* VGRAM_FILES_H_ */
