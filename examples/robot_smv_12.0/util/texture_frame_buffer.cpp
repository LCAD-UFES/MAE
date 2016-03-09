/*
This file is part of MAE.

History:

[2006.02.13 - Helio Perroni Filho] Created.
*/

#include "texture_frame_buffer.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>

using std::ostringstream;
using std::out_of_range;

/*
Constructor Section
*/

TextureFrameBuffer::TextureFrameBuffer()
{
	this->buffer = NULL;
	this->width  = 0;
	this->height = 0;
}

TextureFrameBuffer::TextureFrameBuffer(GLubyte* buffer, int width, int height)
{
	this->buffer = buffer;
	this->width  = width;
	this->height = height;
}

/*
Operator Section
*/

GLubyte& TextureFrameBuffer::operator[](int index)
{
	if (0 <= index && index < size())
		return buffer[index];
	else
	{
		ostringstream o1, o2;
		o1 << index;
		o2 << size();
		throw out_of_range("Index " + o1.str() + " not in valid range [0.." + o2.str() +"]");
	}
}

/*
Method Section
*/

int TextureFrameBuffer::size()
{
	return 3*width*height;
}

/*
Property Section
*/

int TextureFrameBuffer::getHeight()
{
	return height;
}

int TextureFrameBuffer::getWidth()
{
	return width;
}
