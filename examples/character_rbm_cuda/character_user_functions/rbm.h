#ifndef _RBM_H
#define _RBM_H

#include <vector>

#ifdef __cplusplus

#define uint unsigned int

class BoltzmannParameters
{
public:
	BoltzmannParameters()
	{
		weight_step = 0.1;
		acceptable_error = 0.001;

		contrastive_divergence_steps = 1;

		initial_temp = 100.0;
		final_temp = 1.0;
		temp_factor = 0.99;

		batch_size = 100;
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
	uint batch_size;
};

class BoltzmannMachine
{
	enum
	{
		SIGMOID = 1,
		LINEAR
	};
private:
	uint visible_neurons;
	uint hidden_neurons;
	int	 type;

	double *neuron_values;
	double *neuron_biases;
	double *synapse_weights;

	std::vector< std::vector<double> > input;

	double *positive_synapses_statistics;
	double *negative_synapses_statistics;
	double *positive_bias_statistics;
	double *negative_bias_statistics;

private:
	void	build_restricted_machine();
	void	build_statistics();

	void	randomize_neurons();
	void	randomize_bias();
	void	randomize_synapses();

	void	reconstruction(double T);
	void	update_hidden_units(double T);
	void 	contrastive_divergence(uint num_batch);

	void	run_full_net(double T);

	void	init_statistics(double *synapses_statistics, double *bias_statistics);
	void	sumarize_statistics(double *synapses_statistics, double *bias_statistics, int total_statistics);
	void	update_synapses_statistics(double *statistics);
	void	update_neurons_statistics(double *statistics);

	void	set_visible_units(int input_sample);

	void	synapse_update();
	void	bias_update();

	int	convergence_test();
	
	double	get_input(int row, int col);

public:
	BoltzmannParameters params;

public:
	BoltzmannMachine(int visible_units, int hidden_units, int type);
	~BoltzmannMachine();

	void	read_data(double *sample, int sample_size);
	void	clear_data();

	void	train_step(uint epoches);
	void	test_step();

	void	train_network();
	void	test_network(uint num_steps = 20);
	void	annealing_test();

	void	print_neurons();
	void	print_biases();
	void	print_synapses();
	double 	get_visible_neuron_value(uint id);
	void 	set_visible_neuron_value(uint id, double value);
	double 	get_hidden_neuron_value(uint id);
	uint	total_neurons();
	uint	total_synapses();
	void	reload(const char * file_name);
	void	unload(const char * file_name);
};

#endif

#endif
