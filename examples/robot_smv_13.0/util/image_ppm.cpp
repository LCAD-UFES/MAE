/*
This file is part of MAE.

History:

[2006.02.28 - Helio Perroni Filho] Created.
*/

#include "image_ppm.hpp"

#include <cstdio>
#include <iostream>
#include <fstream>

using std::ios;
using std::flush;
using std::ifstream;
using std::ofstream;

using mae::io::ImagePPM;

/*
Definition Section
*/

#define RED(pixel)	((pixel & 0x000000ffL) >> 0)
#define GREEN(pixel)	((pixel & 0x0000ff00L) >> 8)
#define BLUE(pixel)	((pixel & 0x00ff0000L) >> 16)
#define PIXEL(r, g, b)  (((r & 0x000000ffL) << 0) | ((g & 0x000000ffL) << 8) | ((b & 0x000000ffL) << 16))

/*
Method Section
*/

bool ImagePPM::load(string path, vector<int>& data)
{
	FILE* file = fopen(path.c_str(), "r");
	if (file == NULL)
		return false;

	int width, height;
	fscanf(file, "P3\n#MAE Image\n%d %d\n255\n", &width, &height);
	data.resize(width*height);
	
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			unsigned int red   = 0;
			unsigned int green = 0;
			unsigned int blue  = 0;
			fscanf(file, "%u %u %u\t", &red, &green, &blue);
			data[i*width + j] = PIXEL(red, green, blue);
		}
	}
	
	fclose(file);
	
	return true;
}

bool ImagePPM::save(string path, int width, int height, int* data)
{
	FILE* file = fopen(path.c_str(), "w");
	if (file == NULL)
		return false;

	fprintf(file, "P3\n");
	fprintf(file, "#MAE Image\n");
	fprintf(file, "%d %d\n", width, height);
	fprintf(file, "255\n");

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			int value = data[i*width + j];
			unsigned char red   = RED(value);
			unsigned char green = GREEN(value);
			unsigned char blue  = BLUE(value);
			fprintf(file, "%d %d %d\t", red, green, blue);
		}

		fprintf(file, "\n");
	}

	fclose(file);

	return true;
}

bool ImagePPM::cut(string source, string destination, int x0, int y0, int xn, int yn)
{
	string header = "P3\n#MAE Image\n"; // The file header's first two lines.
	string maxval = "\n255\n"; // The biggest value to be found in the file.

	int width = 0, height = 0;
	ifstream input(source.c_str());
	input.seekg(header.size()); // Ignores the header's first two lines.
	input >> width >> height; // Reads the image's width and height.
	input.seekg(maxval.size(), ios::cur); // Ignores the remaining of the header.
	
	ofstream output(destination.c_str());
	output // Builds the output file header.
		<< header
		<< (1 + xn - x0) << " " << (1 + yn - y0)
		<< maxval << flush;

	for (int i = 0, n = yn + 1; i < n; i++)
	{
		for (int j = 0; j < width; j++)
		{
			int red, green, blue;
			input >> red >> green >> blue;
			
			if (y0 <= i && x0 <= j && j <= xn)
				output << red << " " << green << " " << blue << "\t";			
		}

		if (y0 <= i)
			output << "\n" << flush;
	}
	
	return true;
}
