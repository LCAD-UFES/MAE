#include <stdlib.h>
#include "model_3d.hpp"
#include "stereo_api.h"

extern "C"
{
#include "stereo_volume.h"
}

#define 	RED(pixel)	((pixel & 0x000000ffL) >> 0)
#define 	GREEN(pixel)	((pixel & 0x0000ff00L) >> 8)
#define 	BLUE(pixel)	((pixel & 0x00ff0000L) >> 16)

/*
Seção de Construtores & Inicializadores
*/

void model_3d_init(MODEL_3D *model)
{
	MODEL_3D_FACE *faces = model->faces;
	for (int i = 0; i < NUM_CAMERA_PAIRS; i++)
	{
		faces[i].cylinders = NULL;
		faces[i].rightWorldPoints = NULL;
		faces[i].size = 0;

		for (int j = 0; j < 3; j++)
		{
			faces[i].eulerAngles[j]  = 0.0;
			faces[i].displacement[j] = 0.0;
		}
	}
}

/*
Seção de Funções
*/

int model_3d_face_init(MODEL_3D_FACE *face, int size)
{
	if (face->rightWorldPoints == NULL)
	{
		face->rightWorldPoints = (double *) malloc(size * 3 * sizeof (double));

		face->red   = (GLubyte*) malloc(size * sizeof(GLubyte));
		face->green = (GLubyte*) malloc(size * sizeof(GLubyte));
		face->blue  = (GLubyte*) malloc(size * sizeof(GLubyte));

		return 1;
	}
	else
		return 0;
}

void model_3d_face_compute_mass_center(MODEL_3D_FACE *face)
{
	double *rightWorldPoints = face->rightWorldPoints;
	double *massCenter = face->massCenter;
	int n = face->size;

	massCenter[0] = 0.0;
	massCenter[1] = 0.0;
	massCenter[2] = 0.0;

	for (int i = 0; i < n; i += 3)
	{
		massCenter[0] += rightWorldPoints[i + 0];
		massCenter[1] += rightWorldPoints[i + 1];
		massCenter[2] += rightWorldPoints[i + 2];
	}

	massCenter[0] /= (double) n / 3.0;
	massCenter[1] /= (double) n / 3.0;
	massCenter[2] /= (double) n / 3.0;
}

void model_3d_fill_face(MODEL_3D *model, int index, int points, double *leftPoints, double *rightPoints, int *colors)
{
	MODEL_3D_FACE *face = &(model->faces[index]);
	int compute_mass_center = model_3d_face_init(face, points);

	double *leftWorldPoints = (double *) malloc(points * 3 * sizeof (double));
	StereoTriangulation(points, leftWorldPoints, face->rightWorldPoints, leftPoints, rightPoints);
	free(leftWorldPoints);

	face->size = points * 3;

	for (int i = 0; i < points; i++)
	{
		int color = colors[i];
		face->red[i] = RED(color);
		face->green[i] = GREEN(color);
		face->blue[i] = BLUE(color);
	}

	if (compute_mass_center)
		model_3d_face_compute_mass_center(face);
}

void model_3d_rotate_face(MODEL_3D *model, int face, int axis, double value)
{
	model->faces[face].eulerAngles[axis] += value;
}

void model_3d_straight_face(MODEL_3D *model, int face)
{
	model->faces[face].eulerAngles[MODEL_3D_X_AXIS] = 0.0;
	model->faces[face].eulerAngles[MODEL_3D_Y_AXIS] = 0.0;
	model->faces[face].eulerAngles[MODEL_3D_Z_AXIS] = 0.0;
}

void model_3d_translate_face(MODEL_3D *model, int face, int axis, double value)
{
	model->faces[face].displacement[axis] += value;
}

void model_3d_center_face(MODEL_3D *model, int face)
{
	model->faces[face].displacement[MODEL_3D_X_AXIS] = 0.0;
	model->faces[face].displacement[MODEL_3D_Y_AXIS] = 0.0;
	model->faces[face].displacement[MODEL_3D_Z_AXIS] = 0.0;
}

void cylinder_transform(CYLINDER *cylinder, MODEL_3D_FACE *face)
{
	double center[] = {cylinder->x, cylinder->y, cylinder->z};
	double centerTransformed[3];

	double *eulerAngles  = face->eulerAngles;
	double *displacement = face->displacement;
	double *massCenter   = face->massCenter;

	TransformPoints(1, centerTransformed, center, eulerAngles, displacement, massCenter);
	
	cylinder->x = centerTransformed[0];
	cylinder->y = centerTransformed[1];
	cylinder->z = centerTransformed[2];
}

void model_3d_face_copy_cylinders(MODEL_3D_FACE *face, CYLINDER_LIST *list, int step)
{
	CYLINDER_LIST *source = face->cylinders;
	if (source == NULL) return;

	int j = 0, n = cylinder_list_size(source);
	for (j = (step > 0 ? 0 : n - 1); 0 <= j && j < n; j += step)
	{
		CYLINDER cylinder = *(cylinder_list_get(source, j));
		cylinder_transform(&cylinder, face);
		cylinder_list_insert(list, cylinder);
	}
}

CYLINDER_LIST *model_3d_all_cylinders(MODEL_3D *model)
{
	CYLINDER_LIST *list = alloc_cylinder_list();
	MODEL_3D_FACE *faces = model->faces;

	for (int i = 0; i < NUM_CAMERA_PAIRS - 1; i++)
		model_3d_face_copy_cylinders(&(faces[i]), list, 1);
	
	model_3d_face_copy_cylinders(&(faces[NUM_CAMERA_PAIRS - 1]), list, -1);

	return list;
}

double model_3d_stereo_volume(MODEL_3D *model)
{
	CYLINDER_LIST *face_left  = alloc_cylinder_list();
	CYLINDER_LIST *face_top   = alloc_cylinder_list();
	CYLINDER_LIST *face_right = alloc_cylinder_list();
	MODEL_3D_FACE *faces = model->faces;

	model_3d_face_copy_cylinders(&(faces[0]), face_left,  1);
	model_3d_face_copy_cylinders(&(faces[1]), face_top,   1);
	model_3d_face_copy_cylinders(&(faces[2]), face_right, 1);

	return compute_stereo_volume(face_left, face_top, face_right);
}

/*
Seção de Propriedades
*/

void model_3d_set_cylinders(MODEL_3D *model, int index, CYLINDER_LIST *cylinders)
{
	model->faces[index].cylinders = cylinders;
}
