/*
This file is part of SMV.

History:

[2006.02.06 - Helio Perroni Filho] Created.
*/

#ifdef __cplusplus
extern "C" {
#endif

#include "cylinder_list.h"

/*
Function Section
*/

void robot_neuron_network_execute();

long robot_neuron_network_merge_offset();

void robot_neuron_network_setup_circle_packing(CYLINDER_LIST* list);

double robot_neuron_network_execute_circle_packing(CYLINDER_LIST* list);

#ifdef __cplusplus
}
#endif
