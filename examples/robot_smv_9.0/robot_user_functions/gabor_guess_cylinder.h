#ifndef __GABOR_GUESS_CYLINDER_H
#define __GABOR_GUESS_CYLINDER_H
#include "mae.h"
#include "cylinder.h"

CYLINDER_LIST * guess_cylinder_list (int *working_area, NEURON_LAYER *trunk_segmentation_map, 
		     		     NEURON_LAYER *diparity_map);

#endif
