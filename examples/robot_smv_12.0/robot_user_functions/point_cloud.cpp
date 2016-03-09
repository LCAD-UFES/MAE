/*
This file is part of SMV.

History:

[2006.02.22 - Helio Perroni Filho] Created.
*/

#include "point_cloud.hpp"

using mae::smv::PointCloud;

/*
Constructor Section
*/

PointCloud::PointCloud(int width, int height): leftPoints(0), rightPoints(0), colors(0)
{
	this->width  = width;
	this->height = height;
}

/*
Method Section
*/

void PointCloud::resize(int newWidth, int newHeight)
{
	vector<double> oldLeftPoints  = leftPoints;
	vector<double> oldRightPoints = rightPoints;
	vector<int>    oldColors      = colors;

	int size = newWidth * newHeight;
	leftPoints.resize(size*2);
	rightPoints.resize(size*2);
	colors.resize(size);

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			int oldIndex = i * width    + j;
			int newIndex = i * newWidth + j;

			leftPoints [2*newIndex + 0] = oldLeftPoints [2*oldIndex + 0];
			leftPoints [2*newIndex + 1] = oldLeftPoints [2*oldIndex + 1];
			rightPoints[2*newIndex + 0] = oldRightPoints[2*oldIndex + 0];
			rightPoints[2*newIndex + 1] = oldRightPoints[2*oldIndex + 1];
			colors[newIndex] = oldColors[oldIndex];
		}
	}

	this->width  = newWidth;
	this->height = newHeight;
}

void PointCloud::appendPoints(
	int w0, int offset, vector<double>& moreLeftPoints, vector<double>& moreRightPoints, vector<int>& moreColors)
{
	int newWidth = (offset + moreColors.size() - w0) / height;
	resize(newWidth, height);

	int moreWidth = moreColors.size() / height;

	for (int i = 0; i < height; i++)
	{
		int n = moreWidth - w0;
		for (int j = 0; j < n; j++)
		{
			int index = i * width + (j + offset);
			int moreIndex = i * moreWidth + (j + w0);

			leftPoints [2*index + 0] = moreLeftPoints [2*moreIndex + 0];
			leftPoints [2*index + 1] = moreLeftPoints [2*moreIndex + 1];
			rightPoints[2*index + 0] = moreRightPoints[2*moreIndex + 0];
			rightPoints[2*index + 1] = moreRightPoints[2*moreIndex + 1];
			colors[index] = moreColors[moreIndex];
		}
	}	
}

/*
Property Section
*/

vector<double>& PointCloud::getLeftPoints()
{
	return leftPoints;
}

vector<double>& PointCloud::getRightPoints()
{
	return rightPoints;
}

vector<int>& PointCloud::getPointColors()
{
	return colors;
}

int PointCloud::getSize()
{
	return width * height;
}
