/*
This file is part of SMV.

History:

[2006.02.07 - Helio Perroni Filho] Now faces will only recalculate its mass center if the config setting "faces.mode" is set to "maintenance".
*/

#include <float.h>

#include "model_3d.hpp"
using mae::smv::Model3D;

#include <math.h>
#include <stdlib.h>

#include <stdexcept>
using std::runtime_error;

#include <string>
using std::string;

#include "config_default.hpp"

#include "logging.hpp"
using mae::logging::log;
using mae::logging::trace;

#include "primitives.hpp"
using mae::util::Primitives;

#include "robot_neuron_network_c.h"
#include "stereo_api.h"

#include "stereo_volume.cpp"

/*
Definition Section
*/

#define 	RED(pixel)	((pixel & 0x000000ffL) >> 0)
#define 	GREEN(pixel)	((pixel & 0x0000ff00L) >> 8)
#define 	BLUE(pixel)	((pixel & 0x00ff0000L) >> 16)

/*
Model3D::Face Constructor Section
*/

Model3D::Face::Face():
	rightWorldPoints(), red(), green(), blue(),
	eulerAngles(3), displacement(3), massCenter(3),
	transformed()
{
	cylinders = NULL;
	
	trace("Model3D::Face::Face", "", stateString());
}

/*
Model3D::Face Method Section
*/

void Model3D::Face::computeMassCenter()
{
	trace("Model3D::computeMassCenter", "", stateString());
	
	massCenter.at(0) = 0.0;
	massCenter.at(1) = 0.0;
	massCenter.at(2) = 0.0;

	double n = getSize();
	for (int i = 0; i < n; i += 3)
	{
		massCenter.at(0) += rightWorldPoints.at(i + 0);
		massCenter.at(1) += rightWorldPoints.at(i + 1);
		massCenter.at(2) += rightWorldPoints.at(i + 2);
	}

	massCenter.at(0) /= (double) n / 3.0;
	massCenter.at(1) /= (double) n / 3.0;
	massCenter.at(2) /= (double) n / 3.0;
}

void Model3D::Face::transformCylinder(CYLINDER& cylinder)
{
	double center[] = {cylinder.x, cylinder.y, cylinder.z};
	double centerTransformed[3];

	TransformPoints(1, centerTransformed, center, &(eulerAngles[0]), &(displacement[0]), &(massCenter[0]));

	cylinder.x = centerTransformed[0];
	cylinder.y = centerTransformed[1];
	cylinder.z = centerTransformed[2];
}

void Model3D::Face::copyCylinders(CYLINDER_LIST* list, int step)
{
	trace("Model3D::Face::copyCylinders", string() + "list, " + step, stateString());

	if (cylinders == NULL)
		throw runtime_error("Could not copy face cylinders: cylinder list not allocated.");

	int j = 0, n = cylinder_list_size(cylinders);
	if (n == 0)
		throw runtime_error("Could not copy face cylinders: empty cylinder list.");

	for (j = (step > 0 ? 0 : n - 1); 0 <= j && j < n; j += step)
	{
		CYLINDER cylinder = *(cylinder_list_get(cylinders, j));
		transformCylinder(cylinder);
		cylinder_list_append(list, cylinder);
	}

	log(string() + "CYLINDER_LIST{size(" + cylinder_list_size(list) + ")}");
}

void Model3D::Face::extractPointCloud(vector<double>& points, vector<GLubyte> red, vector<GLubyte> green, vector<GLubyte> blue)
{
	trace("Model3D::Face::extractPointCloud", "points, red, green, blue", stateString());

	int n = rightWorldPoints.size();
	points.resize(n);
	TransformPoints(n / 3, &(points[0]), &(rightWorldPoints[0]), &(eulerAngles[0]), &(displacement[0]), &(massCenter[0]));
	
	red   = this->red;
	green = this->green;
	blue  = this->blue;
}

CYLINDER& Model3D::Face::transformedCylinder(int index)
{
	transformed[index] = *cylinder_list_get(cylinders, index);
	transformCylinder(transformed[index]);
	return transformed[index];
}

CYLINDER& Model3D::Face::highestCylinder()
{
	trace("Model3D::Face::highestCylinder", "", stateString());
	
	int n = cylinder_list_size(cylinders);
	double distance_highest = -DBL_MAX;
	int index_highest = -1;
	for (int i = 0; i < n; i++)
	{
		CYLINDER& cylinder = transformedCylinder(i);
		double distance = cylinder.y + cylinder_radius(&cylinder);
		if (distance > distance_highest)
		{
			distance_highest = distance;
			index_highest = i;
		}
	}
	
	if (index_highest == -1)
		throw runtime_error(string() + "Could not find highest cylinder in CYLINDER_LIST{size(" + n + ")}");

	return transformedCylinder(index_highest);
}

