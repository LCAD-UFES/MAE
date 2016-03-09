/*
This file is part of MAE.

History:

[2006.02.16 - Helio Perroni Filho] Created.
*/

#ifndef __MAE_UTIL_PRIMITIVES_HPP
#define __MAE_UTIL_PRIMITIVES_HPP

#include <string>

using std::string;

namespace mae
{
	namespace util
	{
		class Primitives;
	};
};

/**
Utility class to deal with primitive and string types.
*/
class mae::util::Primitives
{
	/*
	Method Section
	*/
	
	/**
	Converts a string to an <code>int</code>.
	
	@param value The string to convert.
	
	@return The converted <code>int</code> value.
	
	@throw runtime_error If the string cannot be converted to <code>int</code>.
	*/
	public: static int parseInt(string value);
	
	/**
	Converts an <code>int</code> value to string.
	
	@param value The <code>int</code> value to convert.
	
	@return The converted string.
	
	@throw runtime_error If the value could not be converted to string.
	*/
	public: static string toString(int value);
};

#endif
