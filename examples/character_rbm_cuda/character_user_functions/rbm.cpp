#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
using namespace std;

#include "rbm.h"
#include "math_functions.h"

void
BoltzmannMachine::build_statistics()
{
	positive_synapses_statistics = (double *) calloc (total_synapses(), sizeof(double));
	negative_synapses_statistics = (double *) calloc (total_synapses(), sizeof(double));
	positive_bias_statistics = (double *) calloc (total_neurons(), sizeof(double));
	negative_bias_statistics = (double *) calloc (total_neurons(), sizeof(double));
}


void
BoltzmannMachine::build_restricted_machine()
{
	neuron_values = (double *) calloc (total_neurons(), sizeof(double));
	neuron_biases = (double *) calloc (total_neurons(), sizeof(double));
	synapse_weights = (double *) calloc (total_synapses(), sizeof(double));
}


BoltzmannMachine::BoltzmannMachine(int _visible_neurons, int _hidden_neurons, int _type)
: visible_neurons(_visible_neurons), hidden_neurons(_hidden_neurons), type(_type)
{
	math_functions_init();

	build_restricted_machine();

	build_statistics();

	randomize_bias();
	randomize_neurons();
	randomize_synapses();
}


BoltzmannMachine::~BoltzmannMachine()
{
	free(positive_synapses_statistics);
	free(negative_synapses_statistics);
	free(positive_bias_statistics);
	free(negative_bias_statistics);

	math_functions_destroy();
}


double
BoltzmannMachine::get_input(int row, int col)
{
	return (input[row][col]);
}


uint
BoltzmannMachine::total_neurons()
{
	return (visible_neurons + hidden_neurons);
}


uint
BoltzmannMachine::total_synapses()
{
	return (visible_neurons * hidden_neurons);
}


double
BoltzmannMachine::get_visible_neuron_value(uint id)
{
	if (id < visible_neurons) 
	{
		return (neuron_values[id]);
	}
	else
	{
		printf("Erro: indice invalido (%d) em BoltzmannMachine::get_visible_neuron_value()!\n", id);
		exit(1);
	}
}


void
BoltzmannMachine::set_visible_neuron_value(uint id, double value)
{
	if (id < visible_neurons) 
	{
		neuron_values[id] = value;
	}
	else
	{
		printf("Erro: indice invalido (%d) em BoltzmannMachine::set_visible_neuron_value()!\n", id);
		exit(1);
	}
}


double
BoltzmannMachine::get_hidden_neuron_value(uint id)
{
	if (id < hidden_neurons) 
	{
		return (neuron_values[id + visible_neurons]);
	}
	else
	{
		printf("Erro: indice invalido (%d) em BoltzmannMachine::get_hidden_neuron_value()!\n", id);
		exit(1);
	}
}


void
BoltzmannMachine::read_data(double *sample, int sample_size)
{
	std::vector<double> data_sample(sample_size);
	
	for (int i = 0; i < sample_size; i++)
		data_sample[i] = sample[i];
		
	input.push_back(data_sample);
}


void
BoltzmannMachine::clear_data()
{
	input.clear();
}


void
BoltzmannMachine::print_neurons()
{
	uint i;
	
	for (i = 0; i < total_neurons(); i++)
		printf("%0.2lf, ", neuron_values[i]);
		
	printf("\n");
}


void
BoltzmannMachine::print_biases()
{
	uint i;
	
	for (i = 0; i < total_neurons(); i++)
		printf("%0.2lf, ", neuron_biases[i]);
		
	printf("\n");
}


void
BoltzmannMachine::print_synapses()
{
	uint i;

	for (i = 0; i < total_synapses(); i++)
		printf("%0.2lf, ", synapse_weights[i]);
		
	printf("\n");
}


void
BoltzmannMachine::randomize_neurons()
{
	uint i;

	for (i = 0; i < total_neurons(); i++)
		neuron_values[i]= (double) (rand() % 2);
}


void
BoltzmannMachine::randomize_bias()
{
	uint i;

	for (i = 0; i < total_neurons(); i++)
		neuron_biases[i] = ((double) ((double) rand() / (double) RAND_MAX) * 2.0 - 1.0);
}


void
BoltzmannMachine::randomize_synapses()
{
	uint i;

	for (i = 0; i < total_synapses(); i++)
		synapse_weights[i] = ((double) ((double) rand() / (double) RAND_MAX) * 2.0 - 1.0);
}


double
logit_function(double t, double T)
{
	return (1.0 / (1.0 + exp(-t / T)));
}


double
logit_sample(double logit)
{
	double sample;

	sample = (double) rand() / (double) RAND_MAX;

	if (sample > logit)
		return 0.0;
	else
		return 1.0;
}


