#ifndef SYN_NEU_H_
#define SYN_NEU_H_

#include <mae.h>

extern int number_of_synapses;
extern int number_of_subtables;
extern int subtable_size;
extern int subtable_rows;

int
hamming_distance (PATTERN *pattern1, PATTERN *pattern2, int connections);

int
get_neuron_memory_size(NEURON *neuron);

void
compute_input_pattern (NEURON *neuron, PATTERN *input_pattern, SENSITIVITY_TYPE sensitivity, OUTPUT_TYPE output_type);
void
compute_input_pattern_minchinton(NEURON *neuron, PATTERN *input_pattern, SENSITIVITY_TYPE sensitivity, OUTPUT_TYPE output_type);

NEURON_OUTPUT
neuron_memory_read (NEURON_LAYER *neuron_layer, int n, PATTERN *input_pattern, int connections, OUTPUT_TYPE output_type);

int
neuron_memory_write (NEURON_LAYER *neuron_layer, int n, PATTERN *input_pattern, int connections, OUTPUT_TYPE output_type);

int
find_nearest_pattern (NEURON *neuron, PATTERN *input_pattern, int connections, int *distance);

void
initialise_neuron_memory (NEURON_LAYER *neuron_layer, int n);

void
initialize_input_pattern (PATTERN **input_pattern, int connections);

int
get_bit(int start_bit, const PATTERN *input_pattern);
int
get_bits(int num_bits, int start_bit, PATTERN *input_pattern, int input_pattern_size);
int
get_one_bit(int start_bit, const PATTERN *input_pattern);

float
obtain_composed_color_synapse_weighted_value(SYNAPSE *synapse);
float
obtain_composed_color_synapse_weighted_value_color_2_grayscale(SYNAPSE *synapse);
float
obtain_composed_color_synapse_weighted_value_hsv(SYNAPSE *synapse);


long long int*
minchinton_fat_fast_neuron_alloc_new_hash_entry(long long int vg_ram_entry_id);
long long int*
minchinton_fat_fast_neuron_realloc_hash_entry(long long int vg_ram_entry_id, long long int *entry);

void
minchinton_fat_fast_neuron_initialize_memory_parameters(int connections, int num_training_samples);

void
initialize_neuron_memory_minchinton_fat_fast_index_only (NEURON_LAYER *neuron_layer, int neuron_index);

#endif /* SYN_NEU_H_ */
