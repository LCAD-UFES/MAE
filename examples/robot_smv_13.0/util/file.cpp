/*
This file is part of MAE.

History:

[2006.02.16 - Helio Perroni Filho] Created.
*/

#include "file.hpp"
using mae::io::File;

#include <cstdio>

#include "logging.hpp"
using mae::logging::trace;

/*
Method Section
*/

bool File::exists(string path)
{
	trace("File::exists", "\"" + path + "\"");
	
   FILE* file = fopen(path.c_str(), "r");
   bool result = (file != NULL);
   if (file != NULL)
	   fclose(file);
   
   return result;
}
