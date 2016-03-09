#include "robot_user_functions.h"
#include "viewer.hpp"
#include "cylinder.h"
#include "estimate_volume.h"
#include "fitting.h"
#include "solver.h"

#include <stdlib.h>
#include <stdio.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_multifit_nlin.h>

#define ITER	 		500
#define MAX_ABSOLUTE_ERROR   	1e-5
#define MAX_RELATIVE_ERROR   	1e-5
#define FACTOR   		0.01


double initial_size;
int size_index;


int 
compair_int (const void *pval1, const void *pval2)
{
	int *val1, *val2;
	
	val1 = (int *) pval1;
	val2 = (int *) pval2;
	if (*val1 == *val2)
		return 0;
	else
		return (*val1 > *val2 ? 1 : -1);
}


int
get_num_distinct_points (int *points, int num_points)
{
	int i, num_distinct_points;
	
	qsort (points, num_points, sizeof (int), compair_int);
	num_distinct_points = 0;
	for (i = 1; i < num_points; i++)
	{
		if (points[i-1] != points[i])
			num_distinct_points++;
	}
	return (num_distinct_points);
}


int
get_disparities (TARGET_DATA *target_data, double *cylinder_disp, double *world_disp)
{
	int num_3D_points, i, w, h, wi, hi, yo, xo;
	double xi_l, xi_r, yi_r, disparity;
	int *index_to_points;
	int index;
	int num_distinct_points;
	
	num_3D_points = target_data->arcs * target_data->circles;
	index_to_points = (int *) alloc_mem (num_3D_points * sizeof (int));
	
	// Dimensoes do cortex (disparity map)
	h = target_data->disparity_map->dimentions.y;
	w = target_data->disparity_map->dimentions.x;

	// Dimensoes da imagem de entrada
	hi = image_right.neuron_layer->dimentions.y;
	wi = image_right.neuron_layer->dimentions.x;

	for (i = 0; i < num_3D_points; i++)
	{
		xi_l = target_data->virtual_cylinder_2D_left[2*i];
		xi_r = target_data->virtual_cylinder_2D_right[2*i];
		if (xi_l < 0 || xi_l >= wi ||
		    xi_r < 0 || xi_r >= wi )
		{
			// @@@@@ O que fazer? Seria bom mudar a função para ela usar o parametro working_area...
//			continue;
		}
		    
		cylinder_disp[i] = xi_l - xi_r;
		yi_r = target_data->virtual_cylinder_2D_right[2*i+1];
		if (yi_r < 0 || yi_r >= hi )
		{
			// @@@@@ O que fazer? Seria bom mudar a função para ela usar o parametro working_area...
//			continue;
		}

		xo = (xi_r * (double) w) / (double) wi + 0.5;
		yo = (yi_r * (double) h) / (double) hi + 0.5;
		
		index = w * yo + xo;
		index_to_points[i] = index;
		disparity = target_data->disparity_map->neuron_vector[index].output.fval;
		if (outside_working_area (target_data->working_area, xi_r, yi_r))
		{
			// @@@@@ O que fazer? Seria bom mudar a função para ela usar o parametro working_area...
//			disparity = infinity_disparity;
		}
		world_disp[i] = disparity;
	}
	num_distinct_points = get_num_distinct_points (index_to_points, num_3D_points);
	free (index_to_points);
	return (num_distinct_points);
}



void
set_pointer_to_cylinder_parameters (double *cylinder_parameter[NUMBER_OF_CYLINDER_PARAMETERS], CYLINDER *cylinder)
{
	cylinder_parameter[0] = &(cylinder->x);
	cylinder_parameter[1] = &(cylinder->y);
	cylinder_parameter[2] = &(cylinder->z);
	cylinder_parameter[3] = &(cylinder->size);
	cylinder_parameter[4] = &(cylinder->diameter);
	cylinder_parameter[5] = &(cylinder->alpha);
	cylinder_parameter[6] = &(cylinder->betha);
}



void
set_cylinder (CYLINDER *cylinder, CYLINDER *guessed_cylinder, const gsl_vector *independent_variables, 
	      int *is_parameter_to_optimize)
{
	int i, j;
	double *cylinder_parameter[NUMBER_OF_CYLINDER_PARAMETERS];
	
	*cylinder = *guessed_cylinder;
	set_pointer_to_cylinder_parameters (cylinder_parameter, cylinder);

	j  = 0;
	for (i = 0; i < NUMBER_OF_CYLINDER_PARAMETERS; i++)
	{
		if (is_parameter_to_optimize[i])
		{
			*(cylinder_parameter[i]) = gsl_vector_get (independent_variables, j);
			j++;
		}
	}
}



