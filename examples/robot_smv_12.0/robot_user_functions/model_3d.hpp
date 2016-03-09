#ifndef __MODEL_3D_H
#define __MODEL_3D_H

#include <GL/glut.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "cylinder_list.h"

#ifdef __cplusplus
}
#endif

/*
Seção de Constantes
*/

#define NUM_CAMERA_PAIRS 3

#define MODEL_3D_X_AXIS 0
#define MODEL_3D_Y_AXIS 1
#define MODEL_3D_Z_AXIS 2

/*
Seção de Tipos de Dados
*/

struct _model_3d_face
{
	double *rightWorldPoints;

	GLubyte *red, *green, *blue;

	int size;

	double eulerAngles[3];

	double displacement[3];

	double massCenter[3];

	CYLINDER_LIST *cylinders;
};

typedef struct _model_3d_face MODEL_3D_FACE;

struct _model_3d
{
	MODEL_3D_FACE faces[NUM_CAMERA_PAIRS];

	int workingArea[4];
	
	double solidVolume;
	
	double stereoVolume;
	
	double length, height, width;
};

typedef struct _model_3d MODEL_3D;

#ifdef __cplusplus
extern "C" {
#endif

/*
Seção de Construtores & Inicializadores
*/

void model_3d_init(MODEL_3D *model);

/*
Seção de Funções
*/

void model_3d_fill_face(MODEL_3D *model, int face, int points, double *leftPoints, double *rightPoints, int *colors);

void model_3d_rotate_face(MODEL_3D *model, int face, int axis, double value);

void model_3d_straight_face(MODEL_3D *model, int face);

void model_3d_translate_face(MODEL_3D *model, int face, int axis, double value);

void model_3d_center_face(MODEL_3D *model, int face);

CYLINDER_LIST *model_3d_all_cylinders(MODEL_3D *model);

void model_3d_stereo_volume(MODEL_3D *model);

/*
Seção de Propriedades
*/

void model_3d_set_cylinders(MODEL_3D *model, int index, CYLINDER_LIST *cylinders);

#ifdef __cplusplus
}
#endif

#endif
