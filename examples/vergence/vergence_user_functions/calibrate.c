#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "calibrate.h"

IMAGE_COORDINATE 
calculate_euclidean_coordinates(IMAGE_COORDINATE point, XY_PAIR image_dimentions)
{
	IMAGE_COORDINATE euclidean_point;
	euclidean_point.x = point.x - ((float) image_dimentions.x * 0.5);
	euclidean_point.y = point.y - ((float)image_dimentions.y * 0.5);
	return euclidean_point;
}

double
calculate_euclidean_distance (WORLD_POINT start_point, WORLD_POINT end_point)
{
	return sqrt ((start_point.x - end_point.x) * (start_point.x - end_point.x) +
		     (start_point.y - end_point.y) * (start_point.y - end_point.y) +
		     (start_point.z - end_point.z) * (start_point.z - end_point.z));
}

WORLD_POINT
calculate_euclidean_world_point (IMAGE_COORDINATE left_point, IMAGE_COORDINATE right_point, XY_PAIR image_dimentions, 
				 double focal_distance, double camera_distance)
{
	double theta_l, theta_r;  	/* Vergence Angles */
	double z; 			/* Projection of r in World in zx plane */
	double lambda; 			/* Gaze Angle */
	double theta; 
	WORLD_POINT thePoint; 		/*The Euclidean World Point */
	IMAGE_COORDINATE left_point_coordinate;
	IMAGE_COORDINATE right_point_coordinate;
	
	left_point_coordinate = calculate_euclidean_coordinates (left_point, image_dimentions);
	right_point_coordinate = calculate_euclidean_coordinates (right_point, image_dimentions);
	
	theta_l = atan (-left_point_coordinate.x / focal_distance);
	theta_r = atan (right_point_coordinate.x / focal_distance);
	
	z = sqrt (camera_distance * camera_distance *
		 ((sin (theta_l - theta_r) * sin (theta_l - theta_r) / (4.0 * sin (theta_l + theta_r) * sin (theta_l + theta_r))) +
		 (cos (theta_l) * cos (theta_l) * cos (theta_r) * cos (theta_r) / (sin (theta_l + theta_r) * sin (theta_l + theta_r)))));
	
	lambda = atan (sin (theta_l - theta_r) / (2.0 * cos (theta_l) * cos (theta_r))); 
	theta = atan (right_point_coordinate.y / focal_distance);
	
	thePoint.x = -z * sin (lambda);
	thePoint.z = z * cos (lambda);
	thePoint.y = thePoint.z * tan (theta);
	
	return thePoint;
}

WORLD_POINT
calculate_world_point (IMAGE_COORDINATE left_point, IMAGE_COORDINATE right_point, XY_PAIR image_dimentions, 
				 double focal_distance, double camera_distance)
{
	//double theta_l, theta_r;  	/* Vergence Angles */
	//double z; 			/* Projection of r in World in zx plane */
	//double lambda; 			/* Gaze Angle */
	WORLD_POINT thePoint; 		/*The Euclidean World Point */
	IMAGE_COORDINATE left_point_coordinate;
	IMAGE_COORDINATE right_point_coordinate;
	
	left_point_coordinate = calculate_euclidean_coordinates (left_point, image_dimentions);
	right_point_coordinate = calculate_euclidean_coordinates (right_point, image_dimentions);
	
	thePoint.x = camera_distance * (right_point_coordinate.x + left_point_coordinate.x) / (2.0 * (left_point_coordinate.x - right_point_coordinate.x));
	thePoint.y = camera_distance * right_point_coordinate.y / (left_point_coordinate.x - right_point_coordinate.x);
	thePoint.z = focal_distance * camera_distance / (left_point_coordinate.x - right_point_coordinate.x);
	return thePoint;
}

float
get_image_focal_distance (float calibrate_focal_distance, int calibrate_image_width, int image_width)
{
	return calibrate_focal_distance * ((float) image_width) / ((float) calibrate_image_width);
}

