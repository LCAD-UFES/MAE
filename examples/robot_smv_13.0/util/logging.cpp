/*
This file is part of SMV.

History:

[2006.03.14 - Helio Perroni Filho] Created.
*/

#include "logging.hpp"
using mae::logging::Logger;

#include <csignal>
#include <cstdlib>

#include <fstream>
using std::cerr;
using std::endl;
using std::ifstream;
using std::ofstream;

#include "primitives.hpp"
using mae::util::Primitives;

/*
Attribute Section
*/

/** Standard log output stream. Declared as a static module variable so the signal handler can get it directly. */
static ostream* output;

/*
Function Section
*/

void mae::logging::log(string message)
{
	Logger& logger = Logger::getInstance();
	logger.log(message);
}

void mae::logging::report(exception& e)
{
	Logger& logger = Logger::getInstance();
	logger.report(e);
}

void mae::logging::report(string error)
{
	Logger& logger = Logger::getInstance();
	logger.report(error);
}

void mae::logging::trace(string state, string arguments, string members)
{
	Logger& logger = Logger::getInstance();
	logger.trace(state, arguments, members);
}

void reportSignal(int signal)
{
	Logger& logger = Logger::getInstance();
	logger.report(signal);
}

/*
Constructor Section
*/

Logger::Logger()
{
	output = &cerr;
	verbose = false;
	trace("Logger::Logger");

	// Register our custom signal handler.
	std::signal(SIGSEGV, reportSignal);

	signalHandlers[SIGSEGV] = &Logger::reportSIGSEGV;

	config();
}

Logger::~Logger()
{
}

Logger& Logger::getInstance()
{
	static Logger* singleton = new Logger();

	return *singleton;
}

/*
Method Section
*/

void Logger::reportSIGSEGV()
{
	/* Print the last trace()'d program state. */
	(*output) << "Segmentation fault after entering state \"" << state << "\"." << endl;

	/* Ellaborates on what the message above means. */
	(*output)
		<< "This is, of course, only the last trace()'d state entered before I received the SIGSEGV;"   << endl
		<< "I have no way to tell who actually messed up. It is a good place to start looking, though." << endl
		<< "Dying now, so the bug hunt can start. Good luck for us both..." << endl;

	/* Terminates the application, as there's nothing more we can do. */
	abort();
}

void Logger::log(string message)
{
	(*output) << message << endl;
}

void Logger::redirect(ostream* newOutput)
{
	output = newOutput;
}

void Logger::report(exception& e)
{
	(*output)
		<< "Exception after entering state \"" << state << "\": " << e.what() << endl
		<< "This is, of course, only the last trace()'d state entered before the exception was thrown;\n"
		<< "I have no way to tell who actually messed up. It is a good place to start looking, though." << endl;
}

void Logger::report(string error)
{
	(*output)
		<< "Error after entering state \"" << state << "\": " << error << endl
		<< "This is, of course, only the last trace()'d state entered before the error was found;\n"
		<< "I have no way to tell who actually messed up. It is a good place to start looking, though." << endl;
}

void Logger::report(int signal)
{
	/* Changes the signal handler back to default, avoiding recursion in case this function raises the signal again. */
	std::signal(signal, SIG_DFL);
	
	if (signalHandlers.count(signal) > 0)
	{
		SignalHandler handler = signalHandlers[signal]; 
		(this->*handler)();
	}
	else
	{
		(*output) << "No custom handler found for signal " << signal << ", calling default handler." << endl;
		raise(signal);
	}
}

bool Logger::isIgnored(string state)
{
	for (set<string>::iterator i = ignored.begin(), n = ignored.end(); i != n; i++)
	{
		string prefix = (*i);
		if (state.find(prefix) == 0)
			return true;
	}
	
	return false;
}

void Logger::trace(string state, string arguments, string members)
{
	this->state = state + "(" + arguments + ")" + (members != "" ? ": " : "") + members;
	
	if (verbose && !isIgnored(state))
		(*output) << (this->state) << endl;
}

/*
Input / Output Section
*/

void Logger::config()
{
	ifstream config("logger.txt");

	string flag = "";
	config >> flag;
	verbose = (flag == "verbose");

	string path = "";
	config >> path;
	if (path != "" && path != "stdlog")
	{
		ofstream* out = new ofstream(path.c_str());
		redirect(out);
	}
	
	for (string prefix = ""; config.good();)
	{
		config >> prefix;
		prefix = Primitives::trim(prefix);
		if (prefix != "")
			ignored.insert(prefix);
	}
}

/*
Property Section
*/

void Logger::setVerbose(bool verbose)
{
	this->verbose = verbose;
}
