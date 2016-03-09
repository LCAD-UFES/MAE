#ifndef __GABOR_GUESS_CYLINDER_H
#define __GABOR_GUESS_CYLINDER_H
#include "mae.h"
#include "cylinder.h"
#include "cylinder_list.h"

void segment_find_right_limit (NEURON_LAYER *nl_segment, NEURON_LAYER *nl_image, int *line_points, int *x_limit, int *y_limit);

CYLINDER_LIST * guess_cylinder_list (int *working_area, NEURON_LAYER *trunk_segmentation_map, 
		     		     NEURON_LAYER *diparity_map);

CYLINDER_LIST * fit_cylinder_list (int *working_area, NEURON_LAYER *trunk_segmentation_map, 
			       NEURON_LAYER *disparity_map, NEURON_LAYER *simple_mono_right);
#endif
