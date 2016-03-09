/*
This file is part of MAE.

History:

[2006.02.16 - Helio Perroni Filho] Created.
*/

#include "file.hpp"

#include <sys/stat.h>

using mae::io::File;

/*
Method Section
*/

bool File::exists(string path)
{
   struct stat buf;
   return !stat(path.c_str(), &buf);
}
