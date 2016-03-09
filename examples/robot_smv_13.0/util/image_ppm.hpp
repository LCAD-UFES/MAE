/*
This file is part of MAE.

History:

[2006.02.28 - Helio Perroni Filho] Created.
*/

#ifndef __MAE_IO_IMAGE_PPM_HPP
#define __MAE_IO_IMAGE_PPM_HPP

#include <string>
#include <vector>

using std::string;
using std::vector;

namespace mae
{
	namespace io
	{
		class ImagePPM;
	};
};

/**
An PPM image.
*/
class mae::io::ImagePPM
{
	/*
	Method Section
	*/

	/**
	Loads an ASCII-encoded PPM image into a memory block.

	@param path Path to the PPM file.

	@param data Memory block that is to contain the image's data.

	@return Whether the operation was succesful.
	*/
	public: static bool load(string path, vector<int>& data);

	/**
	Saves a block of data to disk as an ASCII-encoded PPM image.

	@param path Path to the PPM file.

	@param width The image's width.

	@param height The Image's height.

	@param data Memory block containing the image's data.

	@return Whether the operation was succesful.
	*/
	public: static bool save(string path, int width, int height, int* data);
	
	public: static bool cut(string source, string destination, int x0, int y0, int xn, int yn);
};

#endif
