#include <math.h>
#include "vgram_files.h"

void
Error(char *message1, char *message2, char *message3)
{
	printf("Error: %s %s %s\n", message1, message2, message3);
	printf("Hit ENTER to exit.\n");
	getchar();
	exit(1);
}

FILE *
OpenCsvFile(char *file_name, char *csv_head)
{
	FILE *csv_file;
	char csv_head_readed[MAX_CSV_HEAD_SIZE];

	if ((csv_file = fopen(file_name, "r")) == NULL)
		return NULL;

	if (fgets(csv_head_readed, MAX_CSV_HEAD_SIZE, csv_file) == NULL)
		Error("Could not read the head of csv file", file_name, "in OpenCsvFile().");

	if (strcmp(csv_head_readed, csv_head) != 0)
	{
		printf("Head of csv file %s does not match. Head expected: %s, head readed: %s\n", file_name, csv_head, csv_head_readed);
		return NULL;
	}
	return csv_file;
}


void *
ReadCsvFileLine(FILE *csv_file, char *line_format)
{
	int i;
	int integer;
	float floating_point;
	void *csv_line;
	char csv_str_line[MAX_CSV_LINE_SIZE];
	char *aux;

	if ((csv_line = (void *) malloc(strlen(line_format) * sizeof(int))) == NULL)	// Assumimos aqui que int e float ocupam o mesmo número de bytes de memória
		Error("Could not allocate memory in ReadCsvFileLine().", "", "");

	if (fgets(csv_str_line, MAX_CSV_LINE_SIZE-1, csv_file) == NULL)	// fim do arquivo
		return NULL;

	aux = csv_str_line;
	for (i = 0; i < (int) strlen(line_format); i++)
	{
		if (!aux)
			Error("Unexpected end of line in ReadCsvFileLine()", "", "");
		switch(line_format[i])
		{
			case 'i':
				integer = atoi(aux);
				((int *) csv_line)[i] = integer;
				break;
			case 'f':
				floating_point = (float) atof(aux);
				((float *) csv_line)[i] = floating_point;
				break;
			default:
				Error("Unknown character in csv file line_format descriptor: %s", line_format, "");
		}
		aux = strchr(aux, ';');
		if (aux)
			aux++;
	}
	return csv_line;
}


void
LoadNetworkConfiguration(char *file_name, VG_RAM_WNN *host_vg_ram_wnn)
{
	FILE *csv_file;
	void *csv_file_line;

	if ((csv_file = OpenCsvFile(file_name, "O;X;N\n")) == NULL)
		Error("Could not open file", file_name, "in LoadNetworkConfiguration().");

	if ((csv_file_line = ReadCsvFileLine(csv_file, "iii")) == NULL)
		Error("Could not read a csv file line from", file_name, "in LoadNetworkConfiguration().");

	host_vg_ram_wnn->number_of_neurons = ((int *) csv_file_line)[0];
	host_vg_ram_wnn->number_of_synapses_per_neuron = ((int *) csv_file_line)[1];
	host_vg_ram_wnn->network_input_size = ((int *) csv_file_line)[2];

	free(csv_file_line);
}


char *
BuildSynapticInterconectionPatternCsvFileHeader(int num_synapses)
{
	int i;
	char *synaptic_interconection_pattern_csv_file_header;
	char *aux;

	aux = synaptic_interconection_pattern_csv_file_header = (char *) malloc(num_synapses * 10 * sizeof(char)); // permite 10000+ synapses

	aux += sprintf(aux, "O");
	for (i = 0; i < num_synapses; i++)
		aux += sprintf(aux, ";x%d", i);
	sprintf(aux, "\n");

	return synaptic_interconection_pattern_csv_file_header;
}


char *
BuildSynapticInterconectionPatternCsvLineFormat(int num_synapses)
{
	int i;
	char *synaptic_interconection_pattern_csv_line_format;

	synaptic_interconection_pattern_csv_line_format = (char *) malloc((num_synapses + 2) * sizeof(char));

	synaptic_interconection_pattern_csv_line_format[0] = 'i'; // formato do número do neurônio, O
	for (i = 1; i < num_synapses + 1; i++)
		synaptic_interconection_pattern_csv_line_format[i] = 'i';	// formato do número da entrada ni onde a synapse se conecta

	synaptic_interconection_pattern_csv_line_format[i] = '\0';

	return synaptic_interconection_pattern_csv_line_format;
}


