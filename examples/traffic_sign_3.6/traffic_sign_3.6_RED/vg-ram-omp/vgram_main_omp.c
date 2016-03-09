/**
* Controle de versão: 1.1
*
* Esta versão foi modificada a partir do original fornecido pelo Alberto.
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
#include <omp.h>

#include "vgram.h"
#include "vgram_files.h"

// Variaveis globais //
static int bits_in_char[256];
VG_RAM_WNN vg_ram_wnn;
DATA_SET training_set;
DATA_SET testing_set;


// Código //
inline void
SetNeuronMemory(int *mem, int training_set_num_samples, int memory_bit_group_size, int neuron, int sample, int value)
{
	mem[(neuron * training_set_num_samples + sample) * (memory_bit_group_size+1) + memory_bit_group_size] = value;
}

inline int
GetNeuronMemory(int *mem, int training_set_num_samples, int memory_bit_group_size, int neuron, int sample)
{
	return mem[(neuron * training_set_num_samples + sample) * (memory_bit_group_size+1) + memory_bit_group_size];
}

inline int *
GetNeuronMemoryByNeuronAndSample(int *mem, int training_set_num_samples, int memory_bit_group_size, int neuron, int sample)
{
	return &mem[(neuron * training_set_num_samples + sample) * (memory_bit_group_size+1)];
}

inline int *
GetNeuronMemoryByNeuron(int *mem, int training_set_num_samples, int memory_bit_group_size, int neuron)
{
	return &mem[neuron * training_set_num_samples * (memory_bit_group_size+1)];
}

inline int *
GetNeuronMemoryBySample(int *mem, int memory_bit_group_size, int sample)
{
	return &mem[sample * (memory_bit_group_size+1)];
}

void
LoadTrainingSet(char *file_name)
{
	training_set.num_samples = LoadDataSet(file_name, &training_set, vg_ram_wnn.network_input_size);
}


void
LoadTestingSet(char *file_name)
{
	testing_set.num_samples = LoadDataSet(file_name, &testing_set, vg_ram_wnn.network_input_size);
}


void
AllocateNetworkSynapses()
{
	if ((vg_ram_wnn.synapses = (int *) malloc(vg_ram_wnn.number_of_neurons * vg_ram_wnn.number_of_synapses_per_neuron * sizeof(int))) == NULL)
		Error("Could not allocate memory in AllocateNetworkSynapses() for vg_ram_wnn.synapses.", "", "");
}


void
AllocateNetworkMemories()
{
	if ((vg_ram_wnn.number_of_synapses_per_neuron % PATTERN_UNIT_SIZE) == 0)
		vg_ram_wnn.memory_bit_group_size = vg_ram_wnn.number_of_synapses_per_neuron / PATTERN_UNIT_SIZE;
	else
		vg_ram_wnn.memory_bit_group_size = 1 + vg_ram_wnn.number_of_synapses_per_neuron / PATTERN_UNIT_SIZE;

	if ((vg_ram_wnn.memories = (int *) malloc(vg_ram_wnn.number_of_neurons * training_set.num_samples * (vg_ram_wnn.memory_bit_group_size + 1) * sizeof(int))) == NULL)
		Error("Could not allocate memory for memories in AllocateNetworkMemories()", "", "");
}


void
AllocateNetworkOutput()
{
	if ((vg_ram_wnn.neuron_output = (int *) malloc(testing_set.num_samples * vg_ram_wnn.number_of_neurons * sizeof(int))) == NULL)
		Error("Could not allocate memory for the neurons output in AllocateNetworkOutput().", "", "");
}

void
BuildNetwork()
{
	AllocateNetworkSynapses();

	AllocateNetworkMemories();
}


void
BuildBitPattern(unsigned int *bit_pattern, int *synapses,
#ifdef GREYSCALE
		float *network_input)
#else
		int *network_input)
#endif
{
	int shift;
	int synapse;
	int current_bit_pattern_group;
	int bit_value;

	current_bit_pattern_group = -1;
	for (synapse = 0; synapse < vg_ram_wnn.number_of_synapses_per_neuron; synapse++)
	{
		shift=synapse % PATTERN_UNIT_SIZE;
		current_bit_pattern_group=synapse / PATTERN_UNIT_SIZE;
		if (!shift)
			bit_pattern[current_bit_pattern_group] = 0; // Se começou um novo grupo, zero o mesmo.

		if (synapse == (vg_ram_wnn.number_of_synapses_per_neuron - 1))
			// Minchington: Última sinapse compara com a primeira
			bit_value = (network_input[synapses[synapse]] > network_input[synapses[0]]) ? 1 : 0;

		else
		{	// Minchington: Uma sinapse compara com a próxima
			bit_value = (network_input[synapses[synapse]] > network_input[synapses[synapse+1]]) ? 1 :0;
		}
		// Novos bits são inseridos na parte alta do padrão de bits.
		bit_pattern[current_bit_pattern_group] |= bit_value << shift;
	}
	// Novos bits são inseridos na parte alta do padrão de bits. Assim, ao fim da inserção, alinha os bits junto a parte baixa.

}


void
Train()
{
	int sample, neuron;
	int sample_class;
#ifdef GREYSCALE
	float *network_input;
#else
	int *network_input;
#endif

	#pragma omp parallel for default(none) \
			private(sample,neuron,network_input,sample_class) \
			shared(training_set,vg_ram_wnn)
	for (sample = 0; sample < training_set.num_samples; sample++)
	{
		network_input = &training_set.sample[sample * vg_ram_wnn.network_input_size];
		sample_class = training_set.sample_class[sample];
		for (neuron = 0; neuron < vg_ram_wnn.number_of_neurons; neuron++)
		{
			// Escreve diretamente na memória do neurônio o padrão de bits gerado a partir da sample
			BuildBitPattern((unsigned int*)
					GetNeuronMemoryByNeuronAndSample(
							vg_ram_wnn.memories,
							training_set.num_samples,
							vg_ram_wnn.memory_bit_group_size,
							neuron, sample),
					&(vg_ram_wnn.synapses[neuron * vg_ram_wnn.number_of_synapses_per_neuron]),
					network_input);
			// Escreve diretamente na memória do neurônio a classe associada ao padrão de bits
			SetNeuronMemory(vg_ram_wnn.memories,
					training_set.num_samples,
					vg_ram_wnn.memory_bit_group_size,
					neuron, sample, sample_class);

		}
	}
}

int
bitcount(unsigned int n)
{
   /* works for 32-bit numbers only    */
   /* fix last line for 64-bit numbers */

   register unsigned int tmp;

   tmp = n - ((n >> 1) & 033333333333)
           - ((n >> 2) & 011111111111);
   return ((tmp + (tmp >> 3)) & 030707070707) % 63;
}

