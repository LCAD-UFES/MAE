/*
Este arquivo é parte do projeto SMV.

Histórico:

* 2006.02.01 - Criado por Hélio Perroni Filho
*/

#include <stdlib.h>
#include "cconfig.h"
#include "config_default.hpp"

int config_load()
{
	return (ConfigDefault::load() ? 1 : 0);
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
	char buffer[256];
	sprintf(buffer, "%i", value);
	config_set(name, buffer);

}

double config_get_double(char *name)
{
	return ConfigDefault::doubleValue(string(name));
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
