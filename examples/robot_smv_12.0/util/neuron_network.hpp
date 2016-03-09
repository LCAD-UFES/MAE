/*
This file is part of MAE.

History:

[2006.02.13 - Helio Perroni Filho] Created.
*/

#ifndef __NEURON_NETWORK_HPP
#define __NEURON_NETWORK_HPP

#include <map>
#include <string>

#include "input_layer.hpp"
#include "neuron_layer.hpp"

using std::map;
using std::string;

using mae::NeuronLayer;

/** Map of named input layers. */
typedef map<string, InputLayer> InputLayerMap;

/** Map of named neuron layers. */
typedef map<string, NeuronLayer> NeuronLayerMap;

/**
The neuron network as defined in the application's CON file.
*/
class NeuronNetwork
{
	/*
	Attribute Section
	*/

	/** Map of named input layers. */
	private: InputLayerMap inputLayers;

	/** Map of named neuron layers. */
	private: NeuronLayerMap neuronLayers;

	/*
	Constructor Section
	*/

	/**
	Default constructor.
	*/
	private: NeuronNetwork();

	/**
	Returns this class' unique instance.

	@return This class' unique instance.
	*/
	public: static NeuronNetwork& getInstance();

	/*
	Method Section
	*/

	/**
	Starts up the underlying MAE framework.
	*/
	public: void start();

	/**
	Updates the state of all output layers.
	*/
	public: void updateAllOutputs();

	/*
	Input / Output Section
	*/

	/**
	Loads the contents of an input or neuron layer from a file.

	@param name Name of the layer.

	@param path Path to the contents file.
	*/
	public: void load(string name, string path);

	/**
	Saves the contents of an input or neuron layer to a file.

	@param name Name of the layer.

	@param path Path to the contents file.
	*/
	public: void save(string name, string path);

	/*
	Property Section
	*/

	/**
	Returns a reference for a named input layer.

	@param name The input layer's name.

	@return A reference for a named input layer.

	@throw runtime_exception If there is no input layer by the given name.
	*/
	public: InputLayer& getInputLayer(string name);

	/**
	Returns a reference for a named neuron layer.

	@param name The neuron layer's name.

	@return A reference for a named neuron layer.

	@throw runtime_exception If there is no neuron layer by the given name.
	*/
	public: NeuronLayer& getNeuronLayer(string name);
};

#endif
