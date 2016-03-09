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

	@throws runtime_error If the operation was unsuccesful.
	*/
	public: static void load();

	/**
	Loads persisted settings from the given file.
	
	@param path Path to the settings file.
	
	@throws runtime_error If the operation was unsuccesful.
	*/
	public: static void load(string path);
	
	/**
	Saves persisted settings.
	*/
	public: static void save();

	/**
	Returns a setting's value as a <code>bool</code>.

	@param name Name of a setting.

	@return The setting's value as a <code>bool</code>.

	@throw runtime_error If there is no setting by that name, or if the value could not be converted to <code>bool</code>.
	*/
	public: static bool boolValue(string name);
	
	/**
	Returns a setting's value as a <code>bool</code>, or a fallback value if this is not possible.

	@param name Name of a setting.
	
	@param fallback the value to return if the setting cannot be returned.

	@return The setting's value as a <code>bool</code>, or the fallback value if there is no setting by that name, or if the value could not be converted to <code>bool</code>.
	*/
	public: static bool boolValue(string name, bool fallback);

	/**
	Returns a setting's value as an <code>int</code>.

	@param name Name of a setting.

	@return The setting's value as an <code>int</code>.

	@throw runtime_error If there is no setting by that name, or if the value could not be converted to <code>int</code>.
	*/
	public: static int intValue(string name);
	
	/**
	Returns a setting's value as an <code>int</code>, or a fallback value if this is not possible.

	@param name Name of a setting.
	
	@param fallback the value to return if the setting cannot be returned.

	@return The setting's value as an <code>int</code>, or the fallback value if there is no setting by that name, or if the value could not be converted to <code>int</code>.
	*/
	public: static int intValue(string name, int fallback);

	/**
	Returns a setting's value as a <code>float</code>.

	@param name Name of a setting.

	@return The setting's value as a <code>float</code>.

	@throw runtime_error If there is no setting by that name, or if the value could not be converted to <code>float</code>.
	*/
	public: static float floatValue(string name);

	/**
	Returns a setting's value as a <code>float</code>, or a fallback value if this is not possible.

	@param name Name of a setting.
	
	@param fallback the value to return if the setting cannot be returned.

	@return The setting's value as a <code>float</code>, or the fallback value if there is no setting by that name, or if the value could not be converted to <code>float</code>.
	*/
	public: static float floatValue(string name, float fallback);

	/**
	Returns a setting's value as a <code>double</code>.

	@param name Name of a setting.

	@return The setting's value as a <code>double</code>.

	@throw runtime_error If there is no setting by that name, or if the value could not be converted to <code>double</code>.
	*/
	public: static double doubleValue(string name);

	/**
	Returns a setting's value as a <code>double</code>.

	@param name Name of a setting.

	@param fallback the value to return if the setting cannot be returned.

	@return The setting's value as a <code>double</code>, or the fallback value if there is no setting by that name, or if the value could not be converted to <code>double</code>.
	*/
	public: static double doubleValue(string name, double fallback);
	
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
	
	/**
	Sets a setting value by name.

	@param name Name of a setting.

	@param value Value of a setting.
	*/
	public: static void setSetting(string name, int value);
	
	/**
	Sets a setting value by name.

	@param name Name of a setting.

	@param value Value of a setting.
	*/
	public: static void setSetting(string name, float value);
	
	/**
	Sets a setting value by name.

	@param name Name of a setting.

	@param value Value of a setting.
	*/
	public: static void setSetting(string name, double value);
};

#endif
