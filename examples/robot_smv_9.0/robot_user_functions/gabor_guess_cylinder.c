#include "gabor_guess_cylinder.h"
#include "robot_user_functions.h"
#include "fitting.h"
#include <stdlib.h>
#include <math.h>
#include "geometric_operations.h"

#define INITIAL_ALPHA			(-10.0 * acos(-1.0)/180.0)
#define FINAL_ALPHA 			(+10.0 * acos(-1.0)/180.0)
#define STEP_ALPHA 			((FINAL_ALPHA-INITIAL_ALPHA)/10.0)

#define ALPHA_MIN_LIMIT			-10
#define ALPHA_MAX_LIMIT			10
#define DIAMETER_MIN_LIMIT		10
#define DIAMETER_MAX_LIMIT		50
#define SIZE_MIN_LIMIT			300
#define SIZE_MAX_LIMIT			800
#define BETHA_MIN_LIMIT			-20
#define BETHA_MAX_LIMIT			20


int 
get_line (NEURON_LAYER *trunk_segmentation_map, int initial_line, int *working_area, 
                    double alpha, double *line)
{
	int x, y, w, radius;

	w = trunk_segmentation_map->dimentions.x;
	radius = 0;
	x = (int) ((double) radius * cos (alpha) + (double) working_area[0] + 0.5);
	y = (int) ((double) radius * sin (alpha) + (double) initial_line + 0.5);
	while ((x >= working_area[0]) && (x < working_area[2]) && (y >= working_area[1]) && (y < working_area[3]))
	{	
		line[radius] = trunk_segmentation_map->neuron_vector[y*w + x].output.fval;
		radius++;
		x = (int) ((double) radius * cos (alpha) + (double) working_area[0] + 0.5);
		y = (int) ((double) radius * sin (alpha) + (double) initial_line + 0.5);
	}
	return (radius);
}

int
set_line (NEURON_LAYER *trunk_segmentation_map, int initial_line, int *working_area, 
                    double alpha, double val)
{
	int x, y, w, radius;

	w = trunk_segmentation_map->dimentions.x;
	radius = 0;
	x = (int) ((double) radius * cos (alpha) + (double) working_area[0] + 0.5);
	y = (int) ((double) radius * sin (alpha) + (double) initial_line + 0.5);
	while ((x >= working_area[0]) && (x < working_area[2]) && (y >= working_area[1]) && (y < working_area[3]))
	{	
		trunk_segmentation_map->neuron_vector[y*w + x].output.fval = val;
		radius++;
		x = (int) ((double) radius * cos (alpha) + (double) working_area[0] + 0.5);
		y = (int) ((double) radius * sin (alpha) + (double) initial_line + 0.5);
	}
	return (radius);
}


int
rep_line (NEURON_LAYER *trunk_segmentation_map, int initial_line, int *working_area, 
                    double alpha, double *line)
{
	int x, y, w, radius;

	w = trunk_segmentation_map->dimentions.x;
	radius = 0;
	x = (int) ((double) radius * cos (alpha) + (double) working_area[0] + 0.5);
	y = (int) ((double) radius * sin (alpha) + (double) initial_line + 0.5);
	while ((x >= working_area[0]) && (x < working_area[2]) && (y >= working_area[1]) && (y < working_area[3]))
	{	
		trunk_segmentation_map->neuron_vector[y*w + x].output.fval = line[radius];
		radius++;
		x = (int) ((double) radius * cos (alpha) + (double) working_area[0] + 0.5);
		y = (int) ((double) radius * sin (alpha) + (double) initial_line + 0.5);
	}
	return (radius);
}


int 
line_average(NEURON_LAYER *trunk_segmentation_map, int initial_line, int *working_area, 
             double alpha, double *line, double * rtn_average, double * rtn_neg_number)
{
	int w, size, i;
	double average, neg_number;
	//PESQUISA:Apenas pra visualizar o cilindro
	static OUTPUT_DESC *output = NULL;

	if (output == NULL)
		output = get_output_by_neural_layer (trunk_segmentation_map);
	//PESQUISA:FIM
		
	w = trunk_segmentation_map->dimentions.x;
	
	size = get_line (trunk_segmentation_map, initial_line, working_area, alpha, line);

	//PESQUISA:Apenas pra visualizar as linhas
	set_line (trunk_segmentation_map, initial_line, working_area, alpha, 0.0);

	update_output_image (output);
	
	size = rep_line (trunk_segmentation_map, initial_line, working_area, alpha, line);
	//PESQUISA:FIM
	
	if (size < (working_area[2] - working_area[0]))
		return -1;
		
	neg_number = average = 0.0;
	for (i = 0; i < size; i++)
	{
		if (line[i] >= 0)
			average += line[i];
		else
			neg_number += 1;	
	}
	
	average /= (double)(size);
	
	*rtn_average = average;
	*rtn_neg_number = neg_number;
	
	return 1;
}

