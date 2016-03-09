#include "model_3d_io.hpp"
#include "cylinder_list.h"
#include "robot_io.h"
#include "stereo_api.h"
#include "viewer.hpp"

#include <stdio.h>
#include <stdlib.h>

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
			
//			fprintf(stderr, "(%lf, %lf, %lf) = (%d, %d, %d)\n", x, y, z, red, green, blue);
		}

		cylinder_list_view_transformed(faces[i].cylinders, eulerAngles, displacement, massCenter);
	}
	
	free(rightWorldPointsTransformed);
}


void model_3d_load(MODEL_3D *model, FILE *file)
{
	MODEL_3D_FACE *faces = model->faces;
	for (int i = 0; i < NUM_CAMERA_PAIRS; i++)
	{
		double *eulerAngles  = faces[i].eulerAngles;
		double *displacement = faces[i].displacement;
		double *massCenter   = faces[i].massCenter;
		
		fscanf(file, "%lf %lf %lf\n", &(eulerAngles[0]),  &(eulerAngles[1]),  &(eulerAngles[2]));
		fscanf(file, "%lf %lf %lf\n", &(displacement[0]), &(displacement[1]), &(displacement[2]));
		fscanf(file, "%lf %lf %lf\n", &(massCenter[0]),   &(massCenter[1]),   &(massCenter[2]));
	}
}

void model_3d_save(MODEL_3D *model, FILE *file)
{
	MODEL_3D_FACE *faces = model->faces;
	for (int i = 0; i < NUM_CAMERA_PAIRS; i++)
	{
		double *eulerAngles  = faces[i].eulerAngles;
		double *displacement = faces[i].displacement;
		double *massCenter   = faces[i].massCenter;
		
		fprintf(file, "%lf %lf %lf\n", eulerAngles[0],  eulerAngles[1],  eulerAngles[2]);
		fprintf(file, "%lf %lf %lf\n", displacement[0], displacement[1], displacement[2]);
		fprintf(file, "%lf %lf %lf\n", massCenter[0],   massCenter[1],   massCenter[2]);
	}
}
