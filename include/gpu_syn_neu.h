/*
*********************************************************************************
* Module : CUDA Kernels and Kernel wrappers Definitions				*
* version: 1.0									*
*    date: 26/07/2011								*
*      By: Jorcy de Oliveira Neto						*
********************************************************************************* 
*/

#ifndef __GPU_SYN_NEU_H
#define __GPU_SYN_NEU_H

__device__	NEURON_OUTPUT*	in_device_get_neuron_synapse_sensitivity_vector(NEURON_LAYER *neuron_layer,long int neuron);
__device__	NEURON**	in_device_get_neuron_source_vector(NEURON_LAYER *neuron_layer,long int neuron);
__device__	NEURON_OUTPUT**	in_device_get_neuron_output_source_vector(NEURON_LAYER *neuron_layer,long int neuron);
__device__ 	NEURON_LAYER**	in_device_get_neuron_output_neuron_layer(NEURON_LAYER *neuron_layer,long int neuron);
__device__ 	int*		in_device_get_neuron_synapse_x_position_vector(NEURON_LAYER *neuron_layer,long int neuron);
__device__ 	int*		in_device_get_neuron_synapse_y_position_vector(NEURON_LAYER *neuron_layer,long int neuron);
__device__	GPU_PATTERN*	in_device_get_neuron_pattern_vector_for_gpu_pattern(NEURON_LAYER *neuron_layer, int neuron, int sample);
__device__	void		in_device_set_neuron_pattern_vector(NEURON_LAYER *neuron_layer, int neuron, int sample, int pos,GPU_PATTERN val);
__device__ 	NEURON_OUTPUT*	in_device_get_neuron_output_gpu_pointer(NEURON_LAYER *neuron_layer,long int neuron);
__device__ 	NEURON_OUTPUT	in_device_get_neuron_output_gpu_value(NEURON_LAYER *neuron_layer,long int neuron);
__device__	void		in_device_set_neuron_output_gpu_value(NEURON_LAYER *neuron_layer,long int neuron,NEURON_OUTPUT neuron_output);
__device__	void		in_device_set_neuron_association(NEURON_LAYER *neuron_layer,long int neuron,long int sample, NEURON_OUTPUT neuron_output);
__device__	void		in_device_set_neuron_dentrite_dentrite(NEURON_LAYER *neuron_layer,long int neuron, NEURON_OUTPUT neuron_output);
__device__	NEURON_OUTPUT	in_device_get_associated_neuron_output(NEURON_LAYER *neuron_layer,long int neuron);

__device__ 	int		in_device_hamming_distance(int *bit_pattern1,int *bit_pattern2,int neuron_pattern_size);
__device__ 	int 		in_device_get_bit_value(NEURON_LAYER *source_neuron_layer,int i,NEURON_OUTPUT **neuron_synapse);
__device__	void		in_device_build_bit_pattern(NEURON_LAYER *neuron_layer,int bid,int bid2,int tid,GPU_PATTERN *bit_pattern);

__global__	void	in_device_train_distributed_memory_neuron_layer(NEURON_LAYER *neuron_layer);
__global__	void	in_device_all_dendrites_update_distributed_memory(NEURON_LAYER *neuron_layer,int *randoms);
__global__	void	in_device_train_shared_memory_neuron_layer(NEURON_LAYER *neuron_layer);
__global__	void	in_device_all_dendrites_update_shared_memory(NEURON_LAYER *neuron_layer,int *randoms);

#endif
