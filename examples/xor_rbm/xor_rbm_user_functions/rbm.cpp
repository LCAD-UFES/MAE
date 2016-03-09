#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "rbm.h"

using namespace std;


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
permute(int *vector, int size)
{
	int i, j, aux;

	for (i = size - 1; i >= 1; i--)
	{
		j = rand() % (i + 1);
		aux = vector[i];
		vector[i] = vector[j];
		vector[j] = aux;
	}
}


BoltzmannMachine::BoltzmannMachine(int visible_units, int hidden_units)
{
	build_restricted_machine(visible_units, hidden_units);

	build_statistics();

	randomize_neurons();
	randomize_bias();
	randomize_synapses();
}


BoltzmannMachine::~BoltzmannMachine()
{
	free(positive_synapses_statistics);
	free(negative_synapses_statistics);
	free(positive_bias_statistics);
	free(negative_bias_statistics);
}


double
BoltzmannMachine::get_input(int row, int col)
{
	return (input[row][col]);
}


double
BoltzmannMachine::get_neuron(int id)
{
	if ( (id >= 0) && ((uint)id < neurons.size()) )
		return (neurons[id].value);
	else
		exit(printf("Erro: indice invalido!\n"));
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
	
	for (i = 0; i < neurons.size(); i++)
		printf("%0.2lf, ", neurons[i].value);
		
	printf("\n");
}


int
BoltzmannMachine::check_neurons()
{
	uint i;
	double diff = 0.0;

	//TODO: should be better to compare neurons' output with input samples
	for (i = 0; i < neurons.size(); i++)
		if (neurons[i].is_visible)
			diff -= neurons[i].value;

	return (fabs(diff) == 1.0 ? 1 : 0);
}


void
BoltzmannMachine::print_biases()
{
	uint i;
	
	for (i = 0; i < neurons.size(); i++)
		printf("%0.2lf, ", neurons[i].bias);
		
	printf("\n");
}


void
BoltzmannMachine::print_synapses()
{
	uint i;
	
	for (i = 0; i < synapses.size(); i++)
		printf("%0.2lf, ", synapses[i].weight);
		
	printf("\n");
}


void
BoltzmannMachine::randomize_neurons()
{
	uint i;
	
	for (i = 0; i < neurons.size(); i++)
		neurons[i].value = (double) (rand() % 2);
}


void
BoltzmannMachine::randomize_bias()
{
	uint i;
	
	for (i = 0; i < neurons.size(); i++)
		// neurons[i].bias = ((double) ((double) rand() / (double) RAND_MAX) * 2.0 - 1.0);
		neurons[i].bias = 0.0;
}


void
BoltzmannMachine::randomize_synapses()
{
	uint i;

	for (i = 0; i < synapses.size(); i++)
		// synapsis[i].weight = ((double) ((double) rand() / (double) RAND_MAX) * 2.0 - 1.0);
		synapses[i].weight = 0.0;
}


double
BoltzmannMachine::evaluate_neuron_logit_function(int neuron_id, double T)
{
	uint i;
	double value = 0;

	for (i = 0; i < synapses.size(); i++)
	{
		if (synapses[i].from == neuron_id)
			value += (synapses[i].weight * neurons[synapses[i].to].value);
		else if (synapses[i].to == neuron_id)
			value += (synapses[i].weight * neurons[synapses[i].from].value);
	}

	value += neurons[neuron_id].bias;
	return (logit_function(value, T));
}


void
BoltzmannMachine::run_hidden_net_negative_phase(double T)
{
	uint i;
	double logit;

	for (i = 0; i < neurons.size(); i++)
	{
		if (!neurons[i].is_visible)
		{
			logit = evaluate_neuron_logit_function(i, T);
			neurons[i].value = logit_sample(logit);
		}
	}
}


void
BoltzmannMachine::reconstruction(double T)
{
	uint i;
	double logit;

	for (i = 0; i < neurons.size(); i++)
	{
		if (neurons[i].is_visible)
		{
			logit = evaluate_neuron_logit_function(i, T);
			neurons[i].value = logit_sample(logit);
		}
	}
}


