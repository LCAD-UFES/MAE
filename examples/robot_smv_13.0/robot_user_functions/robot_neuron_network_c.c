/*
This file is part of SMV.

History:

[2006.03.26 - Helio Perroni Filho] Created.
*/

#include "robot_neuron_network_c.h"

#include "circle_packing.h"
#include "mae.h"
#include "robot.h"
#include "robot_nl_functions.h"

/*
Function Section
*/

void robot_neuron_network_execute()
{
	time_t start, end;

	rectify_all_images();

	//Update Gabor layers
	start = time(NULL);
	update_simple_mono_cells_filters();
	end = time(NULL);
	printf("Time to update mono cells: %f s\n", difftime(end, start));

	//Generate disparity map
	start = time(NULL);
	generate_disparity_map_plan(&nl_mt_gaussian, &nl_mt_gaussian_map, &nl_disparity_map, "nl_mt");
	end = time(NULL);
	printf("Time to generate disparity map: %f s\n", difftime(end, start));
	
	//Disparity Map ready, then call segmentation filter
	filter_find_update("nl_segment_filter");

	all_outputs_update();
}

long robot_neuron_network_merge_offset()
{
	return compute_horizontal_offset();
}

void robot_neuron_network_setup_circle_packing(CYLINDER_LIST* list)
{
	circle_packing_set_cylinder_list(list);
}

double robot_neuron_network_execute_circle_packing(CYLINDER_LIST* list)
{
	circle_packing_set_cylinder_list(list);
	return circle_packing_execute();
}
