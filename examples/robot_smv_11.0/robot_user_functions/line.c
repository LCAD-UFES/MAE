#include "robot_user_functions.h"
#include "viewer.hpp"
#include <math.h>
#include <gsl/gsl_fit.h>
#include "line.h"

#define PI 3.14159265358979323846264338327
#define PI_HALF (PI / 2.0)
#define POW2(a) ((a)*(a))

/*
Alocação & Inicialização
*/

LINE *
line_alloc (double x0, double y0, double z0, double a, double b, double c)
{
	LINE *line = (LINE*) malloc (sizeof(LINE));

	if (line != NULL)
		line_init (line, x0, y0, z0, a, b, c);
	
	return line;
}

LINE *
line_alloc2 (double x0, double y0, double z0, double alpha, double betha)
{
	LINE *line = (LINE*) malloc (sizeof(LINE));

	if (line != NULL)
	{
		line_set_origin (line, x0, y0, z0);
		line_set_direction_angles (line, alpha, betha);
	}

	return line;
}

void
line_init (LINE *line, double x0, double y0, double z0, double a, double b, double c)
{
	line_set_origin (line, x0, y0, z0);
	line_set_direction (line, a, b, c);	
}

double line_fit_slope (double *x, double *y, int n)
{
	double c0, c1, cov00, cov01, cov11, chisq;
	
	gsl_fit_linear (x, 1, y, 1, n, 
                        &c0, &c1, &cov00, &cov01, &cov11, 
                        &chisq);

	printf("# m = %f\n\n", c1);

	return c1;	
}

LINE *
line_fit (double x0, double y0, double z0, double *x, double *y, int n)
{
	double m = line_fit_slope (x, y, n);
	double norm = sqrt (1 + POW2 (m));

	LINE *line = line_alloc (x0, y0, z0, 0, 0, 0);
	line->a = 0.0;
	line->b = fabs (m) / norm;
	line->c = (m < 0 ? -1.0 : 1.0) / norm;

	return line;
}

/*
See http://mathforum.org/library/drmath/view/54985.html
*/
double
line_distance (LINE *l1, LINE *l2)
{
	// Product vector.
	double p[3] = {(l1->b * l2->c - l1->c * l2->b),
		      -(l1->a * l2->c - l1->c * l2->a),
		       (l1->a * l2->b - l1->b * l2->a)};

	// Norm of the product vector.
	double norm = sqrt (POW2 (p[0]) + POW2 (p[1]) + POW2 (p[2]));

	// Vector connecting the origins of the lines.
	double o[3] = {(l1->x0 - l2->x0),
		       (l1->y0 - l2->y0),
		       (l1->z0 - l2->z0)};

	// Shortest distance between the lines.
	double distance = (o[0] * p[0] + o[1] * p[1] + o[2] * p[2]) / norm;
	
	return distance;
}

/*
Visualização
*/

void
line_3D_view (LINE *line, int length)
{
	double cosAlpha = line_get_cos_alpha (line);
	double cosBetha = line_get_cos_betha (line);
	double cosGama  = line_get_cos_gama  (line);
	double x = 0, x0 = line->x0;
	double y = 0, y0 = line->y0;
	double z = 0, z0 = line->z0;
	int i;
	
	for (i = 0; i < length; i++)
	{
		x = x0 + i * cosAlpha;
		y = y0 + i * cosBetha;
		z = z0 + i * cosGama;
		ViewerUpdateMap(1, z, -x, y, 0, 0, 0);
		
		x = x0 - i * cosAlpha;
		y = y0 - i * cosBetha;
		z = z0 - i * cosGama;
		ViewerUpdateMap(1, z, -x, y, 0, 0, 0);
	}	
}

/*
Propriedades
*/

double
line_get_cos_alpha (LINE *line)
{
	double a = line->a;
	double b = line->b;
	double c = line->c;
	double cosAlpha = a / sqrt (POW2 (a) + POW2 (b) + POW2 (c));
	
	return cosAlpha;
}

double
line_get_cos_betha (LINE *line)
{
	double a = line->a;
	double b = line->b;
	double c = line->c;
	double cosBetha = b / sqrt (POW2 (a) + POW2 (b) + POW2 (c));
	
	return cosBetha;
}

double
line_get_cos_gama (LINE *line)
{
	double a = line->a;
	double b = line->b;
	double c = line->c;
	double cosGama = c / sqrt (POW2 (a) + POW2 (b) + POW2 (c));
	
	return cosGama;
}

void
line_set_origin (LINE *line, double x0, double y0, double z0)
{
	line->x0 = x0;
	line->y0 = y0;
	line->z0 = z0;
}

void
line_set_direction (LINE *line, double a, double b, double c)
{
	line->a = a;
	line->b = b;
	line->c = c;
}

void
line_set_direction_angles (LINE *line, double alpha, double betha)
{
	double a = cos (alpha);
	double b = cos (PI_HALF - betha);
	double c2 = 1 - POW2 (a) - POW2 (b);
	double c = (c2 < 0.0 ? 0.0 : sqrt (c2));
	
	line->a = a;
	line->b = b;
	line->c = c;
}
