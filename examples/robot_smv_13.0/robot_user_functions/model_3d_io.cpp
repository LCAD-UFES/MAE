#include "model_3d_io.hpp"

#include <string>
#include <stdio.h>
#include <stdlib.h>

#include "config_default.hpp"
#include "cylinder_list.h"
#include "crobot_smv.h"

#include "logging.hpp"
using mae::logging::trace;

#include "stereo_api.h"
#include "viewer.hpp"

#include "primitives.hpp"
using mae::util::Primitives;

void model_3d_face_view_point_cloud(vector<double> points, vector<GLubyte> red, vector<GLubyte> green, vector<GLubyte> blue)
{
	trace("model_3d_face_view_point_cloud",
		string() + "points(" + points.size() + "), " +
			"red(" + red.size() + "), green(" + green.size() + "), blue(" + blue.size() + ")");

	for (int j = 0, k = 0, n = points.size(); j < n; j += 3, k++)
	{
		double x = points.at(j + 0);
		double y = points.at(j + 1);
		double z = points.at(j + 2);

		// Updates the map and rotates the frame coordinates
		ViewerUpdateMap(0, z, -x, y, red.at(k), green.at(k), blue.at(k));
	}
}

void model_3d_face_view_point_cloud(Model3D& model3d, int index)
{
	trace("model_3d_face_view_point_cloud", string() + "model3d, " + index);
	
	ConfigDefault::load();
	bool viewPointCloud = ConfigDefault::boolValue("smv.model.pointCloud.visible", true);
	if (!viewPointCloud) return;
		
	vector<double> rightWorldPoints;
	vector<GLubyte> red, green, blue;
	Model3D::Face& face = model3d.getFace(index);
	face.extractPointCloud(rightWorldPoints, red, green, blue);
	
}

void model_3d_face_view_cylinders(Model3D& model3d, int index)
{
	trace("model_3d_face_view_cylinders", string() + "model3d, " + index);
	
	ConfigDefault::load();
	bool viewCylinders = ConfigDefault::boolValue("smv.model.solid3d.visible", true);
	if (!viewCylinders) return;

	Model3D::Face& face  = model3d.getFace(index);
	for (int i = 0, n = face.getCylindersSize(); i < n; i++)
	{
		CYLINDER& cylinder = face.transformedCylinder(i);
		view_3D_cylinder(&cylinder, ARCS, CIRCLES);
	}
}


void model_3d_view(Model3D& model3d)
{
	trace("model_3d_view", "model3d");
	
	ViewerEraseMap();

	for (int i = 0, m = get_current_camera_pair(); i <= m; i++)
	{
		model_3d_face_view_point_cloud(model3d, i);
		model_3d_face_view_cylinders(model3d, i);
	}

	trace("ViewerRedisplay");
	ViewerRedisplay();
}

void model_3d_load_config(Model3D& model3d)
{
	ConfigDefault::load();
	for (int i = 0; i < NUM_CAMERA_PAIRS; i++)
	{
		string basename = string() + "faces." + i;
		Model3D::Face& face = model3d.getFace(i);
		face.loadConfig(basename);
	}
}

void model_3d_save_config(Model3D& model3d)
{
	for (int i = 0; i < NUM_CAMERA_PAIRS; i++)
	{
		string basename = string() + "faces." + i;
		Model3D::Face& face = model3d.getFace(i);
		face.saveConfig(basename);
	}

	ConfigDefault::save();
}
