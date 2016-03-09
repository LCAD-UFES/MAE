/*
This file is part of MAE.

History:

[2006.02.13 - Helio Perroni Filho] Created.
*/

#ifndef __INPUT_LAYER_HPP
#define __INPUT_LAYER_HPP

#include <string>

#include "cinput_layer.h"
#include "texture_frame_buffer.hpp"

using std::string;

/**
C++ wrapper to a MAE input layer.
*/
class InputLayer
{
	/*
	Attribute Section
	*/

	/** Pointer to the underlying MAE input layer. */
	private: mae_input_layer layer;

	/** Wrapper to the input layer's frame buffer. */
	private: TextureFrameBuffer buffer;

	/*
	Constructor Section
	*/

	/**
	Default Constructor.
	*/
	public: InputLayer();

	/**
	Creates a wrapper around a named input layer.

	@param name Name of the underlying MAE input layer.

	@throw runtime_exception If there is no input layer by that name.
	*/
	public: InputLayer(string name);
	
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
	
	/**
	Updates the state of the input layer.
	*/
	public: void update();

	/*
	Input / Output Section
	*/

	/**
	Loads the contents of this input layer from a file.

	@param path Path to the file to load.
	*/
	public: void load(string path);

	/**
	Saves the contents of this input layer to a file. The file is overwritten if it already exists.

	@param path Path to the file to save.
	*/
	public: void save(string path);
	
	/*
	Property Section
	*/
	
	/**
	Returns a pointer to the texture frame buffer, 
	*/
	public: TextureFrameBuffer& getTextureFrameBuffer();

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
	Returns the layer's visible height.

	@return The layer's visible height.
	*/
	public: int getVisibleHeight();

	/**
	Returns the layer's visible width.

	@return The layer's visible width.
	*/
	public: int getVisibleWidth();
};

#endif
