/*
This file is part of MAE.

History:

[2006.02.13 - Helio Perroni Filho] Created.
*/

#include "neuron_network.hpp"

#include "garbage_collector.hpp"
using mae::manage;

#include "logging.hpp"
using mae::logging::trace;

#include "mae_neuron_network.h"

/*
Constructor Section
*/

NeuronNetwork::NeuronNetwork()
{
	trace("NeuronNetwork::NeuronNetwork");
	manage<NeuronNetwork>(this);
}

NeuronNetwork::~NeuronNetwork()
{
	trace("NeuronNetwork::~NeuronNetwork");
}

NeuronNetwork& NeuronNetwork::getInstance()
{
	static NeuronNetwork* singleton = new NeuronNetwork();

	return *singleton;
}
	
/*
Method Section
*/

void NeuronNetwork::start()
{
	trace("NeuronNetwork::start");
	mae_neuron_network_start();
}

string NeuronNetwork::toString()
{
	return string() + "mae::NeuronNetwork [" + ((int) this) + "]";
}

void NeuronNetwork::updateAllOutputs()
{
	trace("NeuronNetwork::updateAllOutputs");
	mae_neuron_network_update_all_outputs();
}

/*
Input / Output Section
*/

void NeuronNetwork::load(string name, string path)
{
	trace("NeuronNetwork::load", "\"" + name + "\", \"" + path + "\"");
	mae_neuron_network_load(name.c_str(), path.c_str());
}

void NeuronNetwork::save(string name, string path)
{
	trace("NeuronNetwork::save", "\"" + name + "\", \"" + path + "\"");
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
