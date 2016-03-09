
typedef struct _target_data
{
	int arcs;
	int circles;
	int number_of_independent_variables;
	CYLINDER *guessed_cylinder;
	int *is_parameter_to_optimize;
	double *delta_cylinder;
	double *world_points_2D_left;
	double *virtual_cylinder_2D_right;
	double *virtual_cylinder_3D;
	double *virtual_cylinder_3D_plus_delta;
	double *virtual_cylinder_3D_plus_two_delta;
	double *world_points_3D;
	double *world_points_3D_plus_delta;
	double *world_points_3D_plus_two_delta;
	double *world_points_3D_not_used; /* Necessary due to StereoTriangulation two camare views output */
	NEURON_LAYER *disparity_map;
	int *working_area;
} TARGET_DATA;


int fit_cylinder (NEURON_LAYER *disparity_map, CYLINDER *guessed_cylinder, int *working_area);
