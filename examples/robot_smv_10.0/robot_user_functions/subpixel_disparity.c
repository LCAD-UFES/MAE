#include "subpixel_disparity.h"
#include <math.h>
#define CUT_POINT 0.0

double
calculate_subpixel_disparity (double sc_l1_45, double sc_r1_0, double sc_q_l1_135, 
			      double sc_q_r1_90, double sc_l2_0, double sc_r2_45, 
			      double sc_q_l2_90, double sc_q_r2_135, 
			      double deltaPhase1, double deltaPhase2, int kernel_size)
{
	double disp;
	double	c1, c2, a, b, d, sin_val, w0;

	w0 = (3.0 * pi) / (double) (4.0 * kernel_size);

	c1 = (sc_l1_45 + sc_r1_0) * (sc_l1_45 + sc_r1_0) + 
	     (sc_q_l1_135 + sc_q_r1_90) * (sc_q_l1_135 + sc_q_r1_90);

	c2 = (sc_l2_0 + sc_r2_45) * (sc_l2_0 + sc_r2_45) + 
	     (sc_q_l2_90 + sc_q_r2_135) * (sc_q_l2_90 + sc_q_r2_135);

	if ((sqrt (c1) < CUT_POINT) && (sqrt (c2) < CUT_POINT))
	{
		return 0.0;
	}	       

	a = c2 * cos (deltaPhase1) - c1 * cos (deltaPhase2);
	b = c2 * sin (deltaPhase1) - c1 * sin (deltaPhase2);

	if ((a == 0.0) && (b == 0.0))
	{
		return 0.0;
	}

	if (b != 0.0)
		d = atan (a / b);
	else
	{
		if (a > 0.0)
			d = pi/2.0;
		else
			d = -pi/2.0;
	}

	sin_val = (c2 - c1) / sqrt (a * a + b * b);

	if (sin_val > 1.0)
		sin_val = 1.0;
	if (sin_val < -1.0)
		sin_val = -1.0;

	disp = (asin (sin_val) - d) / w0;
	return disp;
}

void
robot_mt_cell_subpixel_disparity (NEURON_LAYER *subpixel_disparity_map, NEURON_LAYER *disparity_map,
				  NEURON_LAYER *sc_l1_nv_45P, NEURON_LAYER *sc_r1_nv_0,
				  NEURON_LAYER *sc_q_l1_nv_135P, NEURON_LAYER *sc_q_r1_nv_90P,
				  NEURON_LAYER *sc_l2_nv_0, NEURON_LAYER *sc_r2_nv_135N,
				  NEURON_LAYER *sc_q_l2_nv_90P, NEURON_LAYER *sc_q_r2_nv_45N,
				  double deltaPhase1, double deltaPhase2, int kernel_size)
{
	int 	hi, wi, ho, wo, yo, xo, yi, xir, xil;
	double 	sc_l1_45, sc_r1_0, sc_q_l1_135, sc_q_r1_90, sc_l2_0, sc_r2_45, sc_q_l2_90, 
		sc_q_r2_135;
		
	// Get Dimentions
	hi = sc_l1_nv_45P->dimentions.y;
	wi = sc_l1_nv_45P->dimentions.x;
	ho = subpixel_disparity_map->dimentions.y;
	wo = subpixel_disparity_map->dimentions.x;

	//compute each mt cell
	for (yo = 0; yo < ho; yo++)
	{
		for (xo = 0; xo < wo; xo++)
		{
			
			yi = (int) ((float) (yo * hi) / (float) ho + 0.5);
			xir = (int) ((float) (xo * wi) / (float) wo + 0.5);
			xil = xir + (int)disparity_map->neuron_vector[(yo*wo) + xo].output.fval;

			//Check bounds
			if (xil < 0 || xil >= wi)
			{ 
				subpixel_disparity_map->neuron_vector[(yo*wo) + xo].output.fval = 0.0;
				continue;
			}

			sc_l1_45   = sc_l1_nv_45P->neuron_vector[(yi*wi) + xil].output.fval;
			sc_r1_0    = sc_r1_nv_0->neuron_vector[(yi*wi) + xir].output.fval;
			sc_q_l1_135= sc_q_l1_nv_135P->neuron_vector[(yi*wi) + xil].output.fval;
			sc_q_r1_90 = sc_q_r1_nv_90P->neuron_vector[(yi*wi) + xir].output.fval;

			sc_l2_0    = sc_l2_nv_0->neuron_vector[(yi*wi) + xil].output.fval;
			sc_r2_45   = -sc_r2_nv_135N->neuron_vector[(yi*wi) + xir].output.fval;
			sc_q_l2_90 = sc_q_l2_nv_90P->neuron_vector[(yi*wi) + xil].output.fval;
			sc_q_r2_135= -sc_q_r2_nv_45N->neuron_vector[(yi*wi) + xir].output.fval;

			subpixel_disparity_map->neuron_vector[(yo*wo) + xo].output.fval = 
				(float)calculate_subpixel_disparity (sc_l1_45,  sc_r1_0, sc_q_l1_135, 
			     				     sc_q_r1_90, sc_l2_0, sc_r2_45, 
			     				     sc_q_l2_90, sc_q_r2_135,
							     deltaPhase1, deltaPhase2,
							     kernel_size);
		}
	}

}
