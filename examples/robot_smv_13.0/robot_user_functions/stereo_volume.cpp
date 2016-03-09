#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <gsl/gsl_fit.h>

#include <stdexcept>
using std::runtime_error;

#include <vector>
using std::vector;

#include "logging.hpp"
using mae::logging::trace;

#include "primitives.hpp"
using mae::util::Primitives;

#include "line.hpp"

#define HALF_PI pi / 2.0
#define POSITION 75.0
#define POW2(a) ((a)*(a))

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

void cylinder_list_extract_centers(Model3D::Face& face, double x0, vector<double>& x, vector<double>& y)
{
	int n = face.getCylindersSize();
	x.resize(n);
	y.resize(n);

	for (int i = 0; i < n; i++)
	{
		CYLINDER& cylinder = face.transformedCylinder(i);
		double *point = cylinder_center_point_at(&cylinder, x0);
		x.at(i) = point[1];
		y.at(i) = point[2];

		free(point);
	}
}

double cylinder_list_selective_fit(Model3D::Face& face, double x0)
{
	trace("cylinder_list_selective_fit", string() + "face, " + x0, string() + "face(cylinders[" + face.getCylindersSize() + "])");

	vector<double> x, y;
	cylinder_list_extract_centers(face, x0, x, y);
	int n = Primitives::min(x.size(), y.size());

	trace(string() + "gsl_fit_linear(" + x + ", 1, " + y + ", 1, " + n + ", &c0, &c1, &cov00, &cov01, &cov11, &sumsq)");
	double c0, c1, cov00, cov01, cov11, sumsq;
	gsl_fit_linear(
		&(x[0]), 1, &(y[0]), 1, n,
		&c0, &c1, &cov00, &cov01, &cov11,
		&sumsq);

	double err_max = sumsq * 4 / n;
	for (int i = 0; i < n; i++)
	{
		double y_val, y_err;
		gsl_fit_linear_est(x.at(i), c0, c1, cov00, cov01, cov11, &y_val, &y_err);
		y_err = POW2(y_val - y.at(i));

		trace(string() + "y_err[" + i + "] = " + y_err + ", err_max = " + err_max);
		if (y_err > err_max)
		{
			trace(string() + "Excluded: list[" + i + "] = (" + x.at(i) + ", " + y.at(i) +")");
			face.removeCylinder(i);
			return cylinder_list_selective_fit(face, x0);
		}
	}

	return 1 / (isnan(c1) ? DBL_MIN : c1);
}

