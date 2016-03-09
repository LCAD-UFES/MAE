#include "geometric_operations.h"
#include <math.h>
#include <stdio.h>

double
distance_3D (double *point_3D_1, double *point_3D_2)
{
	double distance = sqrt(	(point_3D_1[0] - point_3D_2[0])*(point_3D_1[0] - point_3D_2[0]) +
			(point_3D_1[1] - point_3D_2[1])*(point_3D_1[1] - point_3D_2[1]) +
			(point_3D_1[2] - point_3D_2[2])*(point_3D_1[2] - point_3D_2[2])
			);
	return distance;
}

