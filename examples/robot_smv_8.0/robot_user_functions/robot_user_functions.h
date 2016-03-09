#ifndef __ROBOT_USER_FUNCTIONS_H
#define __ROBOT_USER_FUNCTIONS_H

// Includes
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include "filter.h"
#include "../robot.h"
#include "matlab_scripts/stereo_api.h"

// Definitions
#define INVALID_SOCKET			-1

#define ST_WAIT				0
#define ST_GET_IMAGE			1

#define MSG_SNAP_MONO_256		"snap_mono_256"
#define MSG_SNAP_332			"snap_332"
#define MSG_SNAP_MONO_8			"snap_mono_8"

#define COLOR_MONO_256			0
#define COLOR_332				1
#define COLOR_MONO_8			2

#define INPUT_IMAGE_LEFT		0
#define INPUT_IMAGE_RIGHT		1

#define MAX_PACKAGE				100000

#define POINT_SIZE				2

#define NUM_SAMPLES				5

// Macros
#define ERRO_CONEXAO    		{printf("Nao foi possivel estabelecer conexao com o servidor de imagens. Programa abortado!\n"); exit(1);}
#define closesocket(s)			close(s);

#ifndef M_PI
#define M_PI                            3.1415926535897932384626433832795f
#endif

#define DRAWSQUARE(x, y, w, h) \
{   glBegin(GL_LINES); glVertex2i ((x), (y)); glVertex2i ((x)+(w), (y)); glEnd (); \
    glBegin(GL_LINES); glVertex2i ((x)+(w), (y)); glVertex2i ((x)+(w), (y)+(h)); glEnd (); \
    glBegin(GL_LINES); glVertex2i ((x)+(w), (y)+(h)); glVertex2i ((x), (y)+(h)); glEnd (); \
    glBegin(GL_LINES); glVertex2i ((x), (y)+(h)); glVertex2i ((x), (y)); glEnd (); \
}

#define DRAWCROSS(x, y, w, h) \
{   glBegin(GL_LINES); glVertex2i ((x-w/50), (y)); glVertex2i ((x+w/50), (y)); glEnd (); \
    glBegin(GL_LINES); glVertex2i ((x), (y-h/50)); glVertex2i ((x), (y+h/50)); glEnd (); \
}

#define DRAW_XY_MARK(x, y, w, h) \
{   glBegin(GL_LINES); glVertex2i ((x), (h)); glVertex2i ((x), (h+h/20)); glEnd (); \
    glBegin(GL_LINES); glVertex2i ((x), (0)); glVertex2i ((x), (-h/20)); glEnd (); \
    glBegin(GL_LINES); glVertex2i ((w), (y)); glVertex2i ((w+h/20), (y)); glEnd (); \
    glBegin(GL_LINES); glVertex2i ((0), (y)); glVertex2i ((-h/20), (y)); glEnd (); \
}

#define DRAWLINE(x0, y0, x1, y1) \
{   glBegin(GL_LINES); glVertex2f (x0, y0); glVertex2f (x1, y1); glEnd (); \
}

#define XMAP2IMAGE(xMap, map, image) \
(double) ( xMap * image->dimentions.x) / (double) map->dimentions.x;

#define YMAP2IMAGE(yMap, map, image) \
(double) ( yMap * image->dimentions.y) / (double) map->dimentions.y;

#define XIMAGE2MAP(xImage, map, image) \
(double) ( xImage * map->dimentions.x) / (double) image->dimentions.x;

#define YIMAGE2MAP(yImage, map, image) \
(double) ( yImage * map->dimentions.y) / (double) image->dimentions.y;

#define POW2(a) ((a)*(a))

// Structs
typedef int SOCKET;

typedef struct
{
	double val;
	double conf;	// confidence rate
	int pos;
} SAMPLES;
	
typedef struct
{
	char *neg_slope;
	SAMPLES **samples;
} DISP_DATA;

// Prototypes
void set_vergence (void);
void display_points (int num_points, double *left_points, double *right_points);
void project_points (int num_points, double *p_dblRightWorldPoint, double *left_points, double *right_points);
void test_StereoTriangulation_CameraProjection (void);
void exchange_to_disparity_map ();
void update_map ();
int outside_working_area (int *g_pWorkingArea, int x, int y);

// Exportable Variables
extern double g_dblNearVergenceCutPlane;
extern double g_dblFarVergenceCutPlane;

extern int g_nCurrDisparity;

#endif