CYLINDER& Model3D::Face::lowestCylinder()
{
	trace("Model3D::Face::lowestCylinder", "", stateString());
	
	int n = cylinder_list_size(cylinders);
	double distance_lowest = DBL_MAX;
	int index_lowest = -1;
	for (int i = 0; i < n; i++)
	{
		CYLINDER& cylinder = transformedCylinder(i);
		double distance = cylinder.y - cylinder_radius(&cylinder);
		if (distance < distance_lowest)
		{
			distance_lowest = distance;
			index_lowest = i;
		}
	}
	
	if (index_lowest == -1)
		throw runtime_error(string() + "Could not find highest cylinder in CYLINDER_LIST{size(" + n + ")}");

	return transformedCylinder(index_lowest);
}

void Model3D::Face::removeCylinder(int index)
{
	trace("Model3D::Face::removeCylinder", string() + index, stateString());

	if (cylinders == NULL)
		return;
	
	cylinder_list_delete(cylinders, index);
}

void Model3D::Face::center()
{
	trace("Model3D::Face::center", "", stateString());

	displacement.at(MODEL_3D_X_AXIS) = 0.0;
	displacement.at(MODEL_3D_Y_AXIS) = 0.0;
	displacement.at(MODEL_3D_Z_AXIS) = 0.0;
}

void Model3D::Face::rotate(int axis, double value)
{
	trace("Model3D::Face::rotate", string() + axis + ", " + value, stateString());

	eulerAngles.at(axis) += value;
}

void Model3D::Face::straight()
{
	trace("Model3D::Face::straight", "", stateString());

	eulerAngles.at(MODEL_3D_X_AXIS) = 0.0;
	eulerAngles.at(MODEL_3D_Y_AXIS) = 0.0;
	eulerAngles.at(MODEL_3D_Z_AXIS) = 0.0;
}

void Model3D::Face::translate(int axis, double value)
{
	trace("Model3D::Face::translate", string() + axis + ", " + value, stateString());

	displacement.at(axis) += value;
}

string Model3D::Face::stateString()
{
	return string() + "id(" + ((int) this) + "), cylinders(" + getCylindersSize() + ")";
}

string Model3D::Face::toString()
{
	return "Model3D::Face{" + stateString() + "}";
}

/*
Model3D::Face Input / Output Section
*/

void Model3D::Face::loadConfig(string basename)
{
	trace("Model3D::Face::loadConfig", "\"" + basename + "\"", stateString());

	eulerAngles.at(0) = ConfigDefault::doubleValue(basename + ".eulerAngles.alpha", 0.0);
	eulerAngles.at(1) = ConfigDefault::doubleValue(basename + ".eulerAngles.betha", 0.0);
	eulerAngles.at(2) = ConfigDefault::doubleValue(basename + ".eulerAngles.gama", 0.0);

	displacement.at(0) = ConfigDefault::doubleValue(basename + ".displacement.x", 0.0);
	displacement.at(1) = ConfigDefault::doubleValue(basename + ".displacement.y", 0.0);
	displacement.at(2) = ConfigDefault::doubleValue(basename + ".displacement.z", 0.0);
}

void Model3D::Face::saveConfig(string basename)
{
	trace("Model3D::Face::saveConfig", "\"" + basename + "\"", stateString());

	ConfigDefault::setSetting(basename + ".eulerAngles.alpha", eulerAngles.at(0));
	ConfigDefault::setSetting(basename + ".eulerAngles.betha", eulerAngles.at(1));
	ConfigDefault::setSetting(basename + ".eulerAngles.gama",  eulerAngles.at(2));

	ConfigDefault::setSetting(basename + ".displacement.x", displacement.at(0));
	ConfigDefault::setSetting(basename + ".displacement.y", displacement.at(1));
	ConfigDefault::setSetting(basename + ".displacement.z", displacement.at(2));
}

/*
Model3D::Face Property Section
*/

CYLINDER_LIST* Model3D::Face::getCylinders()
{
	return cylinders;
}

CYLINDER& Model3D::Face::getCylinders(int index)
{
	CYLINDER& cylinder = *cylinder_list_get(cylinders, index);
	return cylinder;
}

void Model3D::Face::setCylinders(CYLINDER_LIST* cylinders)
{
	trace("Model3D::Face::setCylinders", "cylinders", stateString());

	this->cylinders = cylinders;
}

int Model3D::Face::getCylindersSize()
{
	return (cylinders != NULL ? cylinder_list_size(cylinders) : 0);
}

