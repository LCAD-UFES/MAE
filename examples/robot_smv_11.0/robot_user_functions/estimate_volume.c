#include "cylinder.h"
#include "cylinder_list.h"
#include "estimate_volume.h"
#include "robot_user_functions.h"
#include "fitting.h"
#include <stdlib.h>
#include <math.h>
#include "geometric_operations.h"

#define MAX_ACCEPTABLE_NUMBER_OF_LINES 	10
#define INITIAL_ALPHA			(-3.0 * acos(-1.0)/180.0)
#define FINAL_ALPHA 			(+3.0 * acos(-1.0)/180.0)
#define STEP_ALPHA 			((FINAL_ALPHA-INITIAL_ALPHA)/10.0)
#define SIZE_FIRST_GUESS_PERCENT	0.85

#define ALPHA_MIN_LIMIT			-5
#define ALPHA_MAX_LIMIT			5
#define DIAMETER_MIN_LIMIT		10
#define DIAMETER_MAX_LIMIT		50
#define SIZE_MIN_LIMIT			300
#define SIZE_MAX_LIMIT			800
#define BETHA_MIN_LIMIT			-20
#define BETHA_MAX_LIMIT			20

#define MINIMUM_WORKING_AREA_SIZE	MAX_ACCEPTABLE_NUMBER_OF_LINES*3.0/4.0
#define WORKING_AREA_STEP		MAX_ACCEPTABLE_NUMBER_OF_LINES/5.0

double infinity_disparity;

void
correct_cylinder_limits(CYLINDER *cylinder)
{
	double PI = acos(-1);
	double alphaMin = PI * ((double)ALPHA_MIN_LIMIT)/180.0;
	double alphaMax = PI * ((double)ALPHA_MAX_LIMIT)/180.0;
	double bethaMin = PI * ((double)BETHA_MIN_LIMIT)/180.0;
	double bethaMax = PI * ((double)BETHA_MAX_LIMIT)/180.0;
	
	cylinder->size *= SIZE_FIRST_GUESS_PERCENT;
	
	//Corrige cada parametro do cilindro para os limites permitidos
	if (cylinder->alpha < alphaMin)
		cylinder->alpha = alphaMin;
	else if (cylinder->alpha > alphaMax)
		cylinder->alpha = alphaMax;

	if (cylinder->diameter < DIAMETER_MIN_LIMIT)
		cylinder->diameter = (double)DIAMETER_MIN_LIMIT;
	else if (cylinder->diameter > DIAMETER_MAX_LIMIT)
		cylinder->diameter = (double)DIAMETER_MAX_LIMIT;

	if (cylinder->size < SIZE_MIN_LIMIT)
		cylinder->size = (double)SIZE_MIN_LIMIT;
	else if (cylinder->size > SIZE_MAX_LIMIT)
		cylinder->size = (double)SIZE_MAX_LIMIT;

	if (cylinder->betha < bethaMin)
		cylinder->betha = bethaMin;
	else if (cylinder->betha > bethaMax)
		cylinder->betha = bethaMax;

	if (cylinder->z < NEAREST_DISTANCE)
		cylinder->z = NEAREST_DISTANCE;
	else if (cylinder->z > FAREST_DISTANCE)
		cylinder->z = FAREST_DISTANCE;
}

int 
compair_double (const void *pval1, const void *pval2)
{
	double *val1, *val2;
	
	val1 = (double *) pval1;
	val2 = (double *) pval2;
	if (*val1 == *val2)
		return 0;
	else
		return (*val1 > *val2 ? 1 : -1);
}

void init_infinity_disparity()
{
	double p_dblRightPoint[2], p_dblLeftPoint[2], p_dblWorldPoint[3];

	// Define the scaning range
	p_dblRightPoint[0] = 0;
	p_dblRightPoint[1] = 0;

	// Calculates the infinity distance position
	GetWorldPointAtDistanceRight(p_dblWorldPoint, p_dblRightPoint, INFINITY_DISTANCE, RIGHT_CAMERA);
	CameraProjectionRight(1, p_dblLeftPoint, p_dblWorldPoint, LEFT_CAMERA);
	infinity_disparity = p_dblLeftPoint[0];
}


