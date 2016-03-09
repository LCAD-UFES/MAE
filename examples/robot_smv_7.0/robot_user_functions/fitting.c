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
#define ARCS			100
#define CIRCLES			100



void
get_word_projetion_in_left_camera (TARGET_DATA *target_data)
{
	int num_3D_points, i, w, h, wi, hi, yo, xo;
	double xi_r, yi_r, disparity;
	
	num_3D_points = target_data->arcs * target_data->circles;

	// Dimensoes do cortex (disparity map)
	h = target_data->disparity_map->dimentions.y;
	w = target_data->disparity_map->dimentions.x;

	// Dimensoes da imagem de entrada
	hi = image_right.neuron_layer->dimentions.y;
	wi = image_right.neuron_layer->dimentions.x;

	for (i = 0; i < num_3D_points; i++)
	{
		/* (xi, yi) is the projection of the virtual cylinder in the right camera.
		   The virtual cylinder and the world share the same right camera projection. */	
		xi_r = target_data->virtual_cylinder_2D_right[2*i];
		yi_r = target_data->virtual_cylinder_2D_right[2*i+1];

		xo = (xi_r * (double) w) / (double) wi + 0.5;
		yo = (yi_r * (double) h) / (double) hi + 0.5;
		
		disparity = target_data->disparity_map->neuron_vector[w * yo + xo].output.fval;
		if (outside_working_area (target_data->working_area, xi_r, yi_r))
		{
			// @@@@@ O que fazer? Seria bom mudar a função para ela usar o parametro working_area...
//			disparity = infinity_disparity;
		}

		/* (xi + disparity, yi) is the projection of the virtual cylinder in the right camera */
		target_data->world_points_2D_left[2*i] = xi_r + disparity;
		target_data->world_points_2D_left[2*i+1] = yi_r;
	}
}



