/*
This file is part of SMV.

History:

[2006.02.22 - Helio Perroni Filho] Created.
*/

#ifndef __WORKING_AREA_HPP
#define __WORKING_AREA_HPP

#include <string>

using std::string;

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
	
	/**
	Creates a new Working Area out of a set of corner coordinates <code>{(x0, y0), (xn, yn)}</code>.
	
	@param x0 The x value for the area's bottom-left corner.
	@param y0 The y value for the area's bottom-left corner.
	@param xn The x value for the area's top-right corner.
	@param xy The y value for the area's top-right corner.
	*/
	public: WorkingArea(int x0, int y0, int xn, int yn);

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
	
	/**
	Returns a string representation of this working area.

	@return String representation of this working area.
	*/
	public: string toString();

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

/*
Top-level Operator Section
*/

inline string operator+(string a, mae::smv::WorkingArea& b)
{
	return a + b.toString();
};

inline string operator+(mae::smv::WorkingArea& a, string b)
{
	return a.toString() + b;
};

inline string operator+(char* a, mae::smv::WorkingArea& b)
{
	return string(a) + b;
};

inline string operator+(mae::smv::WorkingArea& a, char* b)
{
	return a + string(b);
};

#endif
