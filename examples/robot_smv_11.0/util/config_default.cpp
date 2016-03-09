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

bool ConfigDefault::load()
{
	return config.load();
}

void ConfigDefault::save()
{
	config.save();
}

int ConfigDefault::intValue(string name)
{
	return config.intValue(name);
}

double ConfigDefault::doubleValue(string name)
{
	return config.doubleValue(name);
}

string ConfigDefault::getSetting(string name)
{
	return config.getSetting(name);
}

void ConfigDefault::setSetting(string name, string value)
{
	config.setSetting(name, value);
}
