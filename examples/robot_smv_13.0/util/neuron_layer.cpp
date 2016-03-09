/*
This file is part of MAE.

History:

[2006.02.14 - Helio Perroni Filho] Created.
*/

#include "neuron_layer.hpp"

#include <cstdlib>
#include <sstream>
#include <stdexcept>

using std::ostringstream;
using std::out_of_range;
using std::runtime_error;

using mae::NeuronLayer;

/*
Constructor Section
*/

NeuronLayer::NeuronLayer()
{
	layer = NULL;
}

NeuronLayer::NeuronLayer(string name)
{
	layer = mae_neuron_layer_get(name.c_str());
	if (layer == NULL)
		throw runtime_error("Neuron layer \"" + name + "\" not known");
}

/*
Method Section
*/

int NeuronLayer::size()
{
	return mae_neuron_layer_get_width(layer) * mae_neuron_layer_get_height(layer);
}

double NeuronLayer::doubleOutput(int index)
{
	return floatOutput(index);
}

float NeuronLayer::floatOutput(int index)
{
	if (0 <= index && index < size())
		return mae_neuron_layer_output_fval(layer, index);
	else
	{
		ostringstream o1, o2;
		o1 << index;
		o2 << size();
		throw out_of_range("Index " + o1.str() + " not in valid range [0.." + o2.str() +"]");
	}
}

int NeuronLayer::intOutput(int index)
{
	if (0 <= index && index < size())
		return mae_neuron_layer_output_ival(layer, index);
	else
	{
		ostringstream o1, o2;
		o1 << index;
		o2 << size();
		throw out_of_range("Index " + o1.str() + " not in valid range [0.." + o2.str() +"]");
	}
}

/*
Input / Output Section
*/

void NeuronLayer::load(string path)
{
	mae_neuron_layer_load(layer, path.c_str());
}

void NeuronLayer::save(string path)
{
	mae_neuron_layer_save(layer, path.c_str());
}

/*
Property Section
*/

int NeuronLayer::getWidth()
{
	return mae_neuron_layer_get_width(layer);
}

int NeuronLayer::getHeight()
{
	return mae_neuron_layer_get_height(layer);
}

void NeuronLayer::setOutput(int index, int value)
{
	if (0 <= index && index < size())
		return mae_neuron_layer_set_output_ival(layer, index, value);
	else
	{
		ostringstream o1, o2;
		o1 << index;
		o2 << size();
		throw out_of_range("Index " + o1.str() + " not in valid range [0.." + o2.str() +"]");
	}
}
	
void NeuronLayer::setOutput(int index, float value)
{
	if (0 <= index && index < size())
		return mae_neuron_layer_set_output_fval(layer, index, value);
	else
	{
		ostringstream o1, o2;
		o1 << index;
		o2 << size();
		throw out_of_range("Index " + o1.str() + " not in valid range [0.." + o2.str() +"]");
	}
}
