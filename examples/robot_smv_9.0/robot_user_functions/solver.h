/* Includes da GNU Scientific Library (GSL). */
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_multifit_nlin.h>

/*

We want to fit a model based on a certain number (number_of_independent_variables) of independent 
variables (independent_variables) to experimental target_data (target_data) - this can be done using Nonlinear
Least-Squares Fitting.

To that, we evaluate a number (number_of_functions) of functions (fuctions_to_minimize ()) that compute 
the diference (residuals) between number_of_functions instances of the model and the corresponding 
number_of_functions experimental target_data elements. 

We also need to compute the jacobian (using jacobian_of_fuctions_to_minimize ())
of each of these functions with respect to each independent variable. 

For optimization purposes, the evaluation of the functions and the computation of its residuals can
also be done at once.

All functions must be grouped into a sigle structure (functions_structure).

There are several types of solvers (solver_type) for Least-Squares Fitting in the GNU Scientific 
Library (GSL) that can be used - the complete specification of a solver (solver) can be used as 
a paramenter of one of them.

*/

typedef struct _solver_desc
{
	size_t number_of_independent_variables;
	size_t number_of_functions;
	
	gsl_vector *independent_variables_initial_values;
	gsl_vector *independent_variables;
	void *void_target_data;

	int (*functions_to_minimize) (const gsl_vector *independent_variables, void *target_data, 
			             gsl_vector *residuals);
	int (*jacobian_of_functions_to_minimize) (const gsl_vector *independent_variables, void *target_data, 
			            	         gsl_matrix *jacobian);
	int (*functions_to_minimize_and_its_jacobian) (const gsl_vector *independent_variables, void *target_data, 
					              gsl_vector *residuals, gsl_matrix *jacobian);
	gsl_multifit_function_fdf functions_structure;
	
	const gsl_multifit_fdfsolver_type *solver_type;
	gsl_multifit_fdfsolver *solver;

	double max_absolute_error;
	double max_relative_error;

	void (*free_target_data) (struct _solver_desc *);
		
} SOLVER_DESC;


int  init_solver (SOLVER_DESC *solver_desc);
void free_solver (SOLVER_DESC *solver_desc);
int iterate_solver (SOLVER_DESC *solver_desc);

/* iterate_solver return values */
#define SOLVER_SUCCESS GSL_SUCCESS
#define SOLVER_CONTINUE GSL_CONTINUE
#define SOLVER_FAILURE GSL_FAILURE