int
functions_to_minimize (const gsl_vector *independent_variables, void *void_target_data, gsl_vector *fi)
{
	TARGET_DATA *target_data;
	int num_3D_points, i;
	double sigma;
	CYLINDER cylinder;

	target_data =  (TARGET_DATA *) void_target_data;
	num_3D_points = target_data->arcs * target_data->circles;

	if (target_data->optimizing_size)
	{
	       sigma = 2 * gsl_vector_get (independent_variables, size_index) / initial_size;
	}
	else
	{
		sigma = 1;
	}
	set_cylinder (&cylinder, target_data->guessed_cylinder, independent_variables, target_data->is_parameter_to_optimize);

	compute_3D_cylinder (&cylinder, target_data->virtual_cylinder_3D, target_data->arcs, target_data->circles, 180);
	CameraProjectionRight (num_3D_points, target_data->virtual_cylinder_2D_left, target_data->virtual_cylinder_3D, LEFT_CAMERA);
	CameraProjectionRight (num_3D_points, target_data->virtual_cylinder_2D_right, target_data->virtual_cylinder_3D, RIGHT_CAMERA);
	get_disparities (target_data, target_data->virtual_cylinder_disp, target_data->world_points_disp);

	for (i = 0; i < num_3D_points; i++)
	{
		gsl_vector_set (fi, i, (target_data->virtual_cylinder_disp[i] - target_data->world_points_disp[i])/sigma);
	}

  	return (GSL_SUCCESS);
}



double
compute_cylinder_plus_delta (CYLINDER *cylinder, CYLINDER *guessed_cylinder, const gsl_vector *independent_variables, 
			     int *is_parameter_to_optimize, int independent_variable, double *delta_cylinder, double delta)
{
	int i, j;
	double *cylinder_parameter[NUMBER_OF_CYLINDER_PARAMETERS];
	double independent_variable_delta;
	
	*cylinder = *guessed_cylinder;
	set_pointer_to_cylinder_parameters (cylinder_parameter, cylinder);

	j  = 0;
	for (i = 0; i < NUMBER_OF_CYLINDER_PARAMETERS; i++)
	{
		if (is_parameter_to_optimize[i])
		{
			*(cylinder_parameter[i]) = gsl_vector_get (independent_variables, j);
			if (j == independent_variable)
			{
				*(cylinder_parameter[i]) += delta * delta_cylinder[i];
				independent_variable_delta = delta_cylinder[i];
			}
			j++;
		}
	}
	return (independent_variable_delta);
}



int
jacobian_of_functions_to_minimize (const gsl_vector *independent_variables, void *void_target_data, gsl_matrix *jacobian)
{
	TARGET_DATA *target_data;
	int num_3D_points;
	int i, j;
	double sigma, fi_plus_delta, fi_minus_delta, delta, derivative;
	CYLINDER cylinder;
	
	target_data =  (TARGET_DATA *) void_target_data;
	num_3D_points = target_data->arcs * target_data->circles;

	if (target_data->optimizing_size)
	{
		sigma = 2.0 * gsl_vector_get (independent_variables, size_index) / initial_size;
	}
	else
	{
		sigma = 1;
	}
	for (j = 0; j < target_data->number_of_independent_variables; j++)
	{
		compute_cylinder_plus_delta (&cylinder, target_data->guessed_cylinder, independent_variables, target_data->is_parameter_to_optimize, 
					     j, target_data->delta_cylinder, 1.0);

		compute_3D_cylinder (&cylinder, target_data->virtual_cylinder_3D, target_data->arcs, target_data->circles, 180);
		CameraProjectionRight (num_3D_points, target_data->virtual_cylinder_2D_left, target_data->virtual_cylinder_3D, LEFT_CAMERA);
		CameraProjectionRight (num_3D_points, target_data->virtual_cylinder_2D_right, target_data->virtual_cylinder_3D, RIGHT_CAMERA);
		get_disparities (target_data, target_data->virtual_cylinder_disp_plus_delta, target_data->world_points_disp_plus_delta);

		delta = compute_cylinder_plus_delta (&cylinder, target_data->guessed_cylinder, independent_variables, target_data->is_parameter_to_optimize, 
						     j, target_data->delta_cylinder, -1.0);
		compute_3D_cylinder (&cylinder, target_data->virtual_cylinder_3D, target_data->arcs, target_data->circles, 180);
		CameraProjectionRight (num_3D_points, target_data->virtual_cylinder_2D_left, target_data->virtual_cylinder_3D, LEFT_CAMERA);
		CameraProjectionRight (num_3D_points, target_data->virtual_cylinder_2D_right, target_data->virtual_cylinder_3D, RIGHT_CAMERA);
		get_disparities (target_data, target_data->virtual_cylinder_disp_minus_delta, target_data->world_points_disp_minus_delta);
		for (i = 0; i < num_3D_points; i++)
		{

			/* This computes the finite difference derivative of the difference between each 
			   cylinder coordinate (x, y, and z) and the corresponding world coordinate 
			   with respect to each cylinder parameter */
			fi_plus_delta = (target_data->virtual_cylinder_disp_plus_delta[i] - target_data->world_points_disp_plus_delta[i])/sigma;
			fi_minus_delta = (target_data->virtual_cylinder_disp_minus_delta[i] - target_data->world_points_disp_minus_delta[i])/sigma;
			derivative = (fi_plus_delta - fi_minus_delta) / (2.0 * delta);
			gsl_matrix_set (jacobian, i, j, derivative);
		}
	}
  	return (GSL_SUCCESS);
}


