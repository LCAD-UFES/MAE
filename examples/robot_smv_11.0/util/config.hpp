/*
This file is part of MAE.

History:

* 2006.02.02 - Created by Helio Perroni Filho
*/

#ifndef __CONFIG_HPP
#define __CONFIG_HPP

#include <map>
#include <string>

using std::map;
using std::string;

/** Map of configuration settings. */
typedef map<string, string> StringMap;

/** Iterator over a map of configuration settings. */
typedef map<string, string>::iterator StringIterator;

/**
Storage for a MAE application's global configuration settings.
*/
class Configuration
{
	/*
	Attribute Section
	*/

	/** Map of configration settings. */
	private: StringMap settings;

	/** Path to the file of persisted settings. */
	private: string path;

	/** Tells whether the object should attempt to load settings from a file at construction time, and save its current state at destruction time. */
	private: bool automatic;

	/*
	Constructor Section
	*/

	/**
	Main constructor. Automatic tries to load a configuration set from a file named "config.txt", from the current directory.
	*/
	public: Configuration();

	/**
	Creates a new configuration object, associated to a settings file in the filesystem.

	@param path Path to the settings persistence file.

	@param automatic Whether the object should attempt to load settings from a file at construction time, and save its current state at destruction time.
	*/
	public: Configuration(string path, bool automatic);

	/**
	Main destructor.
	*/
	public: ~Configuration();

	/*
	Method Section
	*/

	/** Loads settings from the current file. */
	public: bool load();

	/** Saves settings to the current file. */
	public: void save();

	/**
	Returns whether a named setting exists.

	@param name Name of a setting.

	@return <code>true</code> or <code>false</code>, depending on the setting's existance.
	*/
	public: bool exists(string name);

	/**
	Returns a setting's value as an <code>int</code>.

	@param name Name of a setting.

	@return The setting's value as an <code>int</code>.

	@throw runtime_error If there is no setting by that name.
	*/
	public: int intValue(string name);

	/**
	Returns a setting's value as a <code>double</code>.

	@param name Name of a setting.

	@return The setting's value as a <code>double</code>.

	@throw runtime_error If there is no setting by that name.
	*/
	public: double doubleValue(string name);

	/*
	Property Section
	*/

	/**
	Returns a setting's value.

	@param name Name of a setting.

	@return The setting's value as a <code>string</code>, or the empty string if there is no setting by that name.
	*/
	public: string getSetting(string name);

	/**
	Sets a setting's value.

	@param name Name of a setting.

	@param value Value of a setting.
	*/
	public: void setSetting(string name, string value);
};

#endif
