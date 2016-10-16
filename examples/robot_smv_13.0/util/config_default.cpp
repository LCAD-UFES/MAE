/*
This file is part of MAE.

History:

* 2006.02.02 - Created by Helio Perroni Filho
*/

#include "config_default.hpp"

/*
Attribute Section
*/

Configuration ConfigDefault::config;

/*
Method Section
*/

void ConfigDefault::load()
{
	config.load();
}

void ConfigDefault::load(string path)
{
	config.load(path);
}

void ConfigDefault::save()
{
	config.save();
}

bool ConfigDefault::boolValue(string name)
{
	return config.boolValue(name);
}

bool ConfigDefault::boolValue(string name, bool fallback)
{
	try
	{
		return config.boolValue(name);
	}
	catch (...)
	{
		return fallback;
	}
}

int ConfigDefault::intValue(string name)
{
	return config.intValue(name);
}

int ConfigDefault::intValue(string name, int fallback)
{
	try
	{
		return config.intValue(name);
	}
	catch (...)
	{
		return fallback;
	}
}

float ConfigDefault::floatValue(string name)
{
	return config.floatValue(name);
}

float ConfigDefault::floatValue(string name, float fallback)
{
	try
	{
		return config.floatValue(name);
	}
	catch (...)
	{
		return fallback;
	}
}

double ConfigDefault::doubleValue(string name)
{
	return config.doubleValue(name);
}

double ConfigDefault::doubleValue(string name, double fallback)
{
	try
	{
		return config.doubleValue(name);
	}
	catch (...)
	{
		return fallback;
	}
}

string ConfigDefault::getSetting(string name)
{
	return config.getSetting(name);
}

void ConfigDefault::setSetting(string name, string value)
{
	config.setSetting(name, value);
}

void ConfigDefault::setSetting(string name, int value)
{
	config.setSetting(name, value);
}

void ConfigDefault::setSetting(string name, float value)
{
	config.setSetting(name, value);
}

void ConfigDefault::setSetting(string name, double value)
{
	config.setSetting(name, value);
}