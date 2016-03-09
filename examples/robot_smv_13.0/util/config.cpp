/*
This file is part of MAE.

History:

[2006.02.02 - Helio Perroni Filho] Created.
*/

#include "config.hpp"

#include <cstdio>

#include <stdexcept>
using std::runtime_error;

#include <string>
using std::string;

#include "logging.hpp"
using mae::logging::trace;

#include "primitives.hpp"
using mae::util::Primitives;

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
	if (automatic) try
	{
		load();
	}
	catch (...)
	{
	}
}

Configuration::~Configuration()
{
	if (automatic)
		save();
}

/*
Method Section
*/

void Configuration::load()
{
	load(path);
}

void Configuration::save()
{
	save(path);
}

void Configuration::load(string path)
{
	trace("Configuration::load", "\"" + path + "\"");
	
	char name[256], value[256];
	FILE *file = fopen(path.c_str(), "r");
	if (file == NULL)
		throw runtime_error("File not found: " + path);

	while (fscanf(file, "%s = %s", name, value) != EOF)
		setSetting(string(name), string(value));

	fclose(file);
}

void Configuration::save(string path)
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

bool Configuration::boolValue(string name)
{
	if (!exists(name))
		throw runtime_error("Setting "  + name + " does not exist");

	string value = getSetting(name);
	try
	{
		return Primitives::parseBool(value);
	}
	catch (...)
	{
		throw runtime_error("Setting "  + name + "=\"" + value + "\" is not boolean");
	}
}

int Configuration::intValue(string name)
{
	if (!exists(name))
		throw runtime_error("Setting "  + name + " does not exist");

	string value = getSetting(name);
	try
	{
		return Primitives::parseInt(value);
	}
	catch (...)
	{
		throw runtime_error("Setting "  + name + "=\"" + value + "\" is not numeric");
	}
}

float Configuration::floatValue(string name)
{
	if (!exists(name))
		throw runtime_error("Setting "  + name + " does not exist");

	string value = getSetting(name);
	try
	{
		return Primitives::parseFloat(value);
	}
	catch (...)
	{
		throw runtime_error("Setting "  + name + "=\"" + value + "\" is not numeric");
	}
}

double Configuration::doubleValue(string name)
{
	if (!exists(name))
		throw runtime_error("Setting "  + name + " does not exist");

	string value = getSetting(name);
	try
	{
		return Primitives::parseDouble(value);
	}
	catch (...)
	{
		throw runtime_error("Setting "  + name + "=\"" + value + "\" is not numeric");
	}
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

void Configuration::setSetting(string name, bool value)
{
   setSetting(name, value ? "true" : "false");
}

void Configuration::setSetting(string name, int value)
{
   setSetting(name, Primitives::toString(value));
}

void Configuration::setSetting(string name, float value)
{
   setSetting(name, Primitives::toString(value));
}

void Configuration::setSetting(string name, double value)
{
   setSetting(name, Primitives::toString(value));
}
