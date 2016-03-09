#ifndef VGRAM_H_
#define VGRAM_H_

// Definiçőes //
#define MAX_CSV_HEAD_SIZE 100000
#define MAX_CSV_LINE_SIZE 100000
#define PATTERN_UNIT_SIZE (8*sizeof(int))
#define BLOCK_DIM (PATTERN_UNIT_SIZE*8)
//#define GREYSCALE

#define N_DEFINED_SIGNS 43
#define NL_SCALE 2 	//AJUSTAR MANUALMENTE !!
#define	NL_WIDTH 17 * (NL_SCALE+1)
#define NL_HEIGHT 9 * (NL_SCALE+1)
#define VOTE_VARIANCE 2.5*(NL_WIDTH/NL_HEIGHT)*NL_SCALE

// Tipos //
struct _vg_ram_wnn
{
	int number_of_neurons;				// |O|
	int number_of_synapses_per_neuron;	// |X|
	int network_input_size;				// |N|

	// neuron_output é um vetor que armazena a saída corrente da rede neural
	int *neuron_output;

	// network_input é um vetor que armazena a entrada corrente da rede neural
#ifdef GREYSCALE
	float *network_input;
#else
	int *network_input;
#endif

	// synapses é um vetor de vetores de sinapses: um vetor de sinapses para cada neurônio da rede neural.
	// O vetor de sinapses de cada neurônio é lido uma vez e não é mais alterado
	// TODO: Alocar na memória de constantes!
	int *synapses;

	// memories é um vetor de matrizes de memórias: uma matriz de memórias para cada neurônio da rede neural.
	// Cada linha na matriz de memórias de um neurônio contęm um vetor de inteiros. Com excessão do último elemento,
	// todos os elementos de cada um destes vetores compôem, em conjunto, um padrão de bits aprendido durante a fase de
	// treino para uma dada entrada da rede neural. O último elemento de cada vetor é a saída vista junto
	// com esta dada entrada.
	int *memories;
	/**
	 * A memória do neurônio está organizada da seguinte forma considerando um cubo:
	 * com profundidade z, representando as camadas de neurônios
	 * com largura x, representando as amostras treinadas
	 * com altura y, representando o agrupamento de bits (memória propriamente dita do neurônio) e a classe da amostra associada.
	 *
	 * Um exemplo de representação linear da memória para 4 amostras de treinamento e agrupamento de bits igual a 2 + 1 classe de amostra:
	 * N0-S0-P0P1P2 N0-S1-P0P1P2 N0-S2-P0P1P2 N0-S3-P0P1P2
	 * N1-S0-P0P1P2 ...
	 */

	// memory_bit_group_size guarda o tamanho da linha das matrizes de memória dos neurônios.
	int memory_bit_group_size;
};
typedef struct _vg_ram_wnn VG_RAM_WNN;

struct _data_set
{
	int num_samples;
	int *sample_class;
#ifdef GREYSCALE
	float *sample;
#else
	int *sample;
#endif
};
typedef struct _data_set DATA_SET;


#endif /* VGRAM_H_ */