int 
get_disparity_line (NEURON_LAYER *disparity_map, int initial_line, int *working_area, 
                    double alpha, double *disparity_line)
{
	int x, y, w, radius;

	w = disparity_map->dimentions.x;
	radius = 0;
	x = (int) ((double) radius * cos (alpha) + (double) working_area[0] + 0.5);
	y = (int) ((double) radius * sin (alpha) + (double) initial_line + 0.5);
	while ((x >= working_area[0]) && (x < working_area[2]) && (y >= working_area[1]) && (y < working_area[3]))
	{	
		disparity_line[radius] = disparity_map->neuron_vector[y*w + x].output.fval;
		radius++;
		x = (int) ((double) radius * cos (alpha) + (double) working_area[0] + 0.5);
		y = (int) ((double) radius * sin (alpha) + (double) initial_line + 0.5);
	}
	return (radius);
}


int
set_disparity_line (NEURON_LAYER *disparity_map, int initial_line, int *working_area, 
                    double alpha, double val)
{
	int x, y, w, radius;

	w = disparity_map->dimentions.x;
	radius = 0;
	x = (int) ((double) radius * cos (alpha) + (double) working_area[0] + 0.5);
	y = (int) ((double) radius * sin (alpha) + (double) initial_line + 0.5);
	while ((x >= working_area[0]) && (x < working_area[2]) && (y >= working_area[1]) && (y < working_area[3]))
	{	
		disparity_map->neuron_vector[y*w + x].output.fval = val;
		radius++;
		x = (int) ((double) radius * cos (alpha) + (double) working_area[0] + 0.5);
		y = (int) ((double) radius * sin (alpha) + (double) initial_line + 0.5);
	}
	return (radius);
}


int
rep_disparity_line (NEURON_LAYER *disparity_map, int initial_line, int *working_area, 
                    double alpha, double *disparity_line)
{
	int x, y, w, radius;

	w = disparity_map->dimentions.x;
	radius = 0;
	x = (int) ((double) radius * cos (alpha) + (double) working_area[0] + 0.5);
	y = (int) ((double) radius * sin (alpha) + (double) initial_line + 0.5);
	while ((x >= working_area[0]) && (x < working_area[2]) && (y >= working_area[1]) && (y < working_area[3]))
	{	
		disparity_map->neuron_vector[y*w + x].output.fval = disparity_line[radius];
		radius++;
		x = (int) ((double) radius * cos (alpha) + (double) working_area[0] + 0.5);
		y = (int) ((double) radius * sin (alpha) + (double) initial_line + 0.5);
	}
	return (radius);
}


double 
average_of_the_two_center_disparity_quartis (NEURON_LAYER *disparity_map, int initial_line, int *working_area, 
                                             double alpha, double *disparity_line)
{
	int w, size, i;
	double average;
	//PESQUISA:Apenas pra visualizar o cilindro
	static OUTPUT_DESC *output = NULL;

	if (output == NULL)
		output = get_output_by_neural_layer (disparity_map);
	//PESQUISA:FIM
		
	w = disparity_map->dimentions.x;
	
	size = get_disparity_line (disparity_map, initial_line, working_area, alpha, disparity_line);

	//PESQUISA:Apenas pra visualizar as linhas
	set_disparity_line (disparity_map, initial_line, working_area, alpha, 0.0);
	
	if (output != NULL) update_output_image (output);
	
	size = rep_disparity_line (disparity_map, initial_line, working_area, alpha, disparity_line);
	//PESQUISA:FIM
	
	if (size < (working_area[2] - working_area[0]))
		return (-10000.0);
		
	qsort (disparity_line, size, sizeof (double), compair_double);

	average = 0.0;
	for (i = size/4; i < 3*size/4; i++)
		average += disparity_line[i];
		
	return (average/(double)(size/2.0));
}



int 
check_zero_derivative_in_average_disparity (double *average_disparity, int i)
{
	if (i > 0)
	{
		if (average_disparity[i] < average_disparity[i-1])
			return i;
	}	
	return -1;
}