void
BoltzmannMachine::run_hidden_net_positive_phase(double T)
{
	uint i;

	for (i = 0; i < neurons.size(); i++)
	{
		if (!neurons[i].is_visible)
			neurons[i].value = evaluate_neuron_logit_function(i, T);
	}
}


void
BoltzmannMachine::run_full_net(double T)
{
	uint i;
	int selected;
	double logit;
	int neuron_sequence[neurons.size()];

	for (i = 0; i < neurons.size(); i++)
		neuron_sequence[i] = i;

	permute(neuron_sequence, neurons.size());
	
	for (i = 0; i < neurons.size(); i++)
	{
		selected = neuron_sequence[i];
		logit = evaluate_neuron_logit_function(selected, T);
		neurons[selected].value = logit_sample(logit);
	}
}


void
BoltzmannMachine::update_synapses_statistics(double *statistics)
{
	uint i;
	int to;
	int from;
	
	for (i = 0; i < synapses.size(); i++)
	{
		to = synapses[i].to;
		from = synapses[i].from;
		statistics[i] += (neurons[from].value * neurons[to].value);
	}
}


void
BoltzmannMachine::update_neurons_statistics(double *statistics)
{
	uint i;
	
	for (i = 0; i < neurons.size(); i++)
		statistics[i] += neurons[i].value;
}


void
BoltzmannMachine::init_statistics(double *synapses_statistics, double *bias_statistics)
{
	uint i;

	for (i = 0; i < synapses.size(); i++)
		synapses_statistics[i] = 0.0;
	for (i = 0; i < neurons.size(); i++)
		bias_statistics[i] = 0.0;
}


void
BoltzmannMachine::sumarize_statistics(double *synapses_statistics, double *bias_statistics, int total_statistics)
{
	uint i;

	for (i = 0; i < synapses.size(); i++)
		synapses_statistics[i] /= ((double) total_statistics);

	for (i = 0; i < neurons.size(); i++)
		bias_statistics[i] /= ((double) total_statistics);
}


void
BoltzmannMachine::set_visible_units(int input_sample)
{
	uint i, j;

	for (i = j = 0; i < neurons.size(); i++)
	{
		if (neurons[i].is_visible)
		{
			// OBS: assumes that data dimension is equal to the number of visible neurons
			// OBS: add the data in the same order that the visible neurons show up
			neurons[i].value = get_input(input_sample, j);
			j++;
		}
	}
}


void
BoltzmannMachine::positive_phase()
{
	uint total_statistics = 0;
	uint input_sample;
	
	init_statistics(positive_synapses_statistics, positive_bias_statistics);

	for (input_sample = 0; input_sample < input.size(); input_sample++)
	{
		set_visible_units(input_sample);

		run_hidden_net_positive_phase(1.0);
		update_synapses_statistics(positive_synapses_statistics);
		update_neurons_statistics(positive_bias_statistics);
		total_statistics++;
	}

	sumarize_statistics(positive_synapses_statistics, positive_bias_statistics, total_statistics);
}


void
BoltzmannMachine::negative_phase()
{
	uint i, j, total_statistics = 0;
	uint input_sample;
	
	init_statistics(negative_synapses_statistics, negative_bias_statistics);
	
	for (i = 0; i < params.num_iterations_negative_phase; i++)
	{
		for (input_sample = 0; input_sample < input.size(); input_sample++)
		{
			set_visible_units(input_sample);

			for (j = 0; j < params.contrastive_divergence_steps; j++)
			{
				run_hidden_net_negative_phase(1.0); // Generate internal representation of the input data in the hidden units
				reconstruction(1.0);
				run_hidden_net_negative_phase(1.0);
			}

			update_synapses_statistics(negative_synapses_statistics);
			update_neurons_statistics(negative_bias_statistics);
			total_statistics++;
		}
	}

	sumarize_statistics(negative_synapses_statistics, negative_bias_statistics, total_statistics);
}


