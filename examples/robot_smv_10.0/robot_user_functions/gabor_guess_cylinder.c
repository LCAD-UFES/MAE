#include <stdlib.h>
#include <math.h>
#include "gabor_guess_cylinder.h"
#include "robot_user_functions.h"
#include "fitting.h"
#include "segment.hpp"
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

#define DIAMETER_MIN_LIMIT_GUESSED	5
#define DIAMETER_MAX_LIMIT_GUESSED	50

#define WORKING_AREA_PERCENTAGE		0.20
#define MAX_LENGTH_DISPARITY		3
#define MAX_LENGTH_DISPARITY_TOLERANCE	1

#define LINE_PERCENT			0.9
#define LINE_PERCENT_TRANSITION		0.5

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


int 
line_average(double * rtn_average, int * rtn_number, int * rtn_zero_number, int positive,
	     NEURON_LAYER *nl, int initial_y, int *working_area, 
             double alpha, double *line, int line_size)
{
	int w, size, i, number, zero_number;
	double average;
	//PESQUISA:Apenas pra visualizar o cilindro
	static OUTPUT_DESC *output = NULL;

	if (output == NULL)
		output = get_output_by_neural_layer (nl);
	//PESQUISA:FIM
		
	w = nl->dimentions.x;
	
	size = get_line_from_nl (nl, initial_y, working_area, alpha, line, line_size);

	//PESQUISA:Apenas pra visualizar as linhas
	set_line_in_nl (nl, initial_y, working_area, alpha, line_size, 0.0);

	update_output_image (output);
	
	size = rep_line_from_nl (nl, initial_y, working_area, alpha, line, line_size);
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

#define UP		 1
#define DOWN		-1
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

/*
void
find_line_transitions (int *down_transition, int *up_transition, double *alphas, 
		       int *ys, int lines_number, double *line, int line_size, 
	    	       NEURON_LAYER *trunk_segmentation_map, int *working_area)
{
	int l, prev_y, next_y;
	int down_value, up_value;
	
	for(l = 0; l < lines_number; l+=2)
	{
	
		if (l-1 < 0) 
			prev_y = working_area[1];
		else
			prev_y = ys[l-1];
			
		if (l+1 >= lines_number) 
			next_y = working_area[3];
		else
			next_y = ys[l+1];

//		find_best_transition (&down_value, &up_value, 
//				      alphas[l], ys[l], prev_y, next_y, 
//				      trunk_segmentation_map, working_area, 
//				      line, line_size);
		
		down_transition[l] = down_value;
		up_transition[l] = up_value;
		printf("Achou up_transition=%d\n", up_value);
		printf("Achou down_transition=%d\n", down_value);
	}
}
*/

void
guess_cylinder (CYLINDER *guessed_cylinder, 
		double *down_transition, double *up_transition, double *center, double alpha)
{
	
	guessed_cylinder->alpha = alpha;
	guessed_cylinder->betha = 0.0;
	
	guessed_cylinder->diameter = cos(alpha) * distance_3D(up_transition, down_transition) * 2;
	
	guessed_cylinder->x = center[0];
	guessed_cylinder->y = center[1];
	guessed_cylinder->z = center[2];

	guessed_cylinder->size = 300;
}

int
is_valid_guessed_cylinder (CYLINDER *guessed_cylinder)
{
	
	if (guessed_cylinder->diameter < DIAMETER_MIN_LIMIT_GUESSED ||
	    guessed_cylinder->diameter > DIAMETER_MAX_LIMIT_GUESSED)
		return 0;
	
	return 1;
}

CYLINDER_LIST * 
guess_cylinder_list (int *working_area_total, NEURON_LAYER *trunk_segmentation_map, 
		     NEURON_LAYER *disparity_map)
{
	CYLINDER guessed_cylinder;
	CYLINDER_LIST *cylinder_list = alloc_cylinder_list ();
	double *line;
	int lines_number, l;
	int line_size;
	double *alphas;
	int *ys;
	int working_area_percent[4];
	int *down_transition, *up_transition; 							     
	double *point_3D_left, *point_3D_right;

	working_area_percent[0] = 
			(working_area_total[0] + working_area_total[2])/2.0 -
			WORKING_AREA_PERCENTAGE/2.0 * (working_area_total[2]-working_area_total[0]) + 0.5;
	working_area_percent[1] = working_area_total[1];
	working_area_percent[2] = 
			(working_area_total[0] + working_area_total[2])/2.0 +
			WORKING_AREA_PERCENTAGE/2.0 * (working_area_total[2]-working_area_total[0]) + 0.5;
	working_area_percent[3] = working_area_total[3];
	
	line_size = working_area_percent[2] - working_area_percent[0] + 1;
	line = (double *) malloc (line_size * sizeof (double));
	alphas = (double *) malloc ((working_area_percent[3] - working_area_percent[1]) * sizeof (double));
	ys = (int *) malloc ((working_area_percent[3] - working_area_percent[1]) * sizeof (int));
	down_transition = (int *) malloc ((working_area_percent[3] - working_area_percent[1]) * sizeof (int));
	up_transition = (int *) malloc ((working_area_percent[3] - working_area_percent[1]) * sizeof (int));

	find_lines (alphas, ys, down_transition, up_transition, 
		    &lines_number, line, line_size,
	    	    trunk_segmentation_map, working_area_percent);

	convert_2D_to_3D (&point_3D_left, &point_3D_right,
			  alphas, ys, down_transition, up_transition, 
	    		  lines_number, trunk_segmentation_map,
	    		  disparity_map, working_area_percent, line, line_size);

	for(l = 0; l < lines_number; l++)
	{
		guess_cylinder (&guessed_cylinder, 
				&(point_3D_right[l*12+3]), &(point_3D_right[l*12+6]), 
				&(point_3D_right[l*12+9]), alphas[l]);
		
		cylinder_list_add (cylinder_list, guessed_cylinder);
		
	}	
	
	free (point_3D_left);
	free (point_3D_right);
	free (down_transition);
	free (up_transition);
	free (line);
	free (alphas);
	free (ys);
	
	return cylinder_list;
}

CYLINDER_LIST * 
fit_cylinder_list (int *working_area_total, NEURON_LAYER *trunk_segmentation_map, 
		     NEURON_LAYER *disparity_map, NEURON_LAYER *simple_mono_right)
{
	
	CYLINDER cylinder, guessed_cylinder;
	CYLINDER_LIST *guessed_cylinder_list = NULL,
			*cylinder_list = alloc_cylinder_list ();
	static int parameter_to_optimize_z_y_diam_alpha_betha[NUMBER_OF_CYLINDER_PARAMETERS] = {0, 0, 1, 0,    0,        0,     1};
	guessed_cylinder_list = guess_cylinder_list (working_area_total, trunk_segmentation_map, 
		     				     disparity_map);

	cylinder_list_move_first(guessed_cylinder_list);
	while (!is_cylinder_list_out (guessed_cylinder_list))
	{
		guessed_cylinder = *(cylinder_list_get_current (guessed_cylinder_list));
		
		if (is_valid_guessed_cylinder(&guessed_cylinder))
		{	
			fit_cylinder (&cylinder, &guessed_cylinder, disparity_map, working_area_total, 
				      parameter_to_optimize_z_y_diam_alpha_betha);

			cylinder_list_add (cylinder_list, cylinder);
		}
		else
		{
			cylinder_print(&guessed_cylinder, "Eliminated cylinder!\n");
		}
		
		cylinder_list_move_next(guessed_cylinder_list);
	}

	cylinder_list_free (guessed_cylinder_list);

		    
//	estimate_limits_left_right (trunk_segmentation_map,
//				    simple_mono_right, 
//				    disparity_map,
//			    	    working_area_total, 
//				    cylinder_list);

	return cylinder_list;
}

char *region_of_interest (NEURON_LAYER *nl_input, int x_center, int y_center, int width, int height)
{
	int xi, yi, wi, hi, xo, yo, wo, ho;
	NEURON_OUTPUT value;
	
	char * output = malloc(width * height * 3 * sizeof(char));
	
	wi = nl_input->dimentions.x;
	hi = nl_input->dimentions.y;
	
	wo = width;
	ho = height;
	
	for (yo = 0; yo < ho; yo++)
	{
		for (xo = 0; xo < wo; xo++)
		{			
			xi = (int) ((double) xo + .5) + x_center - wo/2;
			yi = (int) ((double) yo + .5) + y_center - ho/2;
			
			if (xi >= 0 && xi < wi && yi >= 0 && yi < hi)
				value = nl_input->neuron_vector[xi + wi * yi].output;
			else
				value.ival = 0;
			
			output[3 * (yo * wo + xo) + 0] = RED(value.ival);
			output[3 * (yo * wo + xo) + 1] = GREEN(value.ival);
			output[3 * (yo * wo + xo) + 2] = BLUE(value.ival);
		}
	}
	
	return output;
}

void segment_set_neural_layer (NEURON_LAYER *nl_segment, NEURON_LAYER *nl_image, int center_x, int center_y)
{
	int width  = nl_segment->dimentions.x;
	int height = nl_segment->dimentions.y;
	char *reg_int = region_of_interest (nl_image, center_x, center_y, width, height);
	char *img = segment_image (0.5, 50, 20, reg_int, nl_segment->dimentions.x, nl_segment->dimentions.y);

	int x, y;
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++) 
		{
			nl_segment->neuron_vector[y * nl_segment->dimentions.x + x].output.ival =
				PIXEL(img[3 * (y * nl_segment->dimentions.x + x) + 0],
				      img[3 * (y * nl_segment->dimentions.x + x) + 1],
				      img[3 * (y * nl_segment->dimentions.x + x) + 2]);
		}
	}
	
	all_outputs_update ();

	free(img);
	free(reg_int);
}