void 
estimate_points_diameter  (int *point_2D_center, int *point_2D_radius, 
		    int *point_2D_left, int *point_2D_right, 
	            NEURON_LAYER *disparity_map, 
	            int initial_line, NEURON_LAYER *right_image)
{
	int initial_line_image;
	
	//Calcula o ponto central
	point_2D_center[0] = (int) (point_2D_left[0] + point_2D_right[0])/ 2.0;
	point_2D_center[1] = (int) (point_2D_left[1] + point_2D_right[1])/ 2.0;
	point_2D_center[2] = (int) (point_2D_left[2] + point_2D_right[2])/ 2.0;
	point_2D_center[3] = (int) (point_2D_left[3] + point_2D_right[3])/ 2.0;
	
	//Converte o initial line para a imagem
	initial_line_image = YMAP2IMAGE(initial_line, disparity_map, right_image); 
	
	//Calcula o ponto do raio relativo ao centro
	point_2D_radius[0] = point_2D_center[0];
	point_2D_radius[1] = initial_line_image;
	point_2D_radius[2] = point_2D_center[2];
	point_2D_radius[3] = point_2D_radius[1];
	
}


double
vector_average (double *vector, int size)
{
	int i;
	double average;
	
	average = 0.0;
	for (i = 0; i < size; i++)
		average += vector[i];
	return (average / (double) size);
}

double
vector_standard_deviation (double *vector, int size, double average)
{
	int i;
	double sigma;
	
	for (i = 0; i < size; i++)
		sigma += (vector[i] - average) * (vector[i] - average);
	sigma = sqrt (sigma / (double) size);
	return (sigma);
}

void 
get_x_y_of_the_cylinder_left_and_right_points (int *point_2D_left, int *point_2D_right, 
					       int *working_area, NEURON_LAYER *disparity_map, 
					       int good_line, double good_alpha, 
					       double *disparity_line, NEURON_LAYER *right_image)
{
	int size;
	int i, w, x_left, y_left, x_right, y_right;
	double average, sigma;
	//PESQUISA:Apenas pra visualizar o cilindro
	static OUTPUT_DESC *output = NULL;

	if (output == NULL)
		output = get_output_by_neural_layer (disparity_map);
	//PESQUISA:FIM

	w = disparity_map->dimentions.x;
	size = get_disparity_line (disparity_map, good_line, working_area, good_alpha, disparity_line);
	
	average = vector_average (disparity_line, size);
	sigma = vector_standard_deviation (disparity_line, size, average);
	for (i = 0; i < size/15; i++)
		if (fabs (disparity_line[i] - average) < sigma)
			break;

	x_left = (int) ((double) i * cos (good_alpha) + (double) working_area[0] + 0.5);
	y_left = (int) ((double) i * sin (good_alpha) + (double) good_line + 0.5);

	for (i = size-1; i > size - size/15; i--)
		if (fabs (disparity_line[i] - average) < sigma)
			break;

	x_right = (int) ((double) i * cos (good_alpha) + (double) working_area[0] + 0.5);
	y_right = (int) ((double) i * sin (good_alpha) + (double) good_line + 0.5);
	
	//PESQUISA:Apenas pra visualizar a linha e os pontos
	set_disparity_line (disparity_map, good_line, working_area, good_alpha, 0.0);
	disparity_map->neuron_vector[y_left*w + x_left].output.fval = -100.0;
	disparity_map->neuron_vector[y_right*w + x_right].output.fval = -100.0;
	
	if (output != NULL) update_output_image (output);
	size = rep_disparity_line (disparity_map, good_line, working_area, good_alpha, disparity_line);
	//PESQUISA:FIM

	//Mapeia pontos do mapa para a imagem
	point_2D_left[0] = XMAP2IMAGE(x_left, disparity_map, right_image); 
	point_2D_left[1] = YMAP2IMAGE(y_left, disparity_map, right_image);
	point_2D_left[2] = point_2D_left[0] + (int) average; 
	point_2D_left[3] = point_2D_left[1]; 

	point_2D_right[0] = XMAP2IMAGE(x_right, disparity_map, right_image); 
	point_2D_right[1] = YMAP2IMAGE(y_right, disparity_map, right_image);
	point_2D_right[2] = point_2D_right[0] + (int) average; 
	point_2D_right[3] = point_2D_right[1]; 


}


//double 
//estimate_betha (double *point_3D_left, double *point_3D_righ)
//{
//	double diffZ, diffX;
//	diffX = fabs(point_3D_left[0] - point_3D_righ[0]);
//	diffZ = fabs(point_3D_left[2] - point_3D_righ[2]);
	
//	return atan(diffZ/diffX);
//	return 0.0;
//}