void
BoltzmannMachine::synapse_update()
{
	uint i;
	
	for (i = 0; i < synapses.size(); i++)
	{
		if (fabs(positive_synapses_statistics[i] - negative_synapses_statistics[i]) != 0.0)
			synapses[i].weight = synapses[i].weight + params.weight_step * ((positive_synapses_statistics[i] - negative_synapses_statistics[i])) / fabs((positive_synapses_statistics[i] - negative_synapses_statistics[i]));

		//fprintf(stderr, "%lf ", synapses[i].weight);
	}

}


void
BoltzmannMachine::bias_update()
{
	uint i;
	
	for (i = 0; i < neurons.size(); i++)
	{
		if (fabs(positive_bias_statistics[i] - negative_bias_statistics[i]) != 0.0)
			neurons[i].bias = neurons[i].bias + params.weight_step * ((positive_bias_statistics[i] - negative_bias_statistics[i])) / fabs((positive_bias_statistics[i] - negative_bias_statistics[i]));

		//fprintf(stderr, "%lf ", neurons[i].bias);
	}

	//fprintf(stderr, "\n");
}


int
BoltzmannMachine::convergence_test()
{
	uint i;
	double error = 0.0;

	for (i = 0; i < synapses.size(); i++)
		error += (positive_synapses_statistics[i] - negative_synapses_statistics[i]) * (positive_synapses_statistics[i] - negative_synapses_statistics[i]);

	for (i = 0; i < neurons.size(); i++)
		error += (positive_bias_statistics[i] - negative_bias_statistics[i]) * (positive_bias_statistics[i] - negative_bias_statistics[i]);

	error = sqrt(error);
	fprintf(stderr, "%lf\n", error);

	if (error < params.acceptable_error)
		return (1);
	else
		return (0);
}


void
BoltzmannMachine::build_statistics()
{
	positive_synapses_statistics = (double *) calloc (synapses.size(), sizeof(double));
	negative_synapses_statistics = (double *) calloc (synapses.size(), sizeof(double));
	positive_bias_statistics = (double *) calloc (neurons.size(), sizeof(double));
	negative_bias_statistics = (double *) calloc (neurons.size(), sizeof(double));
}


void
BoltzmannMachine::build_restricted_machine(int visible_units, int hidden_units)
{
	for(int i=0; i < visible_units; i++)
		neurons.push_back(Neuron(true));	// is_visible = true
	for(int i=0; i < hidden_units; i++)
		neurons.push_back(Neuron(false));	// is_visible = false

	for(int i=0; i < (visible_units+hidden_units); i++)
		if (neurons[i].is_visible)
			for(int j=0; j < (visible_units+hidden_units); j++)
				if (!neurons[j].is_visible)
					synapses.push_back(Synapse(i, j)); // synapse between visible neuron i and hidden neuron j

}


void
BoltzmannMachine::train_step()
{
	positive_phase();
	negative_phase();
	synapse_update();
	bias_update();
}


void
BoltzmannMachine::train_network()
{
	int converged = 0;
	int num_iteractions = 0;
	int max_iteractions = (params.num_iterations_for_convergence * neurons.size());
	
	while (!converged && (num_iteractions < max_iteractions))
	{
		train_step();
		converged = convergence_test();
		num_iteractions++;
	}

	printf("converged = %d\n\n", converged);
}


void
BoltzmannMachine::test_step()
{
	double T;

	randomize_neurons();

	for (T = params.initial_temp; T >= params.final_temp; T = T * params.temp_factor)
		run_full_net(T);

	run_full_net(params.final_temp);
}


void
BoltzmannMachine::test_network(int num_steps)
{
	int i, ok = 1;

	for (i = 0; i < num_steps; i++)
	{
		test_step();
		print_neurons();

		if (ok)
			ok = check_neurons();
	}

	if (ok)
		printf("SUCCESS\n");
	else
		printf("FAIL\n");

}