void Model3D::Face::setPointCloud(PointCloud& cloud)
{
	trace("Model3D::Face::setPointCloud", "cloud", stateString());

	int points = cloud.size();
	vector<double> leftWorldPoints(3*points);
	vector<double>& leftPoints = cloud.getLeftPoints();
	vector<double>& rightPoints = cloud.getRightPoints();

	setSize(points);

	StereoTriangulation(points, &(leftWorldPoints[0]), &(rightWorldPoints[0]), &(leftPoints[0]), &(rightPoints[0]));

	for (int i = 0; i < points; i++)
	{
		int color = cloud.getPointColor(i);
		red.at(i) = RED(color);
		green.at(i) = GREEN(color);
		blue.at(i) = BLUE(color);
	}

	computeMassCenter();
}

int Model3D::Face::getSize()
{
	trace("Model3D::Face::getSize", "", stateString());

	return red.size();
}

void Model3D::Face::setSize(int size)
{
	trace("Model3D::Face::setSize", string() + size, stateString());

	if (size != getSize())
	{
		rightWorldPoints.resize(3 * size);
		red.resize(size);
		green.resize(size);
		blue.resize(size);
	}
}

/*
Constructor Section
*/

Model3D::Model3D(): faces(NUM_CAMERA_PAIRS)
{
	solidVolume = INVALID_VOLUME;
	stereoVolume = INVALID_VOLUME;
}

/*
Method Section
*/

void Model3D::computeSolidVolume()
{
	trace("Model3D::computeSolidVolume");

	CYLINDER_LIST *list = getAllCylinders();
	solidVolume = 0.0;
	if (list == NULL)
	{
		solidVolume = INVALID_VOLUME;
		throw runtime_error("Failed gathering model cylinders");
	}

	solidVolume = robot_neuron_network_execute_circle_packing(list);
	cylinder_list_free(list);

	log(string() + "Model3D{solidVolume(" + solidVolume + ")}");
}

void Model3D::computeStereoVolume()
{
	trace("Model3D::computeStereoVolume");

	// O comprimento, largura e altura da pilha, em milímetros.
	height = compute_average_height(faces[0], faces[1], faces[2]);
	width  = compute_average_width(faces[0], faces[2]);
	length = compute_average_length(faces[0], faces[1], faces[2]);

	// O volume estéreo, em metros cúbicos.
	stereoVolume = length * width * height / pow(10, 9);

	log(string() + "Model3D{" +
		"length(" + length + "), " +
		"width(" + width + "), " +
		"height(" + height + "), " +
		"stereoVolume(" + stereoVolume + ")}");
}

void Model3D::center(int index)
{
	Face& face = faces.at(index);
	face.center();
}

void Model3D::rotate(int index, int axis, double value)
{
	Face& face = faces.at(index);
	face.rotate(axis, value);
}

void Model3D::straight(int index)
{
	Face& face = faces.at(index);
	face.straight();
}

void Model3D::translate(int index, int axis, double value)
{
	Face& face = faces.at(index);
	face.translate(axis, value);
}

/*
Property Section
*/

CYLINDER_LIST* Model3D::getAllCylinders()
{
	CYLINDER_LIST *list = alloc_cylinder_list();
	for (int i = 0, step = 1; i < NUM_CAMERA_PAIRS; i++, step = 1)
	{
		Face& face = faces.at(i);
		face.copyCylinders(list, step);
	}

	return list;
}

Model3D::Face& Model3D::getFace(int index)
{
	return faces.at(index);
}

double Model3D::getLength()
{
	if (stereoVolume == INVALID_VOLUME)
		computeStereoVolume();

	return length;
}

double Model3D::getHeight()
{
	if (stereoVolume == INVALID_VOLUME)
		computeStereoVolume();

	return height;
}

double Model3D::getWidth()
{
	if (stereoVolume == INVALID_VOLUME)
		computeStereoVolume();

	return width;
}

double Model3D::getSolidVolume()
{
	if (solidVolume == INVALID_VOLUME)
		computeSolidVolume();

	return solidVolume;
}

double Model3D::getStereoVolume()
{
	if (stereoVolume == INVALID_VOLUME)
		computeStereoVolume();

	return stereoVolume;
}

void Model3D::setCylinders(int index, CYLINDER_LIST *cylinders)
{
	trace("Model3D::setCylinders", string() + index + ", cylinders");

	solidVolume = INVALID_VOLUME;
	stereoVolume = INVALID_VOLUME;

	Face& face = faces.at(index);
	face.setCylinders(cylinders);
}

void Model3D::setPointCloud(int index, PointCloud& cloud)
{
	trace("Model3D::setPointCloud", string() + index + ", cloud");

	solidVolume = INVALID_VOLUME;
	stereoVolume = INVALID_VOLUME;

	Face& face = faces.at(index);
	face.setPointCloud(cloud);
}
