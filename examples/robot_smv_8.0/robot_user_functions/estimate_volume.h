#ifndef __ESTIMATE_VOLUME_H
#define __ESTIMATE_VOLUME_H
#include "mae.h"

extern double infinity_disparity;

double estimate_volume (int *rectangle, NEURON_LAYER *disparity_map, NEURON_LAYER *right_image, NEURON_LAYER *cylinder_map);

#endif
