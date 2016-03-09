/*
This file is part of MAE.

History:

* 2006.02.02 - Created by Helio Perroni Filho
*/

#ifndef __CONFIG_DEFAULT_HPP
#define __CONFIG_DEFAULT_HPP

#include "config.hpp"

/**
A facade for the <code>Configuration</code> class. Provides a default settings repository for the whole application.
*/
class ConfigDefault
{
	/*
	Attribute Section
	*/
	
	/** The unique configuration object. */
	private: static Configuration config;
	
	/*
	Method Section
	*/
	
	/**
	Loads persisted settings.
	*/
	public: static bool load();
	
	/**
	Saves persisted settings.
	*/
	public: static void save();

	/**
	Returns a setting's value as an <code>int</code>.

	@param name Name of a setting.

	@return The setting's value as an <code>int</code>.

	@throw runtime_error If there is no setting by that name.
	*/
	public: static int intValue(string name);

	/**
	Returns a setting's value as a <code>double</code>.

	@param name Name of a setting.

	@return The setting's value as a <code>double</code>.

	@throw runtime_error If there is no setting by that name.
	*/
	public: static double doubleValue(string name);
	
	/**
	Retrieves a setting by name.

	@param name Name of a setting.

	@return The setting's value as a <code>string</code>, or the empty string if there is no setting by that name.
	*/
	public: static string getSetting(string name);
	
	/**
	Sets a setting value by name.

	@param name Name of a setting.

	@param value Value of a setting.
	*/
	public: static void setSetting(string name, string value);
};

#endif
