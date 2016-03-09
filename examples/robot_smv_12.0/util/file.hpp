/*
This file is part of MAE.

History:

[2006.02.16 - Helio Perroni Filho] Created.
*/

#ifndef __MAE_IO_FILE_HPP
#define __MAE_IO_FILE_HPP

#include <string>

using std::string;

namespace mae
{
	namespace io
	{
		class File;
	};
};

/**
Utility File class.
*/
class mae::io::File
{
	/*
	Method Section
	*/

	/**
	Returns whether a given file exists.
	
	@param path The file path.
	
	@return <code>true</code> or <code>false</code>, depending on whether the file exists.
	*/
	public: static bool exists(string path);
};

#endif