void
estimate_alpha (CYLINDER *guessed_cylinder, NEURON_LAYER *trunk_segmentation_map, int *working_area, 
		int y_init_cylinder, int y_end_cylinder, double *line)
{
	double max_average, min_neg_number;
	double alpha, good_alpha = 0;
	double rtn_average, rtn_neg_number;
	int initial_line;
	

	initial_line = (int)(y_init_cylinder + y_end_cylinder)/2.0 + 0.5;
	max_average = -DBL_MAX;
	min_neg_number = DBL_MAX;

	for (alpha = INITIAL_ALPHA; alpha < FINAL_ALPHA; alpha += STEP_ALPHA)
	{
		if (line_average(trunk_segmentation_map, initial_line, working_area, alpha, line, 
				 &rtn_average, &rtn_neg_number) < 0)
			continue;
				       
		if (min_neg_number > rtn_neg_number)
		{
			min_neg_number = rtn_neg_number;
			max_average = rtn_average;
			good_alpha = alpha;
		}
		else if (min_neg_number == rtn_neg_number)
		{
			if (max_average < rtn_average)
			{
				max_average = rtn_average;
				good_alpha = alpha;
			}		
		}
	}
	
	guessed_cylinder->alpha = good_alpha;

}

int
get_line_disparities (double *disparity_line, NEURON_LAYER * trunk_segmentation_map, 
		NEURON_LAYER * diparity_map, int *working_area, double good_alpha, int initial_line)
{
	int xi, yi, xo, yo, wo, radius;

	wo = diparity_map->dimentions.x;
	
	radius = 0;
	xi = (int) ((double) radius * cos (good_alpha) + (double) working_area[0] + 0.5);
	yi = (int) ((double) radius * sin (good_alpha) + (double) initial_line + 0.5);
	while ((xi >= working_area[0]) && (xi < working_area[2]) && (yi >= working_area[1]) && (yi < working_area[3]))
	{	

		xo = XIMAGE2MAP(xi, diparity_map, trunk_segmentation_map); 
		yo = YIMAGE2MAP(yi, diparity_map, trunk_segmentation_map);
		
		disparity_line[radius] = diparity_map->neuron_vector[yo*wo + xo].output.fval;
		radius++;
		xi = (int) ((double) radius * cos (good_alpha) + (double) working_area[0] + 0.5);
		yi = (int) ((double) radius * sin (good_alpha) + (double) initial_line + 0.5);
	}

	if (radius < (working_area[2] - working_area[0]))
		return -1;

	return radius;
}

int 
compaire_double (const void *pval1, const void *pval2)
{
	double *val1, *val2;
	
	val1 = (double *) pval1;
	val2 = (double *) pval2;
	if (*val1 == *val2)
		return 0;
	else
		return (*val1 > *val2 ? 1 : -1);
}

double
average_center_quartis_disparity (NEURON_LAYER * trunk_segmentation_map, NEURON_LAYER * diparity_map, 
				  int *working_area, double *disparity_line, double good_alpha, 
				  int initial_line)
{
	int size, i;
	double average;
	
	size = get_line_disparities (disparity_line, trunk_segmentation_map, 
			       diparity_map, working_area, good_alpha, initial_line);
		
	if ( size < 0 )
		return -DBL_MAX;
		
	qsort (disparity_line, size, sizeof (double), compaire_double);

	average = 0.0;
	for (i = size/4; i < 3*size/4; i++)
		average += disparity_line[i];
		
	return (average/(double)(size/2.0));
}

int
get_center_y (int x_center, int y_init_cylinder, int *working_area, double good_alpha)
{
	int radius;
	
	radius = x_center / cos (good_alpha);
	
	return (int) ((double) radius * sin (good_alpha) + (double) y_init_cylinder + 0.5);
}

