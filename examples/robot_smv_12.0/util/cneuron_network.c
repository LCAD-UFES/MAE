/*
This file is part of MAE.

History:

[2006.02.13 - Helio Perroni Filho] Created.
*/

#include "cneuron_network.h"

#include "mae.h"

/*
Function Section
*/

void neuron_network_start()
{
	init_pattern_xor_table();
	init_measuraments();
	build_network();
}
