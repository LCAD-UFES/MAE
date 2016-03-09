#ifndef CYLINDER_H
#define CYLINDER_H

/*
Description of a cylinder used to approximate the volume of a wood log.
*/
#define NUMBER_OF_CYLINDER_PARAMETERS 7

#define ARCS			30
#define CIRCLES			100

/*
Tipos de Dados
*/

struct _cylinder
{
	double x, y, z;
	double size;
	double diameter;
	double alpha, betha;
	
	double x_init_cylinder;
	double y_init_cylinder;
	double x_end_cylinder;
	double y_end_cylinder;
	double x_center;
	double y_center;
	double avg_disparity;
};

typedef struct _cylinder CYLINDER;

/*
Inicialização
*/

void copy_c2_to_c1 (CYLINDER *c1, CYLINDER *c2);

/*
Comparação
*/

double cylinder_centers_distance (CYLINDER *c1, CYLINDER *c2);

int is_too_close (CYLINDER *c1, CYLINDER *c2);

/*
Visualização
*/

void cylinder_3D_view_color (CYLINDER *cylinder, int arcs, int circles, int red, int green, int blue);

void cylinder_view_transformed(CYLINDER *cylinder, double *eulerAngles, double *displacement, double *massCenter);

void cylinder (double x0, double y0, double z0, double size, double diameter, double alpha, double betha, int arcs, int circles);

void cylinder_print (CYLINDER *cylinder, char * initial_msg);

void compute_3D_cylinder (CYLINDER *cylinder, double *cylinder_3D_points, int arcs, int circles, int size, double angle);

void project_cylinder_2D_points_into_disparity_map (int num_points, double *left_points, double *right_points);

void view_3D_cylinder (CYLINDER *cylinder, int arcs, int circles);

/*
Propriedades
*/

double cylinder_radius (CYLINDER *cylinder);

#endif
