#ifndef _RBM_H
#define _RBM_H

#include <vector>

#ifdef __cplusplus

#define uint unsigned int

class Synapse
{
public:
	int to;
	int from;
	double weight;

	Synapse(int _from = -1, int _to = -1, double _weight = 0.0)
	{
		from = _from;
		to = _to;
		weight = _weight;
	}
};


class Neuron
{
public:
	int is_visible;
	double bias;
	double value;

	Neuron(bool _is_visible = false, double _bias = 0.0, double _value = 0.0)
	{
		bias = _bias;
		value = _value;
		is_visible = _is_visible;
	}
};


class BoltzmannParameters
{
public:
	BoltzmannParameters()
	{
		weight_step = 0.1;
		acceptable_error = 0.001;

		num_iterations_negative_phase = 1000;
		num_iterations_for_convergence = 200; /* this value is multiplied by the number of neurons */

		contrastive_divergence_steps = 1;

		initial_temp = 200.0;
		final_temp = 1.0;
		temp_factor = 0.99;
	}
public:
	double weight_step;
	double acceptable_error;

	double num_iterations_negative_phase;
	double num_iterations_for_convergence;

	double contrastive_divergence_steps;

	double initial_temp;
	double final_temp;
	double temp_factor;
};

class BoltzmannMachine
{
private:
	std::vector<Neuron> neurons;
	std::vector<Synapse> synapses;
	std::vector< std::vector<double> > input;

	double *positive_synapses_statistics;
	double *negative_synapses_statistics;
	double *positive_bias_statistics;
	double *negative_bias_statistics;

private:
	void	build_restricted_machine(int visible_units, int hidden_units);
	void	build_statistics();

	void	randomize_neurons();
	void	randomize_bias();
	void	randomize_synapses();

	double	evaluate_neuron_logit_function(int neuron_id, double T);
	void	run_hidden_net_negative_phase(double T);
	void	run_hidden_net_positive_phase(double T);
	void	reconstruction(double T);

	void	run_full_net(double T);

	void	init_statistics(double *synapses_statistics, double *bias_statistics);
	void	sumarize_statistics(double *synapses_statistics, double *bias_statistics, int total_statistics);
	void	update_synapses_statistics(double *statistics);
	void	update_neurons_statistics(double *statistics);

	void	set_visible_units(int input_sample);

	void	positive_phase();
	void	negative_phase();

	void	synapse_update();
	void	bias_update();

	int		convergence_test();
	int		check_neurons();
	
	double	get_input(int row, int col);

public:
	BoltzmannParameters params;

public:
	BoltzmannMachine(int visible_units, int hidden_units);
	~BoltzmannMachine();

	void	read_data(double *sample, int sample_size);
	void	clear_data();

	void	train_step();
	void	test_step();

	void	train_network();
	void	test_network(int num_steps = 20);

	void	print_neurons();
	void	print_biases();
	void	print_synapses();
	double 	get_neuron(int id);
};

#endif

#endif