void
LoadSynapticInterconnectionPattern(char *file_name, VG_RAM_WNN *host_vg_ram_wnn)
{
	int neuron, synapse;
	FILE *csv_file;
	void *csv_file_line;
	char *synaptic_interconection_pattern_csv_file_header;
	char *synaptic_interconection_pattern_csv_line_format;

	synaptic_interconection_pattern_csv_file_header = BuildSynapticInterconectionPatternCsvFileHeader(host_vg_ram_wnn->number_of_synapses_per_neuron);
	if ((csv_file = OpenCsvFile(file_name, synaptic_interconection_pattern_csv_file_header)) == NULL)
		Error("Could not open file", file_name, "in LoadSynapticInterconnectionPattern().");
	free(synaptic_interconection_pattern_csv_file_header);

	synaptic_interconection_pattern_csv_line_format = BuildSynapticInterconectionPatternCsvLineFormat(host_vg_ram_wnn->number_of_synapses_per_neuron);
	for (neuron = 0; neuron < host_vg_ram_wnn->number_of_neurons; neuron++)
	{
		if ((csv_file_line = ReadCsvFileLine(csv_file, synaptic_interconection_pattern_csv_line_format)) == NULL)
			Error("Could not read a csv file line from", file_name, "in LoadSynapticInterconnectionPattern().");

		for (synapse = 0; synapse < host_vg_ram_wnn->number_of_synapses_per_neuron; synapse++)
			host_vg_ram_wnn->synapses[neuron * host_vg_ram_wnn->number_of_synapses_per_neuron + synapse] = ((int *)csv_file_line)[synapse+1]; // primeiro elemento da linha do csv é o número do neurônio

		free(csv_file_line);
	}
	free(synaptic_interconection_pattern_csv_line_format);
}

char *
BuildDataSetCsvFileHeader(int network_input_size)
{
	int i;
	char *data_set_csv_file_header;
	char *aux;

	aux = data_set_csv_file_header = (char *) malloc(network_input_size * 10 * sizeof(char)); // permite 10000+ inputs

	for (i = 0; i < network_input_size; i++)
		aux += sprintf(aux, "v%d;", i);
	sprintf(aux, "ci\n");

	return data_set_csv_file_header;
}


char *
BuildDataSetCsvLineFormat(int network_input_size)
{
	int i;
	char format;
	char *data_set_csv_line_format;
#ifdef GREYSCALE
	format = 'f';
#else
	format = 'i';
#endif

	data_set_csv_line_format = (char *) malloc((network_input_size + 2) * sizeof(char));

	for (i = 0; i < network_input_size; i++)
		data_set_csv_line_format[i] = format; // formato do número que representa cada termo

	data_set_csv_line_format[i] = 'i';		// formato do número que representa a classe ci
	data_set_csv_line_format[i+1] = '\0';

	return data_set_csv_line_format;
}


int
CountNumberOfSamplesInDataSet(char *file_name,
		char *data_set_csv_file_header,
		char *data_set_csv_line_format,
		int num_inputs)
{
	FILE *csv_file;
	int num_samples;

	if ((csv_file = OpenCsvFile(file_name, data_set_csv_file_header)) == NULL)
		Error("Could not open file", file_name, "in CountNumberOfSamplesInDataSet().");

	num_samples = 0;
	fseek(csv_file, (long)num_samples, SEEK_SET);
	while ((ReadCsvFileLine(csv_file, data_set_csv_line_format)) != NULL)
		num_samples++;

	fclose(csv_file);
	return num_samples-1;//discards header
}

void
AllocDataSet(DATA_SET *data_set, int num_samples, int num_inputs, char *file_name)
{
	if ((data_set->sample_class = (int *) malloc(num_samples * sizeof(int))) == NULL)
		Error("Could not allocate memory in AllocDataSet() while loading samples from", file_name, ".");

#ifdef GREYSCALE
	if ((data_set->sample = (float *) malloc(num_samples * num_inputs * sizeof(float))) == NULL)
#else
	if ((data_set->sample = (int *) malloc(num_samples * num_inputs * sizeof(int))) == NULL)
#endif
		Error("Could not allocate memory in AllocDataSet() while loading samples from", file_name, ".");
}


