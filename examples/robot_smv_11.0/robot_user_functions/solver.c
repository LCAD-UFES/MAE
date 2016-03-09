#include "solver.h"


int 
init_solver (SOLVER_DESC *solver_desc)
{
	if (solver_desc->number_of_independent_variables <= 0)
		return (SOLVER_FAILURE);
	if (solver_desc->number_of_functions < solver_desc->number_of_independent_variables)
		return (SOLVER_FAILURE);
	
	if (solver_desc->independent_variables_initial_values == NULL)
		return (SOLVER_FAILURE);
	if (solver_desc->void_target_data == NULL)
		return (SOLVER_FAILURE);
	
	if (solver_desc->functions_to_minimize == NULL)
		return (SOLVER_FAILURE);
	if (solver_desc->jacobian_of_functions_to_minimize == NULL)
		return (SOLVER_FAILURE);
	if (solver_desc->functions_to_minimize_and_its_jacobian == NULL)
		return (SOLVER_FAILURE);

	solver_desc->functions_structure.p = solver_desc->number_of_independent_variables;
	solver_desc->functions_structure.n = solver_desc->number_of_functions;
	solver_desc->functions_structure.params = solver_desc->void_target_data;
	solver_desc->functions_structure.f = solver_desc->functions_to_minimize;
	solver_desc->functions_structure.df = solver_desc->jacobian_of_functions_to_minimize;
	solver_desc->functions_structure.fdf = solver_desc->functions_to_minimize_and_its_jacobian;

	if ((solver_desc->solver = gsl_multifit_fdfsolver_alloc (solver_desc->solver_type, solver_desc->number_of_functions, 
								 solver_desc->number_of_independent_variables)) == NULL)
		return (SOLVER_FAILURE);	
	solver_desc->independent_variables = solver_desc->solver->x;

	if (gsl_multifit_fdfsolver_set (solver_desc->solver, &(solver_desc->functions_structure), 
					solver_desc->independent_variables_initial_values) != GSL_SUCCESS)
		return (SOLVER_FAILURE);

	return (SOLVER_SUCCESS);
}



void 
free_solver (SOLVER_DESC *solver_desc)
{
	gsl_multifit_fdfsolver_free (solver_desc->solver);
	(*solver_desc->free_target_data) (solver_desc);
}


int 
iterate_solver (SOLVER_DESC *solver_desc)
{
	int status;
	
	status = gsl_multifit_fdfsolver_iterate (solver_desc->solver);
	if (status != GSL_SUCCESS)
		return (SOLVER_FAILURE);

	status = gsl_multifit_test_delta(solver_desc->solver->dx, solver_desc->solver->x, 
	                                 solver_desc->max_absolute_error, solver_desc->max_relative_error);
	if (status == GSL_SUCCESS)
		return (SOLVER_SUCCESS);
	else if (status == GSL_CONTINUE)
		return (SOLVER_CONTINUE);
	else
		return (SOLVER_FAILURE);
}
