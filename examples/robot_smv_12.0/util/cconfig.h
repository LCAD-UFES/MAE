/*
Este arquivo � parte do projeto SMV.

Hist�rico:

* 2006.02.01 - Criado por H�lio Perroni Filho
*/

#ifndef __CCONFIG_H
#define __CCONFIG_H

/**
C wrapper for the <code>ConfigDefault</code> class.
*/

#ifdef __cplusplus
extern "C" {
#endif
	int config_load();

	void config_save();

	int config_get_int(char *name);
	
	void config_set_int(char *name, int value);
	
	float config_get_float(char *name);
	
	void config_set_float(char *name, float value);
	
	double config_get_double(char *name);
	
	void config_set_double(char *name, double value);
	
	char *confif_get(char *name);
	
	void config_set(char *name, char *value);
#ifdef __cplusplus
}
#endif

#endif