int
HammingDistance(unsigned int *bit_pattern1, unsigned int *bit_pattern2)
{
	int i;
	unsigned int bit_difference;
	int hamming_distance;

	hamming_distance = 0;
	for (i = 0; i < vg_ram_wnn.memory_bit_group_size; i++)
	{
		bit_difference = bit_pattern1[i] ^ bit_pattern2[i];
		//hamming_distance += bitcount(bit_difference);
		hamming_distance += __builtin_popcount(bit_difference);
	}
	return hamming_distance;
}


int
FindNearestPattern(unsigned int *bit_pattern, int *neuron_memory, int *candidate_patterns)
{
	int learned_pattern, nearest_pattern;
	int cur_ham_dist, new_ham_dist;
	int num_candidates = 1;

	cur_ham_dist = vg_ram_wnn.number_of_synapses_per_neuron;
	for (learned_pattern = 0; learned_pattern < training_set.num_samples; learned_pattern++)
	{
		new_ham_dist = HammingDistance((unsigned int *)
				GetNeuronMemoryBySample(neuron_memory, vg_ram_wnn.memory_bit_group_size, learned_pattern),
				bit_pattern);
		if (new_ham_dist < cur_ham_dist)
		{
			candidate_patterns[0] = learned_pattern;
			num_candidates = 1;
			cur_ham_dist = new_ham_dist;
		}
		else if (new_ham_dist == cur_ham_dist)
		{
			candidate_patterns[num_candidates] = learned_pattern;
			num_candidates++;
		}
	}
	nearest_pattern = candidate_patterns[rand() % num_candidates];
	return (nearest_pattern);
}


