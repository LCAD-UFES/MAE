/*
Este arquivo é parte do projeto SMV.

Histórico:

* 2006.02.01 - Criado por Hélio Perroni Filho
*/

#include <stdlib.h>

#include <stdexcept>
using std::exception;
using std::runtime_error;

#include "cconfig.h"
#include "config_default.hpp"

#include "logging.hpp"
using mae::logging::report;

int config_load()
{
	try
	{
		ConfigDefault::load();
		return 1;
	}
	catch (exception& e)
	{
		report(e);
		return 0;
	}
	catch (...)
	{
		report("Unknown error");
		return 0;
	}
}

void config_save()
{
	ConfigDefault::save();
}

int config_get_int(char *name)
{
	return ConfigDefault::intValue(string(name));
}

void config_set_int(char *name, int value)
{
	ConfigDefault::setSetting(string(name), value);
}

float config_get_float(char *name)
{
	return ConfigDefault::floatValue(string(name));
}

void config_set_float(char *name, float value)
{
	ConfigDefault::setSetting(string(name), value);
}

double config_get_double(char *name)
{
	return ConfigDefault::doubleValue(string(name));
}

void config_set_double(char *name, double value)
{
	ConfigDefault::setSetting(string(name), value);
}

char *config_get(char *name)
{
	string value = ConfigDefault::getSetting(string(name));

	int size = value.length();
	char *buffer = (char*) malloc(size*sizeof(char));
	for (int i = 0; i < size; i++)
		buffer[i] = value[i];

	return buffer;
}

void config_set(char *name, char *value)
{
	ConfigDefault::setSetting(string(name), string(value));
}
