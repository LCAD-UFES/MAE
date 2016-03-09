/*
This file is part of MAE.

History:

[2006.02.28 - Helio Perroni Filho] Created.
*/

#include "image_ppm.hpp"

#include <cstdio>

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
	}
	
	fclose(file);
	
	return true;
}
