/*
This file is part of MAE.

History:

[2006.02.14 - Helio Perroni Filho] Created.
*/

#ifndef __NEURON_LAYER_HPP
#define __NEURON_LAYER_HPP

#include <string>

#include "mae_neuron_layer.h"

using std::string;

namespace mae
{
	class NeuronLayer;
};

/**
C++ wrapper to a MAE neuron layer.
*/
class mae::NeuronLayer
{
	/*
	Attribute Section
	*/

	/** Pointer to the underlying MAE neuron layer.  */
	private: mae_neuron_layer layer;

	/*
	Constructor Section
	*/

	/**
	Default constructor.
	*/
	public: NeuronLayer();

	/**
	Creates a wrapper around a named neuron layer.

	@param name Name of the underlying MAE neuron layer.

	@throw runtime_exception If there is no neuron layer by that name.
	*/
	public: NeuronLayer(string name);

	/*
	Method Section
	*/

	/**
	Returns the number of neurons within the underlying layer.

	@return Number of neurons within the neuron layer.
	*/
	public: int size();

	/**
	Returns the value of an indexed cell as a <code>double</code>.

	@return The value of an indexed cell as a <code>double</code>.

	@throw out_of_range If the index is not within the valid range for this layer.
	*/
	public: double doubleOutput(int index);

	/**
	Returns the value of an indexed cell as a <code>float</code>.

	@return The value of an indexed cell as a <code>float</code>.

	@throw out_of_range If the index is not within the valid range for this layer.
	*/
	public: float floatOutput(int index);

	/**
	Returns the value of an indexed cell as an <code>int</code>.

	@return The value of an indexed cell as an <code>int</code>.

	@throw out_of_range If the index is not within the valid range for this layer.
	*/
	public: int intOutput(int index);

	/*
	Input / Output Section
	*/

	/**
	Loads the contents of this neuron layer from a file.

	@param path Path to the file to load.
	*/
	public: void load(string path);

	/**
	Saves the contents of this neuron layer to a file. The file is overwritten if it already exists.

	@param path Path to the file to save.
	*/
	public: void save(string path);

	/*
	Property Section
	*/

	/**
	Returns the layer's height.

	@return The layer's height.
	*/
	public: int getHeight();

	/**
	Returns the layer's width.

	@return The layer's width.
	*/
	public: int getWidth();
	
	/**
	Sets an indexed cell as an <code>int</code> value.
	
	@param index The cell's index.
	
	@param value The cell's value.
	
	@throw out_of_range If the index is not valid.
	*/
	public: void setOutput(int index, int value);
	
	/**
	Sets an indexed cell as a <code>float</code> value.
	
	@param index The cell's index.
	
	@param value The cell's value.
	
	@throw out_of_range If the index is not valid.
	*/
	public: void setOutput(int index, float value);
};

#endif