void
BoltzmannMachine::update_hidden_units(double T)
{
	uint i;
	uint total_neurons_, visible_neurons_;
	double *neuron_values_, *neuron_biases_;
	double hidden_logit[hidden_neurons];

	multiply_A_by_B(hidden_logit, neuron_values, synapse_weights, 1, visible_neurons, hidden_neurons);

	total_neurons_ = total_neurons();
	visible_neurons_ = visible_neurons;
	neuron_values_ = neuron_values;
	neuron_biases_ = neuron_biases;

	if (type == SIGMOID)
	{
		#ifdef _OPENMP
		#pragma omp parallel for default(none) private(i) shared(hidden_logit,neuron_values_,neuron_biases_,visible_neurons_,total_neurons_,T)
		#endif
		for (i = visible_neurons; i < total_neurons_; i++)
		{
			hidden_logit[i - visible_neurons_] = logit_function(hidden_logit[i - visible_neurons_] + neuron_biases_[i], T);
			// hidden(i) = logit_function( logit(i) )
			neuron_values_[i] = logit_sample(hidden_logit[i - visible_neurons_]);
		}
	} else if (type == LINEAR)
	{
		#ifdef _OPENMP
		#pragma omp parallel for default(none) private(i) shared(hidden_logit,neuron_values_,neuron_biases_,visible_neurons_,total_neurons_,T)
		#endif
		for (i = visible_neurons; i < total_neurons_; i++)
		{
			hidden_logit[i - visible_neurons_] = hidden_logit[i - visible_neurons_] + neuron_biases_[i];
			// hidden(i) = logit_function( logit(i) )
			neuron_values_[i] = logit_sample(hidden_logit[i - visible_neurons_]);
		}
	}
}


void
BoltzmannMachine::reconstruction(double T)
{
	uint i;
	uint visible_neurons_;
	double *neuron_values_, *neuron_biases_;
	double visible_logit[visible_neurons];

	multiply_A_by_B_transpose(visible_logit, &neuron_values[visible_neurons], synapse_weights, 1, hidden_neurons, visible_neurons);

	visible_neurons_ = visible_neurons;
	neuron_values_ = neuron_values;
	neuron_biases_ = neuron_biases;

	#ifdef _OPENMP
	#pragma omp parallel for default(none) private(i) shared(visible_logit,neuron_values_,neuron_biases_,visible_neurons_,T)
	#endif
	for (i = 0; i < visible_neurons_; i++)
	{
		visible_logit[i] = logit_function(visible_logit[i] + neuron_biases_[i], T);
		// visible(i) = logit_function( logit(i) )
		neuron_values_[i] = /*logit_sample(*/visible_logit[i];//);
	}
}


void
BoltzmannMachine::update_synapses_statistics(double *statistics)
{
	uint i, j;

	for (i = 0; i < visible_neurons; i++)
	{
		for (j = 0; j < hidden_neurons; j++)
		{	// stat_synapse(i, j) = visible(i) * hidden(j)
			statistics[i * hidden_neurons + j] += neuron_values[i] * neuron_values[visible_neurons + j];
		}
	}
}


void
BoltzmannMachine::update_neurons_statistics(double *statistics)
{
	uint i;
	
	for (i = 0; i < total_neurons(); i++)
		statistics[i] += neuron_values[i];
}


void
BoltzmannMachine::init_statistics(double *synapses_statistics, double *bias_statistics)
{
	uint i;

	for (i = 0; i < total_synapses(); i++)
		synapses_statistics[i] = 0.0;
	for (i = 0; i < total_neurons(); i++)
		bias_statistics[i] = 0.0;
}


void
BoltzmannMachine::sumarize_statistics(double *synapses_statistics, double *bias_statistics, int total_statistics)
{
	uint i;

	for (i = 0; i < total_synapses(); i++)
		synapses_statistics[i] /= ((double) total_statistics);

	for (i = 0; i < total_neurons(); i++)
		bias_statistics[i] /= ((double) total_statistics);
}


void
BoltzmannMachine::set_visible_units(int input_sample)
{
	uint i;

	for (i = 0; i < visible_neurons; i++)
		neuron_values[i] = get_input(input_sample, i);
}


void
BoltzmannMachine::contrastive_divergence(uint num_batch)
{
	uint input_sample;
	uint CD;

	for (input_sample = (num_batch * params.batch_size); input_sample < ((num_batch + 1) * params.batch_size); input_sample++)
	{
		set_visible_units(input_sample);

		CD = 0;
		do
		{
			update_hidden_units(1.0); // Generate internal representation of the input data in the hidden units

			if (CD == 0)
			{
				update_synapses_statistics(positive_synapses_statistics);
				update_neurons_statistics(positive_bias_statistics);
			}

			reconstruction(1.0);	  // Generate a reconstruction of the input data

			CD++;
		} while (CD < params.contrastive_divergence_steps);

		update_hidden_units(1.0); // Generate internal representation of the input data in the hidden units

		update_synapses_statistics(negative_synapses_statistics);
		update_neurons_statistics(negative_bias_statistics);
	}
}


void
BoltzmannMachine::synapse_update()
{
	uint i;
	
	for (i = 0; i < total_synapses(); i++)
	{
		if (fabs(positive_synapses_statistics[i] - negative_synapses_statistics[i]) != 0.0)
			synapse_weights[i] += params.weight_step * ((positive_synapses_statistics[i] - negative_synapses_statistics[i])) / fabs((positive_synapses_statistics[i] - negative_synapses_statistics[i]));
			// synapse_weights[i] += params.weight_step * (positive_synapses_statistics[i] - negative_synapses_statistics[i]);
	}

}


