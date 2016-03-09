#include "mae.h"

double
calculate_subpixel_disparity (double sc_l1_45, double sc_r1_0, double sc_q_l1_135,
			      double sc_q_r1_90, double sc_l2_0, double sc_r2_45,
			      double sc_q_l2_90, double sc_q_r2_135,
			      double deltaPhase1, double deltaPhase2, int kernel_size);

void robot_mt_cell_subpixel_disparity (	NEURON_LAYER *subpixel_disparity_map, NEURON_LAYER *disparity_map,
				  	NEURON_LAYER *sc_l1_nv_45P, NEURON_LAYER *sc_r1_nv_0,
				  	NEURON_LAYER *sc_q_l1_nv_135P, NEURON_LAYER *sc_q_r1_nv_90P,
				  	NEURON_LAYER *sc_l2_nv_0, NEURON_LAYER *sc_r2_nv_135N,
				  	NEURON_LAYER *sc_q_l2_nv_90P, NEURON_LAYER *sc_q_r2_nv_45N,
				        double deltaPhase1, double deltaPhase2, int kernel_size);