void
set_initial_cylinder (CYLINDER *guessed_cylinder, int *point_2D_left, int *point_2D_right,
		      int *point_2D_center, int *point_2D_radius, double good_alpha)
{
	double point_3D_left[12], point_3D_right[12];
	double point_image_left[8], point_image_right[8];
	double point_3D_1[3], point_3D_2[3];

	//Faz a tringulacao dos pontos para o mundo
	//[0] Armazena o x da imagem direita
	//[1] Armazena o y da imagem direita
	point_image_right[0] = point_2D_left[0];
	point_image_right[1] = point_2D_left[1];
	point_image_right[2] = point_2D_right[0];
	point_image_right[3] = point_2D_right[1];
	point_image_right[4] = point_2D_center[0];
	point_image_right[5] = point_2D_center[1];
	point_image_right[6] = point_2D_radius[0];
	point_image_right[7] = point_2D_radius[1];
	//[2] Armazena o x da imagem esquerda
	//[3] Armazena o y da imagem esquerda
	point_image_left[0] = point_2D_left[2];
	point_image_left[1] = point_2D_left[3];
	point_image_left[2] = point_2D_right[2];
	point_image_left[3] = point_2D_right[3];
	point_image_left[4] = point_2D_center[2];
	point_image_left[5] = point_2D_center[3];
	point_image_left[6] = point_2D_radius[2];
	point_image_left[7] = point_2D_radius[3];
	
	StereoTriangulation (4, point_3D_left, point_3D_right, point_image_left, point_image_right);


	guessed_cylinder->alpha = good_alpha;

	point_3D_1[0] = point_3D_right[6];  
	point_3D_1[1] = point_3D_right[7];  
	point_3D_1[2] = point_3D_right[8];  

	point_3D_2[0] = point_3D_right[9];  
	point_3D_2[1] = point_3D_right[10];  
	point_3D_2[2] = point_3D_right[11];  
	guessed_cylinder->diameter = 2 * distance_3D(point_3D_1, point_3D_2);

	point_3D_1[0] = point_3D_right[0];  
	point_3D_1[1] = point_3D_right[1];  
	point_3D_1[2] = point_3D_right[2];  

	point_3D_2[0] = point_3D_right[3];  
	point_3D_2[1] = point_3D_right[4];  
	point_3D_2[2] = point_3D_right[5];  
	guessed_cylinder->size = distance_3D (point_3D_1, point_3D_2);

	guessed_cylinder->x = (point_3D_right[0] + point_3D_right[3])/2.0;
	guessed_cylinder->y = (point_3D_right[1] + point_3D_right[4])/2.0;
	guessed_cylinder->z = (point_3D_right[2] + point_3D_right[5])/2.0 + 
				(guessed_cylinder->diameter/2.0);

	guessed_cylinder->betha = 0;//estimate_betha (point_3D_1, point_3D_2);
	
	correct_cylinder_limits(guessed_cylinder);

}

void
guess_initial_cylinder (CYLINDER *guessed_cylinder,
			NEURON_LAYER *disparity_map,
			int *working_area, 
			NEURON_LAYER *right_image,
			int * error)
{
	int i;
	int line_found = 0, initial_line;
	double average_disparity[MAX_ACCEPTABLE_NUMBER_OF_LINES], max_average_disparity;
	double alpha, good_alpha;
	int good_line;
	int point_of_zero_derivative;
	double *disparity_line;
	int point_2D_left[4], point_2D_right[4];
	int point_2D_center[4], point_2D_radius[4];
	double max_number_of_lines_disparity_map;
	
	max_number_of_lines_disparity_map = 
				YIMAGE2MAP(MAX_ACCEPTABLE_NUMBER_OF_LINES,
					   disparity_map,
					   right_image);

	initial_line = working_area[1];
	max_average_disparity = -DBL_MAX;

	disparity_line = (double *) alloc_mem ((int)((double)(working_area[2] - working_area[0]) * sqrt (2.0) * (double) sizeof (double)) + 1);
	for (alpha = INITIAL_ALPHA; alpha < FINAL_ALPHA; alpha += STEP_ALPHA)
	{
		for (i = 0; i < max_number_of_lines_disparity_map; i++)
		{
			average_disparity[i] = average_of_the_two_center_disparity_quartis (disparity_map, initial_line+i, working_area, alpha, disparity_line);
			point_of_zero_derivative = check_zero_derivative_in_average_disparity (average_disparity, i);
			if (point_of_zero_derivative != -1)
			{
				if (average_disparity[point_of_zero_derivative] > max_average_disparity)
				{
					max_average_disparity = average_disparity[point_of_zero_derivative];
					good_alpha = alpha;
					good_line = initial_line + point_of_zero_derivative;
				}
				line_found = 1;
				break;
			}
		}
	}

	if (!line_found)
	{
		*error = -1;
		return;
	}

	average_of_the_two_center_disparity_quartis (disparity_map, 
					good_line, working_area, good_alpha, disparity_line);
	
	get_x_y_of_the_cylinder_left_and_right_points (point_2D_left, point_2D_right, 
					working_area, disparity_map, good_line, 
					good_alpha, disparity_line, right_image);

	estimate_points_diameter  (point_2D_center, point_2D_radius, 
		    			point_2D_left, point_2D_right, 
	            			disparity_map, initial_line, right_image);
						
	free (disparity_line);
	
	set_initial_cylinder (guessed_cylinder, point_2D_left, point_2D_right,
		      	      point_2D_center, point_2D_radius, good_alpha);
}