void
DeallocateDataSet(DATA_SET *data_set)
{
	free(data_set->sample);

	free(data_set->sample_class);
}

int
LoadDataSet(char *file_name, DATA_SET *data_set, int num_inputs)
{
	int sample, num_samples, term;
	FILE *csv_file;
	void *csv_file_line;
	char *data_set_csv_file_header;
	char *data_set_csv_line_format;

	data_set_csv_file_header = BuildDataSetCsvFileHeader(num_inputs);
	data_set_csv_line_format = BuildDataSetCsvLineFormat(num_inputs);

	num_samples = CountNumberOfSamplesInDataSet(file_name, data_set_csv_file_header, data_set_csv_line_format, num_inputs);
	AllocDataSet(data_set, num_samples, num_inputs, file_name);

	if ((csv_file = OpenCsvFile(file_name, data_set_csv_file_header)) == NULL)
		Error("Could not open file", file_name, "in LoadDataSet().");
	free(data_set_csv_file_header);

	for (sample = 0; sample < num_samples; sample++)
	{
		csv_file_line = ReadCsvFileLine(csv_file, data_set_csv_line_format);
		for (term = 0; term < num_inputs; term++)
		#ifdef GREYSCALE
			data_set->sample[sample * num_inputs + term] = ((float *)csv_file_line)[term];
		#else
			data_set->sample[sample * num_inputs + term] = ((int *)csv_file_line)[term];
		#endif

		data_set->sample_class[sample] = ((int *)csv_file_line)[term];
		free(csv_file_line);
	}
	free(data_set_csv_line_format);
	fclose(csv_file);

	return num_samples;
}

void
PrintPattern(FILE *fp, int memory_bit_group_size, int *bit_pattern)
{
	int bit_group, j;
	int bit_group_index;
	char bits[PATTERN_UNIT_SIZE+1];

	for (bit_group_index = memory_bit_group_size; bit_group_index--;)
	{
		bit_group = bit_pattern[bit_group_index];
		for (j = 0; j < PATTERN_UNIT_SIZE; bit_group>>=1, j++)
			bits[PATTERN_UNIT_SIZE-j-1] = bit_group & 0x1 ? '1' : '0';
		bits[PATTERN_UNIT_SIZE] = 0;
		fprintf(fp, "%s", bits);
	}
	// Imprime a classe associada ao padrão de bits
	fprintf(fp, "%d\n", bit_pattern[memory_bit_group_size]);
}

void
Save(VG_RAM_WNN *host_vg_ram_wnn, DATA_SET *host_training_set, const char *filename)
{
	FILE *fp;
	int sample, neuron;

	// Reference: http://www.cprogramming.com/tutorial/cfileio.html
	fp=fopen(filename, "w");

	for (sample = 0; sample < host_training_set->num_samples; sample++)
	{
		for (neuron = 0; neuron < host_vg_ram_wnn->number_of_neurons; neuron++)
		{
			// Imprime o padrão de bits gerado a partir da sample
			PrintPattern(fp, host_vg_ram_wnn->memory_bit_group_size,
					&(host_vg_ram_wnn->memories[(neuron * host_training_set->num_samples + sample) * (host_vg_ram_wnn->memory_bit_group_size+1)])
					);
		}
	}

	fclose(fp);
}

