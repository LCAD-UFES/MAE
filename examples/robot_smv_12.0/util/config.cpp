/*
This file is part of MAE.

History:

* 2006.02.02 - Created by Helio Perroni Filho
*/

#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>
#include "config.hpp"

using std::istringstream;
using std::ostringstream;
using std::runtime_error;

/*
Constructor Section
*/

Configuration::Configuration()
{
	path = "config.txt";
	automatic = true;
	load();
}

Configuration::Configuration(string path, bool automatic)
{
	this->path = path;
	this->automatic = automatic;
	if (automatic)
		load();
}

Configuration::~Configuration()
{
	if (automatic)
		save();
}

/*
Method Section
*/

bool Configuration::load()
{
	char name[256], value[256];
	FILE *file = fopen(path.c_str(), "r");
	if (file == NULL) return false;

	while (fscanf(file, "%s = %s", name, value) != EOF)
		setSetting(string(name), string(value));

	fclose(file);

	return true;
}

void Configuration::save()
{
	FILE *file = fopen(path.c_str(), "w");
	for (StringIterator i = settings.begin(), n = settings.end(); i != n; i++)
	{
		string name = (*i).first;
		string value = (*i).second;
		fprintf(file, "%s = %s\n", name.c_str(), value.c_str());
	}

	fclose(file);
}

bool Configuration::exists(string name)
{
	return (settings.count(name) > 0);
}

int Configuration::intValue(string name)
{
	if (!exists(name))
		throw runtime_error(string("Property ")  + name + " does not exist");

	string value = getSetting(name);
	istringstream i(value);

	int number;
	if (!(i >> number))
		throw runtime_error(string("Property ")  + name + "=\"" + value + " is not numeric");

	return number;
}

float Configuration::floatValue(string name)
{
	if (!exists(name))
		throw runtime_error(string("Property ")  + name + " does not exist");

	string value = getSetting(name);
	istringstream i(value);

	float number;
	if (!(i >> number))
		throw runtime_error(string("Property ")  + name + "=\"" + value + " is not numeric");

	return number;
}

double Configuration::doubleValue(string name)
{
	if (!exists(name))
		throw runtime_error(string("Property ")  + name + " does not exist");

	string value = getSetting(name);
	istringstream i(value);

	double number;
	if (!(i >> number))
		throw runtime_error(string("Property ")  + name + "=\"" + value + " is not numeric");

	return number;
}

/*
Property Section
*/

string Configuration::getSetting(string name)
{
	return (exists(name) ? settings[name] : string(""));
}

void Configuration::setSetting(string name, string value)
{
	settings[name] = value;
}

void Configuration::setSetting(string name, int value)
{
   ostringstream o;
   if (!(o << value))
     throw runtime_error("Could not convert int value to string");
   setSetting(name, o.str());
}

void Configuration::setSetting(string name, float value)
{
   ostringstream o;
   if (!(o << value))
     throw runtime_error("Could not convert float value to string");
   setSetting(name, o.str());
}

void Configuration::setSetting(string name, double value)
{
   ostringstream o;
   if (!(o << value))
     throw runtime_error("Could not convert double value to string");
   setSetting(name, o.str());
}
