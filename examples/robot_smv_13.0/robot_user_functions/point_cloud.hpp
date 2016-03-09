/*
This file is part of SMV.

History:

[2006.02.22 - Helio Perroni Filho] Created.
*/

#ifndef __MAE_SMV_POINT_CLOUD_HPP
#define __MAE_SMV_POINT_CLOUD_HPP

#include <vector>

#include "working_area.hpp"

using std::vector;

namespace mae
{
	namespace smv
	{
		class PointCloud;
	};
};

/**
A collection of 3D points tanken from a real world oject.
*/
class mae::smv::PointCloud
{
	/*
	Attribute Section
	*/

	/** Points from the left visual stream. */
	private: vector<double> leftPoints;

	/** Points from the right visual stream. */
	private: vector<double> rightPoints;

	/** Color information for the points. */
	private: vector<int> colors;

	/** Point cloud width. */
	private: int width;

	/** Point cloud height. */
	private: int height;

	/*
	Constructor Section
	*/

	/**
	Default constructor.
	*/
	public: PointCloud();

	/*
	Method Section
	*/

	public: void merge(int disparity, PointCloud& halfLeft, PointCloud& halfRight);
	
	public: void resize(int newWidth, int newHeight);

	public: int size();

	/*
	Property Section
	*/

	/**
	Returns a vector of points from the left visual stream. Each point is represented as a pair of values <code>(v[i], v[i + 1]), 0 <= i < n / 2, i % 2 = 0</code>, where <code>v</code> is the point vector, <code>i</code> is any index and <code>n</code> is the vector's size.

	@return Point vector.
	*/
	public: vector<double>& getLeftPoints();

	/**
	Returns a vector of points from the right visual stream. Each point is represented as a pair of values <code>(v[i], v[i + 1]), 0 <= i < n / 2, i % 2 = 0</code>, where <code>v</code> is the point vector, <code>i</code> is any index and <code>n</code> is the vector's size.

	@return Point vector.
	*/
	public: vector<double>& getRightPoints();

	/**
	Returns a vector of integer values representing the colors of each point.

	@return Color vector.
	*/
	public: vector<int>& getPointColors();

	/**
	*/
	public: double getLeftPoint(int index);

	/**
	*/
	public: double getRightPoint(int index);

	/**
	*/
	public: int getPointColor(int index);
	
	public: int getWidth();
	
	public: int getHeight();
};

#endif