int
functions_to_minimize_and_its_jacobian (const gsl_vector *independent_variables, void *void_target_data, gsl_vector *residuals, gsl_matrix *jacobian)
{
	functions_to_minimize (independent_variables, void_target_data, residuals);
	jacobian_of_functions_to_minimize (independent_variables, void_target_data, jacobian);

	return (GSL_SUCCESS);
}



void
set_delta_cylinder (double *delta_cylinder, CYLINDER *guessed_cylinder, double factor)
{
	double PI;

	PI = acos(-1.0);

	delta_cylinder[0] = fabs (guessed_cylinder->z * factor); /* x receives the same delta of z */
	delta_cylinder[1] = fabs (guessed_cylinder->z * factor); /* y receives the same delta of z */
	delta_cylinder[2] = fabs (guessed_cylinder->z * factor);
	delta_cylinder[3] = fabs (guessed_cylinder->size * factor);
	delta_cylinder[4] = fabs (guessed_cylinder->diameter * factor);
	delta_cylinder[5] = fabs ((PI / 4.0) * factor); /* alpha delta */
	delta_cylinder[6] = fabs ((PI / 4.0) * factor); /* betha delta */
}


int
compute_number_of_indenpendent_variables (TARGET_DATA *target_data)
{
	int number_of_independent_variables, i;

	number_of_independent_variables = 0;
	for (i = 0; i < NUMBER_OF_CYLINDER_PARAMETERS; i++)
		if (target_data->is_parameter_to_optimize[i])
			number_of_independent_variables++;	

	target_data->number_of_independent_variables = number_of_independent_variables;
	return (number_of_independent_variables);
}



gsl_vector *
compute_independent_variables_initial_values (TARGET_DATA *target_data)
{
	gsl_vector *independent_variables_initial_values;
	int i, j;
	double *cylinder_parameter[NUMBER_OF_CYLINDER_PARAMETERS];

	independent_variables_initial_values = gsl_vector_alloc (target_data->number_of_independent_variables);
	set_pointer_to_cylinder_parameters (cylinder_parameter, target_data->guessed_cylinder);
	for (i = 0, j = 0; i < NUMBER_OF_CYLINDER_PARAMETERS; i++)
	{
		if (target_data->is_parameter_to_optimize[i])
		{
			gsl_vector_set (independent_variables_initial_values, j, *(cylinder_parameter[i]));
			j++;
		}
	}
	return (independent_variables_initial_values); 
}



