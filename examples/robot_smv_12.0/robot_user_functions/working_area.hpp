/*
This file is part of SMV.

History:

[2006.02.22 - Helio Perroni Filho] Created.
*/

#ifndef __WORKING_AREA_HPP
#define __WORKING_AREA_HPP

namespace mae
{
	namespace smv
	{
		class WorkingArea;
	};
};

/**
The working area delineates a selected region of an input image, within which some operation (usually related to 3D modelling) is to take place. It encapuslates a pair of coordinates &mdash <code>(x0, y0)</code> and <code>(xn, yn)</code> &mdash which respectively point to the bottom-left and top-right corners of a rectangle.
*/
class mae::smv::WorkingArea
{
	/*
	Attribute Section
	*/

	/** The coordinates of the area's corners. */
	private: int corners[4];

	/*
	Constructor Section
	*/

	/**
	Default Constructor.
	*/
	public: WorkingArea();

	/*
	Operator Section
	*/

	/**
	Implementation of the <code>[]</code> (index) operator.

	@param index An index into the underlying coordinate array.

	@return The value for a corner coordinate.

	@throw out_of_range If the index is not valid.
	*/
	public: int& operator[](int index);

	/*
	Method Section
	*/

	/**
	Returns the x value for the area's bottom-left corner.
	
	@return The x value for the area's bottom-left corner.
	*/
	public: int x0();
	
	/**
	Returns the y value for the area's bottom-left corner.
	
	@return The y value for the area's bottom-left corner.
	*/
	public: int y0();
	
	/**
	Returns the x value for the area's top-right corner.
	
	@return The x value for the area's top-right corner.
	*/
	public: int xn();
	
	/**
	Returns the y value for the area's top-right corner.
	
	@return The y value for the area's top-right corner.
	*/
	public: int yn();

	/*
	Property Section
	*/

	/**
	Sets the area's corner's coordinates.
	
	@param x0 The x value for the area's bottom-left corner.
	@param y0 The y value for the area's bottom-left corner.
	@param xn The x value for the area's top-right corner.
	@param xy The y value for the area's top-right corner.
	*/
	public: void setCorners(int x0, int y0, int xn, int yn);
};

#endif