void segment_plot_transitions (NEURON_LAYER *nl_segment)
{
	NEURON *neuron_vector = nl_segment->neuron_vector;
	int width  = nl_segment->dimentions.x;
	int height = nl_segment->dimentions.y;
	int x, y;

	for (x = 0; x < width; x++) 
	{
		int color = neuron_vector[x].output.ival;
		int transitions = 0;
		for (y = 0; y < height; y++)
		{
			int ival = neuron_vector[y * width + x].output.ival;
			if (ival != color)
			{
				transitions++;
				color = ival;
			}
		}
		
		fprintf (stderr, "%d\t%d\n", x, transitions);
	}
}

int segment_count_transitions (NEURON_LAYER *nl_segment, int x_center, int y_center, double alpha)
{
	NEURON *neuron_vector = nl_segment->neuron_vector;
	int width  = nl_segment->dimentions.x;
	int height = nl_segment->dimentions.y;
	
	double dx = cos (alpha), dy = sin (alpha), i = 0;
	int x0 = x_center - (int) (dx * 0.5 * (double) height);
	int y0 = y_center - (int) (dy * 0.5 * (double) height); 
	int transitions = 0;
	
	int color  = neuron_vector[y0 * width + x0].output.ival;
	for (i = 0.0; i < height; i += 1.0)
	{
		int x = x0 + (int) (dx * i);
		int y = y0 + (int) (dy * i);
		if (x >= width || y >= height)
			break;

		int ival = neuron_vector[y * width + x].output.ival;
		if (ival != color)
		{
			transitions++;
			color = ival;
		}
	}
	
	return transitions;
}

