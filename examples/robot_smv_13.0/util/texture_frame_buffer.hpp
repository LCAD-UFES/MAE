/*
This file is part of MAE.

History:

[2006.02.13 - Helio Perroni Filho] Created.
*/

#ifndef __TEXTURE_FRAME_BUFFER_HPP
#define __TEXTURE_FRAME_BUFFER_HPP

#include <GL/glut.h>

/**
Wrapper to a GLUT texture frame buffer.
*/
class TextureFrameBuffer
{
	/*
	Attribute Section
	*/

	/** Pointer to the buffer. */
	private: GLubyte* buffer;

	/** The buffer's width. */
	private: int width;

	/** The buffer's height. */
	private: int height;

	/*
	Constructor Section
	*/

	/**
	Default constructor.
	*/
	public: TextureFrameBuffer();

	/**
	Creates a new buffer wrapper.

	@param buffer Pointer to the wrapped buffer.

	@param width Width of the wrapped buffer.

	@param height Height of the wrapped buffer.
	*/
	public: TextureFrameBuffer(GLubyte* buffer, int width, int height);

	/*
	Operator Section
	*/

	/**
	References an element in the underlying buffer.

	@param index The element's index.

	@return A reference to the element.

	@throw out_of_range If the index is not valid.
	*/
	public: GLubyte& operator[](int index);

	/*
	Method Section
	*/

	/**
	Returns the size of the buffer, defined as the width times de height.

	@return The size of the buffer.
	*/
	public: int size();

	/*
	Property Section
	*/

	/**
	Returns the buffer's height.

	@return The buffer's height.
	*/
	public: int getHeight();

	/**
	Returns the buffer's width.

	@return The buffer's width.
	*/
	public: int getWidth();
};

#endif
