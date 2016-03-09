#include "robot_user_functions.h"
#include "stereo_volume.h"
#include "viewer.hpp"
#include "line.h"
#include <stdlib.h>
#include <float.h>
#include <math.h>

#define HALF_PI pi / 2.0
#define POSITION 75.0

/*
See http://www.geom.uiuc.edu/docs/reference/CRC-formulas/node52.html#eqdirectors

SQRT(a^2 + b^2 + c^2) = 1 for the unit vector.   
*/
LINE *
cylinder_center_line (CYLINDER *cylinder)
{
	double x0 = cylinder->x;
	double y0 = cylinder->y;
	double z0 = cylinder->z;
	double alpha = cylinder->alpha;
	double betha = cylinder->betha;

	return line_alloc2 (x0, y0, z0, alpha, betha);
}

double *
cylinder_center_point_at (CYLINDER *cylinder, double x)
{
	LINE *line = cylinder_center_line (cylinder);
	double c = (x - line->x0) / line->a;
	double y = c * line->b + line->y0;
	double z = c * line->c + line->z0;

	double *point = (double*) malloc (3*sizeof(double));
	point[0] = x;
	point[1] = y;
	point[2] = z;

	free (line);

	return point;
}

LINE *cylinder_list_compute_line(CYLINDER_LIST *list, double position)
{
	int i = 0, n = cylinder_list_size(list);
	CYLINDER *first = cylinder_list_get_first(list);
	double x0 = first->x + position;
	double y0 = first->y;
	double z0 = first->z;

	double *x = (double*) malloc(n * sizeof(double));
	double *y = (double*) malloc(n * sizeof(double));

//	printf("# (x0, y0, z0) = (%f, %f, %f)\n", x0, y0, z0);
	for (i = 0; i < n; i++)
	{
		CYLINDER *cylinder = cylinder_list_get(list, i);
		double *point = cylinder_center_point_at(cylinder, x0);
		x[i] = point[2];
		y[i] = point[1];
		
//		printf("%f\t\t\t%f\n", x[i], y[i]);
		
		free(point);
	}
	
	LINE *line = line_fit (x0, y0, z0, x, y, n);
	
	free(x);
	free(y);
	
	return line;
}

/*
See: http://mathforum.org/library/drmath/view/51734.html
*/
double *line_closest_point (LINE *line, double x, double y, double z)
{
	double x0 = line->x0, y0 = line->y0, z0 = line->z0;
	double u1 = line->a,  u2 = line->b,  u3 = line->c;

	double norm = (x - x0) * u1 + (y - y0) * u2 + (z - z0) * u3;
	double *point = (double*) malloc (3*sizeof(double));

	point[0] = x0 + norm * u1;
	point[1] = y0 + norm * u2;
	point[2] = z0 + norm * u3;

	return point;
}

/*
See: http://www.geom.uiuc.edu/docs/reference/CRC-formulas/node54.html#SECTION02430000000000000000
*/
LINE *
line_perpendicular (LINE *line, double x0, double y0, double z0)
{
	double *line_point = line_closest_point (line, x0, y0, z0);
	double x1 = line_point[0];
	double y1 = line_point[1];
	double z1 = line_point[2];

	LINE *perpendicular = line_alloc (x0, y0, z0, x1 - x0, y1 - y0, z1 - z0);

	free (line_point);

	return perpendicular;
}

CYLINDER *cylinder_list_find_closest(CYLINDER_LIST *list, LINE *line)
{
	CYLINDER *highest = cylinder_list_get_highest (list);
	CYLINDER *closest = NULL;

	double x0 = line->x0, y0 = highest->y + highest->diameter, z0 = line->z0;
	LINE *perpendicular = line_perpendicular (line, x0, y0, z0);
	
	int i = 0, n = cylinder_list_size (list);
	double shortest_distance = DBL_MAX;
	
//	line_3D_view (perpendicular, 1000);

	for (i = 0; i < n; i++)
	{
		CYLINDER *cylinder = cylinder_list_get (list, i);
		LINE *center_line = cylinder_center_line (cylinder);
		double distance = line_distance (perpendicular, center_line);
		if (distance < shortest_distance)
		{
			shortest_distance = distance;
			closest = cylinder;
		}
		
		free (center_line);
	}
	
	
//	line_3D_view (cylinder_center_line (nearest), 1000);
	
	return closest;
}