void
alloc_and_set_target_data (SOLVER_DESC *solver_desc, CYLINDER *guessed_cylinder, 
			   NEURON_LAYER *disparity_map, int *working_area, 
			   int *is_parameter_to_optimize)
{
	TARGET_DATA *target_data;
	int num_3D_points, i;
//	static CYLINDER cylinder = {283.04427212, -137.15358221, 1056.09556013, 365.29919230-55.0, 79.94068419, -0.1, 0.1};
									   /* x, y, z, size, diameter, alpha, betha */
//	static int parameter_to_optimize[NUMBER_OF_CYLINDER_PARAMETERS] = {1, 1, 1, 1,    1,        1,     1};
	
	initial_size = guessed_cylinder->size;
	
	target_data = (TARGET_DATA *) alloc_mem (sizeof (TARGET_DATA));

	//Se o size estiver sendo otmizado
	if (is_parameter_to_optimize[3])	
		target_data->optimizing_size = 1;
	else
		target_data->optimizing_size = 0;	
		
	//Calcula o indice do parametro size dentro do gsl_vector 	
	size_index = -1;
	for(i = 0; i < 4; i++)
	{
		if (is_parameter_to_optimize[i])
			size_index++;
	}

	target_data->guessed_cylinder = guessed_cylinder;
	//target_data->guessed_cylinder = &cylinder;
//	target_data->is_parameter_to_optimize = parameter_to_optimize;
	target_data->is_parameter_to_optimize = is_parameter_to_optimize;

	solver_desc->number_of_independent_variables = compute_number_of_indenpendent_variables (target_data);
	solver_desc->independent_variables_initial_values = compute_independent_variables_initial_values (target_data);

	target_data->arcs = ARCS;
	target_data->circles = CIRCLES;
	num_3D_points = target_data->arcs * target_data->circles;

	target_data->virtual_cylinder_3D = (double *) alloc_mem (num_3D_points * 3 * sizeof (double));
	target_data->virtual_cylinder_2D_left = (double *) alloc_mem (num_3D_points * 2 * sizeof (double));
	target_data->virtual_cylinder_2D_right = (double *) alloc_mem (num_3D_points * 2 * sizeof (double));

	/* We need 3 instances of virtual_cylinder_disp for computing the jacobian via finite differences. */
	target_data->virtual_cylinder_disp = (double *) alloc_mem (num_3D_points * sizeof (double));
	target_data->virtual_cylinder_disp_plus_delta = (double *) alloc_mem (num_3D_points * sizeof (double));
	target_data->virtual_cylinder_disp_minus_delta = (double *) alloc_mem (num_3D_points * sizeof (double));
	
	/* We need 3 instances of world_disp for computing the jacobian via finite differences. */
	target_data->world_points_disp = (double *) alloc_mem (num_3D_points * sizeof (double));
	target_data->world_points_disp_plus_delta = (double *) alloc_mem (num_3D_points * sizeof (double));
	target_data->world_points_disp_minus_delta = (double *) alloc_mem (num_3D_points * sizeof (double));
	
	target_data->disparity_map = disparity_map;
	target_data->working_area = working_area;
	
	/* Used for computing the jacobian via finite differences. */
	target_data->delta_cylinder = (double *) alloc_mem (NUMBER_OF_CYLINDER_PARAMETERS * sizeof (double));
	set_delta_cylinder (target_data->delta_cylinder, guessed_cylinder, FACTOR);

	solver_desc->number_of_functions = 3 * target_data->arcs * target_data->circles;
	solver_desc->void_target_data = (void *) target_data;
}


void
free_target_data (SOLVER_DESC *solver_desc)
{
	TARGET_DATA *target_data;
	
	target_data = (TARGET_DATA *) solver_desc->void_target_data;
	free (target_data->delta_cylinder);
	free (target_data->world_points_disp_minus_delta);
	free (target_data->world_points_disp_plus_delta);
	free (target_data->world_points_disp);
	free (target_data->virtual_cylinder_disp_minus_delta);
	free (target_data->virtual_cylinder_disp_plus_delta);
	free (target_data->virtual_cylinder_disp);
	free (target_data->virtual_cylinder_2D_right);
	free (target_data->virtual_cylinder_2D_left);
	free (target_data->virtual_cylinder_3D);
	free (target_data);
	gsl_vector_free (solver_desc->independent_variables_initial_values);
}