void
estimate_diameter_x_y_z (CYLINDER *guessed_cylinder, int *working_area, int y_init_cylinder, 
			 int y_end_cylinder, double average_disparity, 
			 double *point_3D_left, double *point_3D_right, double good_alpha)
{
	double point_image_left[6], point_image_right[6];

	int x_center = (int)((working_area[2] + working_area[0])/2.0 + 0.5);
	
	//Faz a tringulacao dos pontos para o mundo
	//[0] Armazena o x da imagem direita
	//[1] Armazena o y da imagem direita
	point_image_right[0] = working_area[0];
	point_image_right[1] = y_init_cylinder;
	point_image_right[2] = working_area[0];
	point_image_right[3] = y_end_cylinder;
	point_image_right[4] = x_center;
	point_image_right[5] = get_center_y (x_center, y_init_cylinder, working_area, good_alpha);
	//[2] Armazena o x da imagem esquerda
	//[3] Armazena o y da imagem esquerda
	point_image_left[0] = point_image_right[0] + average_disparity;
	point_image_left[1] = point_image_right[1];
	point_image_left[2] = point_image_right[2] + average_disparity;
	point_image_left[3] = point_image_right[3];
	point_image_left[4] = point_image_right[4] + average_disparity;
	point_image_left[5] = point_image_right[5];
	
	StereoTriangulation (3, point_3D_left, point_3D_right, point_image_left, point_image_right);
	
	guessed_cylinder->diameter = distance_3D(point_3D_right, point_3D_right+3) * 2;
	
	guessed_cylinder->x = point_3D_right[6];
	guessed_cylinder->y = point_3D_right[7];
	guessed_cylinder->z = point_3D_right[8];
}

void
estimate_betha (CYLINDER *guessed_cylinder)
{
	guessed_cylinder->betha = 0;
}

void
estimate_size (CYLINDER *guessed_cylinder, int *working_area)
{
	guessed_cylinder->size = SIZE_MIN_LIMIT;
}

void
guess_cylinder (CYLINDER *guessed_cylinder, NEURON_LAYER *trunk_segmentation_map, 
		NEURON_LAYER *diparity_map, int *working_area, 
		int y_init_cylinder, int y_end_cylinder, double *line)
{
	double average_disparity;
	int initial_line;
	double point_3D_left[9], point_3D_right[9];
	
	estimate_alpha (guessed_cylinder, trunk_segmentation_map, working_area, 
			y_init_cylinder, y_end_cylinder, line);
	
	initial_line = (int)(y_init_cylinder + y_end_cylinder)/2.0 + 0.5;
	average_disparity = average_center_quartis_disparity (trunk_segmentation_map, 
							      diparity_map, working_area, 
							      line, guessed_cylinder->alpha, 
							      initial_line);

	estimate_diameter_x_y_z (guessed_cylinder, working_area, y_init_cylinder, 
			 	 y_end_cylinder, average_disparity, 
			 	 point_3D_left, point_3D_right, guessed_cylinder->alpha);
		
	estimate_betha (guessed_cylinder);

	estimate_size (guessed_cylinder, working_area);
	
}

CYLINDER_LIST * 
guess_cylinder_list (int *working_area, NEURON_LAYER *trunk_segmentation_map, 
		     NEURON_LAYER *diparity_map)
{
	int y, x, w = trunk_segmentation_map->dimentions.x;
	int y_init_cylinder, y_end_cylinder;
	double value;
	int inside_cylinder = 0;
	
	CYLINDER guessed_cylinder;
	CYLINDER_LIST *cylinder_list = NULL, *curr_cylinder_list = NULL;
	double *line = (double *) alloc_mem ((int)((double)(working_area[2] - working_area[0]) * sqrt (2.0) * (double) sizeof (double)) + 1);

	
	y = working_area[1];
	x = working_area[0];
	while (y < working_area[3])
	{
		value = trunk_segmentation_map->neuron_vector[y*w + x + 0].output.fval;
		value += trunk_segmentation_map->neuron_vector[y*w + x + 1].output.fval;
		value += trunk_segmentation_map->neuron_vector[y*w + x + 2].output.fval;
		if (!inside_cylinder && value >= 0)
		{
			y_init_cylinder = y;
			inside_cylinder = 1;
		}
		else if (inside_cylinder && value < 0)
		{
			y_end_cylinder = y;
			inside_cylinder = 0;
			
			guess_cylinder (&guessed_cylinder, trunk_segmentation_map, 
					diparity_map, working_area, y_init_cylinder,
					y_end_cylinder, line);
			
			print_cylinder(&guessed_cylinder, "Guessed_cylinder");

			add_cylinder (&cylinder_list, &curr_cylinder_list, guessed_cylinder);
		}
		
		y++;
	}

	free (line);
	
	print_cylinder_list(cylinder_list);
	
	draw_cylinder_list(cylinder_list);

	return cylinder_list;
}