LINE *cylinder_list_compute_line(Model3D::Face& face, double position)
{
	trace(string() + "cylinder_list_compute_line(face, " + position + ")");

	CYLINDER& first = face.lowestCylinder();
	cylinder_3D_view_color(&first, ARCS, CIRCLES, 0, 0, 0);

	double x0 = first.x + position;
	double y0 = first.y;
	double z0 = first.z;

	double m = cylinder_list_selective_fit(face, x0);

	return line_alloc_slope(x0, y0, z0, m);
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
LINE* line_perpendicular(LINE *line, double x0, double y0, double z0)
{
	double *line_point = line_closest_point(line, x0, y0, z0);
	double x1 = line_point[0];
	double y1 = line_point[1];
	double z1 = line_point[2];

	LINE *perpendicular = line_alloc(x0, y0, z0, x1 - x0, y1 - y0, z1 - z0);

	free(line_point);

	return perpendicular;
}

CYLINDER* cylinder_list_find_closest(Model3D::Face& face, LINE *line)
{
	CYLINDER& highest = face.highestCylinder();
	CYLINDER *closest = NULL;

	double x0 = line->x0, y0 = highest.y + highest.diameter, z0 = line->z0;
	LINE *perpendicular = line_perpendicular (line, x0, y0, z0);

	int i = 0, n = face.getCylindersSize();
	double shortest_distance = DBL_MAX;

	line_3D_view (perpendicular, 1000);

	for (i = 0; i < n; i++)
	{
		CYLINDER& cylinder = face.transformedCylinder(i);
		LINE *center_line = cylinder_center_line(&cylinder);
		double distance = line_distance (perpendicular, center_line);
		if (distance < shortest_distance)
		{
			shortest_distance = distance;
			closest = &cylinder;
		}

		free(center_line);
	}

	cylinder_3D_view_color(closest, ARCS, CIRCLES, 0, 0, 0);

	return closest;
}

double cylinder_compute_height(CYLINDER *cylinder, LINE *height_line)
{
/*	LINE *perpendicular = line_perpendicular (height_line, cylinder->x, cylinder->y, cylinder->z);
	line_3D_view(perpendicular, 1000); */

	double x = cylinder->x;
	double y = cylinder->y + cylinder_radius(cylinder);
	double z = cylinder->z;
	double *point = line_closest_point(height_line, x, y, z);
	double height = sqrt(POW2(height_line->x0 - point[0]) +
	                     POW2(height_line->y0 - point[1]) +
			     POW2(height_line->z0 - point[2]));

	return height;
}

double cylinder_list_compute_height(Model3D::Face& side, Model3D::Face& top, double position)
{
	trace(string() + "cylinder_list_compute_height(side, top, " + position + ")");

	LINE *height_line = cylinder_list_compute_line(side, position);
	CYLINDER& first   = side.transformedCylinder(0);
	CYLINDER *closest = cylinder_list_find_closest(top, height_line);
	double height = cylinder_compute_height(closest, height_line) + cylinder_radius(&first);

	line_3D_view(height_line, 1000);

	free(height_line);

	return height;
}

double compute_average_height(Model3D::Face& face_left, Model3D::Face& face_top, Model3D::Face& face_right)
{
	trace("compute_average_height", "face_left, face_top, face_right");

	double h1 = cylinder_list_compute_height(face_left,  face_top, 0.0);
	double h2 = cylinder_list_compute_height(face_left,  face_top, -POSITION);
	double h3 = cylinder_list_compute_height(face_left,  face_top, POSITION);
	double h4 = cylinder_list_compute_height(face_right, face_top, 0.0);
	double h5 = cylinder_list_compute_height(face_right, face_top, -POSITION);
	double h6 = cylinder_list_compute_height(face_right, face_top, POSITION);

	return (h1 + h2 + h3 + h4 + h5 + h6) / 6.0;
}

double compute_average_width(Model3D::Face& left, Model3D::Face& right)
{
	trace("compute_average_width", "left, right");

	LINE* height_line = cylinder_list_compute_line(right, 0.0);
	int i = 0, n = left.getCylindersSize();
	double width = 0.0;

	for (i = 0; i < n; i++)
	{
		CYLINDER& cylinder  = left.transformedCylinder(i);
		double x0 = cylinder.x;
		double y0 = cylinder.y;
		double z0 = cylinder.z;

		double *point = line_closest_point(height_line, x0, y0, z0);
		double distance = sqrt(
			POW2(point[0] - x0) +
			POW2(point[1] - y0) +
			POW2(point[2] - z0));

		free(point);
		width += distance;
	}

	free(height_line);

	return width / n;
}

double cylinder_list_average_length(Model3D::Face& face)
{
	trace("cylinder_list_average_length", "face");

	double length = 0.0;
	int i = 0, n = face.getCylindersSize();
	for (i = 0; i < n; i++)
	{
		CYLINDER& cylinder = face.transformedCylinder(i);
		length += cylinder.size;
	}

	return length / (double) n;
}

double compute_average_length(Model3D::Face& face_left, Model3D::Face& face_top, Model3D::Face& face_right)
{
	trace("compute_average_length", "face_left, face_top, face_right");

	return (cylinder_list_average_length(face_left)
	      + cylinder_list_average_length(face_top)
	      + cylinder_list_average_length(face_right)) / 3.0;
}