void
BoltzmannMachine::bias_update()
{
	uint i;
	
	for (i = 0; i < total_neurons(); i++)
	{
		if (fabs(positive_bias_statistics[i] - negative_bias_statistics[i]) != 0.0)
			neuron_biases[i] += params.weight_step * ((positive_bias_statistics[i] - negative_bias_statistics[i])) / fabs((positive_bias_statistics[i] - negative_bias_statistics[i]));
			// neuron_biases[i] += params.weight_step * (positive_bias_statistics[i] - negative_bias_statistics[i]);
	}
}


int
BoltzmannMachine::convergence_test()
{
	uint i;
	double error = 0.0;

	for (i = 0; i < total_synapses(); i++)
		error += (positive_synapses_statistics[i] - negative_synapses_statistics[i]) * (positive_synapses_statistics[i] - negative_synapses_statistics[i]);

	for (i = 0; i < total_neurons(); i++)
		error += (positive_bias_statistics[i] - negative_bias_statistics[i]) * (positive_bias_statistics[i] - negative_bias_statistics[i]);

	error = sqrt(error);
	fprintf(stderr, "%lf\n", error);

	if (error < params.acceptable_error)
		return (1);
	else
		return (0);
}


void
BoltzmannMachine::train_step(uint epoches)
{
	for (uint i = 0; i < epoches; i++)
	{
		printf("about to train epoch %d from %d\n", i+1, epoches);
		for (uint j = 0; j < (input.size()/params.batch_size); j++)
		{
			init_statistics(positive_synapses_statistics, positive_bias_statistics);
			init_statistics(negative_synapses_statistics, negative_bias_statistics);

			contrastive_divergence(j);

			sumarize_statistics(positive_synapses_statistics, positive_bias_statistics, params.batch_size);
			sumarize_statistics(negative_synapses_statistics, negative_bias_statistics, params.batch_size);

			synapse_update();
			bias_update();
		}
	}
}


void
BoltzmannMachine::train_network()
{
	int converged = 0;
	int num_iteractions = 0;
	int max_iteractions = (params.num_iterations_for_convergence * total_neurons());
	
	while (!converged && (num_iteractions < max_iteractions))
	{
		train_step(10);
		converged = convergence_test();
		num_iteractions++;
	}

	printf("converged = %d\n\n", converged);
}


void
BoltzmannMachine::run_full_net(double T)
{
	update_hidden_units(T); // Generate internal representation of the input data in the hidden units
	reconstruction(T);	  // Generate a reconstruction of the input data
}


void
BoltzmannMachine::annealing_test()
{
	double T;

	randomize_neurons();

	for (T = params.initial_temp; T >= params.final_temp; T = T * params.temp_factor)
		run_full_net(T);

	run_full_net(params.final_temp);
}


void
BoltzmannMachine::test_network(uint num_steps)
{
	uint i;

	for (i = 0; i < num_steps; i++)
		run_full_net(1.0);
}

void
BoltzmannMachine::unload(const char * file_name)
{
	FILE * file_dump;

	if((file_dump = fopen(file_name, "w")) == NULL)
	{
   	 	fprintf(stderr,"Error creating file.\n");
		exit (0);
	}

	for(uint i = 0; i < total_neurons(); i++)
	{
		if (fprintf(file_dump, "%.20lf;%.20lf;", neuron_values[i], neuron_biases[i]) < 0)
		{
	        fprintf(stderr,"Error writing file.\n");
			exit (0);
		}
	}

	fprintf(file_dump, "\n");

	for(uint i = 0; i < total_synapses(); i++)
	{
		if (fprintf(file_dump, "%.20lf;", synapse_weights[i]) < 0)
		{
	        fprintf(stderr,"Error writing file.\n");
			exit (0);
		}
	}

	fclose(file_dump);
}


void
BoltzmannMachine::reload(const char * file_name)
{
	FILE * file_dump;

	if((file_dump = fopen(file_name, "r")) == NULL)
	{
   	 	fprintf(stderr,"Error reading file.\n");
		exit (0);
	}

	for(uint i = 0; i < total_neurons(); i++)
	{
		if (fscanf(file_dump, "%lf;%lf;", &(neuron_values[i]), &(neuron_biases[i])) < 0)
		{
			fprintf(stderr,"Error parsing file.\n");
			exit (0);
		}
	}

	if (fscanf(file_dump, "\n") < 0)
	{
		fprintf(stderr,"Error parsing file.\n");
		exit (0);
	}

	for(uint i = 0; i < total_synapses(); i++)
	{
		if (fscanf(file_dump, "%lf;", &(synapse_weights[i])) < 0)
		{
	        fprintf(stderr,"Error parsing file.\n");
			exit (0);
		}
	}
	fclose(file_dump);
}
