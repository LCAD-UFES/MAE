#include "mae.h"

float
calculate_subpixel_disparity (double sc_l1, double sc_r1, double sc_q_l1, double sc_q_r1, 
			      double sc_l2, double sc_r2, double sc_q_l2, double sc_q_r2, 
			      double phase_left1, double phase_right1, double phase_left2, double phase_right2, 
			      int kernel_size);

void
robot_mt_cell_subpixel_disparity (NEURON_LAYER *subpixel_disparity_map, NEURON_LAYER *disparity_map,
				  NEURON_LAYER *sc_l1_nv, NEURON_LAYER *sc_r1_nv,
				  NEURON_LAYER *sc_q_l1_nv, NEURON_LAYER *sc_q_r1_nv,
				  NEURON_LAYER *sc_l2_nv, NEURON_LAYER *sc_r2_nv,
				  NEURON_LAYER *sc_q_l2_nv, NEURON_LAYER *sc_q_r2_nv,
				  double phase_left1, double phase_right1, double phase_left2, double phase_right2, 
				  int kernel_size);
