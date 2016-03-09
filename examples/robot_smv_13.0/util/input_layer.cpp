/*
This file is part of MAE.

History:

[2006.02.13 - Helio Perroni Filho] Created.
*/

#include "input_layer.hpp"

#include <cstdlib>
#include <sstream>
#include <stdexcept>

using std::ostringstream;
using std::out_of_range;
using std::runtime_error;

/*
Constructor Section
*/

InputLayer::InputLayer()
{
	layer = NULL;
}

InputLayer::InputLayer(string name)
{
	layer = mae_input_layer_get(name.c_str());
	if (layer == NULL)
		throw runtime_error("Input layer \"" + name + "\" not known");

	int width  = mae_input_layer_get_texture_frame_width(layer);
	int height = mae_input_layer_get_texture_frame_height(layer);
	GLubyte* buffer = mae_input_layer_get_texture_frame_buffer(layer);
	this->buffer = TextureFrameBuffer(buffer, width, height);
}

/*
Method Section
*/

int InputLayer::size()
{
	return mae_input_layer_get_width(layer) * mae_input_layer_get_height(layer);
}

double InputLayer::doubleOutput(int index)
{
	return floatOutput(index);
}

float InputLayer::floatOutput(int index)
{
	if (0 <= index && index < size())
		return mae_input_layer_output_fval(layer, index);
	else
	{
		ostringstream o1, o2;
		o1 << index;
		o2 << size();
		throw out_of_range("Index " + o1.str() + " not in valid range [0.." + o2.str() +"]");
	}
}

int InputLayer::intOutput(int index)
{
	if (0 <= index && index < size())
		return mae_input_layer_output_ival(layer, index);
	else
	{
		ostringstream o1, o2;
		o1 << index;
		o2 << size();
		throw out_of_range("Index " + o1.str() + " not in valid range [0.." + o2.str() +"]");
	}
}

void InputLayer::update()
{
	mae_input_layer_update(layer);
}

/*
Input / Output Section
*/

void InputLayer::load(string path)
{
	mae_input_network_load(layer, path.c_str());
}

void InputLayer::save(string path)
{
	mae_input_network_save(layer, path.c_str());
}

/*
Property Section
*/

TextureFrameBuffer& InputLayer::getTextureFrameBuffer()
{
	return buffer;
}

int InputLayer::getWidth()
{
	return mae_input_layer_get_width(layer);
}

int InputLayer::getHeight()
{
	return mae_input_layer_get_height(layer);
}

int InputLayer::getVisibleWidth()
{
	return mae_input_layer_get_visible_width(layer);
}

int InputLayer::getVisibleHeight()
{
	return mae_input_layer_get_visible_height(layer);
}
