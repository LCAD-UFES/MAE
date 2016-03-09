/*
This file is part of SMV.

History:

[2006.02.22 - Helio Perroni Filho] Created.
*/

#include "point_cloud.hpp"

#include "primitives.hpp"

using mae::smv::PointCloud;
using mae::util::Primitives;

/*
Constructor Section
*/

PointCloud::PointCloud(): leftPoints(0), rightPoints(0), colors(0)
{
	this->width  = 0;
	this->height = 0;
}

/*
Method Section
*/

void PointCloud::resize(int width, int height)
{
	this->width  = width;
	this->height = height;
	int points = width * height;
	
	leftPoints.resize(2*points);
	rightPoints.resize(2*points);
	colors.resize(points);
}

void PointCloud::merge(int disparity, PointCloud& halfLeft, PointCloud& halfRight)
{
	int widthLeft  = halfLeft.getWidth();
	int widthRight = halfRight.getWidth();
	int width = widthLeft + widthRight - disparity;
	int height = Primitives::min(halfLeft.getHeight(), halfRight.getHeight());

	resize(width, height);

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < widthLeft; j++)
		{
			int index = i * width + j;
			int leftIndex = i * widthLeft + j;

			leftPoints [2*index + 0] = halfLeft.getLeftPoint(2*leftIndex + 0);
			leftPoints [2*index + 1] = halfLeft.getLeftPoint(2*leftIndex + 1);
			rightPoints[2*index + 0] = halfLeft.getRightPoint(2*leftIndex + 0);
			rightPoints[2*index + 1] = halfLeft.getRightPoint(2*leftIndex + 1);
			colors[index] = halfLeft.getPointColor(leftIndex);
		}

		for (int j = 0; j < widthRight; j++)
		{
			int index = (i * width + j) + (widthLeft - disparity);
			int rightIndex = i * widthRight + j;
			leftPoints [2*index + 0] = halfRight.getLeftPoint(2*rightIndex + 0);
			leftPoints [2*index + 1] = halfRight.getLeftPoint(2*rightIndex + 1);
			rightPoints[2*index + 0] = halfRight.getRightPoint(2*rightIndex + 0);
			rightPoints[2*index + 1] = halfRight.getRightPoint(2*rightIndex + 1);
			colors[index] = halfRight.getPointColor(rightIndex);
		}
	}	
}

int PointCloud::size()
{
	return width * height;
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

double PointCloud::getLeftPoint(int index)
{
	return leftPoints.at(index);
}

double PointCloud::getRightPoint(int index)
{
	return rightPoints.at(index);
}

int PointCloud::getPointColor(int index)
{
	return colors.at(index);
}

int PointCloud::getWidth()
{
	return width;
}

int PointCloud::getHeight()
{
	return height;
}