double
cylinder_compute_height(CYLINDER *cylinder, LINE *height_line)
{
	double *point = line_closest_point(height_line, cylinder->x, cylinder->y, cylinder->z);
	double height = sqrt(POW2(height_line->x0 - point[0]) +
	                     POW2(height_line->y0 - point[1]) +
			     POW2(height_line->z0 - point[2]));

	return height;
}

double cylinder_list_compute_height(CYLINDER_LIST *face_side, CYLINDER_LIST *face_top, double position)
{
	LINE *height_line = cylinder_list_compute_line(face_side, position);
	CYLINDER *first   = cylinder_list_get_first(face_side);
	CYLINDER *closest = cylinder_list_find_closest(face_top, height_line);
	double height = cylinder_compute_height(closest, height_line) + cylinder_radius(first);
	
//	line_3D_view (height_line, 1000);

	free(height_line);

	return height;
}

double compute_average_height(CYLINDER_LIST *face_left, CYLINDER_LIST *face_top, CYLINDER_LIST *face_right)
{
	double h1 = cylinder_list_compute_height(face_left,  face_top, -POSITION);
	double h2 = cylinder_list_compute_height(face_left,  face_top, 0.0);
	double h3 = cylinder_list_compute_height(face_left,  face_top, POSITION); 
	double h4 = cylinder_list_compute_height(face_right, face_top, -POSITION);
	double h5 = cylinder_list_compute_height(face_right, face_top, 0.0);
	double h6 = cylinder_list_compute_height(face_right, face_top, POSITION); 

	return (h1 + h2 + h3 + h4 + h5 + h6) / 6.0;
}

double compute_average_width(CYLINDER_LIST *face_left, CYLINDER_LIST *face_right)
{
	LINE *height_line = cylinder_list_compute_line(face_right, 0.0);
	int i = 0, n = cylinder_list_size(face_left);
	double width = 0.0;

	for (i = 0; i < n; i++)
	{
		CYLINDER *cylinder  = cylinder_list_get(face_left,  i);
		double x0 = cylinder->x;
		double y0 = cylinder->y;
		double z0 = cylinder->z;
		
		double *point = line_closest_point(height_line, x0, y0, z0);
		double distance = sqrt(POW2(point[0] - x0)
		                     + POW2(point[1] - y0)
				     + POW2(point[2] - z0));
		
		free(point);
		width += distance;
	}
	
	free(height_line);
	
	return width / n;
}

double
cylinder_list_average_length(CYLINDER_LIST *list)
{
	double width = 0.0;
	int i = 0, n = cylinder_list_size(list);
	for (i = 0; i < n; i++)
	{
		CYLINDER *cylinder = cylinder_list_get(list, i);
		width += cylinder->size;
	}

	return width / (double) n;
}

double compute_average_length(CYLINDER_LIST *face_left, CYLINDER_LIST *face_top, CYLINDER_LIST *face_right)
{
	return (cylinder_list_average_length(face_left)
	      + cylinder_list_average_length(face_top)
	      + cylinder_list_average_length(face_right)) / 3.0;
}

double compute_stereo_volume(CYLINDER_LIST *face_left, CYLINDER_LIST *face_top, CYLINDER_LIST *face_right)
{
	// O comprimento, largura e altura da pilha, em milímetros.
	double height = compute_average_height(face_left, face_top, face_right);
	double width  = compute_average_width(face_left, face_right); 
	double length = compute_average_length(face_left, face_top, face_right);
	
	printf("--- Calculo do Volume Estereo ---\nComprimento: %f\nAltura: %f\nLargura: %f\n", length, height, width);

	// O volume estéreo, em metros cúbicos.	
	return length * width * height / pow(10, 9);
}
