#include <stdlib.h>
#include <math.h>
#include "gabor_guess_cylinder.h"
#include "robot_user_functions.h"
#include "fitting.h"
#include "segment.hpp"
#include "geometric_operations.h"
#include "mae.h"

#define INITIAL_ALPHA			(-10.0 * acos(-1.0)/180.0)
#define FINAL_ALPHA 			(+10.0 * acos(-1.0)/180.0)
#define STEP_ALPHA 			((FINAL_ALPHA-INITIAL_ALPHA)/10.0)

#define ALPHA_MIN_LIMIT			-10
#define ALPHA_MAX_LIMIT			10
#define DIAMETER_MIN_LIMIT		10
#define DIAMETER_MAX_LIMIT		50
#define DIAMETER_AVG			(DIAMETER_MAX_LIMIT + DIAMETER_MIN_LIMIT)/2 
//#define SIZE_MIN_LIMIT			300 // não está sendo utilizado
//#define SIZE_MAX_LIMIT			800 // não está sendo utilizado
#define BETHA_MIN_LIMIT			-20
#define BETHA_MAX_LIMIT			20

#define DIAMETER_MIN_LIMIT_GUESSED	5
#define DIAMETER_MAX_LIMIT_GUESSED	250

#define SIZE_MIN_LIMIT_GUESSED_3	2400 // De acordo com o contrato do fornecedor de eucalipto para a Aracruz
#define SIZE_MAX_LIMIT_GUESSED_3	3600 // as toras podem variar apenas + ou - 20 %

#define SIZE_MIN_LIMIT_GUESSED_6	5400 // Variação de + ou - 10% para toras de 6 metros
#define SIZE_MAX_LIMIT_GUESSED_6	6600 // 

#define WORKING_AREA_PERCENTAGE		0.20
#define MAX_LENGTH_DISPARITY		3
#define MAX_LENGTH_DISPARITY_TOLERANCE	1

#define LINE_PERCENT			0.9
#define LINE_PERCENT_TRANSITION		0.5

#define UP		 1
#define DOWN		-1

#define ZOOM(value, fromScale, toScale) (int) ((float) (value * toScale) / (float) fromScale + 0.5)

int 
get_line_from_nl (NEURON_LAYER *nl, int initial_y, int *working_area, 
                    double alpha, double *line, int line_size)
{
	int x, y, w, radius, initial_x = working_area[0];

	w = nl->dimentions.x;

	for(radius = 0; radius < line_size; radius++) 
	{

		x = (int) ((double) radius * cos (alpha) + (double) initial_x + 0.5);
		y = (int) ((double) radius * sin (alpha) + (double) initial_y + 0.5);

		if ((x < working_area[0]) && (x >= working_area[2]) && (y < working_area[1]) && (y >= working_area[3]))
		{
			return radius;
		}
			
		line[radius] = nl->neuron_vector[y*w + x].output.fval;
	}

	return (radius);
}

int
get_line_disparities (double *disparity_line, int line_size, NEURON_LAYER * trunk_segmentation_map, 
		      NEURON_LAYER * disparity_map, int *working_area, double alpha, int initial_y)
{
	int xi, yi, xo, yo, wo, radius, initial_x = working_area[0];

	wo = disparity_map->dimentions.x;

	for(radius = 0; radius < line_size; radius++) 
	{

		xi = (int) ((double) radius * cos (alpha) + (double) initial_x + 0.5);
		yi = (int) ((double) radius * sin (alpha) + (double) initial_y + 0.5);

		if ((xi < working_area[0]) && (xi >= working_area[2]) && (yi < working_area[1]) && (yi >= working_area[3]))
		{
			return radius;
		}
			
		xo = XIMAGE2MAP(xi, disparity_map, trunk_segmentation_map); 
		yo = YIMAGE2MAP(yi, disparity_map, trunk_segmentation_map);
		
		disparity_line[radius] = disparity_map->neuron_vector[yo*wo + xo].output.fval;
	}

	return (radius);

}

/*
int
set_line_in_nl (NEURON_LAYER *nl, int initial_y, int *working_area, 
                double alpha, int line_size, double value)
{
	int x, y, w, radius, initial_x = working_area[0];

	w = nl->dimentions.x;

	for(radius = 0; radius < line_size; radius++) 
	{

		x = (int) ((double) radius * cos (alpha) + (double) initial_x + 0.5);
		y = (int) ((double) radius * sin (alpha) + (double) initial_y + 0.5);

		if ((x < working_area[0]) && (x >= working_area[2]) && (y < working_area[1]) && (y >= working_area[3]))
		{
			return radius;
		}
			
		nl->neuron_vector[y*w + x].output.fval = value;
	}

	return (radius);

}
*/

