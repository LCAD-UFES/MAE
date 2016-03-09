/*
This file is part of MAE.

History:

[2006.03.14 - Helio Perroni Filho] Created.
*/

#ifndef __LOGGING_HPP
#define __LOGGING_HPP

#include <iostream>
using std::ostream;

#include <map>
using std::map;

#include <set>
using std::set;

#include <stdexcept>
using std::exception;
#include <string>
using std::getline;
using std::string;

/**
Simple logging module. Provides functions to write to the <i>standard log output</i> (which defaults to <code>cerr</code>) as well as redirecting that output to other streams such as files.
*/

namespace mae
{
	namespace logging
	{
		class Logger;

		typedef void (Logger::*SignalHandler)(void);
		
		typedef map<int, SignalHandler> SignalHandlerMap;

		/*
		Helper Section
		*/

		void log(string message);

		void report(exception& e);

		void report(string error);

		void trace(string state, string arguments = "", string members = "");
	};
};

class mae::logging::Logger
{
	/*
	Attribute Section
	*/

	/** Map relating signal numbers to their handlers. */	
	private: SignalHandlerMap signalHandlers;
	
	/** Set of trace()'d states not to print out, even on verbose mode. */
	private: set<string> ignored;
	
	/** String representing the current program state. */
	private: string state;

	/** Whether the logger will log every trace()'d call or just the fisrt to send out a signal. */
	private: bool verbose;

	/*
	Constructor Section
	*/

	/**
	Default constructor.
	*/
	private: Logger();

	/**
	Default destructor.
	*/
	private: ~Logger();

	/**
	Returns this class' single instance.

	@return This class' single instance.
	*/
	public: static Logger& getInstance();

	/*
	Method Section
	*/

	private: bool isIgnored(string state);
	
	/**
	Reports a received SIGSEGV signal to the standard log output. The report message includes the latest <code>trace()</code>'d program state.
	*/
	private: void reportSIGSEGV();

	/**
	Sends a message to the standard log output.

	@param message Message to log.
	*/
	public: void log(string message);

	/**
	Redirects the log output to a new stream.

	@param output Stream to receive the log messages from now on.
	*/
	public: void redirect(ostream* output);

	/**
	Reports an exception to the standard log output. The report message includes the latest <code>trace()</code>'d program state.

	@param e Exception to be reported.
	*/
	public: void report(exception& e);

	/**
	Reports an error to the standard log output. The report message includes the latest <code>trace()</code>'d program state.

	@param error Error condition to be reported.
	*/
	public: void report(string error);

	/**
	Reports a received signal to the standard log output. The report message includes the latest <code>trace()</code>'d program state.

	@param signal Signal to be reported.
	*/
	public: void report(int signal);

	/**
	Stores a string representing the program execution state at the time of this method's invokation. This string is added to an error message which is printed to the standard log output if the application receives a <code>SIGSEGV</code> signal.

	A simple way to use this method is invoke it at the beggining of every method, passing it the name of the invoked method along with its arguments. For example:

	<pre>
		void C::f(int a, int b)
		{
			trace("C::f(" + Primitives::toString(a) + ", " + Primitives::toString(b) + ")");

			// ... Method body follows
		}
	</pre>

	This provides a straightforward, if not very human-friendly, account of control flow throughout the program.

	@param state String representing the current program state.
	*/
	public: void trace(string state, string arguments = "", string members = "");

	/*
	Input / Output Section
	*/

	/**
	Loads the verbosity setting and the path to the output log file from a <code>logging.txt</code> file in the program's current directory. The configuration file is expected to have two lines: the first containing the verbosity setting (<code>normal</code> or <code>verbose</code>), and the second containing a valid output file path.

	If <code>logging.txt</code> does not exist or is empty, verbosity is set to <code>normal</code> and the standard error output is used as log output. The latter also holds true if the file contains only the verbosity setting, but it is not possible to specify an output file without also specifying verbosity.
	*/
	public: void config();

	/*
	Property Section
	*/

	/**
	Sets whether the logger will log every trace()'d call or just the fisrt to send out a signal.

	@param verbose Whether to be verbose or silent about trace()'d calls.
	*/
	public: void setVerbose(bool verbose);
};

#endif