double
calculate_volume (CYLINDER_LIST *cylinder_list)
{
	return 0;
}

int
is_valid_cylinder(CYLINDER *cylinder, double *working_area_3D)
{
/*	double PI = acos(-1);
	double alphaMin = PI * ((double)ALPHA_MIN_LIMIT)/180.0;
	double alphaMax = PI * ((double)ALPHA_MAX_LIMIT)/180.0;
	double bethaMin = PI * ((double)BETHA_MIN_LIMIT)/180.0;
	double bethaMax = PI * ((double)BETHA_MAX_LIMIT)/180.0; */

	// Verifica se os parâmetros do cilindro estão dentro dos limites.
	if (
//	 cylinder->alpha < alphaMin || cylinder->alpha > alphaMax
//	 || 
	 cylinder->diameter < DIAMETER_MIN_LIMIT 
	 || cylinder->diameter > DIAMETER_MAX_LIMIT
//	 || cylinder->size < SIZE_MIN_LIMIT || cylinder->size > SIZE_MAX_LIMIT
//	 || cylinder->betha < bethaMin || cylinder->betha > bethaMax
	 || cylinder->x < working_area_3D[0] || cylinder->x > working_area_3D[3]
	 || cylinder->y < working_area_3D[1] || cylinder->y > working_area_3D[4]
	 || cylinder->z < working_area_3D[2] || cylinder->z > working_area_3D[5]
	 )
		return 0;

//	if (last_cylinder != NULL && is_too_close(cylinder, last_cylinder)
//		return 0;

	return 1;
}

int
adjust_area_of_interest (NEURON_LAYER *disparity_map, int *rectangle, CYLINDER *cylinder)
{
	double image_point[2], world_point[3];
	int next_base;
	
	if (cylinder != NULL)
	{
		world_point[0] = cylinder->x;
		world_point[1] = cylinder->y + cylinder->diameter / 2.0;
		world_point[2] = cylinder->z;

		CameraProjectionRight(1, image_point, world_point, RIGHT_CAMERA);

		printf ("base original: %d\nbase nova: %f\ntopo: %d\n",
			rectangle[1], image_point[1], rectangle[3]);
		
		next_base = (int) (image_point[1] + 0.5);
		if (next_base < rectangle[1])
			rectangle[1] += WORKING_AREA_STEP;
		else
			rectangle[1] = next_base;
	}
	else
		rectangle[1] += WORKING_AREA_STEP;

	return (rectangle[3] - rectangle[1] > MINIMUM_WORKING_AREA_SIZE);
}