void segment_draw_line (NEURON_LAYER *nl_segment, int x_center, int y_center, double alpha)
{
	NEURON *neuron_vector = nl_segment->neuron_vector;
	int width  = nl_segment->dimentions.x;
	int height = nl_segment->dimentions.y;
	
	double dx = cos (alpha), dy = sin (alpha), i = 0;
	int x0 = x_center - (int) (dx * 0.5 * (double) height);
	int y0 = y_center - (int) (dy * 0.5 * (double) height);
	
	for (i = 0.0; i < height; i += 1.0)
	{
		int x = x0 + (int) (dx * i);
		int y = y0 + (int) (dy * i);
		if (x  < 0 || x >= width || y < 0 || y >= height)
			break;
		
		neuron_vector[y * width + x].output.ival = 0;
	}
	
	all_outputs_update ();
}

double get_alpha (int *linePoints)
{
	double num = linePoints[3] - linePoints[1];
	double den = linePoints[2] - linePoints[0];
	if (den != 0)
		return atan(num/den);
	else if (num > 0)
		return pi/2.0;
	else if (num < 0)
		return -pi/2.0;
	else
		return 0.0;
}

void segment_find_alpha (NEURON_LAYER *nl_segment, double *best_alpha, int *min_transitions, int *max_transitions)
{
	int width  = nl_segment->dimentions.x;
	int height = nl_segment->dimentions.y;
	int line[4] = {0, 0, width / 2, height / 2};
	int x = 0;

	*best_alpha = 0.0;
	*max_transitions = 0;
	*min_transitions = INT_MAX;
	for (x = 0; x < width; x++) 
	{
		line[0] = x;
		double alpha = get_alpha (line);
		int transitions = segment_count_transitions (nl_segment, width / 2, height / 2, alpha);
		
		if (transitions < *min_transitions)
		{
			*min_transitions = transitions;
			*best_alpha = alpha;
		}
		else if (transitions > *max_transitions)
			*max_transitions = transitions;
	}
}

