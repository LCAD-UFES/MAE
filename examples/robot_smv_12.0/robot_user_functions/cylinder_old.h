#ifndef __CYLINDER_H
#define __CYLINDER_H


/*
Description of a cylinder used to approximate the volume of a wood log.
*/
#define NUMBER_OF_CYLINDER_PARAMETERS 7

#define ARCS			30
#define CIRCLES			100

struct _cylinder
{
	double x, y, z;
	double size;
	double diameter;
	double alpha, betha;
};

typedef struct _cylinder CYLINDER;

struct _cylinder_list
{
	CYLINDER cylinder;
	struct _cylinder_list *next;
};
typedef struct _cylinder_list CYLINDER_LIST;

void cylinder (double x0, double y0, double z0, double size, double diameter, double alpha, double betha, int arcs, int circles);

void compute_3D_cylinder (CYLINDER *cylinder, double *cylinder_3D_points, int arcs, int circles, double angle);

int is_too_close(CYLINDER *c1, CYLINDER *c2);

void project_cylinder_2D_points_into_disparity_map (int num_points, double *left_points, double *right_points);

void view_3D_cylinder (CYLINDER *cylinder, int arcs, int circles);

void copy_c2_to_c1(CYLINDER *c1, CYLINDER *c2);

void add_cylinder (CYLINDER_LIST **cylinder_list, CYLINDER_LIST **curr_cylinder_list, CYLINDER cylinder);

void free_cylinder_list (CYLINDER_LIST *cylinder_list);

void draw_cylinder_list(CYLINDER_LIST *cylinder_list);

void print_cylinder(CYLINDER *cylinder, char * initial_msg);

void print_cylinder_list(CYLINDER_LIST *cylinder_list);

#endif
