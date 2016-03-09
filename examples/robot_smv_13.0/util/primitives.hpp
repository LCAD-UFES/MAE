/*
This file is part of MAE.

History:

[2006.02.16 - Helio Perroni Filho] Created.
*/

#ifndef __MAE_UTIL_PRIMITIVES_HPP
#define __MAE_UTIL_PRIMITIVES_HPP

#include <string>
using std::string;

#include <vector>
using std::vector;

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
	Returns the smallest between two integers.

	@param comparing An integer value.

	@param compared Another integer value.

	@return The smallest between two integers.
	*/
	public: static int min(int comparing, int compared);

	/**
	Converts a string to a <code>bool</code>.

	@param value The string to convert.

	@return The converted <code>bool</code> value.

	@throw runtime_error If the string cannot be converted to <code>bool</code>.
	*/
	public: static bool parseBool(string value);

	/**
	Converts a string to a <code>double</code>.

	@param value The string to convert.

	@return The converted <code>double</code> value.

	@throw runtime_error If the string cannot be converted to <code>double</code>.
	*/
	public: static double parseDouble(string value);

	/**
	Converts a string to a <code>float</code>.

	@param value The string to convert.

	@return The converted <code>float</code> value.

	@throw runtime_error If the string cannot be converted to <code>float</code>.
	*/
	public: static float parseFloat(string value);

	/**
	Converts a string to an <code>int</code>.

	@param value The string to convert.

	@return The converted <code>int</code> value.

	@throw runtime_error If the string cannot be converted to <code>int</code>.
	*/
	public: static int parseInt(string value);

	/**
	Converts a <code>double</code> value to string.

	@param value The <code>double</code> value to convert.

	@return The converted string.

	@throw runtime_error If the value could not be converted to string.
	*/
	public: static string toString(double value);

	/**
	Converts an <code>int</code> value to string.

	@param value The <code>int</code> value to convert.

	@return The converted string.

	@throw runtime_error If the value could not be converted to string.
	*/
	public: static string toString(int value);

	/**
	Converts an <code>unsigned int</code> value to string.

	@param value The <code>unsigned int</code> value to convert.

	@return The converted string.

	@throw runtime_error If the value could not be converted to string.
	*/
	public: static string toString(unsigned int value);

	/**
	Converts a <code>vector&lt;double&gt;</code> value to string.

	@param value The <code>vector&lt;double&gt;</code> value to convert.

	@return The converted string.

	@throw runtime_error If the value could not be converted to string.
	*/
	public: static string toString(vector<double> value);
	
	/**
	Returns a copy of the string with all leading and trailing whitespace removed.
	
	@param value A string.
	
	@return A copy of this string with all leading and trailing whitespace removed, or this string if it has no leading or trailing whitespace.
	*/
	public: static string trim(string value);
};

/*
Top-level Operator Section
*/

inline string operator+(string a, bool b)
{
	return a + mae::util::Primitives::toString(b);
};

inline string operator+(bool a, string b)
{
	return mae::util::Primitives::toString(a) + b;
};

inline string operator+(string a, float b)
{
	return a + mae::util::Primitives::toString(b);
};

inline string operator+(float a, string b)
{
	return mae::util::Primitives::toString(a) + b;
};

inline string operator+(string a, double b)
{
	return a + mae::util::Primitives::toString(b);
};

inline string operator+(double a, string b)
{
	return mae::util::Primitives::toString(a) + b;
};

inline string operator+(string a, int b)
{
	return a + mae::util::Primitives::toString(b);
};

inline string operator+(int a, string b)
{
	return mae::util::Primitives::toString(a) + b;
};

inline string operator+(string a, unsigned int b)
{
	return a + mae::util::Primitives::toString(b);
};

inline string operator+(unsigned int a, string b)
{
	return mae::util::Primitives::toString(a) + b;
};

inline string operator+(string a, vector<double> b)
{
	return a + mae::util::Primitives::toString(b);
};

#endif
