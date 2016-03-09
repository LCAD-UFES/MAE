/*
This file is part of MAE.

History:

[2006.02.16 - Helio Perroni Filho] Created.
*/

#include "primitives.hpp"

#include <sstream>
#include <stdexcept>

using std::istringstream;
using std::ostringstream;
using std::runtime_error;

using mae::util::Primitives;

/*
Method Section
*/

int Primitives::parseInt(string value)
{
	int number;
	istringstream stream;
	if (!(stream >> number))
		throw runtime_error("String "  + value + " cannot be converted to int");

	return number;
}

string Primitives::toString(int value)
{
	ostringstream stream;
	if (!(stream << value))
		throw runtime_error("Could not convert int value to string");

	return stream.str();
}
