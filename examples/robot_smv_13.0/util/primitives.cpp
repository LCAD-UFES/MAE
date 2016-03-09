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

int Primitives::min(int comparing, int compared)
{
	return (comparing < compared ? comparing : compared);
}

bool Primitives::parseBool(string value)
{
	if (value == "true")
		return true;
	else if (value == "false")
		return false;
	else
		throw runtime_error("String "  + value + " cannot be converted to bool");
}

double Primitives::parseDouble(string value)
{
	double number;
	istringstream stream(value);
	if (!(stream >> number))
		throw runtime_error("String \""  + value + "\" cannot be converted to double");

	return number;
}

float Primitives::parseFloat(string value)
{
	float number;
	istringstream stream(value);
	if (!(stream >> number))
		throw runtime_error("String \""  + value + "\" cannot be converted to float");

	return number;
}

int Primitives::parseInt(string value)
{
	int number;
	istringstream stream(value);
	if (!(stream >> number))
		throw runtime_error("String \""  + value + "\" cannot be converted to int");

	return number;
}

string Primitives::toString(double value)
{
	ostringstream stream;
	if (!(stream << value))
		throw runtime_error("Could not convert double value to string");

	return stream.str();
}

string Primitives::toString(int value)
{
	ostringstream stream;
	if (!(stream << value))
		throw runtime_error("Could not convert int value to string");

	return stream.str();
}

string Primitives::toString(unsigned int value)
{
	ostringstream stream;
	if (!(stream << value))
		throw runtime_error("Could not convert int value to string");

	return stream.str();
}

string Primitives::toString(vector<double> value)
{
	string result = "{";
	for (vector<double>::iterator i = value.begin(), n = value.end(); i != n;)
	{
		result += Primitives::toString(*i);
		if (++i == n) break;
		result += ", ";
	}
	
	result += "}";
	return result;
}

string Primitives::trim(string value)
{
	int leftIndex  = value.find_first_not_of(" ");
	int rightIndex = value.find_last_not_of(" ") + 1;
	
	return value.erase(rightIndex).erase(0, leftIndex);
}