/*
int
rep_line_from_nl (NEURON_LAYER *nl, int initial_y, int *working_area, 
                    double alpha, double *line, int line_size)
{
	int x, y, w, radius, initial_x = working_area[0];

	w = nl->dimentions.x;

	for(radius = 0; radius < line_size; radius++) 
	{
		x = (int) ((double) radius * cos (alpha) + (double) initial_x + 0.5);
		y = (int) ((double) radius * sin (alpha) + (double) initial_y + 0.5);

		if ((x < working_area[0]) && (x >= working_area[2]) && (y < working_area[1]) && (y >= working_area[3]))
		{
			return radius;
		}
			
		nl->neuron_vector[y*w + x].output.fval = line[radius];
	}

	return (radius);
}
*/

int 
line_average(double * rtn_average, int * rtn_number, int * rtn_zero_number, int positive,
	     NEURON_LAYER *nl, int initial_y, int *working_area, 
             double alpha, double *line, int line_size)
{
	int w, size, i, number, zero_number;
	double average;
	//PESQUISA:Apenas pra visualizar o cilindro
/*	static OUTPUT_DESC *output = NULL;

	if (output == NULL)
		output = get_output_by_neural_layer (nl);
*/
	//PESQUISA:FIM
		
	w = nl->dimentions.x;
	
	size = get_line_from_nl (nl, initial_y, working_area, alpha, line, line_size);

	//PESQUISA:Apenas pra visualizar as linhas
/*	set_line_in_nl (nl, initial_y, working_area, alpha, line_size, 0.0);

	if (output != NULL) update_output_image (output);
	
	size = rep_line_from_nl (nl, initial_y, working_area, alpha, line, line_size);
*/
	//PESQUISA:FIM
	
	if (size != line_size)
		return -1;
		
	zero_number = number = average = 0.0;
	for (i = 0; i < size; i++)
	{
		if (((double)positive * line[i]) > 0)
		{
			average += line[i];
			number++;	
		}
		else if (line[i] == 0)
		{
			zero_number++;
		}
	}
	
	average /= (double)(number);
	
	*rtn_average = fabs(average);
	*rtn_number = number;
	*rtn_zero_number = zero_number;
	
	return size;
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
average_center_quartis_disparity (NEURON_LAYER * trunk_segmentation_map, NEURON_LAYER * disparity_map, 
				  int *working_area, double *disparity_line, int line_size, double alpha, 
				  int initial_y)
{
	int size, i;
	double average;
	
	size = get_line_disparities (disparity_line, line_size, trunk_segmentation_map, 
				     disparity_map, working_area, alpha, initial_y);
		
	if ( size < line_size )
		return -DBL_MAX;
		
	qsort (disparity_line, size, sizeof (double), compaire_double);

	average = 0.0;
	for (i = size/4; i < 3*size/4; i++)
		average += disparity_line[i];
		
	return (average/(double)(size/2.0));
}

int
estimate_line_alpha (double * good_alpha, double *max_average, 
		     NEURON_LAYER *trunk_segmentation_map, int *working_area, 
		     int y, double *line, int line_size, int positive)
{
	double alpha;
	double rtn_average;
	int rtn_number, rtn_zero_number;

	*max_average = -DBL_MAX;
	*good_alpha = 0;
	for (alpha = INITIAL_ALPHA; alpha < FINAL_ALPHA; alpha += STEP_ALPHA)
	{
		if (line_average(&rtn_average, &rtn_number, &rtn_zero_number, positive, 
				 trunk_segmentation_map, y, working_area, alpha, 
				 line, line_size) < 0)
			continue;
				       
		if ((rtn_number + rtn_zero_number) >= (LINE_PERCENT * line_size) && 
		    *max_average < rtn_average)
		{
			*max_average = rtn_average;
			*good_alpha = alpha;
		}		
	}
	
	if (*max_average == -DBL_MAX)
		return -1;
	else
		return 1;
}

void
find_best_line (double *good_alpha, int *good_y, int initial_y, 
		NEURON_LAYER *trunk_segmentation_map, int *working_area, 
		double *line, int line_size, int positive)
{
	double max_average, max_average_prev;
	int y, state = 0, state_prev = 0;

	max_average_prev = 0;

	*good_alpha = 0;
	*good_y = working_area[3];
	y = initial_y;
	while (y < working_area[3])
	{
		if (estimate_line_alpha (good_alpha, &max_average, 
		     		     	 trunk_segmentation_map, working_area, 
		     		     	 y, line, line_size, positive) < 0)
		{
			y++;
			continue;
		}
	
		if (max_average_prev < max_average)
		{
//			printf("UP!Entrou min_neg_number_prev == min_neg_number && max_average > max_average_prev!min_neg_number_prev=%f, min_neg_number=%f\n", max_number_prev, max_number);
//			printf("UP!Entrou min_neg_number_prev == min_neg_number && max_average > max_average_prev!max_average_prev=%f, max_average=%f\n", max_average_prev, max_average);
			state = UP;
		}
		else if (max_average_prev >= max_average)
		{
//			printf("DOWN!Entrou min_neg_number_prev == min_neg_number && max_average <= max_average_prev!min_neg_number_prev=%f, min_neg_number=%f\n", max_number_prev, max_number);
//			printf("DOWN!Entrou min_neg_number_prev == min_neg_number && max_average <= max_average_prev!max_average_prev=%f, max_average=%f\n", max_average_prev, max_average);
			state = DOWN;
		}

		if (state != state_prev && state == DOWN)
		{
			*good_y = y;
			printf("Achou good_y=%d\n", *good_y);
			break;
		}

		state_prev = state;			
		max_average_prev = max_average;
		y++;
	}
	

}

void
find_best_transition (int *down_transition, int *up_transition, 
		      double good_alpha, int initial_y,
		      NEURON_LAYER *trunk_segmentation_map, int *working_area, 
		      double *line, int line_size)
{
	double rtn_average;
	int rtn_number, rtn_zero_number;
	int y;

	*up_transition = working_area[3];
	y = initial_y;
	while (y < working_area[3])
	{
		if (line_average(&rtn_average, &rtn_number, &rtn_zero_number, -1,
			     trunk_segmentation_map, y, working_area, 
        		     good_alpha, line, line_size) < 0)
		{
			y++;
			continue;
		}
	
		if (rtn_number > (LINE_PERCENT_TRANSITION * line_size))
		{
			*up_transition = y;
			break;
		}

		y++;
	}
	
	*down_transition = working_area[1];
	y = initial_y;
	while (y > working_area[1])
	{
		if (line_average(&rtn_average, &rtn_number, &rtn_zero_number, -1,
			     trunk_segmentation_map, y, working_area, 
        		     good_alpha, line, line_size) < 0)
		{
			y--;
			continue;
		}
	
		if (rtn_number > (LINE_PERCENT_TRANSITION * line_size))
		{
			*down_transition = y;
			break;
		}

		y--;
	}

}

void
find_lines (double *alphas, int *ys, int *down_transition, int *up_transition, 
	    int *lines_number, double *line, int line_size, 
	    NEURON_LAYER *trunk_segmentation_map, int *working_area)
{
	int y = working_area[1];
	double good_alpha;
	int good_y;
	int positive = 1;
	int down_value, up_value;
	

	*lines_number = 0;
	
	while (y < working_area[3])
	{
		find_best_line (&good_alpha, &good_y, y, 
				trunk_segmentation_map, working_area, 
				line, line_size, positive);

		printf("Achou uma linha!good_alpha=%f, good_y=%d, positive=%d\n", good_alpha, good_y, positive);
		alphas[*lines_number] = good_alpha;
		ys[*lines_number] = good_y;

		if (positive == 1)
		{
			find_best_transition (&down_value, &up_value, 
					      good_alpha, good_y, 
					      trunk_segmentation_map, working_area, 
					      line, line_size);
		
			up_transition[*lines_number] = up_value;
			down_transition[*lines_number] = down_value;

			printf("Achou up_transition=%d\n", up_value);
			printf("Achou down_transition=%d\n", down_value);

			(*lines_number)++;
		}
		else
		{
			up_value = good_y;
		}
		
		positive -= 2 * positive; 		
		y = up_value;
	}
	printf("numero de linhas encontradas = %d!\n", *lines_number);

}

int
get_y (int x_center, int y_init_cylinder, double good_alpha)
{
	double radius;
	
	radius = x_center / cos (good_alpha);
	
	return (int) ((double) radius * sin (good_alpha) + (double) y_init_cylinder + 0.5);
}

float 
calc_avg_disp(NEURON_LAYER *disparity_map, int x1, int y1, int x2, int y2)
{
	int x, y, deltax, deltay;
	float avg = 0;

	deltax = x2 - x1;
	deltay = y2 - y1;
	if (deltax == 0)
		return 0.0;
	for (x = x1; x <= x2; x++)
	{
		y = (int)((double)((double)deltay/(double)deltax) * (double)(x - x1) + y1);
		avg += disparity_map->neuron_vector[y * disparity_map->dimentions.x + x].output.fval; 
		//printf("x = %i, y = %i \n", x, y);
	}	
	return avg/(float) deltax;
}

float
size_3D_from_2D (CYLINDER *cylinder, NEURON_LAYER *disparity_map)
{
	double p_dblRightWorldPoint1[3], p_dblLeftWorldPoint1[3], p_dblLeftPoint1[2], p_dblRightPoint1[2];
	double p_dblRightWorldPoint2[3], p_dblLeftWorldPoint2[3], p_dblLeftPoint2[2], p_dblRightPoint2[2];
	double disp1, disp2, distance;
	int wo = disparity_map->dimentions.x;
	int ho = disparity_map->dimentions.y;
	int wi = image_right.ww;
	int hi = image_right.wh;
	int x1 = ZOOM(cylinder->x_init_cylinder, wo, wi),
            y1 = ZOOM(cylinder->y_init_cylinder, ho, hi),
	    x2 = ZOOM(cylinder->x_end_cylinder, wo, wi),
	    y2 = ZOOM(cylinder->y_end_cylinder, ho, hi);

	p_dblRightPoint1[0] = (double) x1;
	p_dblRightPoint1[1] = (double) y1;
	p_dblRightPoint2[0] = (double) x2;
	p_dblRightPoint2[1] = (double) y2;

	disp1 = disp2 = calc_avg_disp(disparity_map, x1, y1, x2, y2);

	p_dblLeftPoint1[0] = (double) (x1 + disp1);
	p_dblLeftPoint1[1] = (double) y1;
	p_dblLeftPoint2[0] = (double) (x2 + disp2);
	p_dblLeftPoint2[1] = (double) y2;

	StereoTriangulation (1, p_dblLeftWorldPoint1, p_dblRightWorldPoint1, p_dblLeftPoint1, p_dblRightPoint1);
	StereoTriangulation (1, p_dblLeftWorldPoint2, p_dblRightWorldPoint2, p_dblLeftPoint2, p_dblRightPoint2);
	distance = sqrt (
	      (p_dblRightWorldPoint1[0] - p_dblRightWorldPoint2[0]) * (p_dblRightWorldPoint1[0] - p_dblRightWorldPoint2[0]) +
	      (p_dblRightWorldPoint1[1] - p_dblRightWorldPoint2[1]) * (p_dblRightWorldPoint1[1] - p_dblRightWorldPoint2[1]) +
	      (p_dblRightWorldPoint1[2] - p_dblRightWorldPoint2[2]) * (p_dblRightWorldPoint1[2] - p_dblRightWorldPoint2[2]));

	printf ("Point 1: x1=%d y1=%d disp1=%lf\n", x1, y1, disp1);
	printf ("Point 2: x2=%d y2=%d disp2=%lf\n", x2, y2, disp2);
	printf ("Point 1: x=%lf y=%lf z=%lf\n", p_dblRightWorldPoint1[0], p_dblRightWorldPoint1[1], p_dblRightWorldPoint1[2]);
	printf ("Point 2: x=%lf y=%lf z=%lf\n", p_dblRightWorldPoint2[0], p_dblRightWorldPoint2[1], p_dblRightWorldPoint2[2]);
	printf ("Distance: %lf \n", distance);
	return distance;
}

void
convert_2D_to_3D (double **point_3D_left, double **point_3D_right,
		  double *alphas, int *ys, int *down_transition, int *up_transition, 
	    	  int lines_number, NEURON_LAYER *trunk_segmentation_map,
	    	  NEURON_LAYER *disparity_map, int *working_area, double *line, int line_size)
{
	double average_disparity;
	int l, i;
	double point_image_left[lines_number*4*2], point_image_right[lines_number*4*2];
	int x_center = (int)((working_area[2] + working_area[0])/2.0 + 0.5);
	
	*point_3D_left = (double *) malloc (lines_number*4*3 * sizeof (double));
	*point_3D_right = (double *) malloc (lines_number*4*3 * sizeof (double));

	for(l = 0; l < lines_number; l++)
	{
		average_disparity = average_center_quartis_disparity (trunk_segmentation_map, 
								      disparity_map, working_area, 
								      line, line_size, alphas[l], 
								      ys[l]);
		i = l*8;
		point_image_right[i+0] = working_area[0];
		point_image_right[i+1] = ys[l];
		point_image_right[i+2] = working_area[0];
		point_image_right[i+3] = down_transition[l];
		point_image_right[i+4] = working_area[0];
		point_image_right[i+5] = up_transition[l];
		point_image_right[i+6] = x_center;
		point_image_right[i+7] = get_y (x_center - working_area[0], ys[l], alphas[l]);
		
		point_image_left[i+0] = point_image_right[i+0] + average_disparity;
		point_image_left[i+1] = point_image_right[i+1];
		point_image_left[i+2] = point_image_right[i+2] + average_disparity;
		point_image_left[i+3] = point_image_right[i+3];
		point_image_left[i+4] = point_image_right[i+4] + average_disparity;
		point_image_left[i+5] = point_image_right[i+5];
		point_image_left[i+6] = point_image_right[i+6] + average_disparity;
		point_image_left[i+7] = point_image_right[i+7];
	}

	StereoTriangulation (lines_number*4, 
			     *point_3D_left, *point_3D_right, 
			     point_image_left, point_image_right);
}

void
guess_cylinder (CYLINDER *guessed_cylinder, double *down_transition, double *up_transition, 
		double *center, double alpha, NEURON_LAYER *disparity_map)
{
	guessed_cylinder->alpha = alpha;
	guessed_cylinder->betha = 0.0;
	
	guessed_cylinder->diameter = cos(alpha) * distance_3D(up_transition, down_transition) * 2;
	
	guessed_cylinder->x = center[0];
	guessed_cylinder->y = center[1];
	guessed_cylinder->z = center[2];

	guessed_cylinder->size = size_3D_from_2D (guessed_cylinder, disparity_map);
}

int
is_valid_guessed_cylinder (CYLINDER *guessed_cylinder, int halves)
{
	if (guessed_cylinder->diameter < DIAMETER_MIN_LIMIT_GUESSED ||
	    guessed_cylinder->diameter > DIAMETER_MAX_LIMIT_GUESSED)
		return 0;
	
	if ((guessed_cylinder->y_end_cylinder == 0.0) ||
	    (guessed_cylinder->y_end_cylinder == (double)(nl_segment.dimentions.y - 1)))
		return 0;

	if (halves == 1)
	{
		if (guessed_cylinder->size < SIZE_MIN_LIMIT_GUESSED_3)
			guessed_cylinder->size = SIZE_MIN_LIMIT_GUESSED_3;
	}

	if (halves == 2)
	{
		if (guessed_cylinder->size > SIZE_MAX_LIMIT_GUESSED_6)
			guessed_cylinder->size = SIZE_MAX_LIMIT_GUESSED_6;
	}

	return 1;
}

void 
draw_pole_as_lines_in_neural_layer(int *ys, double * alphas, int lines_number, int x_base, NEURON_LAYER *nl, float color)
{
	static OUTPUT_DESC *output = NULL;
	int x, y, l;

	if (output == NULL)
		output = get_output_by_neural_layer (nl);
	
	for (l = 0; l < lines_number; l++)
	{
		for (x = x_base; x < nl->dimentions.x; x++)
		{
			y = (int)(tan(alphas[l]) * (double)(x - x_base) + ys[l]);
			nl->neuron_vector[x*nl->dimentions.x + (nl->dimentions.x - y)].output.fval = color; 
		}	
		for (x = x_base; x >= 0; x--)
		{
			y = (int)(tan(-alphas[l]) * (double)(x_base - x) + ys[l]);
			nl->neuron_vector[x*nl->dimentions.x + (nl->dimentions.x - y)].output.fval = color; 
		}	
	}
}

void
guess_pole_line_list (NEURON_LAYER *trunk_segmentation_map, int **ys, double **alphas, int *x_base, int *lines_number, int *working_area)
{
	double *line;
	int line_size;
	int working_area_percent[4], *down_transition, *up_transition; 							     
	
	*x_base = working_area_percent[0] = working_area[1];
	working_area_percent[1] = working_area[0];
	working_area_percent[2] = working_area[3];
	working_area_percent[3] = working_area[2];
	
	line_size 	= working_area_percent[2] - working_area_percent[0] + 1;
	line 		= (double *) alloc_mem (line_size * sizeof (double));
	*alphas 	= (double *) alloc_mem ((working_area_percent[3] - working_area_percent[1]) * sizeof (double));
	*ys 		= (int *) alloc_mem ((working_area_percent[3] - working_area_percent[1]) * sizeof (int));
	down_transition = (int *) alloc_mem ((working_area_percent[3] - working_area_percent[1]) * sizeof (int));
	up_transition 	= (int *) alloc_mem ((working_area_percent[3] - working_area_percent[1]) * sizeof (int));

	find_lines (*alphas, *ys, down_transition, up_transition, lines_number, line, line_size,
	    	    trunk_segmentation_map, working_area_percent);

	free (down_transition);
	free (up_transition);
	free (line);
}

void
draw_lines_left_right(int guess_x_left, int guess_x_right, NEURON_LAYER *segment)
{
	int j;
	//printf("Valores de Guessed: guess_x_left = %i, guess_x_right = %i \n", guess_x_left, guess_x_right);
	for (j = 0; j < segment->dimentions.y; j++)
	{
		segment->neuron_vector[j * segment->dimentions.x + guess_x_left].output.fval = 0.0f;
		segment->neuron_vector[j * segment->dimentions.x +  guess_x_right].output.fval = 0.0f;
	}
}

void
fill_cylinder_2D(CYLINDER *one_cylinder, NEURON_LAYER *segment, int width, int x_center, int y_center, double alpha, int y_up,
		int guess_x_left, int guess_x_right)
{
        float value;
	int i, y;
	
	//printf("Valores de Guessed: guess_x_left = %i, guess_x_right = %i \n", guess_x_left, guess_x_right);
	one_cylinder->alpha = alpha;
	one_cylinder->x_center = x_center;
	
	// explicação das duas linhas abaixo:
	// As duas são excludentes. A diferença entre uma e outra é que a primeira tenta efetuar a medição do 
	// comprimento das toras no centro da tora e a outra linha tenta fazer esta medição pelo limite inferior 
	one_cylinder->y_center = ((double)y_center + (double)y_up)/2.0;
	//one_cylinder->y_center = (double)y_center;

	y = (int)(tan(one_cylinder->alpha) * (guess_x_right - one_cylinder->x_center) + one_cylinder->y_center);
	value   = segment->neuron_vector[y * width + guess_x_right].output.fval;
	for (i = guess_x_right; i < width; i++) 
	{
		y = (int)(tan(one_cylinder->alpha) * (i - one_cylinder->x_center) + one_cylinder->y_center);
		//printf("iteracao: y = %i, x = %i \n", y, i);

		if (y >= (segment->dimentions.y - 1)) 
		{
			one_cylinder->x_end_cylinder = (double)i;
			one_cylinder->y_end_cylinder = (double)(segment->dimentions.y - 1);
			break;
		}
		if (y <= 0)
		{
			one_cylinder->x_end_cylinder = (double)i;
			one_cylinder->y_end_cylinder = 0.0;
			break;
		}
		if (value  != segment->neuron_vector[y * width + i].output.fval)
		{
			one_cylinder->x_end_cylinder = (double)i;
			one_cylinder->y_end_cylinder = (double)y;
			printf("ACHOU. no cilindro: y = %f, x = %f \n", one_cylinder->y_end_cylinder, one_cylinder->x_end_cylinder);
			break;
		}
	}

	y = (int)(tan(one_cylinder->alpha) * (guess_x_left - one_cylinder->x_center) + one_cylinder->y_center);
	value   = segment->neuron_vector[y * width + guess_x_left].output.fval;
	for (i = guess_x_left; i > 0; i--) 
	{
		y = (int)(tan(one_cylinder->alpha) * (i - one_cylinder->x_center) + one_cylinder->y_center);
		//printf("iteracao: y = %i, x = %i \n", y, i);
		if (y >= (segment->dimentions.y - 1)) 
		{
			one_cylinder->x_end_cylinder = (double)i;
			one_cylinder->y_end_cylinder = (double)(segment->dimentions.y - 1);
			break;
		}
		if (y <= 0)
		{
			one_cylinder->x_end_cylinder = (double)i;
			one_cylinder->y_end_cylinder = 0.0;
			break;
		}
		if (value  != segment->neuron_vector[y * width + i].output.fval)
		{
			one_cylinder->x_init_cylinder = (double)i;
			one_cylinder->y_init_cylinder = (double)y;
			printf("ACHOU. No cilindro: y = %f, x = %f \n", one_cylinder->y_init_cylinder, one_cylinder->x_init_cylinder);
			break;
		}
	} 
	return;
}

void
guess_x_left_right (CYLINDER guessed_pole_left, CYLINDER guessed_pole_right, NEURON_LAYER *segment,
		int *working_area, int *guessed_x_left, int *guessed_x_right)
{
        *guessed_x_right = (int)(0.72 * segment->dimentions.x); // valor default, encontrado nos testes 
        *guessed_x_left = (int)(0.25 * segment->dimentions.x); // valor default, encontrado nos testes

	if ((guessed_pole_left.y_init_cylinder != 0) || (guessed_pole_left.y_end_cylinder != 0))
	{
		float deltax = (float)(guessed_pole_left.x_end_cylinder - guessed_pole_left.x_init_cylinder);
		float deltay = (float)(guessed_pole_left.y_end_cylinder - guessed_pole_left.y_init_cylinder);
		*guessed_x_left = (deltax/deltay) * (working_area[3] - guessed_pole_left.y_init_cylinder) 
				+ guessed_pole_left.x_init_cylinder;
	}

	if ((guessed_pole_right.y_init_cylinder != 0) || (guessed_pole_right.y_end_cylinder != 0))
	{
		float deltax = (float)(guessed_pole_right.x_end_cylinder - guessed_pole_right.x_init_cylinder);
		float deltay = (float)(guessed_pole_right.y_end_cylinder - guessed_pole_right.y_init_cylinder);
		*guessed_x_right = (deltax/deltay) * (working_area[3] - guessed_pole_right.y_init_cylinder) 
				+ guessed_pole_right.x_init_cylinder;
	}
}

void
guess_pole_left_right (NEURON_LAYER *nl_vertical, int *pole_ys, double *pole_alphas, int pole_x_base, int pole_lines_number, 
		CYLINDER *guessed_pole_left, CYLINDER *guessed_pole_right, NEURON_LAYER *segment)
{
	if (pole_lines_number >= 2)
	{
		guessed_pole_left->y_init_cylinder = 0;
		guessed_pole_left->x_init_cylinder = (nl_vertical->dimentions.y - 
					(tan(-pole_alphas[pole_lines_number - 1]) * (pole_x_base - guessed_pole_left->y_init_cylinder) + 
					pole_ys[pole_lines_number - 1]));

		guessed_pole_left->y_end_cylinder = nl_vertical->dimentions.x - 1;
		guessed_pole_left->x_end_cylinder = (nl_vertical->dimentions.y - 
					(tan(pole_alphas[pole_lines_number - 1]) * (guessed_pole_left->y_end_cylinder - pole_x_base) + 
					pole_ys[pole_lines_number - 1]));

		guessed_pole_right->y_init_cylinder = 0;
		guessed_pole_right->x_init_cylinder = (nl_vertical->dimentions.y - 
					(tan(-pole_alphas[0]) * (pole_x_base - guessed_pole_left->y_init_cylinder) + 
					pole_ys[0]));

		guessed_pole_right->y_end_cylinder = nl_vertical->dimentions.x - 1;
		guessed_pole_right->x_end_cylinder = (nl_vertical->dimentions.y - 
					(tan(pole_alphas[0]) * (guessed_pole_left->y_end_cylinder - pole_x_base) + 
					pole_ys[0]));

		printf("LEFT => x1 = %f, y1 = %f, x2 = %f, y2 = %f \n", 
			guessed_pole_left->x_init_cylinder, guessed_pole_left->y_init_cylinder,
			guessed_pole_left->x_end_cylinder, guessed_pole_left->y_end_cylinder);
		printf("RIGHT => x1 = %f, y1 = %f, x2 = %f, y2 = %f \n", 
			guessed_pole_right->x_init_cylinder, guessed_pole_right->y_init_cylinder,
			guessed_pole_right->x_end_cylinder, guessed_pole_right->y_end_cylinder);

		if (nl_vertical->dimentions.x != segment->dimentions.x)
		{
			guessed_pole_left->x_end_cylinder = ZOOM(guessed_pole_left->x_end_cylinder, 
							nl_vertical->dimentions.x, segment->dimentions.y);
			guessed_pole_right->x_end_cylinder = ZOOM(guessed_pole_right->x_end_cylinder, 
							nl_vertical->dimentions.x, segment->dimentions.y);
			guessed_pole_left->x_init_cylinder = ZOOM(guessed_pole_left->x_init_cylinder, 
							nl_vertical->dimentions.x, segment->dimentions.y);
			guessed_pole_right->x_init_cylinder = ZOOM(guessed_pole_right->x_init_cylinder, 
							nl_vertical->dimentions.x, segment->dimentions.y);
		}
	
		if (nl_vertical->dimentions.y != segment->dimentions.y)
		{
			guessed_pole_left->y_end_cylinder = ZOOM(guessed_pole_left->y_end_cylinder, 
							nl_vertical->dimentions.x, segment->dimentions.y);
	
			guessed_pole_right->y_end_cylinder = ZOOM(guessed_pole_right->y_end_cylinder, 
							nl_vertical->dimentions.x, segment->dimentions.y);
		}
	}
	else
	{
		guessed_pole_left->y_init_cylinder = 0;
		guessed_pole_left->y_end_cylinder = 0;

		guessed_pole_right->y_init_cylinder = 0;
		guessed_pole_right->y_end_cylinder = 0;
	}
}

CYLINDER_LIST * 
guess_cylinder_list (int *working_area_total, NEURON_LAYER *trunk_segmentation_map, NEURON_LAYER *disparity_map, int halves)
{
	CYLINDER guessed_cylinder, guessed_pole_left, guessed_pole_right;
	CYLINDER_LIST *cylinder_list = alloc_cylinder_list ();
	double *line, *alphas, *point_3D_left, *point_3D_right;
	int lines_number, l, line_size, guessed_x_right, guessed_x_left;
	int working_area_percent[4];
	int *down_transition, *up_transition, *ys;    

	int *pole_ys, pole_x_base, pole_lines_number;
	double *pole_alphas;

	working_area_percent[0] = (working_area_total[0] + working_area_total[2])/2.0 -
				  WORKING_AREA_PERCENTAGE/2.0 * (working_area_total[2]-working_area_total[0]) + 0.5;
	working_area_percent[1] = working_area_total[1];
	working_area_percent[2] = (working_area_total[0] + working_area_total[2])/2.0 +
				  WORKING_AREA_PERCENTAGE/2.0 * (working_area_total[2]-working_area_total[0]) + 0.5;
	working_area_percent[3] = working_area_total[3];
	
	line_size 	= working_area_percent[2] - working_area_percent[0] + 1;
	line 		= (double *) alloc_mem (line_size * sizeof (double));
	alphas 		= (double *) alloc_mem ((working_area_percent[3] - working_area_percent[1]) * sizeof (double));
	ys 		= (int *) alloc_mem ((working_area_percent[3] - working_area_percent[1]) * sizeof (int));
	down_transition = (int *) alloc_mem ((working_area_percent[3] - working_area_percent[1]) * sizeof (int));
	up_transition 	= (int *) alloc_mem ((working_area_percent[3] - working_area_percent[1]) * sizeof (int));

	find_lines (alphas, ys, down_transition, up_transition, &lines_number, line, line_size,
	    	    trunk_segmentation_map, working_area_percent);

	convert_2D_to_3D (&point_3D_left, &point_3D_right,  alphas, ys, down_transition, up_transition, 
	    		  lines_number, trunk_segmentation_map, disparity_map, working_area_percent, line, line_size);

	guess_pole_line_list (&nl_trunk_segmentation_map_vertical_rotate, &pole_ys, &pole_alphas, &pole_x_base, 
				&pole_lines_number, working_area_total);

	// o preenchimento dos cilindros abaixo, já é feita no tamanho e orientação da imagem de segmentação
	guess_pole_left_right (&nl_trunk_segmentation_map_vertical_rotate, pole_ys, pole_alphas, pole_x_base, pole_lines_number, 
				&guessed_pole_left, &guessed_pole_right, &nl_segment);

	guess_x_left_right (guessed_pole_left, guessed_pole_right, &nl_segment, working_area_total, 
				&guessed_x_left, &guessed_x_right);

	for(l = 0; l < lines_number; l++)
	{
		int y = ys[l];
                int x = working_area_percent[0];

		if (l == (lines_number - 1))
			fill_cylinder_2D(&guessed_cylinder, &nl_segment, disparity_map->dimentions.x, x, y, alphas[l], 
					y + DIAMETER_AVG/2, guessed_x_left, guessed_x_right);
		else
			fill_cylinder_2D(&guessed_cylinder, &nl_segment, disparity_map->dimentions.x, x, y, alphas[l], 
					ys[l + 1], guessed_x_left, guessed_x_right);

		guess_cylinder (&guessed_cylinder, &(point_3D_right[l*12+3]), &(point_3D_right[l*12+6]), 
				&(point_3D_right[l*12+9]), alphas[l], disparity_map);
		
		cylinder_list_add (cylinder_list, guessed_cylinder);
	}
	
	draw_pole_as_lines_in_neural_layer(pole_ys, pole_alphas, pole_lines_number, pole_x_base, &nl_segment, 0.0);
	draw_lines_left_right(guessed_x_left, guessed_x_right, &nl_segment);

	free (point_3D_left);
	free (point_3D_right);
	free (down_transition);
	free (up_transition);
	free (line);
	free (alphas);
	free (ys);
	free (pole_alphas);
	free (pole_ys);
	
	return cylinder_list;
}

void 
draw_cylinder_as_lines_in_segment(CYLINDER_LIST * cylinder_list, NEURON_LAYER *segment)
{
	CYLINDER cylinder;
	static OUTPUT_DESC *output = NULL;
	int deltax, deltay, x, y;

	if (output == NULL)
		output = get_output_by_neural_layer (segment);
	
	cylinder_list_move_first(cylinder_list);
	while (!is_cylinder_list_out (cylinder_list))
	{
		cylinder = *(cylinder_list_get_current (cylinder_list));
     		deltax = cylinder.x_end_cylinder - cylinder.x_init_cylinder;
		deltay = cylinder.y_end_cylinder - cylinder.y_init_cylinder;

		printf("desenhando cilindro de xi = %f; yi = %f; xe = %f; ye = %f \n", 
                      cylinder.x_init_cylinder, cylinder.y_init_cylinder, cylinder.x_end_cylinder, cylinder.y_end_cylinder);

		for (x = cylinder.x_init_cylinder; x <= cylinder.x_end_cylinder; x++)
		{
			y = (int)((double)((double)deltay/(double)deltax) * (double)(x - cylinder.x_init_cylinder) 
                                  + cylinder.y_init_cylinder);
			segment->neuron_vector[y*segment->dimentions.x + x].output.fval = 0.0f; 
			//printf("x = %i, y = %i \n", x, y);
		}	
		cylinder_list_move_next(cylinder_list);
	}
}

CYLINDER_LIST* fit_cylinder_list(int* working_area_total, int halves)
{
	CYLINDER cylinder, guessed_cylinder;
	CYLINDER_LIST* guessed_cylinder_list = NULL;
	CYLINDER_LIST* cylinder_list = alloc_cylinder_list ();
	static int parameter_to_optimize_z_y_diam_alpha_betha[NUMBER_OF_CYLINDER_PARAMETERS] = {0, 0, 1, 0,    0,        0,     1};
	guessed_cylinder_list = guess_cylinder_list(working_area_total, &nl_trunk_segmentation_map, &nl_disparity_map, halves);

	cylinder_list_move_first(guessed_cylinder_list);
	while (!is_cylinder_list_out (guessed_cylinder_list))
	{
		guessed_cylinder = *(cylinder_list_get_current (guessed_cylinder_list));
		
		if (is_valid_guessed_cylinder(&guessed_cylinder, halves))
		{	
			fit_cylinder (&cylinder, &guessed_cylinder, &nl_disparity_map, working_area_total, 
				      parameter_to_optimize_z_y_diam_alpha_betha);

			cylinder_list_add (cylinder_list, cylinder);
		}
		else
		{
			cylinder_print(&guessed_cylinder, "Eliminated cylinder!\n");
		}
		
		cylinder_list_move_next(guessed_cylinder_list);
	}

	draw_cylinder_as_lines_in_segment(cylinder_list, &nl_segment); 
	cylinder_list_print (guessed_cylinder_list);
	cylinder_list_free(guessed_cylinder_list);
	return cylinder_list;
}

