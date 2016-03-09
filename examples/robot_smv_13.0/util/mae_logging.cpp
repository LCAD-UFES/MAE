/*
This file is part of MAE.

History:

[2006.03.21 - Helio Perroni Filho] Created. 
*/

#include "mae_logging.h"

#include <stdarg.h>

#include <string>
using std::string;

#include "logging.hpp"
using mae::logging::trace;

#define BUFFER_SIZE 256

void mae_logging_trace(const char* call)
{
	mae::logging::trace(string(call));
}


void mae_logging_trace2(const char* call, const char* arglist, ...)
{
	va_list args;
	char buffer[BUFFER_SIZE];
	
	va_start(args, arglist);
	vsnprintf(buffer, BUFFER_SIZE, arglist, args);
	va_end(args);
	
	mae::logging::trace(string(call), string(buffer));
}