void
Test()
{
	int sample, neuron, sample_class, correct;
	int nearest_pattern;

	int num_threads=omp_get_max_threads();

	float largest_function_f_value;
	int network_output_class;
#ifdef GREYSCALE
	float *network_input;
#else
	int *network_input;
#endif
	unsigned int *bit_pattern[num_threads];
	int *candidate_patterns[num_threads];
	//int *neuron_output[num_threads];
	int tid;

	int sum_correct = 0;

	#pragma omp parallel default(none) \
			private(tid,sample,sample_class,neuron,nearest_pattern,network_input,network_output_class,largest_function_f_value,correct) \
			shared(vg_ram_wnn,testing_set,training_set,candidate_patterns,bit_pattern/*,neuron_output*/,sum_correct)
	{
		correct = 0;

		tid = omp_get_thread_num();

		if ((bit_pattern[tid] = (unsigned int *) malloc(vg_ram_wnn.memory_bit_group_size * PATTERN_UNIT_SIZE/8)) == NULL)
			Error("Could not allocate bit_pattern in Test().", "", "");

		if ((candidate_patterns[tid] = (int *) malloc(training_set.num_samples * sizeof(int))) == NULL)
			Error("Could not allocate candidate_patterns in Test().", "", "");

		//neuron_output[tid]=(int *)malloc(vg_ram_wnn.number_of_neurons*sizeof(int));

		//Distribui as entries (samples de teste) entre as threads que ficarão responsáveis por cada subconjunto do dataset de testes.
		#pragma omp for
		for (sample = 0; sample < testing_set.num_samples; sample++)
		{
			network_input = &(testing_set.sample[sample * vg_ram_wnn.network_input_size]);
			sample_class = testing_set.sample_class[sample];
			for (neuron = 0; neuron < vg_ram_wnn.number_of_neurons; neuron++)
			{
				BuildBitPattern(bit_pattern[tid], 
				&(vg_ram_wnn.synapses[neuron * vg_ram_wnn.number_of_synapses_per_neuron]), 
				network_input);
				
				nearest_pattern = FindNearestPattern(bit_pattern[tid], 
				GetNeuronMemoryByNeuron(vg_ram_wnn.memories,
							training_set.num_samples,
							vg_ram_wnn.memory_bit_group_size,
							neuron),
				candidate_patterns[tid]);
				
				// O último inteiro do vetor de memória contém a saída memorizada junto com o padrão de bits
				//neuron_output[tid][neuron] =
				vg_ram_wnn.neuron_output[sample * vg_ram_wnn.number_of_neurons + neuron] =
					GetNeuronMemoryBySample(vg_ram_wnn.memories, vg_ram_wnn.memory_bit_group_size, nearest_pattern)[vg_ram_wnn.memory_bit_group_size];
			}
			/*
			#pragma omp critical
			{
				largest_function_f_value = EvaluateNetworkWeightedOutput(&network_output_class,
						neuron_output[tid],	vg_ram_wnn.number_of_neurons, sample_class);
			}
			if (network_output_class == sample_class)
			{
				printf("Sample = %d, NetworkOutputClass = %d, ExpectedOutput = %d, FunctionFValue %0.2f\n", sample, network_output_class, sample_class, largest_function_f_value);
				correct++;
			}
			else
				printf("Sample = %d, NetworkOutputClass = %d, ExpectedOutput = %d, FunctionFValue %0.2f *\n", sample, network_output_class, sample_class, largest_function_f_value);
			*/
		}

		/*
		#pragma omp critical
		{
			sum_correct+=correct;
		}
		*/
	}

	//printf("Percentage correct = %0.2f%%\n", 100.0 * (float) sum_correct / (float) testing_set.num_samples);
}


int
main (unsigned int argc, char *argv[])
{
	if (argc != 5)
		Error("Wrong number of arguments.\n", "Usage: vg-ram-wnn.exe network-configuration.csv synaptic-interconnection-pattern.csv training-set.csv testing-set.csv\n", "");

	LoadNetworkConfiguration(argv[1], &vg_ram_wnn);
	LoadTrainingSet(argv[3]);
	BuildNetwork();
	LoadSynapticInterconnectionPattern(argv[2], &vg_ram_wnn);

	Train();
	DeallocateDataSet(&training_set);

//	Save(&vg_ram_wnn, &training_set, "memory_omp.log");

	printf("Fim do treinamento.\n");

	LoadTestingSet(argv[4]);
	
	AllocateNetworkOutput();
	
	Test();
	
	EvaluateNetworkOutputs(&vg_ram_wnn, &testing_set);

	DeallocateDataSet(&testing_set);

	printf("Program finished OK!");

	return 0;
}