void
set_working_area (int *working_area, int *image_working_area, NEURON_LAYER *disparity_map, NEURON_LAYER *right_image)
{
	working_area[0] = (double) (image_working_area[0] * disparity_map->dimentions.x) / (double) right_image->dimentions.x;
	working_area[1] = (double) (image_working_area[1] * disparity_map->dimentions.y) / (double) right_image->dimentions.y;
	working_area[2] = (double) (image_working_area[2] * disparity_map->dimentions.x) / (double) right_image->dimentions.x;
	working_area[3] = (double) (image_working_area[3] * disparity_map->dimentions.y) / (double) right_image->dimentions.y;

	if (working_area[0] < 0)
		working_area[0] = 0;
		
	if (working_area[0] >= disparity_map->dimentions.x) 
		working_area[0] = disparity_map->dimentions.x;
		
	if (working_area[1] < 0)
		working_area[1] = 0;
		
	if (working_area[1] >= disparity_map->dimentions.y)
		working_area[1] = disparity_map->dimentions.y;
		
	if (working_area[2] < 0) 
		working_area[2] = 0;
		
	if (working_area[2] >= disparity_map->dimentions.x) 
		working_area[2] = disparity_map->dimentions.x;
	    
	if (working_area[3] < 0)
		working_area[3] = 0;
			
	if (working_area[3] >= disparity_map->dimentions.y)
		working_area[3] = disparity_map->dimentions.y;

}

void
set_working_area_3D(double *working_area_3D, int *working_area)
{
	double image_working_area[4];
	image_working_area[0] = (double) working_area[0];
	image_working_area[1] = (double) working_area[1];
	image_working_area[2] = (double) working_area[2];
	image_working_area[3] = (double) working_area[3];

	GetWorldPointAtDistanceRight (working_area_3D, image_working_area, NEAREST_DISTANCE, RIGHT_CAMERA);
	GetWorldPointAtDistanceRight (working_area_3D + 3, image_working_area + 2, FAREST_DISTANCE, RIGHT_CAMERA);
}


double
estimate_volume (int *working_area, NEURON_LAYER *disparity_map, NEURON_LAYER *right_image)
{
	int there_is_region_of_interest;
	CYLINDER cylinder, guessed_cylinder, *cylinder_pointer = NULL;
	CYLINDER_LIST *cylinder_list = alloc_cylinder_list ();
	double working_area_3D[6];
	
	int image_working_area[4], map_working_area[4];
	double volume, norm;
	int error;
									   		     /* x, y, z, size, diameter, alpha, betha */
	static int parameter_to_optimize_z_y_diam_alpha_betha[NUMBER_OF_CYLINDER_PARAMETERS] = {0, 1, 1, 0,    1,        1,     1};
									       /* x, y, z, size, diameter, alpha, betha */
	static int parameter_to_optimize_x_size[NUMBER_OF_CYLINDER_PARAMETERS] = {1, 0, 0, 1,    0,        0,     0};

	//PESQUISA:Apenas pra visualizar o cilindro
	static OUTPUT_DESC *output = NULL;
	if (output == NULL)
		output = get_output_by_neural_layer (disparity_map);
	//PESQUISA:FIM
	
	image_working_area[0] = working_area[0];
	image_working_area[1] = working_area[1];
	image_working_area[2] = working_area[2];
	image_working_area[3] = working_area[3];
	init_infinity_disparity();
	set_working_area_3D(working_area_3D, image_working_area);

	do
	{
		error = 0;
		set_working_area (map_working_area, image_working_area, disparity_map, right_image);

		guess_initial_cylinder (&guessed_cylinder, disparity_map, map_working_area, 
					right_image, &error);
		
		cylinder_print(&guessed_cylinder, "Guessed_cylinder");

		if (!error)
		{
			norm = fit_cylinder (&cylinder, &guessed_cylinder, disparity_map, image_working_area, parameter_to_optimize_z_y_diam_alpha_betha);
			
			copy_c2_to_c1(&guessed_cylinder, &cylinder);

			norm = fit_cylinder (&cylinder, &guessed_cylinder, disparity_map, image_working_area, parameter_to_optimize_x_size);

			cylinder_list_add (cylinder_list, cylinder);
			if (is_valid_cylinder (&cylinder, working_area_3D))
			{
				cylinder_pointer = &cylinder;
			}
			else
			{
				printf("@@@@@@ Invalid cylinder in step!\n");
				cylinder_pointer = NULL;
			}
		}
		else
		{
			printf("@@@@@@ Cylinder not guessed in step!\n");
			cylinder_pointer = NULL;
		}
		
		there_is_region_of_interest = adjust_area_of_interest (disparity_map, image_working_area, cylinder_pointer);
	} while (there_is_region_of_interest);

	cylinder_list_print(cylinder_list);
	cylinder_list_view(cylinder_list, ARCS, CIRCLES);

	volume = calculate_volume (cylinder_list);
	cylinder_list_free (cylinder_list);
	return (volume);
}
