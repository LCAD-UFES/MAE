#ifndef _CALIBRATE_H
#define _CALIBRATE_H

#include "mae.h"

struct _world_point
    { 
     	double 			x; 
     	double 			y;
	double			z;
	
    }; 
typedef struct _world_point WORLD_POINT;

struct _spherical_coordinate
    { 
     	double 			r; 
     	double 			theta;
	double			fi;
    }; 
typedef struct _spherical_coordinate SPHERICAL_COORDINATE;

struct _image_coordinate
    { 
     	double 			x; 
     	double 			y;
    }; 
typedef struct _image_coordinate IMAGE_COORDINATE;

IMAGE_COORDINATE calculate_euclidean_coordinates(IMAGE_COORDINATE point, XY_PAIR image_dimentions);
double calculate_euclidean_distance (WORLD_POINT start_point, WORLD_POINT end_point);
WORLD_POINT calculate_euclidean_world_point (IMAGE_COORDINATE left_point, IMAGE_COORDINATE right_point, XY_PAIR image_dimentions, double focal_distance, double camera_distance);
WORLD_POINT calculate_world_point (IMAGE_COORDINATE left_point, IMAGE_COORDINATE right_point, XY_PAIR image_dimentions, double focal_distance, double camera_distance);
float get_image_focal_distance (float calibrate_focal_distance, int calibrate_image_width, int image_width);
double distanceBetweenImagePoints (XY_PAIR start_point, XY_PAIR end_point);

#endif