void
get_corresponding_world_3D_projetion (TARGET_DATA *target_data, double *seleted_world_points_3D)
{
	int num_3D_points;
	
	num_3D_points = target_data->arcs * target_data->circles;
	
	get_word_projetion_in_left_camera (target_data);
	/* There are four instances of world_points_3D.
	   (i) one to compare with instance 0 of the cylinder 
	   (ii) one to compare with instance 0 + delta of the cylinder (used for computing the jacobian)
	   (iii) one to compare with instance 0 - delta of the cylinder (used for computing the jacobian)
	   (iv) one for the world points in left camera coordinates - not used */
	StereoTriangulation (num_3D_points, 
			     seleted_world_points_3D,			/* World points in left camera coordinates */
			     target_data->world_points_3D_not_used, 	/* World points in right camera coordinates - not used */
			     target_data->world_points_2D_left, 	/* Projection of the world in the left camera */
			     target_data->virtual_cylinder_2D_right); 	/* The virtual cylinder and the world share the 
			     						   same right camera projection */
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
functions_to_minimize (const gsl_vector *independent_variables, void *void_target_data, gsl_vector *residuals)
{
	TARGET_DATA *target_data;
	int num_3D_points, i;
	double sigma;
	CYLINDER cylinder;

	sigma = 1.0;
	target_data =  (TARGET_DATA *) void_target_data;
	num_3D_points = target_data->arcs * target_data->circles;

	set_cylinder (&cylinder, target_data->guessed_cylinder, independent_variables, target_data->is_parameter_to_optimize);
	compute_3D_cylinder (&cylinder, target_data->virtual_cylinder_3D, target_data->arcs, target_data->circles);
	CameraProjection (num_3D_points, target_data->virtual_cylinder_2D_right, target_data->virtual_cylinder_3D, RIGHT_CAMERA);
	get_corresponding_world_3D_projetion (target_data, target_data->world_points_3D);

	for (i = 0; i < (num_3D_points*3); i++)
		gsl_vector_set (residuals, i, (target_data->virtual_cylinder_3D[i] - target_data->world_points_3D[i])/sigma);

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
	double sigma, xi, xi_plus_delta, xi_plus_two_delta, delta, derivative;
	CYLINDER cylinder;
	
	sigma = 1.0;
	target_data =  (TARGET_DATA *) void_target_data;
	num_3D_points = target_data->arcs * target_data->circles;

//	set_cylinder (&cylinder, target_data->guessed_cylinder, independent_variables, target_data->is_parameter_to_optimize);
//	compute_3D_cylinder (&cylinder, target_data->virtual_cylinder_3D, target_data->arcs, target_data->circles);
//	CameraProjection (num_3D_points, target_data->virtual_cylinder_2D_right, target_data->virtual_cylinder_3D, RIGHT_CAMERA);
//	get_corresponding_world_3D_projetion (target_data, target_data->world_points_3D);
	for (j = 0; j < target_data->number_of_independent_variables; j++)
	{
		compute_cylinder_plus_delta (&cylinder, target_data->guessed_cylinder, independent_variables, target_data->is_parameter_to_optimize, 
					     j, target_data->delta_cylinder, 1.0);
		compute_3D_cylinder (&cylinder, target_data->virtual_cylinder_3D_plus_delta, target_data->arcs, target_data->circles);
		CameraProjection (num_3D_points, target_data->virtual_cylinder_2D_right, target_data->virtual_cylinder_3D_plus_delta, RIGHT_CAMERA);
		get_corresponding_world_3D_projetion (target_data, target_data->world_points_3D_plus_delta);

		delta = compute_cylinder_plus_delta (&cylinder, target_data->guessed_cylinder, independent_variables, target_data->is_parameter_to_optimize, 
						     j, target_data->delta_cylinder, 2.0);
		compute_3D_cylinder (&cylinder, target_data->virtual_cylinder_3D_plus_two_delta, target_data->arcs, target_data->circles);
		CameraProjection (num_3D_points, target_data->virtual_cylinder_2D_right, target_data->virtual_cylinder_3D_plus_two_delta, RIGHT_CAMERA);
		get_corresponding_world_3D_projetion (target_data, target_data->world_points_3D_plus_two_delta);
		for (i = 0; i < (num_3D_points*3); i++)
		{
			/* This computes the finite difference derivative of the difference between each 
			   cylinder coordinate (x, y, and z) and the corresponding world coordinate 
			   with respect to each cylinder parameter */
			xi = (target_data->virtual_cylinder_3D[i] - target_data->world_points_3D[i])/sigma;
			xi_plus_delta = (target_data->virtual_cylinder_3D_plus_delta[i] - target_data->world_points_3D_plus_delta[i])/sigma;
			xi_plus_two_delta = (target_data->virtual_cylinder_3D_plus_two_delta[i] - target_data->world_points_3D_plus_two_delta[i])/sigma;
			derivative = -(xi_plus_two_delta - 4.0*xi_plus_delta + 3.0*xi) / (2.0 * delta);
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
	delta_cylinder[5] = fabs ((PI / 4.0) * factor); 	/* alpha delta */
	delta_cylinder[6] = fabs ((PI / 4.0) * factor); 	/* betha delta */
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
alloc_and_set_target_data (SOLVER_DESC *solver_desc, CYLINDER *guessed_cylinder, NEURON_LAYER *disparity_map, int *working_area)
{
	TARGET_DATA *target_data;
	int num_3D_points;
	static CYLINDER cylinder = {283.04427212, -137.15358221, 1056.09556013, 365.29919230, 79.94068419, 0.2, 0.0};
									   /* x, y, z, size, diameter, alpha, betha */
	static int is_parameter_to_optimize[NUMBER_OF_CYLINDER_PARAMETERS] = {0, 0, 0, 0,    0,        1,     0};
	
	target_data = (TARGET_DATA *) alloc_mem (sizeof (TARGET_DATA));

	// target_data->guessed_cylinder = guessed_cylinder;
	target_data->guessed_cylinder = &cylinder;
	target_data->is_parameter_to_optimize = is_parameter_to_optimize;

	solver_desc->number_of_independent_variables = compute_number_of_indenpendent_variables (target_data);
	solver_desc->independent_variables_initial_values = compute_independent_variables_initial_values (target_data);

	target_data->arcs = ARCS;
	target_data->circles = CIRCLES;
	num_3D_points = target_data->arcs * target_data->circles;

	target_data->world_points_2D_left = (double *) alloc_mem (num_3D_points * 2 * sizeof (double));
	target_data->virtual_cylinder_2D_right = (double *) alloc_mem (num_3D_points * 2 * sizeof (double));

	/* We need 3 instances of virtual_cylinder_3D for computing the jacobian via finite differences. */
	target_data->virtual_cylinder_3D = (double *) alloc_mem (num_3D_points * 3 * sizeof (double));
	target_data->virtual_cylinder_3D_plus_delta = (double *) alloc_mem (num_3D_points * 3 * sizeof (double));
	target_data->virtual_cylinder_3D_plus_two_delta = (double *) alloc_mem (num_3D_points * 3 * sizeof (double));
	
	/* We need 4 instances of world_3D_points for computing the jacobian via finite differences.
	   One of them is unecessary and is due to fact that StereoTriangulation gerates 3D projections 
	   with references on each camera (two 3D projections). */
	target_data->world_points_3D = (double *) alloc_mem (num_3D_points * 3 * sizeof (double));
	target_data->world_points_3D_plus_delta = (double *) alloc_mem (num_3D_points * 3 * sizeof (double));
	target_data->world_points_3D_plus_two_delta = (double *) alloc_mem (num_3D_points * 3 * sizeof (double));
	target_data->world_points_3D_not_used = (double *) alloc_mem (num_3D_points * 3 * sizeof (double));
	
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
	free (target_data->world_points_3D_not_used);
	free (target_data->world_points_3D_plus_two_delta);
	free (target_data->world_points_3D_plus_delta);
	free (target_data->world_points_3D);
	free (target_data->virtual_cylinder_3D_plus_two_delta);
	free (target_data->virtual_cylinder_3D_plus_delta);
	free (target_data->virtual_cylinder_3D);
	free (target_data->virtual_cylinder_2D_right);
	free (target_data->world_points_2D_left);
	free (target_data);
	gsl_vector_free (solver_desc->independent_variables_initial_values);
}


void 
set_solver (SOLVER_DESC *solver_desc, NEURON_LAYER *disparity_map, CYLINDER *guessed_cylinder, int *working_area)
{
	alloc_and_set_target_data (solver_desc, guessed_cylinder, disparity_map, working_area);
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


int
fit_cylinder (NEURON_LAYER *disparity_map, CYLINDER *guessed_cylinder, int *working_area)
{
	SOLVER_DESC solver_desc;
	int i, status;
	static OUTPUT_DESC *output = NULL;
	TARGET_DATA *target_data;

	set_solver (&solver_desc, disparity_map, guessed_cylinder, working_area);
	if (init_solver (&solver_desc) != SOLVER_SUCCESS)
		return (1);
	
	target_data = (TARGET_DATA *) solver_desc.void_target_data;
	if (output == NULL)
		output = get_output_by_neural_layer (target_data->disparity_map);
	i = 0;
	print_state (i, &solver_desc);
	do
	{
		i++;
		status = iterate_solver (&solver_desc);
		print_state (i, &solver_desc);

		//PESQUISA:Apenas pra visualizar o cilindro
		CameraProjection (target_data->arcs * target_data->circles, target_data->world_points_2D_left, target_data->virtual_cylinder_3D, LEFT_CAMERA);
		project_cylinder_2D_points_into_disparity_map (target_data->arcs * target_data->circles, target_data->world_points_2D_left, target_data->virtual_cylinder_2D_right);
		view_3D_cylinder(target_data->arcs, target_data->circles);
		exchange_to_disparity_map();
		update_output_image (output);
		update_output_image (&out_cylinder_map);
		exchange_to_disparity_map();
		//PESQUISA:FIM

	} while ((status == SOLVER_CONTINUE) && (i < ITER));
	
	check_result (&solver_desc, status);
	
	free_solver (&solver_desc);
	
	return (0);
}
