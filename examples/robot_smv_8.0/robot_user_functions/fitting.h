
typedef struct _target_data
{
	int arcs;
	int circles;
	int number_of_independent_variables;
	CYLINDER *guessed_cylinder;
	int *is_parameter_to_optimize;
	double *delta_cylinder;
	double *virtual_cylinder_3D;
	double *virtual_cylinder_2D_left;
	double *virtual_cylinder_2D_right;
	double *virtual_cylinder_disp;
	double *virtual_cylinder_disp_plus_delta;
	double *virtual_cylinder_disp_minus_delta;
	double *world_points_disp;
	double *world_points_disp_plus_delta;
	double *world_points_disp_minus_delta;
	NEURON_LAYER *disparity_map;
	int *working_area;
} TARGET_DATA;


double fit_cylinder (CYLINDER *cylinder,
		     CYLINDER *guessed_cylinder,
		     NEURON_LAYER *disparity_map,
		     int *working_area);