void get_right_trunk_limit (NEURON_LAYER *nl_segment, int initial_x, int initial_y, int *x_limit, int *y_limit)
{
	int xo, yo, i;
	int trunk_value, value;
	
	trunk_value = nl_segment->neuron_vector[initial_y * nl_segment->dimentions.x + initial_x].output.ival;
	
	xo = initial_x;
	yo = initial_y;
	while(1)
	{
		nl_segment->neuron_vector[yo * nl_segment->dimentions.x + xo].output.ival = 0;
		all_outputs_update ();
		if ((xo + 1) < nl_segment->dimentions.x)
		{
			value = nl_segment->neuron_vector[yo * nl_segment->dimentions.x + (xo+1)].output.ival;
			if (value == trunk_value)
			{
				xo++;
				continue;
			}
		}
		if ((xo + 1) < nl_segment->dimentions.x && (yo + 1) < nl_segment->dimentions.y)
		{
			value = nl_segment->neuron_vector[(yo+1) * nl_segment->dimentions.x + (xo+1)].output.ival;
			if (value == trunk_value)
			{
				xo++;
				yo++;
				continue;
			}
		}
		if ((xo + 1) < nl_segment->dimentions.x && (yo - 1) < nl_segment->dimentions.y)
		{
			value = nl_segment->neuron_vector[(yo-1) * nl_segment->dimentions.x + (xo+1)].output.ival;
			if (value == trunk_value)
			{
				xo++;
				yo--;
				continue;
			}
		}
		if ((xo + 1) < nl_segment->dimentions.x)
		{
			for(i = 0; i < nl_segment->dimentions.y; i++)
			{
				value = nl_segment->neuron_vector[i * nl_segment->dimentions.x + (xo+1)].output.ival;
				if (value == trunk_value)
				{
					xo++;
					yo = i;
					break;
				}

			}
			if (i < nl_segment->dimentions.y)
				continue;
		}
		*x_limit = xo;
		*y_limit = yo;
		break;
	}
	
}

void segment_find_right_limit (NEURON_LAYER *nl_segment, NEURON_LAYER *nl_image, int *line_points, int *x_limit, int *y_limit)
{
	segment_set_neural_layer (nl_segment, nl_image, line_points[0], line_points[1]);

	//Procura o final da tora
	double alpha  = get_alpha (line_points);
	double radius = (-nl_segment->dimentions.x / 2.0) / cos (alpha);
//	int transition_area_x = line_points[0] - nl_segment->dimentions.x/2.0;
	int transition_area_y = (int) ((double) radius * sin (alpha) + (double) line_points[1] + 0.5);
	int initial_x = 0;
	int initial_y = nl_segment->dimentions.y/2.0 - line_points[1] + transition_area_y;

	if (initial_x < 0 || initial_x >= nl_segment->dimentions.x ||
	    initial_y < 0 || initial_y >= nl_segment->dimentions.y)
	{
		printf ("Fora dos limites da area segmentada!\n");
		return;
	}

	get_right_trunk_limit (nl_segment, initial_x, initial_y, x_limit, y_limit);

	printf ("alpha = %f\n", alpha);
	printf ("radius = %f\n", radius);
	printf ("transition_area_y = %d line_points[1]= %d\n", transition_area_y, line_points[1]);
	printf ("init_x = %d, init_y = %d\n", initial_x, initial_y);
	printf ("Limite horizontal da tora: (%d, %d)\n", *x_limit, *y_limit);

	fprintf (stderr, "Linha original: (%d, %d), (%d, %d)\n ", line_points[0], line_points[1], line_points[2], line_points[3]);
	fprintf (stderr, "Centro da camada de segmento: (%d, %d)\n", nl_segment->dimentions.x / 2, nl_segment->dimentions.y / 2);
	fprintf (stderr, "Limite horizontal da tora: (%d, %d)\n\n", *x_limit, *y_limit);
}

int segment_find_border (NEURON_LAYER *nl_segment, int y, double alpha, int max_transitions, int step)
{
	int width = nl_segment->dimentions.x;
	int x = 0, transitions = 0;

	for (x = width / 2; 0 <= x && x < width && transitions < max_transitions; x += step)
		transitions = segment_count_transitions (nl_segment, x, y, alpha);
	return x;
}

void find_segmentation_borders (NEURON_LAYER *nl_segment, NEURON_LAYER *nl_image, int x, int y, int bottom, int top)
{
	double alpha = 0.0;
	int min_transitions = 0, max_transitions = 0, mean_transitions = 0;
	int x_center_left = 0, x_center_right = 0;
	int y_center = nl_segment->dimentions.y / 2;

	segment_set_neural_layer (nl_segment, nl_image, x, y);
	segment_find_alpha (nl_segment, &alpha, &min_transitions, &max_transitions);
	
	mean_transitions = (max_transitions + min_transitions) / 2;
	x_center_left  = segment_find_border (nl_segment, y, alpha, mean_transitions, -1);
	x_center_right = segment_find_border (nl_segment, y, alpha, mean_transitions,  1);

	segment_draw_line (nl_segment, x_center_left,  y_center, alpha);
	segment_draw_line (nl_segment, x_center_right, y_center, alpha);	
}
