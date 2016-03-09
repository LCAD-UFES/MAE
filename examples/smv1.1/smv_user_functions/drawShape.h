#ifndef _DRAWSHAPE_H
#define _DRAWSHAPE_H

#include "calibrate.h"

#define TURN_ON				1
#define TURN_OFF			0
#define MAX_POINTS 			1000
#define INITIAL_BREAKLINEFACTOR		3
#define INITIAL_STACKWIDTH		4.5
#define MIN_POINTS_TO_GENERATE_PLANE 	3
#define MIN_POINTS_TO_GENERATE_SPLINE 	3
#define MIN_POINTS_TO_GENERATE_LINE 	2
#define MIN_LINES_TO_GENERATE_PLANE 	3
#define SPLINE_COMMAND 			"Splines(%d) = {"
#define LINE_COMMAND 			"Line(%d) = {%d, %d};\n"
#define LAST_OBJECT			"%d};\n"
#define LINE_LOOP_COMMAND		"Line Loop(%d) = {"
#define OBJECT				"%d, "
#define GMSH_GEO_FILENAME		"stockPileData.geo"
#define GMSH_MSH_FILENAME		"stockPileData.msh"

#define ACCURACY_VERGENCE			0
#define FAST_VERGENCE				1

/* generate shape status */
#define POINTS_NOT_SELECTED		1
#define POINTS_IN_SELECTION		2
#define VERGENCE_STARTED		4
#define VERGENCE_COMPUTED		8
#define AREA_CALCULATED			16
#define VOLUME_CALCULATED		32

#define DRAWCROSS(x, y, w, h) \
{   glBegin(GL_LINES); glVertex2i ((x-w/50), (y)); \
    		       glVertex2i ((x+w/50), (y)); \
    glEnd (); \
    glBegin(GL_LINES); glVertex2i ((x), (y-h/50)); \
    		       glVertex2i ((x), (y+h/50)); \
    glEnd (); \
}
	
#define DRAW_XY_MARK(x, y, w, h) \
{   glBegin(GL_LINES); glVertex2i ((x), (h)); \
    		       glVertex2i ((x), (h+h/20)); \
    glEnd (); \
    glBegin(GL_LINES); glVertex2i ((x), (0)); \
    		       glVertex2i ((x), (-h/20)); \
    glEnd (); \
    glBegin(GL_LINES); glVertex2i ((w), (y)); \
    		       glVertex2i ((w+h/20), (y)); \
    glEnd (); \
    glBegin(GL_LINES); glVertex2i ((0), (y)); \
    		       glVertex2i ((-h/20), (y)); \
    glEnd (); \
}

#define DRAWSQUARE(x, y, w, h) \
    glBegin(GL_LINES); glVertex2i ((x), (y)); glVertex2i ((x)+(w), (y)); glEnd (); \
    glBegin(GL_LINES); glVertex2i ((x)+(w), (y)); glVertex2i ((x)+(w), (y)+(h)); glEnd (); \
    glBegin(GL_LINES); glVertex2i ((x)+(w), (y)+(h)); glVertex2i ((x), (y)+(h)); glEnd (); \
    glBegin(GL_LINES); glVertex2i ((x), (y)+(h)); glVertex2i ((x), (y)); glEnd ()

struct _selection_points
{
	int		maxSize;
	int		lastDominantEyePoint;
	int		generateShapeStatus;
	XY_PAIR		*dominantEyePoints;
	XY_PAIR		*nonDominantEyePoints;
};

typedef struct _selection_points SELECTION_POINTS;

struct _map_point
{
	WORLD_POINT	point;
	int		status;
};

typedef struct _map_point MAP_POINT;

struct _shape_map
{
	int		maxSize;
	int		totalPoints;
	MAP_POINT	*map_point;
	float		max_Y_coordinate;
	float		min_Y_coordinate;
};

typedef struct _shape_map SHAPE_MAP;

struct _node_list
{
	int		maxSize;
	WORLD_POINT	*nodesVector;
	float		max_Y_coordinate;
	float		min_Y_coordinate;
};

typedef struct _node_list NODE_LIST;

struct _triangle
{
	int	indexA;
	int	indexB;
	int	indexC;
};

typedef struct _triangle TRIANGLE;

struct _element_list
{
	int		maxSize;
	TRIANGLE	*elementsVector;
};

typedef struct _element_list ELEMENT_LIST;

void ShapeMapInitialize (SHAPE_MAP *shape_map, int maxSize);
void ShapeMapDispose(SHAPE_MAP *shape_map);
WORLD_POINT GetShapeGeometricCenter(SHAPE_MAP shape_map);
void memorizeWorldPoint (WORLD_POINT world_point, SHAPE_MAP *shape_map);
void prepareMap (SHAPE_MAP *shape_map);
void generateGeoFile (SHAPE_MAP *shape_map, int breakLineFactor);
void drawShape (SHAPE_MAP shape_map, int volumeWinID);
void drawShapeBoundary (SHAPE_MAP shape_map);
void nodeListInitialize (NODE_LIST *node_list);
void elementListInitialize (ELEMENT_LIST *element_list);
float calculateVolume (float area, float width);
float computeArea ();

NODE_LIST node_list;
ELEMENT_LIST element_list;

int breakLineFactor;
float stackWidth;
float shapeArea;
float stockPileVolume;
int processStatus;

SELECTION_POINTS selectedPoints;


#endif
