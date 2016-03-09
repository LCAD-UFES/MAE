/*
This file is part of MAE.

History:

[2006.02.13 - Helio Perroni Filho] Created.
*/

#include "neuron_network.hpp"

#include "mae_neuron_network.h"

/*
Constructor Section
*/

NeuronNetwork::NeuronNetwork()
{
}

NeuronNetwork& NeuronNetwork::getInstance()
{
	static NeuronNetwork network;

	return network;
}
	
/*
Method Section
*/

void NeuronNetwork::start()
{
	mae_neuron_network_start();
}

void NeuronNetwork::updateAllOutputs()
{
	mae_neuron_network_update_all_outputs();
}

/*
Input / Output Section
*/

void NeuronNetwork::load(string name, string path)
{
	mae_neuron_network_load(name.c_str(), path.c_str());
}

void NeuronNetwork::save(string name, string path)
{
	mae_neuron_network_save(name.c_str(), path.c_str());
}
	
/*
Property Section
*/

InputLayer& NeuronNetwork::getInputLayer(string name)
{
	if (inputLayers.count(name) <= 0)
	{
		InputLayer input(name);
		inputLayers[name] = input;
	}

	return inputLayers[name]; 
}

NeuronLayer& NeuronNetwork::getNeuronLayer(string name)
{
	if (neuronLayers.count(name) <= 0)
	{
		NeuronLayer neuronLayer(name);
		neuronLayers[name] = neuronLayer;
	}

	return neuronLayers[name]; 
}