float
EvaluateNetworkOutput(int *network_output_class, int *network_output,
		int number_of_neurons, int expected_sample_class)
{
	int neuron_i, neuron_j;
	int class_aux, most_voted_class = 0;
	int aux, max = 0;
	static int last_expected_sample_class = 0;
	static int last_most_voted_class = 0;
	static float last_confidence = 0;
	float confidence;

	for (neuron_i = 0; neuron_i < number_of_neurons; neuron_i++)
	{
		class_aux = network_output[neuron_i];
		aux = 1;
		for (neuron_j = neuron_i + 1; neuron_j < number_of_neurons; neuron_j++)
		{
			if (network_output[neuron_j] == class_aux)
				aux++;
		}
		if (aux > max)
		{
			max = aux;
			most_voted_class = class_aux;
		}
	}
	confidence = ((float) max / (float) number_of_neurons);

	if (expected_sample_class != last_expected_sample_class)
	{
		last_expected_sample_class = expected_sample_class;
		last_most_voted_class = most_voted_class;
		last_confidence = confidence;
	}
	else
	{
		if (confidence >= last_confidence)
		{
			last_most_voted_class = most_voted_class;
			last_confidence = confidence;
		}
	}

	*network_output_class = last_most_voted_class;
	return last_confidence;
}

float
normal_dist(float x, float mean, float dev)
{
	return (1.0/sqrt(2.0*M_PI*dev*dev))*exp((-1.0)*(((x-mean)*(x-mean))/(2.0*dev*dev)));
}

float
EvaluateNetworkWeightedOutput(int *network_output_class, int *network_output,
		int number_of_neurons, int expected_sample_class)
{
	int class_id,x,y;
	int most_voted_class = 0;
	float max = -1.0f;
	float class_count[N_DEFINED_SIGNS];
	float accumulated_weight = 0.0f ;
	float confidence;

	static int last_expected_sample_class = 0;
	static int last_most_voted_class = 0;
	static float last_confidence = 0;

	//if(number_of_neurons != NL_WIDTH * NL_HEIGHT)
	//	return(0.0f);
	//printf("Number of Neurons = %d\nWidth*Height = %d\n",number_of_neurons,NL_WIDTH * NL_HEIGHT);

	for (class_id = 0; class_id < N_DEFINED_SIGNS; class_id++)
	{
		class_count[class_id] = 0.0;
	}

	for(y = 0 ; y < NL_HEIGHT ; y++)
	{
		for (x = 0; x < NL_WIDTH; x++)
		{
			class_count[network_output[x + y*NL_WIDTH]] += 1.0 * normal_dist((float)x,(float)(NL_WIDTH/2.0), VOTE_VARIANCE);
		}
	}

	for (class_id = 0; class_id < N_DEFINED_SIGNS; class_id++)
	{
		if (class_count[class_id] > max)
	    {
			max = class_count[class_id];
			most_voted_class = class_id;
	    }
		accumulated_weight += class_count[class_id];
	}

	confidence = (float) max / (float) (accumulated_weight);
	//confidence = (float) max / (float) (number_of_neurons);

	if (expected_sample_class != last_expected_sample_class)
	{
		last_expected_sample_class = expected_sample_class;
		last_most_voted_class = most_voted_class;
		last_confidence = confidence;
	}
	else
	{
		if (confidence >= last_confidence)
		{
			last_most_voted_class = most_voted_class;
			last_confidence = confidence;
		}
	}

	*network_output_class = last_most_voted_class;
	return confidence;

}

void
EvaluateNetworkOutputs(const VG_RAM_WNN *vg_ram_wnn, const DATA_SET *testing_set)
{
	int sample;
	int correct;
	int sample_class;
	int network_output_class;
	float largest_function_f_value;

	correct = 0;
	for (sample = 0; sample < testing_set->num_samples; sample ++)
	{
		sample_class = testing_set->sample_class[sample];

		largest_function_f_value = EvaluateNetworkWeightedOutput(
				&network_output_class,
				&(vg_ram_wnn->neuron_output[sample * vg_ram_wnn->number_of_neurons]),
				vg_ram_wnn->number_of_neurons, sample_class);

		if (network_output_class == sample_class)
		{
			printf("Sample = %d, NetworkOutputClass = %d, ExpectedOutput = %d, FunctionFValue %0.2f\n", sample, network_output_class, sample_class, largest_function_f_value);
			correct++;
		}
		else
			printf("Sample = %d, NetworkOutputClass = %d, ExpectedOutput = %d, FunctionFValue %0.2f *\n", sample, network_output_class, sample_class, largest_function_f_value);
	}
	printf("Percentage correct = %0.2f%%\n", 100.0 * (float) correct / (float) testing_set->num_samples);
}