void 
set_solver (SOLVER_DESC *solver_desc, NEURON_LAYER *disparity_map, CYLINDER *guessed_cylinder, 
	    int *working_area, int *is_parameter_to_optimize)
{
	alloc_and_set_target_data (solver_desc, guessed_cylinder, disparity_map, working_area, is_parameter_to_optimize);
	solver_desc->free_target_data = &free_target_data;
	
	solver_desc->functions_to_minimize = &functions_to_minimize;
	solver_desc->jacobian_of_functions_to_minimize = &jacobian_of_functions_to_minimize;
	solver_desc->functions_to_minimize_and_its_jacobian = &functions_to_minimize_and_its_jacobian;

	solver_desc->solver_type = gsl_multifit_fdfsolver_lmsder; /* Levenberg-Marquardt algorithm */

	solver_desc->max_absolute_error = MAX_ABSOLUTE_ERROR;
	solver_desc->max_relative_error = MAX_RELATIVE_ERROR;
}


void
check_result (SOLVER_DESC *solver_desc, int status)
{
	int p, n;
	double chi;
	double dof;
	char mess[3][100] = {"success", "continue", "failure"};
	int i;
	CYLINDER cylinder;
	TARGET_DATA *target_data;
	
	target_data = (TARGET_DATA *) solver_desc->void_target_data;
	p = solver_desc->number_of_independent_variables;
	n = solver_desc->number_of_functions;
	chi = gsl_blas_dnrm2 (solver_desc->solver->f);
	dof = n - p; 

	printf("chisq/dof = %g\n",  pow (chi, 2.0) / dof);

	set_cylinder (&cylinder, target_data->guessed_cylinder, solver_desc->independent_variables, target_data->is_parameter_to_optimize);
	printf ("x        = %.5f\n", cylinder.x);
	printf ("y        = %.5f\n", cylinder.y);
	printf ("z        = %.5f\n", cylinder.z);
	printf ("size     = %.5f\n", cylinder.size);
	printf ("diameter = %.5f\n", cylinder.diameter);
	printf ("alpha    = %.5f\n", cylinder.alpha);
	printf ("betha    = %.5f\n", cylinder.betha);

	if (status == GSL_SUCCESS)
		i = 0;
	else if (status == GSL_CONTINUE)
		i = 1;
	else
		i = 2;

	printf ("status = %s\n", mess[i]);
}


void 
print_state (size_t iter, SOLVER_DESC *solver_desc)
{
	gsl_vector *independent_variables = solver_desc->independent_variables;
	int i = 0, n;
	
	n = independent_variables->size;

	printf ("iter: %3u independent variables = ", iter);
	for (i = 0; i < n; i ++)
		printf ("% 15.8f ", gsl_vector_get (independent_variables, i));
	printf ("|f(x)| = %g\n", gsl_blas_dnrm2 (solver_desc->solver->f));		
}


double
optimize_cylinder (CYLINDER *cylinder, CYLINDER *guessed_cylinder, SOLVER_DESC *solver_desc)
{
	int i, status;
	static OUTPUT_DESC *output = NULL;
	TARGET_DATA *target_data;
	double norm;

	target_data = (TARGET_DATA *) solver_desc->void_target_data;
	if (output == NULL)
		output = get_output_by_neural_layer (target_data->disparity_map);
	i = 0;
	print_state (i, solver_desc);
	do
	{
		i++;
		status = iterate_solver (solver_desc);
		print_state (i, solver_desc);

		//PESQUISA:Apenas pra visualizar o cilindro
		project_cylinder_2D_points_into_disparity_map (target_data->arcs * target_data->circles, target_data->virtual_cylinder_2D_left, target_data->virtual_cylinder_2D_right);
		exchange_to_disparity_map ();
		all_outputs_update ();
		exchange_to_disparity_map ();
		//PESQUISA:FIM

	} while ((status == SOLVER_CONTINUE) && (i < ITER));
	
	check_result (solver_desc, status);
	
	set_cylinder (cylinder, guessed_cylinder, solver_desc->independent_variables, target_data->is_parameter_to_optimize);
	norm = gsl_blas_dnrm2 (solver_desc->solver->f);
	
	free_solver (solver_desc);
	return (norm);
}


double
fit_cylinder (CYLINDER *cylinder, CYLINDER *guessed_cylinder, NEURON_LAYER *disparity_map, 
	      int *working_area, int *is_parameter_to_optimize)
{
	SOLVER_DESC solver_desc;
	double norm;
	
	set_solver (&solver_desc, disparity_map, guessed_cylinder, working_area, is_parameter_to_optimize);
	if (init_solver (&solver_desc) != SOLVER_SUCCESS)
		return (-1.0);

	initial_size = guessed_cylinder->size;
	norm = optimize_cylinder (cylinder, guessed_cylinder, &solver_desc);
	
	return (norm);
}
