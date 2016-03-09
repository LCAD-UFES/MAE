#include "model_3d_io.hpp"

#include <iostream>
#include <sstream>
#include <string>

#include "config_default.hpp"
#include "cylinder_list.h"
#include "crobot_smv.h"
#include "stereo_api.h"
#include "viewer.hpp"

#include <stdio.h>
#include <stdlib.h>

using std::ostringstream;

int model_3d_max_face_size(MODEL_3D *model)
{
	int size = 0;
	MODEL_3D_FACE *faces = model->faces;

	for (int i = 0; i < NUM_CAMERA_PAIRS; i++)
	{
		int n = faces[i].size;
		if (size < n)
			size = n;
	}
	
	return size;
}

void model_3d_view(MODEL_3D *model)
{
	int size = model_3d_max_face_size(model);
	double *rightWorldPointsTransformed = (double *) malloc(size * 3 * sizeof (double));
	MODEL_3D_FACE *faces = model->faces;

	ViewerEraseMap();

	for (int i = 0, m = get_current_camera_pair(); i <= m; i++)
	{
		double *rightWorldPoints = faces[i].rightWorldPoints;
		double *eulerAngles  = faces[i].eulerAngles;
		double *displacement = faces[i].displacement;
		double *massCenter   = faces[i].massCenter;
		int n = faces[i].size;
		
		TransformPoints(n/3, rightWorldPointsTransformed, rightWorldPoints, eulerAngles, displacement, massCenter);

		for (int j = 0, k = 0; j < n; j += 3, k++)
		{
			double x = rightWorldPointsTransformed[j + 0];
			double y = rightWorldPointsTransformed[j + 1];
			double z = rightWorldPointsTransformed[j + 2];

			GLubyte red   = faces[i].red[k];
			GLubyte green = faces[i].green[k];
			GLubyte blue  = faces[i].blue[k];

			// Updates the map and rotates the frame coordinates
			ViewerUpdateMap(0, z, -x, y, red, green, blue);
		}
		
		cylinder_list_view_transformed(faces[i].cylinders, eulerAngles, displacement, massCenter);
	}
	
	free(rightWorldPointsTransformed);
	
	ViewerRedisplay();
}


void model_3d_load_config(MODEL_3D *model)
{
	ConfigDefault::load();

	MODEL_3D_FACE *faces = model->faces;
	for (int i = 0; i < NUM_CAMERA_PAIRS; i++)
	{
		ostringstream o;
		o << "faces." << i;
		string face = o.str();
		
		double *eulerAngles  = faces[i].eulerAngles;
		eulerAngles[0] = ConfigDefault::doubleValue(face + ".eulerAngles.alpha", 0.0);
		eulerAngles[1] = ConfigDefault::doubleValue(face + ".eulerAngles.betha", 0.0);
		eulerAngles[2] = ConfigDefault::doubleValue(face + ".eulerAngles.gama", 0.0); 

		double *displacement = faces[i].displacement;
		displacement[0] = ConfigDefault::doubleValue(face + ".displacement.x", 0.0);
		displacement[1] = ConfigDefault::doubleValue(face + ".displacement.y", 0.0);
		displacement[2] = ConfigDefault::doubleValue(face + ".displacement.z", 0.0); 
		
		double *massCenter   = faces[i].massCenter;
		massCenter[0] = ConfigDefault::doubleValue(face + ".massCenter.x", 0.0);
		massCenter[1] = ConfigDefault::doubleValue(face + ".massCenter.y", 0.0);
		massCenter[2] = ConfigDefault::doubleValue(face + ".massCenter.z", 0.0);
	}
}

void model_3d_save_config(MODEL_3D *model)
{
	MODEL_3D_FACE *faces = model->faces;
	for (int i = 0; i < NUM_CAMERA_PAIRS; i++)
	{
		ostringstream o;
		o << "faces." << i;
		string face = o.str();

		double *eulerAngles  = faces[i].eulerAngles;
		ConfigDefault::setSetting(face + ".eulerAngles.alpha", eulerAngles[0]);
		ConfigDefault::setSetting(face + ".eulerAngles.betha", eulerAngles[1]);
		ConfigDefault::setSetting(face + ".eulerAngles.gama",  eulerAngles[2]); 
		
		double *displacement = faces[i].displacement;
		ConfigDefault::setSetting(face + ".displacement.x", displacement[0]);
		ConfigDefault::setSetting(face + ".displacement.y", displacement[1]);
		ConfigDefault::setSetting(face + ".displacement.z", displacement[2]); 

		double *massCenter   = faces[i].massCenter;
		ConfigDefault::setSetting(face + ".massCenter.x", massCenter[0]);
		ConfigDefault::setSetting(face + ".massCenter.y", massCenter[1]);
		ConfigDefault::setSetting(face + ".massCenter.z", massCenter[2]);
	}

	ConfigDefault::save();
}
