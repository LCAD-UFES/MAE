/**
 * Controle de versão: 3.0
 *
 * Esta versão foi modificada a partir da versão 1.1.
 * I - PATTERN_UNIT_SIZE estava especificado em bytes e nao em bits, agora este
 * valor está especificado em bits... mas eh dividido por 8 nos mallocs, pois estes ainda trabalham em bytes.
 * II - A quase totalidade dos int's foi alterada para unsigned int, uma vez
 * que inteiros com sinal sofrem o Shift aritimetico (http://www.cs.uaf.edu/~cs301/notes/Chapter5/node3.html)
 * quando deslocados para a direita em vez do Shift logico, que era o esperado.
 * III - A taxa de acerto da RN para o Linux permanece a mesma, porem eh alterada para o Windows.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cuda.h>
#include <cuda_runtime_api.h>
#include <device_functions.h>
#include <cutil.h>

#include "vgram.h"
#include "vgram_files.h"

// Variaveis globais //
VG_RAM_WNN cuda_vg_ram_wnn;
VG_RAM_WNN host_vg_ram_wnn;

DATA_SET cuda_training_set;
DATA_SET host_training_set;

DATA_SET cuda_testing_set;
DATA_SET host_testing_set;

int *Rand_h, *Rand_d;
int *candidates_d;
int *bit_pattern_d;

// Código //

__device__ inline void
cudaSetNeuronMemory(int *mem, int training_set_num_samples, int memory_bit_group_size, int neuron, int sample, int value)
{
	mem[(neuron * training_set_num_samples + sample) * (memory_bit_group_size+1) + memory_bit_group_size] = value;
}

__device__ inline int
cudaGetNeuronMemory(int *mem, int training_set_num_samples, int memory_bit_group_size, int neuron, int sample)
{
	return mem[(neuron * training_set_num_samples + sample) * (memory_bit_group_size+1) + memory_bit_group_size];
}

__device__ inline int *
cudaGetNeuronMemoryByNeuronAndSample(int *mem, int training_set_num_samples, int memory_bit_group_size, int neuron, int sample)
{
	return &mem[(neuron * training_set_num_samples + sample) * (memory_bit_group_size+1)];
}

__device__ inline int *
cudaGetNeuronMemoryByNeuron(int *mem, int training_set_num_samples, int memory_bit_group_size, int neuron)
{
	return &mem[neuron * training_set_num_samples * (memory_bit_group_size+1)];
}

__device__ inline int *
cudaGetNeuronMemoryBySample(int *mem, int memory_bit_group_size, int sample)
{
	return &mem[sample * (memory_bit_group_size+1)];
}

void
cudaAllocDataSet(DATA_SET *data_set, int num_samples, int num_inputs)
{
	CUDA_SAFE_CALL(cudaMalloc((void **) &(data_set->sample_class), num_samples * sizeof(int)));
#ifdef GREYSCALE
	CUDA_SAFE_CALL(cudaMalloc((void **) &(data_set->sample), num_inputs * num_samples * sizeof(float)));
#else
	CUDA_SAFE_CALL(cudaMalloc((void **) &(data_set->sample), num_inputs * num_samples * sizeof(int)));
#endif
}


void
cudaDeallocateDataSet(DATA_SET *data_set)
{
	cudaFree(data_set->sample);

	cudaFree(data_set->sample_class);
}

int
cudaLoadDataSet(DATA_SET *data_set_to, const DATA_SET *data_set_from, int num_inputs)
{
	int num_samples = data_set_from->num_samples;

	cudaAllocDataSet(data_set_to, num_samples, num_inputs);

	CUDA_SAFE_CALL(cudaMemcpy(data_set_to->sample_class, data_set_from->sample_class, num_samples * sizeof(int), cudaMemcpyHostToDevice));
#ifdef GREYSCALE
	CUDA_SAFE_CALL(cudaMemcpy(data_set_to->sample, data_set_from->sample, num_samples * num_inputs * sizeof(float), cudaMemcpyHostToDevice));
#else
	CUDA_SAFE_CALL(cudaMemcpy(data_set_to->sample, data_set_from->sample, num_samples * num_inputs * sizeof(int), cudaMemcpyHostToDevice));
#endif

	return num_samples;
}


void
LoadTrainingSet(char *file_name)
{
	host_training_set.num_samples = LoadDataSet(file_name, &host_training_set, host_vg_ram_wnn.network_input_size);
	cuda_training_set.num_samples = cudaLoadDataSet(&cuda_training_set, &host_training_set, host_vg_ram_wnn.network_input_size);

}


void
LoadTestingSet(char *file_name)
{
	host_testing_set.num_samples = LoadDataSet(file_name, &host_testing_set, host_vg_ram_wnn.network_input_size);
	cuda_testing_set.num_samples = cudaLoadDataSet(&cuda_testing_set, &host_testing_set, host_vg_ram_wnn.network_input_size);

}


void
AllocateNetworkSynapses()
{
	if ((host_vg_ram_wnn.synapses = (int *) malloc(host_vg_ram_wnn.number_of_neurons * host_vg_ram_wnn.number_of_synapses_per_neuron * sizeof(int))) == NULL)
		Error("Could not allocate memory in AllocateNetworkSynapses() for host_vg_ram_wnn.synapses.", "", "");
}

void
cudaAllocateNetworkSynapses()
{
	if (cudaMalloc((void **) &(cuda_vg_ram_wnn.synapses), cuda_vg_ram_wnn.number_of_synapses_per_neuron * cuda_vg_ram_wnn.number_of_neurons * sizeof(int)) != cudaSuccess)
		Error("Could not allocate memory in cudaAllocateNetworkSynapses() for vg_ram_wnn.synapses.", "", "");
}

void
AllocateNetworkMemories()
{
	if ((host_vg_ram_wnn.number_of_synapses_per_neuron % PATTERN_UNIT_SIZE) == 0)
		host_vg_ram_wnn.memory_bit_group_size = host_vg_ram_wnn.number_of_synapses_per_neuron / PATTERN_UNIT_SIZE;
	else
		host_vg_ram_wnn.memory_bit_group_size = 1 + host_vg_ram_wnn.number_of_synapses_per_neuron / PATTERN_UNIT_SIZE;

	if ((host_vg_ram_wnn.memories = (int *) malloc(host_vg_ram_wnn.number_of_neurons * host_training_set.num_samples * (host_vg_ram_wnn.memory_bit_group_size + 1) * sizeof(int))) == NULL)
		Error("Could not allocate memory for memories in AllocateNetworkMemories()", "", "");
}

void
cudaAllocateNetworkMemories()
{
	if ((cuda_vg_ram_wnn.number_of_synapses_per_neuron % PATTERN_UNIT_SIZE) == 0)
		cuda_vg_ram_wnn.memory_bit_group_size = cuda_vg_ram_wnn.number_of_synapses_per_neuron / PATTERN_UNIT_SIZE;
	else
		cuda_vg_ram_wnn.memory_bit_group_size = 1 + cuda_vg_ram_wnn.number_of_synapses_per_neuron / PATTERN_UNIT_SIZE;

	if (cudaMalloc((void**) &(cuda_vg_ram_wnn.memories), cuda_vg_ram_wnn.number_of_neurons * cuda_training_set.num_samples * (cuda_vg_ram_wnn.memory_bit_group_size + 1) * sizeof(int)) != cudaSuccess)
		Error("Could not allocate memory for memories in cudaAllocateNetworkMemories()", "", "");
}

void
AllocateNetworkOutput()
{
	if ((host_vg_ram_wnn.neuron_output = (int *) malloc(host_testing_set.num_samples * host_vg_ram_wnn.number_of_neurons * sizeof(int))) == NULL)
		Error("Could not allocate memory for the neurons output in AllocateNetworkOutput().", "", "");
}

void
cudaAllocateNetworkOutput()
{
	if (cudaMalloc((void**) &(cuda_vg_ram_wnn.neuron_output), cuda_testing_set.num_samples * cuda_vg_ram_wnn.number_of_neurons * sizeof(int)) != cudaSuccess)
		Error("Could not allocate memory for the neurons output in cudaAllocateNetworkOutput().", "", "");
}

void
cudaLoadSynapticInterconnectionPattern()
{
	CUDA_SAFE_CALL(cudaMemcpy(cuda_vg_ram_wnn.synapses, host_vg_ram_wnn.synapses, host_vg_ram_wnn.number_of_neurons * host_vg_ram_wnn.number_of_synapses_per_neuron * sizeof(int), cudaMemcpyHostToDevice));
}

void
cudaLoadNetworkConfiguration(const VG_RAM_WNN *host_vg_ram_wnn, VG_RAM_WNN *cuda_vg_ram_wnn)
{
	cuda_vg_ram_wnn->number_of_neurons = host_vg_ram_wnn->number_of_neurons;
	cuda_vg_ram_wnn->number_of_synapses_per_neuron = host_vg_ram_wnn->number_of_synapses_per_neuron;
	cuda_vg_ram_wnn->network_input_size = host_vg_ram_wnn->network_input_size;

}

void
BuildNetwork()
{
	AllocateNetworkSynapses();
	cudaAllocateNetworkSynapses();

	AllocateNetworkMemories();
	cudaAllocateNetworkMemories();
}

void
ClearNetwork()
{
//	TODO: liberar memória
//	DeallocateNetworkOutput();
//	cudaDeallocateNetworkOutput();
//
//	DeallocateNetworkSynapses();
//	cudaDeallocateNetworkSynapses();
//
//	DeallocateNetworkMemories();
//	cudaDeallocateNetworkMemories();
}

__device__ void
cudaBuildBitPattern(unsigned int *bit_pattern, int *synapses,
#ifdef GREYSCALE
		float *network_input,
#else
		int *network_input,
#endif
		int number_of_synapses_per_neuron)
{
	int synapse;
	int current_bit_pattern_group;
	int tid = threadIdx.x%PATTERN_UNIT_SIZE;
	__shared__ unsigned int bp[64];
	unsigned int aux;

	if(threadIdx.x<64) bp[threadIdx.x]=0;

	__syncthreads();

	for (synapse = threadIdx.x; synapse < number_of_synapses_per_neuron -1; synapse += blockDim.x)
	{
		// Cada grupo de padrão de bits tem tamanho igual a PATTERN_UNIT_SIZE.
		current_bit_pattern_group = synapse / PATTERN_UNIT_SIZE;

		// Minchington: Uma sinapse compara com a próxima
		aux = (network_input[synapses[synapse]] > network_input[synapses[synapse+1]]) ? 1 : 0;

		aux  = aux << tid;

		atomicOr(&bp[current_bit_pattern_group],aux);

	}
	__syncthreads();

	if(threadIdx.x == blockDim.x)
	{
		current_bit_pattern_group = synapse / PATTERN_UNIT_SIZE;
		aux  = aux << tid;
		aux = (network_input[synapses[synapse]] > network_input[synapses[0]]) ? 1 : 0;
		atomicOr(&bp[current_bit_pattern_group],aux);
		//atomicExch(&bit_pattern[current_bit_pattern_group], bp[current_bit_pattern_group]);

	}

	__syncthreads();

	if(threadIdx.x < number_of_synapses_per_neuron / PATTERN_UNIT_SIZE)
	{
		bit_pattern[threadIdx.x] =  bp[threadIdx.x];
	}


}

__global__ void
cudaNeuronTrain(VG_RAM_WNN cuda_vg_ram_wnn, DATA_SET cuda_training_set, int sample)
{
	int sample_class;
	cuda_vg_ram_wnn.network_input = &(cuda_training_set.sample[sample * cuda_vg_ram_wnn.network_input_size]);
	sample_class = cuda_training_set.sample_class[sample];
	for (int neuron = blockIdx.x; neuron < cuda_vg_ram_wnn.number_of_neurons; neuron += gridDim.x)
	{
		// Escreve diretamente na memória do neurônio o padrão de bits gerado a partir da sample
		cudaBuildBitPattern((unsigned int *)
				cudaGetNeuronMemoryByNeuronAndSample(
						cuda_vg_ram_wnn.memories,
						cuda_training_set.num_samples,
						cuda_vg_ram_wnn.memory_bit_group_size,
						neuron, sample),
							&(cuda_vg_ram_wnn.synapses[neuron * cuda_vg_ram_wnn.number_of_synapses_per_neuron]),
							cuda_vg_ram_wnn.network_input,
							cuda_vg_ram_wnn.number_of_synapses_per_neuron);

		// Escreve diretamente na memória do neurônio a classe associada ao padrão de bits
		cudaSetNeuronMemory(cuda_vg_ram_wnn.memories,
				cuda_training_set.num_samples,
				cuda_vg_ram_wnn.memory_bit_group_size,
				neuron, sample, sample_class);
	}

}

void
cudaTrain()
{
	for (int sample = 0; sample < cuda_training_set.num_samples; sample ++)
	{
		cudaNeuronTrain<<<1024,BLOCK_DIM>>>(cuda_vg_ram_wnn,cuda_training_set,sample);
		printf("Training sample %d de %d.\n", sample+1, cuda_training_set.num_samples);
	}
}


__device__ int
cudaBitcount(unsigned int n)
{
   unsigned int tmp;

   tmp = n - ((n >> 1) & 033333333333)
           - ((n >> 2) & 011111111111);
   return ((tmp + (tmp >> 3)) & 030707070707) % 63;
}

__device__ int
cudaHammingDistance(unsigned int *bit_pattern1, unsigned int *bit_pattern2,  int memory_bit_group_size)
{
	int i;
	unsigned int bit_difference;
	int hamming_distance;

	hamming_distance = 0;
	for (i = 0; i < memory_bit_group_size; i++)
	{
		bit_difference = bit_pattern1[i] ^ bit_pattern2[i];
		hamming_distance += cudaBitcount(bit_difference);
	}
	return hamming_distance;
}

__device__ void
cudaFindNearestPattern(unsigned int *bit_pattern,
					   int *neuron_memory,
					   int *neuron_output,
					   int training_set_num_samples,
					   int memory_bit_group_size)
{
	int hd_register;
	__shared__ int short_hd;

	int best_hd = 1<<30;
	int best_sample;
	short_hd = 1<<30;

	__syncthreads();

	for (int learned_pattern = threadIdx.x; learned_pattern < training_set_num_samples; learned_pattern += blockDim.x)
	{
		hd_register = cudaHammingDistance((unsigned int *)
				cudaGetNeuronMemoryBySample(neuron_memory, memory_bit_group_size, learned_pattern), bit_pattern, memory_bit_group_size);
		if(hd_register < best_hd)
		{
			best_hd = hd_register;
			best_sample = learned_pattern;
		}
	}

	atomicMin(&short_hd, best_hd);

	__syncthreads();

	if(short_hd == best_hd)
	{
		*neuron_output = cudaGetNeuronMemoryBySample(neuron_memory, memory_bit_group_size, best_sample)[memory_bit_group_size];
	}

}

__global__ void
cudaNeuronTest(VG_RAM_WNN cuda_vg_ram_wnn, DATA_SET cuda_training_set, DATA_SET cuda_testing_set, int sample)
{
	__shared__
	unsigned int bit_pattern[64];

	cuda_vg_ram_wnn.network_input = &(cuda_testing_set.sample[sample * cuda_vg_ram_wnn.network_input_size]);

	for (int neuron = blockIdx.x; neuron < cuda_vg_ram_wnn.number_of_neurons; neuron += gridDim.x)
	{
		cudaBuildBitPattern(bit_pattern,
					&(cuda_vg_ram_wnn.synapses[neuron * cuda_vg_ram_wnn.number_of_synapses_per_neuron]),
					cuda_vg_ram_wnn.network_input, cuda_vg_ram_wnn.number_of_synapses_per_neuron);

		cudaFindNearestPattern(bit_pattern,
					cudaGetNeuronMemoryByNeuron(cuda_vg_ram_wnn.memories,
					cuda_training_set.num_samples,
					cuda_vg_ram_wnn.memory_bit_group_size,
					neuron),
				    &(cuda_vg_ram_wnn.neuron_output[sample * cuda_vg_ram_wnn.number_of_neurons + neuron]),
				    cuda_training_set.num_samples,
				    cuda_vg_ram_wnn.memory_bit_group_size);
	}
}


void
cudaTest()
{
	for (int sample = 0; sample < cuda_testing_set.num_samples; sample++)
	{
		cudaNeuronTest<<<1024,BLOCK_DIM>>>(cuda_vg_ram_wnn,cuda_training_set,cuda_testing_set,sample);
		printf("Testing sample %d de %d.\n", sample+1, cuda_testing_set.num_samples);
	}
}

int
main (int argc, char *argv[])
{
	if (argc != 5)
		Error("Wrong number of arguments.\n", "Usage: vg-ram-wnn.exe network-configuration.csv synaptic-interconnection-pattern.csv training-set.csv testing-set.csv\n", "");

	LoadNetworkConfiguration(argv[1], &host_vg_ram_wnn);
	cudaLoadNetworkConfiguration(&host_vg_ram_wnn, &cuda_vg_ram_wnn);

	LoadTrainingSet(argv[3]);

	BuildNetwork();

	LoadSynapticInterconnectionPattern(argv[2], &host_vg_ram_wnn);
	cudaLoadSynapticInterconnectionPattern();

	cudaTrain();

	DeallocateDataSet(&host_training_set);
	cudaDeallocateDataSet(&cuda_training_set);

//	CUDA_SAFE_CALL(cudaMemcpy(host_vg_ram_wnn.memories, cuda_vg_ram_wnn.memories,
//			cuda_vg_ram_wnn.number_of_neurons * cuda_training_set.num_samples * (cuda_vg_ram_wnn.memory_bit_group_size + 1) * sizeof(int),
//			cudaMemcpyDeviceToHost));
//
//	Save(&host_vg_ram_wnn, &host_training_set, "memory.log");

	printf("Fim do treinamento.\n");

	LoadTestingSet(argv[4]);

	AllocateNetworkOutput();
	cudaAllocateNetworkOutput();

	cudaTest();

	CUDA_SAFE_CALL(cudaMemcpy(host_vg_ram_wnn.neuron_output, cuda_vg_ram_wnn.neuron_output,
			cuda_testing_set.num_samples * cuda_vg_ram_wnn.number_of_neurons * sizeof(int),
			cudaMemcpyDeviceToHost));

	EvaluateNetworkOutputs(&host_vg_ram_wnn, &host_testing_set);

	DeallocateDataSet(&host_testing_set);
	cudaDeallocateDataSet(&cuda_testing_set);

	ClearNetwork();

	printf("Program finished OK!");

	return 0;
}
